// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#include <cstddef> // offsetof

import std;

import stormkit.core;
import stormkit.log;
import stormkit.wsi;
import stormkit.gpu;
import stormkit.image;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/log/log_macro.hpp>
#include <stormkit/main/main_macro.hpp>

LOGGER("stormkit.examples.gpu.textured_cube");

#ifndef SHADER_DIR
    #define SHADER_DIR "../share/shaders"
#endif

#ifndef TEXTURE_DIR
    #define TEXTURE_DIR "../share/textures"
#endif

namespace stdc = std::chrono;
namespace stdr = std::ranges;

using namespace stormkit;

struct SubmissionResource {
    gpu::Fence         in_flight;
    gpu::Semaphore     image_available;
    gpu::CommandBuffer render_cmb;
    gpu::Buffer        viewer_buffer;
    gpu::DescriptorSet descriptor_set;
};

struct SwapchainImageResource {
    Ref<const gpu::Image> image;
    gpu::ImageView        view;
    gpu::Image            depth_image;
    gpu::ImageView        depth_view;
    gpu::FrameBuffer      framebuffer;
    gpu::Semaphore        render_finished;
};

struct Vertex {
    math::vec3f position;
    math::vec2f uv;

    static constexpr auto attribute_descriptions() noexcept
      -> std::array<gpu::VertexInputAttributeDescription, 2> {
        return {
            gpu::VertexInputAttributeDescription {
                                                  0, 0,
                                                  gpu::PixelFormat::RGB32F,
                                                  offsetof(Vertex, position) },
            gpu::VertexInputAttributeDescription {
                                                  1, 0,
                                                  gpu::PixelFormat::RG32F,
                                                  offsetof(Vertex, uv)       },
        };
    }

    static constexpr auto binding_description() noexcept -> gpu::VertexBindingDescription {
        return { 0, sizeof(Vertex), gpu::VertexInputRate::VERTEX };
    }
};

static constexpr auto VERTICES = std::array<Vertex, 36> {
    Vertex { { -1.f, -1.f, -1.f }, { 2.f / 3.f, 3.f / 4.f } }, // -X side
    { { -1.f, -1.f, 1.f },  { 1.f / 3.f, 3.f / 4.f } },
    { { -1.f, 1.f, 1.f },   { 1.f / 3.f, 1.f }       },
    { { -1.f, 1.f, 1.f },   { 1.f / 3.f, 1.f }       },
    { { -1.f, 1.f, -1.f },  { 2.f / 3.f, 1.f }       },
    { { -1.f, -1.f, -1.f }, { 2.f / 3.f, 3.f / 4.f } },

    { { -1.f, -1.f, -1.f }, { 1.f / 3.f, 2.f / 4.f } }, // -Z side
    { { 1.f, 1.f, -1.f },   { 0.f, 1.f / 4.f }       },
    { { 1.f, -1.f, -1.f },  { 0.f, 2.f / 4.f }       },
    { { -1.f, -1.f, -1.f }, { 1.f / 3.f, 2.f / 4.f } },
    { { -1.f, 1.f, -1.f },  { 1.f / 3.f, 1.f / 4.f } },
    { { 1.f, 1.f, -1.f },   { 0.f, 1.f / 4.f }       },

    { { -1.f, -1.f, -1.f }, { 2.f / 3.f, 2.f / 4.f } }, // -Y side
    { { 1.f, -1.f, -1.f },  { 2.f / 3.f, 3.f / 4.f } },
    { { 1.f, -1.f, 1.f },   { 1.f / 3.f, 3.f / 4.f } },
    { { -1.f, -1.f, -1.f }, { 2.f / 3.f, 2.f / 4.f } },
    { { 1.f, -1.f, 1.f },   { 1.f / 3.f, 3.f / 4.f } },
    { { -1.f, -1.f, 1.f },  { 1.f / 3.f, 2.f / 4.f } },

    { { -1.f, 1.f, -1.f },  { 2.f / 3.f, 0.f }       }, // +Y side
    { { -1.f, 1.f, 1.f },   { 1.f / 3.f, 0.f }       },
    { { 1.f, 1.f, 1.f },    { 1.f / 3.f, 1.f / 4.f } },
    { { -1.f, 1.f, -1.f },  { 2.f / 3.f, 0.f }       },
    { { 1.f, 1.f, 1.f },    { 1.f / 3.f, 1.f / 4.f } },
    { { 1.f, 1.f, -1.f },   { 2.f / 3.f, 1.f / 4.f } },

    { { 1.f, 1.f, -1.f },   { 2.f / 3.f, 1.f / 4.f } }, // +X side
    { { 1.f, 1.f, 1.f },    { 1.f / 3.f, 1.f / 4.f } },
    { { 1.f, -1.f, 1.f },   { 1.f / 3.f, 2.f / 4.f } },
    { { 1.f, -1.f, 1.f },   { 1.f / 3.f, 2.f / 4.f } },
    { { 1.f, -1.f, -1.f },  { 2.f / 3.f, 2.f / 4.f } },
    { { 1.f, 1.f, -1.f },   { 2.f / 3.f, 1.f / 4.f } },

    { { -1.f, 1.f, 1.f },   { 2.f / 3.f, 1.f / 4.f } }, // +Z side
    { { -1.f, -1.f, 1.f },  { 2.f / 3.f, 2.f / 4.f } },
    { { 1.f, 1.f, 1.f },    { 1.f, 1.f / 4.f }       },
    { { -1.f, -1.f, 1.f },  { 2.f / 3.f, 2.f / 4.f } },
    { { 1.f, -1.f, 1.f },   { 1.f, 2.f / 4.f }       },
    { { 1.f, 1.f, 1.f },    { 1.f, 1.f / 4.f }       },
};

struct ViewerData {
    math::mat4f proj;
    math::mat4f view;
    math::mat4f model;

    static constexpr auto layout_binding() -> gpu::DescriptorSetLayoutBinding {
        return { 0, gpu::DescriptorType::UNIFORM_BUFFER, gpu::ShaderStageFlag::VERTEX, 1 };
    }
};

static constexpr auto VERTICES_SIZE = sizeof(Vertex) * stdr::size(VERTICES);

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
        "Stormkit GPU Textured cube example",
        { 1280u, 800u },
        wsi::WindowFlag::CLOSE | wsi::WindowFlag::EXTERNAL_CONTEXT
    };

    auto event_handler = wsi::EventHandler {};

    // initialize gpu backend (vulkan or webgpu depending the platform)
    gpu::initialize_backend();

    // create gpu instance and attach surface to window
    const auto instance = gpu::Instance::create("textured_cube")
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

    // create pools
    const auto command_pool
      = gpu::CommandPool::create(device)
          .transform_error(monadic::assert("Failed to create raster queue command pool"))
          .value();

    const auto descriptor_pool
      = gpu::DescriptorPool::create(device,
                                    {
                                      gpu::DescriptorPool::Size {
                                                                 .type             = gpu::DescriptorType::UNIFORM_BUFFER,
                                                                 .descriptor_count = BUFFERING_COUNT,
                                                                 },
                                      gpu::DescriptorPool::Size {
                                                                 .type = gpu::DescriptorType::COMBINED_IMAGE_SAMPLER,
                                                                 .descriptor_count = BUFFERING_COUNT }
    },
                                    BUFFERING_COUNT * 2)
          .transform_error(monadic::assert("Failed to create descriptor pool"))
          .value();

    // load shaders
    const auto vertex_shader = gpu::Shader::load_from_file(device,
                                                           SHADER_DIR "/textured_cube.spv",
                                                           gpu::ShaderStageFlag::VERTEX)
                                 .transform_error(monadic::assert("Failed to load vertex shader"))
                                 .value();

    const auto fragment_shader = gpu::Shader::load_from_file(device,
                                                             SHADER_DIR "/textured_cube.spv",
                                                             gpu::ShaderStageFlag::FRAGMENT)
                                   .transform_error(monadic::
                                                      assert("Failed to load fragment shader"))
                                   .value();

    const auto descriptor_set_layout
      = gpu::DescriptorSetLayout::
          create(device,
                 into_dyn_array(ViewerData::layout_binding(),
                                gpu::DescriptorSetLayoutBinding {
                                  1,
                                  gpu::DescriptorType::COMBINED_IMAGE_SAMPLER,
                                  gpu::ShaderStageFlag::FRAGMENT,
                                  1 }))
            .transform_error(monadic::assert("Failed to create descriptor set layout"))
            .value();

    const auto pipeline_layout
      = gpu::PipelineLayout::create(device,
                                    { .descriptor_set_layouts = to_refs(descriptor_set_layout) })
          .transform_error(monadic::assert("Failed to create pipeline layout"))
          .value();

    const auto& window_extent = window.extent();
    const auto  swapchain     = gpu::SwapChain::create(device, surface, window_extent.to<3>())
                             .transform_error(monadic::assert("Failed to create swapchain"))
                             .value();

    // initialize render pass
    const auto depth_format = [&physical_device] {
        const auto formats_properties = physical_device->formats_properties();
        const auto candidates         = std::array { gpu::PixelFormat::DEPTH32F,
                                             gpu::PixelFormat::DEPTH32F_STENCIL8U,
                                             gpu::PixelFormat::DEPTH24_UNORM_STENCIL8U };

        for (const auto format : candidates) {
            const auto properties = stdr::find_if(formats_properties, [format](const auto& pair) {
                return pair.first == format;
            });
            ENSURES(properties != stdr::cend(formats_properties));
            if (check_flag_bit(properties->second.optimal_tiling_features,
                               gpu::FormatFeatureFlag::DEPTH_STENCIL_ATTACHMENT)) {
                return format;
            }
        }

        ensures(false, "No supported depth format found !");
        std::unreachable();
    }();

    const auto depth_aspect_flag = [depth_format] {
        auto flag = gpu::ImageAspectFlag::DEPTH;
        if (gpu::is_depth_stencil_format(depth_format)) flag |= gpu::ImageAspectFlag::STENCIL;
        return flag;
    }();

    const auto render_pass
      = gpu::RenderPass::
          create(device,
                 { .attachments = { {
                     .format = swapchain.pixel_format(),
                   }, {
                       .format = depth_format,
                       .destination_layout = gpu::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL
                   }},
                   .subpasses   = { gpu::Subpass{ .bind_point            = gpu::PipelineBindPoint::GRAPHICS,
                                      .color_attachment_refs = { { .attachment_id = 0u }, }, .depth_attachment_ref = gpu::Subpass::Ref{.attachment_id = 1u, .layout = gpu::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL, }, }, }, })
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
        .rasterization_state = {
            .cull_mode = gpu::CullModeFlag::BACK,
            .front_face = gpu::FrontFace::CLOCKWISE,
        },
        .color_blend_state
        = { .attachments = { { .blend_enable           = true,
                               .src_color_blend_factor = gpu::BlendFactor::SRC_ALPHA,
                               .dst_color_blend_factor = gpu::BlendFactor::ONE_MINUS_SRC_ALPHA,
                               .src_alpha_blend_factor = gpu::BlendFactor::SRC_ALPHA,
                               .dst_alpha_blend_factor = gpu::BlendFactor::ONE_MINUS_SRC_ALPHA,
                               .alpha_blend_operation  = gpu::BlendOperation::ADD, }, }, },
        .shader_state  = to_refs(vertex_shader, fragment_shader),
        .vertex_input_state = {
            .binding_descriptions = into_dyn_array(Vertex::binding_description()),
            .input_attribute_descriptions = to_dyn_array(Vertex::attribute_descriptions()),
        },
            .depth_stencil_state = {
            .depth_test_enable =  true,
            .depth_write_enable = true
        },
    };

    auto pipeline = gpu::Pipeline::create(device, state, pipeline_layout, render_pass)
                      .transform_error(monadic::assert("Failed to create raster pipeline"))
                      .value();

    // load texture
    auto image = image::Image {};
    image.load_from_file(TEXTURE_DIR "/cube.png").transform_error(monadic::assert());

    auto texture = gpu::Image::create(device,
                                      { .extent = image.extent(),
                                        .format = gpu::PixelFormat::RGBA8_UNORM,
                                        .usages = gpu::ImageUsageFlag::SAMPLED
                                                  | gpu::ImageUsageFlag::TRANSFER_DST,
                                        .property = gpu::MemoryPropertyFlag::DEVICE_LOCAL })
                     .transform_error(monadic::assert("Failed to allocate texture"))
                     .value();

    {
        auto staging_buffer
          = gpu::Buffer::create(device,
                                { .usages = gpu::BufferUsageFlag::TRANSFER_SRC,
                                  .size   = image.size() })
              .transform_error(monadic::assert("Failed to allocate gpu texture staging buffer"))
              .value();

        staging_buffer.upload(image.data());

        auto cpy_fence = gpu::Fence::create(device)
                           .transform_error(monadic::
                                              assert("Failed to create copy texture buffer fence"))
                           .value();

        auto copy_cmb = command_pool.create_command_buffer()
                          .transform_error(monadic::assert("Failed to allocate copy texture buffer "
                                                           "commandbuffer"))
                          .value();

        copy_cmb.begin();

        copy_cmb.begin_debug_region("Upload vertex data to vertex buffer")
          .transition_image_layout(texture,
                                   gpu::ImageLayout::UNDEFINED,
                                   gpu::ImageLayout::TRANSFER_DST_OPTIMAL)
          .copy_buffer_to_image(staging_buffer,
                                texture,
                                {
                                  gpu::BufferImageCopy { .buffer_offset       = 0,
                                                        .buffer_row_length   = 0,
                                                        .buffer_image_height = 0,
                                                        .subresource_layers  = {},
                                                        .offset              = {},
                                                        .extent              = image.extent() }
        })
          .transition_image_layout(texture,
                                   gpu::ImageLayout::TRANSFER_DST_OPTIMAL,
                                   gpu::ImageLayout::SHADER_READ_ONLY_OPTIMAL)
          .end_debug_region();

        copy_cmb.end();
        copy_cmb.submit(raster_queue, {}, {}, {}, as_ref(cpy_fence));

        cpy_fence.wait().transform_error(monadic::assert());
    }

    auto texture_view = gpu::ImageView::create(device, texture)
                          .transform_error(monadic::assert("Failed to create texture view"))
                          .value();

    auto sampler = gpu::Sampler::create(device, {})
                     .transform_error(monadic::assert("Failed to create sampler"))
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
                               .transform_error(core::monadic::assert("Failed to create present "
                                                                      "wait semaphore"))
                               .value(),
          .render_cmb = command_pool.create_command_buffer()
                          .transform_error(monadic::assert("Failed to create "
                                                           "transition command "
                                                           "buffers"))
                          .value(),
          .viewer_buffer = gpu::Buffer::create(device,
                                               {
                                                 .usages = gpu::BufferUsageFlag::UNIFORM,
                                                 .size   = sizeof(ViewerData),
                                               },
                                               true)
                             .transform_error(monadic::
                                                assert("Failed to allocate gpu viewer buffer"))
                             .value(),
          .descriptor_set = descriptor_pool.create_descriptor_set(descriptor_set_layout)
                              .transform_error(monadic::assert("Failed to create descriptor set"))
                              .value(),
        });
        auto& res = submission_resources.back();
        res.descriptor_set
          .update({ gpu::Descriptor { gpu::BufferDescriptor {
                      .binding = 0,
                      .buffer  = as_ref(res.viewer_buffer),
                      .range   = sizeof(ViewerData),
                      .offset  = 0,
                    } },
                    { gpu::ImageDescriptor {
                      .binding    = 1,
                      .layout     = gpu::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
                      .image_view = as_ref(texture_view),
                      .sampler    = as_ref(sampler),
                    } } });
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
    for (const auto& swap_image : images) {
        auto view = gpu::ImageView::create(device, swap_image)
                      .transform_error(core::monadic::
                                         assert("Failed to create swapchain image view"))
                      .value();

        auto depth_image = gpu::Image::create(device,
                                              gpu::Image::CreateInfo {
                                                .extent = swap_image.extent(),
                                                .format = depth_format,
                                                .usages = gpu::ImageUsageFlag::
                                                  DEPTH_STENCIL_ATTACHMENT,
                                                .property = gpu::MemoryPropertyFlag::DEVICE_LOCAL })
                             .transform_error(core::monadic::assert("Failed to create depth image"))
                             .value();

        auto depth_view = gpu::ImageView::create(device,
                                                 depth_image,
                                                 gpu::ImageViewType::T2D,
                                                 { .aspect_mask = depth_aspect_flag })
                            .transform_error(core::monadic::
                                               assert("Failed to create depth image view"))
                            .value();

        auto framebuffer = render_pass
                             .create_frame_buffer(device, window_extent, to_refs(view, depth_view))
                             .transform_error(core::monadic::assert(
                               std::format("Failed to create framebuffer for image {}",
                                           image_index)))
                             .value();

        swapchain_image_resources.push_back({
          .image           = as_ref(swap_image),
          .view            = std::move(view),
          .depth_image     = std::move(depth_image),
          .depth_view      = std::move(depth_view),
          .framebuffer     = std::move(framebuffer),
          .render_finished = gpu::Semaphore::create(device)
                               .transform_error(core::monadic::assert("Failed to create render "
                                                                      "signal semaphore"))
                               .value(),
        });

        const auto& resources = swapchain_image_resources.back();

        auto& transition_cmb = transition_cmbs[image_index];
        transition_cmb.begin(true);

        transition_cmb.begin_debug_region(std::format("transition image {}", image_index))
          .transition_image_layout(swap_image,
                                   gpu::ImageLayout::UNDEFINED,
                                   gpu::ImageLayout::PRESENT_SRC)
          .transition_image_layout(resources.depth_image,
                                   gpu::ImageLayout::UNDEFINED,
                                   gpu::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                   { .aspect_mask = depth_aspect_flag })
          .end_debug_region();

        transition_cmb.end();

        ++image_index;
    }

    const auto fence = gpu::Fence::create(device)
                         .transform_error(monadic::assert("Failed to create transition fence"))
                         .value();

    const auto cmbs = to_refs(transition_cmbs);
    raster_queue.submit({ .command_buffers = cmbs }, as_ref(fence));

    // setup vertex buffer
    const auto vertex_buffer
      = gpu::Buffer::create(device,
                            { .usages = gpu::BufferUsageFlag::VERTEX
                                        | gpu::BufferUsageFlag::TRANSFER_DST,
                              .size     = VERTICES_SIZE,
                              .property = gpu::MemoryPropertyFlag::DEVICE_LOCAL })
          .transform_error(monadic::assert("Failed to allocate gpu vertex buffer"))
          .value();

    {
        auto staging_buffer
          = gpu::Buffer::create(device,
                                { .usages = gpu::BufferUsageFlag::TRANSFER_SRC,
                                  .size   = VERTICES_SIZE })
              .transform_error(monadic::assert("Failed to allocate gpu vertex staging buffer"))
              .value();

        staging_buffer.upload(VERTICES);

        auto cpy_fence = gpu::Fence::create(device)
                           .transform_error(monadic::
                                              assert("Failed to create copy vertex buffer fence"))
                           .value();

        auto copy_cmb = command_pool.create_command_buffer()
                          .transform_error(monadic::assert("Failed to allocate copy vertex buffer "
                                                           "commandbuffer"))
                          .value();

        copy_cmb.begin();

        copy_cmb.begin_debug_region("Upload vertex data to vertex buffer")
          .copy_buffer(staging_buffer, vertex_buffer, VERTICES_SIZE)
          .end_debug_region();

        copy_cmb.end();
        copy_cmb.submit(raster_queue, {}, {}, {}, as_ref(cpy_fence));

        cpy_fence.wait().transform_error(monadic::assert());
    }

    event_handler.set_callbacks({
      { wsi::EventType::CLOSED,      [&window](const wsi::Event&) noexcept { window.close(); } },
      { wsi::EventType::KEY_PRESSED,
       [&window](const wsi::Event& event) noexcept {
            const auto key_event = as<wsi::KeyPressedEventData>(event.data);
            if (key_event.key == wsi::Key::ESCAPE) window.close();
        }                                                                                      },
    });

    auto current_frame = 0uz;

    const auto window_extent_f32 = window_extent.to<f32>();
    auto       viewer_data       = ViewerData {
                    .proj = math::perspective(math::radians(45.f),
                                  window_extent_f32.width / window_extent_f32.height,
                                  0.1f,
                                  100.f),
                    .view = math::look_at(math::vec3f { 0.f, 3.f, 5.f }, { 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }),
                    .model = math::mat4f::identity(),
    };

    // wait for transition to be done
    fence.wait().transform_error(monadic::assert());

    using clock   = stdc::high_resolution_clock;
    using SecondF = stdc::duration<float, stdc::seconds::period>;

    auto start_time = clock::now();
    while (window.is_open()) {
        LOG_MODULE.flush();

        const auto current_time = clock::now();

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

        // update viewer data and upload
        const auto time   = stdc::duration_cast<SecondF>(current_time - start_time).count();
        viewer_data.model = math::rotate(math::mat4f::identity(),
                                         time * math::radians(90.f),
                                         math::vec3f { 0.f, 1.f, 0.f });

        auto& viewer_buffer = submission_resource.viewer_buffer;
        viewer_buffer.upload(viewer_data);

        // render in it
        auto&       render_cmb     = submission_resource.render_cmb;
        const auto& descriptor_set = submission_resource.descriptor_set;
        render_cmb.reset();
        render_cmb.begin();

        render_cmb.begin_debug_region("Render textured cube")
          .begin_render_pass(render_pass,
                             framebuffer,
                             { { gpu::ClearColor { .color = RGBColorDef::SILVER<float> } },
                               { gpu::ClearDepthStencil {} } })
          .bind_pipeline(pipeline)
          .bind_vertex_buffers(to_refs(vertex_buffer), { 0uz })
          .bind_descriptor_sets(pipeline, pipeline_layout, as_refs(descriptor_set), {})
          .draw(stdr::size(VERTICES))
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
