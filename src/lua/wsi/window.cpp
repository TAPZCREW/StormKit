// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#include <stormkit/lua/lua.hpp>

module stormkit.lua;

import std;

import stormkit.core;
import stormkit.wsi;

namespace stormkit::lua::wsi {
    using stormkit::wsi::event_type;
    using stormkit::wsi::Key;
    using stormkit::wsi::Monitor;
    using stormkit::wsi::mouse_button;
    using stormkit::wsi::window;
    using stormkit::wsi::window_flag;

    namespace {
        ////////////////////////////////////////
        ////////////////////////////////////////
        template<wsi::event_type EVENT_TYPE, typename... Ts>
        constexpr auto make_lua_closure() noexcept {
            return [](window* window, sol::protected_function closure) static noexcept {
                window->on<EVENT_TYPE>([closure = std::move(closure)](Ts&&... args) noexcept {
                    auto result = closure(std::forward<Ts>(args)...);
                    if (not result.valid())
                        ensures(false,
                                std::format("lua runtime error!\nlua callstack -------------------------\n{}",
                                            sol::error { result }.what()));
                });
            };
        }
    } // namespace

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto open_window(string name, u32 width, u32 height, wsi::window_flag flags) noexcept -> wsi::window {
        return wsi::window::open(std::move(name), { width, height }, flags);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto bind_window(sol::state& global_state, sol::table& metatable) noexcept -> void {
        metatable["window_flag"] = global_state.create_table_with(
          sol::meta_function::as<string>,
          +[](window_flag flags) { return as<string>(flags); },
          "default",
          window_flag::default,
          "borderless",
          window_flag::borderless,
          "resizeable",
          window_flag::resizeable,
          "external_context",
          window_flag::external_context);
        metatable["event_type"] = global_state.create_table_with(
          sol::meta_function::as<string>,
          +[](event_type type) { return as<string>(type); },
          "none",
          event_type::none,
          "closed",
          event_type::closed,
          "monitor_changed",
          event_type::monitor_changed,
          "resized",
          event_type::resized,
          "restored",
          event_type::restored,
          "minimized",
          event_type::minimized,
          "key_down",
          event_type::key_down,
          "key_up",
          event_type::key_up,
          "mouse_button_down",
          event_type::mouse_button_down,
          "mouse_button_up",
          event_type::mouse_button_up,
          "mouse_moved",
          event_type::mouse_moved,
          "activate",
          event_type::activate,
          "deactivate",
          event_type::deactivate);

        auto window_metatable                 = metatable.new_usertype<window>("window");
        window_metatable["open"]              = &open_window;
        window_metatable["window_manager"]    = &window::wm;
        window_metatable["extent"]            = &window::extent;
        window_metatable["set_extent"]        = &window::set_extent;
        window_metatable["close"]             = &window::close;
        window_metatable["title"]             = &window::title;
        window_metatable["set_title"]         = &window::set_title;
        window_metatable["fullscreen"]        = &window::fullscreen;
        window_metatable["toggle_fullscreen"] = &window::toggle_fullscreen;

        window_metatable["is_mouse_hidden"] = sol::
          overload(&window::is_mouse_hidden, +[](window* window) static noexcept { return window->is_mouse_hidden(); });
        window_metatable["toggle_hidden_mouse"] = sol::
          overload(&window::toggle_hidden_mouse, +[](window* window) static noexcept { return window->toggle_hidden_mouse(); });
        window_metatable["is_mouse_locked"] = sol::
          overload(&window::is_mouse_locked, +[](window* window) static noexcept { return window->is_mouse_locked(); });
        window_metatable["toggle_locked_mouse"] = sol::
          overload(&window::toggle_locked_mouse, +[](window* window) static noexcept { return window->toggle_locked_mouse(); });
        window_metatable["is_mouse_confined"] = sol::
          overload(&window::is_mouse_confined, +[](window* window) static noexcept { return window->is_mouse_confined(); });
        window_metatable["toggle_confined_mouse"] = sol::overload(
          &window::toggle_confined_mouse,
          +[](window* window) static noexcept { return window->toggle_confined_mouse(); });
        window_metatable["is_mouse_relative"] = sol::
          overload(&window::is_mouse_relative, +[](window* window) static noexcept { return window->is_mouse_relative(); });
        window_metatable["toggle_relative_mouse"] = sol::overload(
          &window::toggle_relative_mouse,
          +[](window* window) static noexcept { return window->toggle_relative_mouse(); });
        window_metatable["is_key_repeat_enabled"] = sol::overload(
          &window::is_key_repeat_enabled,
          +[](window* window) static noexcept { return window->is_key_repeat_enabled(); });
        window_metatable["toggle_key_repeat"] = sol::
          overload(&window::toggle_key_repeat, +[](window* window) static noexcept { return window->toggle_key_repeat(); });
        window_metatable["clear"] = sol::
          overload(&window::clear, +[](window* window) static noexcept { return window->clear(); });

        window_metatable["event_loop"] = +[](window* window, sol::protected_function closure) static noexcept {
            window->event_loop([closure = std::move(closure)] noexcept {
                auto result = closure();
                if (not result.valid()) ensures(false, sol::error { result }.what());
            });
        };
        window_metatable["on_closed"] = +[](window* window, sol::protected_function closure) static noexcept {
            window->on<event_type::closed>([closure = std::move(closure)] noexcept {
                const auto result = closure();
                if (not result.valid()) ensures(false, sol::error { result }.what());
                auto value = sol::object { result };
                ensures(value.is<bool>(), "on_closed closure must return a boolean value");
                return value.as<bool>();
            });
        };
        window_metatable["on_monitor_changed"] = +make_lua_closure<event_type::monitor_changed, const Monitor&>();
        window_metatable["on_resized"]         = +make_lua_closure<event_type::resized, const math::uextent2&>();
        window_metatable["on_restored"]        = +make_lua_closure<event_type::restored>();
        window_metatable["on_minimized"]       = +make_lua_closure<event_type::minimized>();
        window_metatable["on_key_up"]          = +make_lua_closure<event_type::key_up, u8, Key, char>();
        window_metatable["on_key_down"]        = +make_lua_closure<event_type::key_down, u8, Key, char>();
        window_metatable
          ["on_mouse_button_up"] = +make_lua_closure<event_type::mouse_button_up, u8, mouse_button, const math::ivec2&>();
        window_metatable
          ["on_mouse_button_down"] = +make_lua_closure<event_type::mouse_button_down, u8, mouse_button, const math::ivec2&>();
        window_metatable["on_mouse_moved"] = +make_lua_closure<event_type::mouse_moved, u8, const math::ivec2&>();
        window_metatable["on_activated"]   = +make_lua_closure<event_type::activate>();
        window_metatable["on_deactivated"] = +make_lua_closure<event_type::deactivate>();
    }
} // namespace stormkit::lua::wsi
