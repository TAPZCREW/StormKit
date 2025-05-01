// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.core;
import stormkit.log;
import stormkit.wsi;

#include <stormkit/log/log_macro.hpp>
#include <stormkit/main/main_macro.hpp>

LOGGER("StormKit.Examples.Log.EventHandler");

auto main(std::span<const std::string_view> args) -> int {
    using namespace stormkit;
    using namespace std::literals;

    wsi::parse_args(args);

    auto logger_singleton = log::Logger::create_logger_instance<log::ConsoleLogger>();

    const auto monitors = wsi::Window::get_monitor_settings();
    ilog("--- Monitors ---");
    ilog("{}", monitors);

    auto window = wsi::Window {
        "Hello world",
        { 800u, 600u },
        wsi::WindowStyle::ALL
    };

    auto fullscreen        = false;
    auto toggle_fullscreen = false;

    auto event_handler = wsi::EventHandler {};
    event_handler.set_callbacks({
        { wsi::EventType::CLOSED,                [&](const wsi::Event& _) { window.close(); }             },
        { wsi::EventType::RESIZED,
         [](const wsi::Event& event) {
              const auto& event_data = as<wsi::ResizedEventData>(event.data);
              ilog("Resize event: {}", event_data.extent);
          }                                                                                               },
        { wsi::EventType::MOUSE_MOVED,
         [](const wsi::Event& event) {
              const auto& event_data = as<wsi::MouseMovedEventData>(event.data);
              ilog("Mouse move event: {}", event_data.position);
          }                                                                                               },
        { wsi::EventType::MOUSE_BUTTON_PUSHED,
         [](const wsi::Event& event) {
              const auto& event_data = as<wsi::MouseButtonPushedEventData>(event.data);
              ilog("Mouse button push event: {} {}", event_data.button, event_data.position);
          }                                                                                               },
        { wsi::EventType::MOUSE_BUTTON_RELEASED,
         [](const wsi::Event& event) {
              const auto& event_data = as<wsi::MouseButtonReleasedEventData>(event.data);
              ilog("Mouse button release event: {} {}", event_data.button, event_data.position);
          }                                                                                               },
        { wsi::EventType::MOUSE_ENTERED,         [](const wsi::Event& _) { ilog("Mouse Entered event"); } },
        { wsi::EventType::MOUSE_EXITED,          [](const wsi::Event& _) { ilog("Mouse Exited event"); }  },
        { wsi::EventType::LOST_FOCUS,            [](const wsi::Event& _) { ilog("Lost focus event"); }    },
        { wsi::EventType::GAINED_FOCUS,          [](const wsi::Event& _) { ilog("Gained focus event"); }  },
        { wsi::EventType::KEY_PRESSED,
         [&](const wsi::Event& event) {
              const auto& event_data = as<wsi::KeyPressedEventData>(event.data);

              if (event_data.key == wsi::Key::ESCAPE) {
                  window.close();
                  ilog("Closing window");
              } else if (event_data.key == wsi::Key::F11)
                  toggle_fullscreen = true;

              ilog("Key pressed: {}", event_data.key);
          }                                                                                               },
        { wsi::EventType::KEY_RELEASED,          [](const wsi::Event& event) {
             const auto& event_data = as<wsi::KeyReleasedEventData>(event.data);

             ilog("Key release: {}", event_data.key);
         }                                                         }
    });

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
