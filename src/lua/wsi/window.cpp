// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
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
    using stormkit::wsi::EventType;
    using stormkit::wsi::Key;
    using stormkit::wsi::Monitor;
    using stormkit::wsi::MouseButton;
    using stormkit::wsi::Window;
    using stormkit::wsi::WindowFlag;

    namespace {
        ////////////////////////////////////////
        ////////////////////////////////////////
        template<wsi::EventType EVENT_TYPE, typename... Ts>
        constexpr auto make_lua_closure() noexcept {
            return [](Window* window, sol::protected_function closure) static noexcept {
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
    auto open_window(string name, u32 width, u32 height, wsi::WindowFlag flags) noexcept -> wsi::Window {
        return wsi::Window::open(std::move(name), { width, height }, flags);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto bind_window(sol::state& global_state, sol::table& metatable) noexcept -> void {
        metatable["window_flag"] = global_state.create_table_with(
          sol::meta_function::as<string>,
          +[](WindowFlag flags) { return as<string>(flags); },
          "DEFAULT",
          WindowFlag::DEFAULT,
          "BORDERLESS",
          WindowFlag::BORDERLESS,
          "RESIZEABLE",
          WindowFlag::RESIZEABLE,
          "EXTERNAL_CONTEXT",
          WindowFlag::EXTERNAL_CONTEXT);
        metatable["event_type"] = global_state.create_table_with(
          sol::meta_function::as<string>,
          +[](EventType type) { return as<string>(type); },
          "NONE",
          EventType::NONE,
          "CLOSED",
          EventType::CLOSED,
          "MONITOR_CHANGED",
          EventType::MONITOR_CHANGED,
          "RESIZED",
          EventType::RESIZED,
          "RESTORED",
          EventType::RESTORED,
          "MINIMIZED",
          EventType::MINIMIZED,
          "KEY_DOWN",
          EventType::KEY_DOWN,
          "KEY_UP",
          EventType::KEY_UP,
          "MOUSE_BUTTON_DOWN",
          EventType::MOUSE_BUTTON_DOWN,
          "MOUSE_BUTTON_UP",
          EventType::MOUSE_BUTTON_UP,
          "MOUSE_MOVED",
          EventType::MOUSE_MOVED,
          "ACTIVATE",
          EventType::ACTIVATE,
          "DEACTIVATE",
          EventType::DEACTIVATE);

        auto window_metatable                 = metatable.new_usertype<Window>("window");
        window_metatable["open"]              = &open_window;
        window_metatable["window_manager"]    = &Window::wm;
        window_metatable["extent"]            = &Window::extent;
        window_metatable["set_extent"]        = &Window::set_extent;
        window_metatable["close"]             = &Window::close;
        window_metatable["title"]             = &Window::title;
        window_metatable["set_title"]         = &Window::set_title;
        window_metatable["fullscreen"]        = &Window::fullscreen;
        window_metatable["toggle_fullscreen"] = &Window::toggle_fullscreen;

        window_metatable["is_mouse_hidden"] = sol::
          overload(&Window::is_mouse_hidden, +[](Window* window) static noexcept { return window->is_mouse_hidden(); });
        window_metatable["toggle_hidden_mouse"] = sol::
          overload(&Window::toggle_hidden_mouse, +[](Window* window) static noexcept { return window->toggle_hidden_mouse(); });
        window_metatable["is_mouse_locked"] = sol::
          overload(&Window::is_mouse_locked, +[](Window* window) static noexcept { return window->is_mouse_locked(); });
        window_metatable["toggle_locked_mouse"] = sol::
          overload(&Window::toggle_locked_mouse, +[](Window* window) static noexcept { return window->toggle_locked_mouse(); });
        window_metatable["is_mouse_confined"] = sol::
          overload(&Window::is_mouse_confined, +[](Window* window) static noexcept { return window->is_mouse_confined(); });
        window_metatable["toggle_confined_mouse"] = sol::overload(
          &Window::toggle_confined_mouse,
          +[](Window* window) static noexcept { return window->toggle_confined_mouse(); });
        window_metatable["is_mouse_relative"] = sol::
          overload(&Window::is_mouse_relative, +[](Window* window) static noexcept { return window->is_mouse_relative(); });
        window_metatable["toggle_relative_mouse"] = sol::overload(
          &Window::toggle_relative_mouse,
          +[](Window* window) static noexcept { return window->toggle_relative_mouse(); });
        window_metatable["is_key_repeat_enabled"] = sol::overload(
          &Window::is_key_repeat_enabled,
          +[](Window* window) static noexcept { return window->is_key_repeat_enabled(); });
        window_metatable["toggle_key_repeat"] = sol::
          overload(&Window::toggle_key_repeat, +[](Window* window) static noexcept { return window->toggle_key_repeat(); });
        window_metatable["clear"] = sol::
          overload(&Window::clear, +[](Window* window) static noexcept { return window->clear(); });

        window_metatable["event_loop"] = +[](Window* window, sol::protected_function closure) static noexcept {
            window->event_loop([closure = std::move(closure)] noexcept {
                auto result = closure();
                if (not result.valid()) ensures(false, sol::error { result }.what());
            });
        };
        window_metatable["on_closed"] = +[](Window* window, sol::protected_function closure) static noexcept {
            window->on<EventType::CLOSED>([closure = std::move(closure)] noexcept {
                const auto result = closure();
                if (not result.valid()) ensures(false, sol::error { result }.what());
                auto value = sol::object { result };
                ensures(value.is<bool>(), "on_closed closure must return a boolean value");
                return value.as<bool>();
            });
        };
        window_metatable["on_monitor_changed"] = +make_lua_closure<EventType::MONITOR_CHANGED, const Monitor&>();
        window_metatable["on_resized"]         = +make_lua_closure<EventType::RESIZED, const math::uextent2&>();
        window_metatable["on_restored"]        = +make_lua_closure<EventType::RESTORED>();
        window_metatable["on_minimized"]       = +make_lua_closure<EventType::MINIMIZED>();
        window_metatable["on_key_up"]          = +make_lua_closure<EventType::KEY_UP, u8, Key, char>();
        window_metatable["on_key_down"]        = +make_lua_closure<EventType::KEY_DOWN, u8, Key, char>();
        window_metatable
          ["on_mouse_button_up"] = +make_lua_closure<EventType::MOUSE_BUTTON_UP, u8, MouseButton, const math::ivec2&>();
        window_metatable
          ["on_mouse_button_down"] = +make_lua_closure<EventType::MOUSE_BUTTON_DOWN, u8, MouseButton, const math::ivec2&>();
        window_metatable["on_mouse_moved"] = +make_lua_closure<EventType::MOUSE_MOVED, u8, const math::ivec2&>();
        window_metatable["on_activated"]   = +make_lua_closure<EventType::ACTIVATE>();
        window_metatable["on_deactivated"] = +make_lua_closure<EventType::DEACTIVATE>();
    }
} // namespace stormkit::lua::wsi
