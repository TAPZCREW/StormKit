// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.core;
import stormkit.log;
import stormkit.wsi;

#include <stormkit/log/log_macro.hpp>
#include <stormkit/main/main_macro.hpp>

LOGGER("StormKit.Examples.Log.Polling");

////////////////////////////////////////
////////////////////////////////////////
auto main(std::span<const std::string_view> args) -> int {
    using namespace stormkit;
    using namespace std::literals;

    wsi::parse_args(args);

    auto logger = log::Logger::create_logger_instance<log::ConsoleLogger>();

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
    while (window.is_open()) {
        auto event = wsi::Event {};
        while (window.poll_event(event)) {
            switch (event.type) {
                case wsi::EventType::CLOSED: window.close(); break;
                case wsi::EventType::RESIZED: {
                    const auto& event_data = as<wsi::ResizedEventData>(event.data);
                    ilog("Resize event: {}", event_data.extent);
                    break;
                }
                case wsi::EventType::MOUSE_MOVED: {
                    const auto& event_data = as<wsi::MouseMovedEventData>(event.data);
                    ilog("Mouse move event: {}", event_data.position);
                    break;
                }
                case wsi::EventType::MOUSE_BUTTON_PUSHED: {
                    const auto& event_data = as<wsi::MouseButtonPushedEventData>(event.data);
                    ilog("Mouse button push event: {} {}", event_data.button, event_data.position);
                    break;
                }
                case wsi::EventType::MOUSE_BUTTON_RELEASED: {
                    const auto& event_data = as<wsi::MouseButtonReleasedEventData>(event.data);
                    ilog("Mouse button release event: {} {}",
                         event_data.button,
                         event_data.position);
                    break;
                }
                case wsi::EventType::MOUSE_ENTERED: {
                    ilog("Mouse Entered event");
                    break;
                }
                case wsi::EventType::MOUSE_EXITED: {
                    ilog("Mouse Exited event");
                    break;
                }
                case wsi::EventType::LOST_FOCUS: {
                    ilog("Lost focus event");
                    break;
                }
                case wsi::EventType::GAINED_FOCUS: {
                    ilog("Gained focus event");
                    break;
                }
                case wsi::EventType::KEY_PRESSED: {
                    const auto& event_data = as<wsi::KeyPressedEventData>(event.data);

                    if (event_data.key == wsi::Key::ESCAPE) {
                        window.close();
                        ilog("Closing window");
                    } else if (event_data.key == wsi::Key::F11)
                        toggle_fullscreen = true;

                    ilog("Key pressed: {}", event_data.key);
                    break;
                }
                case wsi::EventType::KEY_RELEASED: {
                    const auto& event_data = as<wsi::KeyReleasedEventData>(event.data);

                    ilog("Key release: {}", event_data.key);
                    break;
                }
                default: break;
            }
        }

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
