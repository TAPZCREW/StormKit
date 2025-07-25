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
        { .width = 800u, .height = 600u },
        wsi::WindowStyle::ALL
    };

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
                    } else if (event_data.key == wsi::Key::F11) {
                        window.toggle_fullscreen();
                        ilog("Toggling fullscreen to {}", window.fullscreen());
                    } else if (event_data.key == wsi::Key::F1) {
                        window.toggle_hidden_mouse();
                        ilog("Toggling hidden mouse to {}", window.is_mouse_hidden());
                    } else if (event_data.key == wsi::Key::F2) {
                        window.toggle_locked_mouse();
                        ilog("Toggling locked mouse to {}", window.is_mouse_locked());
                    } else if (event_data.key == wsi::Key::F3) {
                        window.toggle_confined_mouse();
                        ilog("Toggling confined mouse to {}", window.is_mouse_confined());
                    } else if (event_data.key == wsi::Key::F4) {
                        window.toggle_relative_mouse();
                        ilog("Toggling relative mouse to {}", window.is_mouse_relative());
                    } else if (event_data.key == wsi::Key::F5) {
                        window.toggle_key_repeat();
                        ilog("Toggling key repeat to {}", window.is_key_repeat_enabled());
                    }

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

        window.clear(RGBColorDef::SILVER<u8>);
    }

    return 0;
}
