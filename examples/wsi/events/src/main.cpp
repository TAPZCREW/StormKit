// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit;

#include <stormkit/main/main_macro.hpp>

LOGGER("Events");

using namespace stormkit;
using namespace std::literals;

namespace stdr = std::ranges;

////////////////////////////////////////
////////////////////////////////////////
auto main(array_view<const string_view> args) -> int {
    wsi::parse_args(args);
    log::parse_args(args);

    auto logger = log::logger::create_logger_instance<log::console_logger>();

    const auto monitors = wsi::get_monitors();
    ilog("--- Monitors ---");
    ilog("{}", monitors);

    auto window = wsi::Window::open("StormKit WSI events Example",
                                    { .width = 800u, .height = 600u },
                                    wsi::WindowFlag::RESIZEABLE);
    ilog("wm: {}", window.wm());

    window.on<wsi::EventType::CLOSED>([] static noexcept {
        ilog("Close event");
        return true;
    });

    auto foo = 0;
    window.on(wsi::ResizedEventFunc { [](const math::uextent2& extent) static noexcept { ilog("Resize event: {}", extent); } },
              wsi::MonitorChangedEventFunc { [](const wsi::Monitor& monitor) noexcept {
                  ilog("Monitor changed event: {}", monitor);
              } },
              wsi::MouseMovedEventFunc { [](u8 /*id*/, const math::ivec2& position) noexcept {
                  ilog("Mouse move event: {}", position);
              } },
              wsi::MouseButtonDownEventFunc { [](u8 /*id*/, wsi::MouseButton button, const math::ivec2& position) noexcept {
                  ilog("Mouse button down event: {} {}", button, position);
              } },
              wsi::MouseButtonUpEventFunc { [](u8 /*id*/, wsi::MouseButton button, const math::ivec2& position) noexcept {
                  ilog("Mouse button up event: {} {}", button, position);
              } },
              wsi::RestoredEventFunc { [] noexcept { ilog("Restored event"); } },
              wsi::MinimizedEventFunc { [] noexcept { ilog("Minimized event"); } },
              wsi::ActivateEventFunc { [] noexcept { ilog("Activate event"); } },
              wsi::DeactivateEventFunc { [] noexcept { ilog("Deactivate event"); } },
              wsi::KeyDownEventFunc { [&window, &foo](u8 /*id*/, wsi::Key key, char c) mutable noexcept {
                  switch (key) {
                      case wsi::Key::ESCAPE:
                          window.close();
                          ilog("Closing window");
                          break;
                      case wsi::Key::W: {
                          auto extent = window.extent();
                          extent.width += 10;
                          window.set_extent(extent);
                      } break;
                      case wsi::Key::T: {
                          window.set_title(std::format("StormKit WSI Events Example | T pressed {} times", ++foo));
                      } break;
                      case wsi::Key::H: {
                          auto extent = window.extent();
                          extent.height += 10;
                          window.set_extent(extent);
                      } break;
                      case wsi::Key::F11:
                          window.toggle_fullscreen();
                          ilog("Toggling fullscreen to {}", window.fullscreen());
                          break;
                      case wsi::Key::F1:
                          window.toggle_hidden_mouse();
                          ilog("Toggling hidden mouse to {}", window.is_mouse_hidden());
                          break;
                      case wsi::Key::F2:
                          window.toggle_locked_mouse();
                          ilog("Toggling locked mouse to {}", window.is_mouse_locked());
                          break;
                      case wsi::Key::F3:
                          window.toggle_confined_mouse();
                          ilog("Toggling confined mouse to {}", window.is_mouse_confined());
                          break;
                      case wsi::Key::F4:
                          window.toggle_relative_mouse();
                          ilog("Toggling relative mouse to {}", window.is_mouse_relative());
                          break;
                      case wsi::Key::F5:
                          window.toggle_key_repeat();
                          ilog("Toggling key repeat to {}", window.is_key_repeat_enabled());
                          break;
                      default: break;
                  }

                  ilog("Key down --\n    code: {}\n    value: '{}'\n    raw_value: 0x{:0x})", key, c, c);
              } },
              wsi::KeyUpEventFunc { [](u8 /*id*/, wsi::Key key, char /*c*/) noexcept { ilog("Key up --\n    code: {}", key); } });

    window.event_loop([&] mutable { window.clear(); });

    return 0;
}
