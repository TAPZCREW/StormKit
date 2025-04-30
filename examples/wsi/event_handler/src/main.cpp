// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.core;
import stormkit.log;
import stormkit.Wsi;

#include <stormkit/Log/LogMacro.hpp>
#include <stormkit/main/main_macro.hpp>

NAMED_LOGGER(logger, "StormKit.Examples.Log.EventHandler");

auto main(std::span<const std::string_view> args) -> int {
    using namespace stormkit;
    using namespace std::literals;

    wsi::parseArgs(args);

    auto logger_singleton = log::Logger::create_logger_instance<log::ConsoleLogger>();

    const auto monitors = wsi::Window::getMonitorSettings();
    logger.ilog("--- Monitors ---");
    logger.ilog("{}", monitors);

    auto window = wsi::Window {
        "Hello world",
        { 800u, 600u },
        wsi::WindowStyle::All
    };

    auto fullscreen        = false;
    auto toggle_fullscreen = false;

    auto event_handler = wsi::EventHandler {};
    event_handler.setCallbacks({
        { wsi::EventType::Closed,
         [&]([[maybe_unused]] const wsi::Event& event) { window.close(); }                    },
        { wsi::EventType::Resized,
         [](const wsi::Event& event) {
              const auto& event_data = as<wsi::ResizedEventData>(event.data);
              logger.ilog("Resize event: {}", event_data.extent);
          }                                                                                   },
        { wsi::EventType::MouseMoved,
         [](const wsi::Event& event) {
              const auto& event_data = as<wsi::MouseMovedEventData>(event.data);
              logger.ilog("Mouse move event: {}", event_data.position);
          }                                                                                   },
        { wsi::EventType::MouseButtonPushed,
         [](const wsi::Event& event) {
              const auto& event_data = as<wsi::MouseButtonPushedEventData>(event.data);
              logger.ilog("Mouse button push event: {} {}", event_data.button, event_data.position);
          }                                                                                   },
        { wsi::EventType::MouseButtonReleased,
         [](const wsi::Event& event) {
              const auto& event_data = as<wsi::MouseButtonReleasedEventData>(event.data);
              logger.ilog("Mouse button release event: {} {}",
                          event_data.button,
                          event_data.position);
          }                                                                                   },
        { wsi::EventType::MouseEntered,
         []([[maybe_unused]] const wsi::Event& event) { logger.ilog("Mouse Entered event"); } },
        { wsi::EventType::MouseExited,
         []([[maybe_unused]] const wsi::Event& event) { logger.ilog("Mouse Exited event"); }  },
        { wsi::EventType::LostFocus,
         []([[maybe_unused]] const wsi::Event& event) { logger.ilog("Lost focus event"); }    },
        { wsi::EventType::GainedFocus,
         []([[maybe_unused]] const wsi::Event& event) { logger.ilog("Gained focus event"); }  },
        { wsi::EventType::KeyPressed,
         [&](const wsi::Event& event) {
              const auto& event_data = as<wsi::KeyPressedEventData>(event.data);

              if (event_data.key == wsi::Key::Escape) {
                  window.close();
                  logger.ilog("Closing window");
              } else if (event_data.key == wsi::Key::F11)
                  toggle_fullscreen = true;

              logger.ilog("Key pressed: {}", event_data.key);
          }                                                                                   },
        { wsi::EventType::KeyReleased,         [](const wsi::Event& event) {
             const auto& event_data = as<wsi::KeyReleasedEventData>(event.data);

             logger.ilog("Key release: {}", event_data.key);
         }                                              }
    });

    while (window.isOpen()) {
        event_handler.update(window);

        logger.flush();

        if (toggle_fullscreen) {
            fullscreen = !fullscreen;
            window.setFullscreenEnabled(fullscreen);

            toggle_fullscreen = false;
            logger.ilog("Toggle fullscreen to: {}", fullscreen);
        }
    }

    return 0;
}
