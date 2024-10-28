// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.Core;
import stormkit.Log;
import stormkit.Wsi;

#include <stormkit/Log/LogMacro.hpp>
#include <stormkit/Main/MainMacro.hpp>

NAMED_LOGGER(polling_logger, "StormKit.Examples.Log.Polling");

////////////////////////////////////////
////////////////////////////////////////
auto main(std::span<const std::string_view> args) -> int {
    using namespace stormkit;
    using namespace std::literals;

    wsi::parseArgs(args);

    auto logger = log::Logger::createLoggerInstance<log::ConsoleLogger>();

    const auto monitors = wsi::Window::getMonitorSettings();
    polling_logger.ilog("--- Monitors ---");
    polling_logger.ilog("{}", monitors);

    auto window = wsi::Window { "Hello world", { 800u, 600u }, wsi::WindowStyle::All };

    auto fullscreen        = false;
    auto toggle_fullscreen = false;
    while (window.isOpen()) {
        auto event = wsi::Event {};
        while (window.pollEvent(event)) {
            switch (event.type) {
                case wsi::EventType::Closed: window.close(); break;
                case wsi::EventType::Resized: {
                    const auto& event_data = as<wsi::ResizedEventData>(event.data);
                    polling_logger.ilog("Resize event: {}", event_data.extent);
                    break;
                }
                case wsi::EventType::MouseMoved: {
                    const auto& event_data = as<wsi::MouseMovedEventData>(event.data);
                    polling_logger.ilog("Mouse move event: {}", event_data.position);
                    break;
                }
                case wsi::EventType::MouseButtonPushed: {
                    const auto& event_data = as<wsi::MouseButtonPushedEventData>(event.data);
                    polling_logger.ilog("Mouse button push event: {} {}",
                                        event_data.button,
                                        event_data.position);
                    break;
                }
                case wsi::EventType::MouseButtonReleased: {
                    const auto& event_data = as<wsi::MouseButtonReleasedEventData>(event.data);
                    polling_logger.ilog("Mouse button release event: {} {}",
                                        event_data.button,
                                        event_data.position);
                    break;
                }
                case wsi::EventType::MouseEntered: {
                    polling_logger.ilog("Mouse Entered event");
                    break;
                }
                case wsi::EventType::MouseExited: {
                    polling_logger.ilog("Mouse Exited event");
                    break;
                }
                case wsi::EventType::LostFocus: {
                    polling_logger.ilog("Lost focus event");
                    break;
                }
                case wsi::EventType::GainedFocus: {
                    polling_logger.ilog("Gained focus event");
                    break;
                }
                case wsi::EventType::KeyPressed: {
                    const auto& event_data = as<wsi::KeyPressedEventData>(event.data);

                    if (event_data.key == wsi::Key::Escape) {
                        window.close();
                        polling_logger.ilog("Closing window");
                    } else if (event_data.key == wsi::Key::F11)
                        toggle_fullscreen = true;

                    polling_logger.ilog("Key pressed: {}", event_data.key);
                    break;
                }
                case wsi::EventType::KeyReleased: {
                    const auto& event_data = as<wsi::KeyReleasedEventData>(event.data);

                    polling_logger.ilog("Key release: {}", event_data.key);
                    break;
                }
                default: break;
            }
        }
        polling_logger.flush();

        if (toggle_fullscreen) {
            fullscreen = !fullscreen;
            window.setFullscreenEnabled(fullscreen);

            toggle_fullscreen = false;
            polling_logger.ilog("Toggle fullscreen to: {}", fullscreen);
        }
    }

    return 0;
}
