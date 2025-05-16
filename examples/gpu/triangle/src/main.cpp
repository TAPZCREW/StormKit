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

LOGGER("StormKit.Examples.Log.EventHandler");

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

    auto instance = *gpu::Instance::create("Triangle")
                       .transform_error(core::monadic::assert("Failed to initialize gpu instance"));
    auto surface = *gpu::Surface::create_from_window(instance, window)
                      .transform_error(core::monadic::
                                         assert("Failed to initialize window gpu surface"));

    const auto& physical_devices = instance.physical_devices();
    if (std::ranges::empty(physical_devices)) {
        std::println("No render physical device found!");
        return 0;
    }
    std::println("Physical devices: {}", physical_devices);

    auto physical_device = as_ref(physical_devices.front());
    auto score           = gpu::score_physical_device(physical_device);
    for (auto i = 1u; i < std::ranges::size(physical_devices); ++i) {
        const auto& d       = physical_devices[i];
        const auto  d_score = gpu::score_physical_device(d);
        if (d_score > score) {
            physical_device = as_ref(d);
            score           = d_score;
        }
    }

    auto device = *gpu::Device::create(*physical_device, instance)
                     .transform_error(core::monadic::assert("Failed to initialize gpu device"));

    while (window.is_open()) {
        event_handler.update(window);

        LOG_MODULE.flush();

        if (toggle_fullscreen) {
            fullscreen = !fullscreen;
            window.toggle_fullscreen(fullscreen);

            toggle_fullscreen = false;
            ilog("Toggle fullscreen to: {}", fullscreen);
        }
    }

    return 0;
}
