// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit;

#include <stormkit/main/main_macro.hpp>

using namespace stormkit;
using namespace std::literals;

namespace stdr = std::ranges;

constexpr auto LOG_MODULE = log::module { "events" };

template<class... Ts>
inline auto ilog(std::format_string<Ts...> format, Ts&&... args) noexcept -> void {
    LOG_MODULE.ilog(std::move(format), std::forward<Ts>(args)...);
}

////////////////////////////////////////
////////////////////////////////////////
auto main(array_view<const string_view> args) -> int {
    wsi::parse_args(args);
    log::parse_args(args);

    auto logger = log::logger::create_logger_instance<log::console_logger>();

    const auto monitors = wsi::get_monitors();
    ilog("--- Monitors ---");
    ilog("{}", monitors);

    auto window = wsi::window::open("StormKit WSI events Example",
                                    { .width = 800u, .height = 600u },
                                    wsi::window_flag::resizeable);
    ilog("wm: {}", window.wm());

    window.on<wsi::event_type::closed>([] static noexcept {
        ilog("Close event");
        return true;
    });

    auto foo = 0;
    window.on(wsi::resized_event_cb_type { [](const math::uextent2& extent) static noexcept {
                  ilog("Resize event: {}", extent);
              } },
              wsi::monitor_changed_event_cb_type { [](const wsi::monitor& monitor) noexcept {
                  ilog("Monitor changed event: {}", monitor);
              } },
              wsi::mouse_moved_event_cb_type { [](u8 /*id*/, const math::ivec2& position) noexcept {
                  ilog("Mouse move event: {}", position);
              } },
              wsi::mouse_button_down_event_cb_type {
                [](u8 /*id*/, wsi::mouse_button button, const math::ivec2& position) noexcept {
                    ilog("Mouse button down event: {} {}", button, position);
                } },
              wsi::mouse_button_up_event_cb_type { [](u8 /*id*/, wsi::mouse_button button, const math::ivec2& position) noexcept {
                  ilog("Mouse button up event: {} {}", button, position);
              } },
              wsi::restored_event_cb_type { [] noexcept { ilog("Restored event"); } },
              wsi::minimized_event_cb_type { [] noexcept { ilog("Minimized event"); } },
              wsi::activate_event_cb_type { [] noexcept { ilog("Activate event"); } },
              wsi::deactivate_event_cb_type { [] noexcept { ilog("Deactivate event"); } },
              wsi::key_down_event_cb_type { [&window, &foo](u8 /*id*/, wsi::key key, char c) mutable noexcept {
                  switch (key) {
                      case wsi::key::escape:
                          window.close();
                          ilog("Closing window");
                          break;
                      case wsi::key::w: {
                          auto extent = window.extent();
                          extent.width += 10;
                          window.set_extent(extent);
                      } break;
                      case wsi::key::t: {
                          window.set_title(std::format("StormKit WSI Events Example | T pressed {} times", ++foo));
                      } break;
                      case wsi::key::h: {
                          auto extent = window.extent();
                          extent.height += 10;
                          window.set_extent(extent);
                      } break;
                      case wsi::key::f11:
                          window.toggle_fullscreen();
                          ilog("Toggling fullscreen to {}", window.fullscreen());
                          break;
                      case wsi::key::f1:
                          window.toggle_hidden_mouse();
                          ilog("Toggling hidden mouse to {}", window.is_mouse_hidden());
                          break;
                      case wsi::key::f2:
                          window.toggle_locked_mouse();
                          ilog("Toggling locked mouse to {}", window.is_mouse_locked());
                          break;
                      case wsi::key::f3:
                          window.toggle_confined_mouse();
                          ilog("Toggling confined mouse to {}", window.is_mouse_confined());
                          break;
                      case wsi::key::f4:
                          window.toggle_relative_mouse();
                          ilog("Toggling relative mouse to {}", window.is_mouse_relative());
                          break;
                      case wsi::key::f5:
                          window.toggle_key_repeat();
                          ilog("Toggling key repeat to {}", window.is_key_repeat_enabled());
                          break;
                      default: break;
                  }

                  ilog("Key down --\n    code: {}\n    value: '{}'\n    raw_value: 0x{:0x})", key, c, c);
              } },
              wsi::key_up_event_cb_type { [](u8 /*id*/, wsi::key key, char /*c*/) noexcept {
                  ilog("Key up --\n    code: {}", key);
              } });

    window.event_loop([&] mutable { window.clear(); });

    return 0;
}
