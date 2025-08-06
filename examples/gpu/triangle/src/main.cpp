// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.core;
import stormkit.log;
import stormkit.wsi;
import stormkit.gpu;

#include <stormkit/log/log_macro.hpp>
#include <stormkit/main/main_macro.hpp>

LOGGER("stormkit.examples.gpu.triangle");

#ifndef SHADER_DIR
    #define SHADER_DIR "../share/shaders"
#endif

namespace stdr = std::ranges;

using namespace stormkit;

struct SubmissionResource {
    gpu::Fence         in_flight;
    gpu::Semaphore     image_available;
    gpu::CommandBuffer render_cmb;
};

struct SwapchainImageResource {
    Ref<const gpu::Image> image;
    gpu::ImageView        view;
    gpu::FrameBuffer      framebuffer;
    gpu::Semaphore        render_finished;
};

static constexpr auto BUFFERING_COUNT = 2;

auto main(std::span<const std::string_view> args) -> int {
    using namespace std::literals;

    wsi::parse_args(args);

    // initialize logger
    auto logger_singleton = log::Logger::create_logger_instance<log::ConsoleLogger>();

    const auto monitors = wsi::Window::get_monitor_settings();
    ilog("--- Monitors ---");
    ilog("{}", monitors);

    // initialize WSI
    auto window = wsi::Window {
        "Stormkit GPU Triangle example",
        { 800u, 600u },
        wsi::WindowFlag::CLOSE | wsi::WindowFlag::EXTERNAL_CONTEXT
    };

    auto event_handler = wsi::EventHandler {};

    // initialize gpu backend (vulkan or webgpu depending the platform)
    gpu::initialize_backend();

    // create gpu instance and attach surface to window
    const auto instance = gpu::Instance::create("Triangle")
                            .transform_error(monadic::assert("Failed to initialize gpu instance"))
                            .value();
    const auto surface = gpu::Surface::create_from_window(instance, window)
                           .transform_error(monadic::
                                              assert("Failed to initialize window gpu surface"))
                           .value();

    // pick the best physical device
    const auto& physical_devices = instance.physical_devices();
    if (stdr::empty(physical_devices)) {
        elog("No render physical device found!");
        return 0;
    }
    ilog("Physical devices: {}", physical_devices);

    auto physical_device = as_ref(physical_devices.front());
    auto score           = gpu::score_physical_device(physical_device);
    for (auto i = 1u; i < stdr::size(physical_devices); ++i) {
        const auto& d       = physical_devices[i];
        const auto  d_score = gpu::score_physical_device(d);
        if (d_score > score) {
            physical_device = as_ref(d);
            score           = d_score;
        }
    }

    ilog("Picked gpu: {}", *physical_device);

    // create gpu device
    const auto device = gpu::Device::create(*physical_device, instance)
                          .transform_error(monadic::assert("Failed to initialize gpu device"))
                          .value();

    const auto raster_queue = gpu::Queue::create(device, device.raster_queue_entry());

    // transition swapchain image to present image
    const auto command_pool
      = gpu::CommandPool::create(device)
          .transform_error(monadic::assert("Failed to create raster queue command pool"))
          .value();

    // load shaders
    const auto vertex_shader = gpu::Shader::load_from_file(device,
                                                           SHADER_DIR "/triangle.spv",
                                                           gpu::ShaderStageFlag::VERTEX)
                                 .transform_error(monadic::assert("Failed to load vertex shader"))
                                 .value();

    const auto fragment_shader = gpu::Shader::load_from_file(device,
                                                             SHADER_DIR "/triangle.spv",
                                                             gpu::ShaderStageFlag::FRAGMENT)
                                   .transform_error(monadic::
                                                      assert("Failed to load fragment shader"))
                                   .value();

    const auto pipeline_layout = gpu::PipelineLayout::create(device, {})
                                   .transform_error(monadic::
                                                      assert("Failed to create pipeline layout"))
                                   .value();

    const auto window_extent = window.extent();
    const auto swapchain     = gpu::SwapChain::create(device, surface, window_extent.to<3>())
                             .transform_error(monadic::assert("Failed to create swapchain"))
                             .value();

    // initialize render pass
    const auto render_pass
      = gpu::RenderPass::
          create(device,
                 { .attachments = { { .format = swapchain.pixel_format() } },
                   .subpasses   = { { .bind_point            = gpu::PipelineBindPoint::GRAPHICS,
                                      .color_attachment_refs = { { .attachment_id = 0u } } } } })
            .transform_error(monadic::assert("Failed to create render pass"))
            .value();

    // initialize render pipeline
    const auto window_viewport = gpu::Viewport {
        .position = { 0.f, 0.f },
        .extent   = window_extent.to<f32>(),
        .depth    = { 0.f, 1.f },
    };
    const auto scissor = gpu::Scissor {
        .offset = { 0, 0 },
        .extent = window_extent,
    };

    const auto state = gpu::RasterPipelineState {
        .input_assembly_state = { .topology = gpu::PrimitiveTopology::TRIANGLE_LIST, },
        .viewport_state       = { .viewports = { window_viewport },
                                 .scissors  = { scissor }, },
        .color_blend_state
        = { .attachments = { { .blend_enable           = true,
                               .src_color_blend_factor = gpu::BlendFactor::SRC_ALPHA,
                               .dst_color_blend_factor = gpu::BlendFactor::ONE_MINUS_SRC_ALPHA,
                               .src_alpha_blend_factor = gpu::BlendFactor::SRC_ALPHA,
                               .dst_alpha_blend_factor = gpu::BlendFactor::ONE_MINUS_SRC_ALPHA,
                               .alpha_blend_operation  = gpu::BlendOperation::ADD, }, }, },
        .shader_state  = to_refs(vertex_shader, fragment_shader),
    };

    const auto pipeline = gpu::Pipeline::create(device, state, pipeline_layout, render_pass)
                            .transform_error(monadic::assert("Failed to create raster pipeline"))
                            .value();

    // create present engine resources
    auto submission_resources = std::vector<SubmissionResource> {};
    submission_resources.reserve(BUFFERING_COUNT);

    for (auto _ : range(BUFFERING_COUNT)) {
        submission_resources.push_back({
          .in_flight = gpu::Fence::create_signaled(device)
                         .transform_error(core::monadic::assert("Failed to create swapchain image "
                                                                "in flight fence"))
                         .value(),
          .image_available = gpu::Semaphore::create(device)
                               .transform_error(core::monadic::
                                                  assert("Failed to create present wait semaphore"))
                               .value(),
          .render_cmb = command_pool.create_command_buffer()
                          .transform_error(monadic::
                                             assert("Failed to create transition command buffers"))
                          .value(),
        });
    }

    const auto& images = swapchain.images();

    const auto image_count = stdr::size(images);
    auto       transition_cmbs
      = command_pool.create_command_buffers(image_count)
          .transform_error(monadic::assert("Failed to create transition command buffers"))
          .value();

    auto swapchain_image_resources = std::vector<SwapchainImageResource> {};
    swapchain_image_resources.reserve(stdr::size(images));

    auto image_index = 0u;
    for (const auto& image : images) {
        auto view = gpu::ImageView::create(device, image)
                      .transform_error(core::monadic::
                                         assert("Failed to create swapchain image view"))
                      .value();
        auto framebuffer = render_pass.create_frame_buffer(device, window_extent, to_refs(view))
                             .transform_error(core::monadic::assert(
                               std::format("Failed to create framebuffer for image {}",
                                           image_index)))
                             .value();

        swapchain_image_resources.push_back({
          .image           = as_ref(image),
          .view            = std::move(view),
          .framebuffer     = std::move(framebuffer),
          .render_finished = gpu::Semaphore::create(device)
                               .transform_error(core::monadic::assert("Failed to create render "
                                                                      "signal semaphore"))
                               .value(),
        });

        auto& transition_cmb = transition_cmbs[image_index];
        dlog("{}", image_index);
        transition_cmb.begin(true);

        transition_cmb.begin_debug_region(std::format("transition image {}", image_index))
          .transition_image_layout(image,
                                   gpu::ImageLayout::UNDEFINED,
                                   gpu::ImageLayout::PRESENT_SRC)
          .end_debug_region();

        transition_cmb.end();

        ++image_index;
    }

    const auto fence = gpu::Fence::create(device)
                         .transform_error(monadic::assert("Failed to create transition fence"))
                         .value();

    const auto cmbs = to_refs(transition_cmbs);
    raster_queue.submit({ .command_buffers = cmbs }, as_ref(fence));

    event_handler.set_callbacks({
      { wsi::EventType::CLOSED,      [&window](const wsi::Event&) noexcept { window.close(); } },
      { wsi::EventType::KEY_PRESSED,
       [&window](const wsi::Event& event) noexcept {
            const auto key_event = as<wsi::KeyPressedEventData>(event.data);
            if (key_event.key == wsi::Key::ESCAPE) window.close();
        }                                                                                      },
    });

    auto current_frame = 0uz;

    // wait for transition to be done
    fence.wait().transform_error(monadic::assert());

    while (window.is_open()) {
        LOG_MODULE.flush();

        event_handler.update(window);

        // get next swapchain image
        auto& submission_resource = submission_resources[current_frame];

        const auto& wait      = submission_resource.image_available;
        auto&       in_flight = submission_resource.in_flight;

        const auto acquire_next_image = bind_front(&gpu::SwapChain::acquire_next_image,
                                                   &swapchain,
                                                   100ms,
                                                   std::cref(wait));
        const auto extract_index      = [](auto&& _result) static noexcept {
            auto&& [result, _image_index] = _result;
            return _image_index;
        };

        image_index = in_flight.wait()
                        .transform([&in_flight](auto&&) noexcept { in_flight.reset(); })
                        .and_then(acquire_next_image)
                        .transform(extract_index)
                        .transform_error(monadic::assert("Failed to acquire next swapchain image"))
                        .value();

        const auto& swapchain_image_resource = swapchain_image_resources[image_index];
        const auto& framebuffer              = swapchain_image_resource.framebuffer;
        const auto& signal                   = swapchain_image_resource.render_finished;

        // render in it
        auto& render_cmb = submission_resource.render_cmb;
        render_cmb.reset();
        render_cmb.begin();

        render_cmb.begin_debug_region("Render triangle")
          .begin_render_pass(render_pass, framebuffer)
          .bind_pipeline(pipeline)
          .draw(3)
          .end_render_pass()
          .end_debug_region();

        render_cmb.end();
        render_cmb.submit(raster_queue,
                          as_refs(wait),
                          { gpu::PipelineStageFlag::COLOR_ATTACHMENT_OUTPUT },
                          as_refs(signal),
                          as_ref(in_flight));

        // present it
        auto update_current_frame = [&current_frame](auto&&) mutable noexcept {
            if (++current_frame >= BUFFERING_COUNT) current_frame = 0;
        };

        raster_queue.present(as_refs(swapchain), as_refs(signal), as_view(image_index))
          .transform(update_current_frame)
          .transform_error(monadic::assert("Failed to present swapchain image"));
    }

    raster_queue.wait_idle();
    device.wait_idle();

    return 0;
}
