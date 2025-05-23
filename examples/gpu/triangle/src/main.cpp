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
namespace stdv = std::views;

auto main(std::span<const std::string_view> args) -> int {
    using namespace stormkit;
    using namespace std::literals;

    wsi::parse_args(args);

    // initialize logger
    auto logger_singleton = log::Logger::create_logger_instance<log::ConsoleLogger>();

    const auto monitors = wsi::Window::get_monitor_settings();
    ilog("--- Monitors ---");
    ilog("{}", monitors);

    // initialize WSI
    auto window = wsi::Window {
        "Hello world",
        { 800u, 600u },
        wsi::WindowStyle::ALL
    };

    auto fullscreen        = false;
    auto toggle_fullscreen = false;

    auto event_handler = wsi::EventHandler {};
    event_handler.set_callbacks({
      { wsi::EventType::CLOSED, [&](const wsi::Event&) { window.close(); } },
    });

    // initialize gpu
    gpu::initialize_backend();

    // create gpu instance and attach surface to window
    auto instance = gpu::Instance::create("Triangle")
                      .transform_error(monadic::assert("Failed to initialize gpu instance"))
                      .value();
    auto surface = gpu::Surface::create_from_window(instance, window)
                     .transform_error(monadic::assert("Failed to initialize window gpu surface"))
                     .value();

    // pick a physical device
    const auto& physical_devices = instance.physical_devices();
    if (stdr::empty(physical_devices)) {
        std::println("No render physical device found!");
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
    auto device = gpu::Device::allocate(*physical_device, instance)
                    .transform_error(monadic::assert("Failed to initialize gpu device"))
                    .value();
    dlog("{}", device->native_handle());
    std::this_thread::sleep_for(5000ms);
    auto raster_queue = gpu::Queue::create(*device, device->raster_queue_entry());

    // create swapchain
    auto swapchain = gpu::SwapChain::create(*device, surface, window.extent().to<3>())
                       .transform_error(monadic::assert("Failed to create swapchain"))
                       .value();

    // create present engine resources
    auto current_frame    = 0uz;
    auto image_availables = std::vector<gpu::Semaphore> {};
    auto render_finisheds = std::vector<gpu::Semaphore> {};
    auto in_flight_fences = std::vector<gpu::Fence> {};
    for (auto _ : range(stdr::size(swapchain.images()))) {
        gpu::Semaphore::create(*device)
          .transform(monadic::emplace_to(image_availables))
          .transform_error(core::monadic::assert("Failed to create gpu semaphore"));

        gpu::Semaphore::create(*device)
          .transform(monadic::emplace_to(render_finisheds))
          .transform_error(core::monadic::assert("Failed to create gpu semaphore"));

        gpu::Fence::create(*device)
          .transform(monadic::emplace_to(in_flight_fences))
          .transform_error(core::monadic::assert("Failed to create gpu semaphore"));
    }

    const auto command_pool
      = gpu::CommandPool::create(*device)
          .transform_error(monadic::assert("Failed to create raster queue command pool"))
          .value();

    auto transition_command_buffers
      = command_pool.create_command_buffers(stdr::size(swapchain.images()))
          .transform_error(monadic::assert("Failed to create raster queue command buffers"))
          .value();

    for (auto i : range(stdr::size(transition_command_buffers))) {
        auto&& image                     = swapchain.images()[i];
        auto&& transition_command_buffer = transition_command_buffers[i];

        transition_command_buffer.begin(true);
        transition_command_buffer.transition_image_layout(image,
                                                          gpu::ImageLayout::UNDEFINED,
                                                          gpu::ImageLayout::PRESENT_SRC);
        transition_command_buffer.end();
    }

    auto fence = gpu::Fence::create(*device)
                   .transform_error(monadic::map(monadic::narrow<gpu::Result>(),
                                                 monadic::throw_as_exception()))
                   .value();

    auto cmbs = to_refs(transition_command_buffers);
    raster_queue.submit({ .command_buffers = cmbs }, as_ref(fence));

    fence.wait().transform_error(monadic::assert());

    auto vertex_shader = gpu::Shader::load_from_file(*device,
                                                     SHADER_DIR "/shader.spv",
                                                     gpu::ShaderStageFlag::VERTEX)
                           .transform_error(monadic::assert("Failed to load vertex shader"))
                           .value();

    auto fragment_shader = gpu::Shader::load_from_file(*device,
                                                       SHADER_DIR "/shader.spv",
                                                       gpu::ShaderStageFlag::FRAGMENT)
                             .transform_error(monadic::assert("Failed to load fragment shader"))
                             .value();

    while (window.is_open()) {
        event_handler.update(window);

        LOG_MODULE.flush();

        if (toggle_fullscreen) {
            fullscreen = !fullscreen;
            window.toggle_fullscreen(fullscreen);

            toggle_fullscreen = false;
            ilog("Toggle fullscreen to: {}", fullscreen);
        }
        const auto& image_available = image_availables[current_frame];
        const auto& render_finished = render_finisheds[current_frame];
        auto&       in_flight       = in_flight_fences[current_frame];

        auto image_index = in_flight.wait()
                             .transform([&in_flight](auto&&) noexcept { in_flight.reset(); })
                             .and_then(bind_front(&gpu::SwapChain::acquire_next_image,
                                                  &swapchain,
                                                  100ms,
                                                  std::cref(image_available)))
                             .transform([](auto&& _result) static noexcept {
                                 auto&& [result, _image_index] = _result;
                                 return _image_index;
                             })
                             .value();

        const auto image_indices   = std::array { image_index };
        const auto wait_semaphores = as_refs<std::array>(render_finished);
        const auto swapchains      = as_refs<std::array>(swapchain);

        raster_queue.present(swapchains, wait_semaphores, image_indices)
          .transform([&current_frame](auto&& _) mutable noexcept {
              if (++current_frame >= 4) current_frame = 0;
          });
    }

    return 0;
}
