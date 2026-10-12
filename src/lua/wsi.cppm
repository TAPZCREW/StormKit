// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#include <stormkit/lua/lua.hpp>

module stormkit.lua:wsi;

import std;

import stormkit.core;
import stormkit.wsi;

// template<>
// struct lb::Stack<stormkit::wsi::window_flag>: lb::Enum<stormkit::wsi::window_flag> {};

// template<>
// struct lb::Stack<stormkit::wsi::window_manager>
//     : lb::Enum<stormkit::wsi::window_manager,
//                stormkit::wsi::window_manager::win32,
//                stormkit::wsi::window_manager::wayland,
//                stormkit::wsi::window_manager::x11,
//                stormkit::wsi::window_manager::android,
//                stormkit::wsi::window_manager::macos,
//                stormkit::wsi::window_manager::IOS,
//                stormkit::wsi::window_manager::TVOS,
//                stormkit::wsi::window_manager::SWITCH> {};

// template<>
// struct lb::Stack<stormkit::wsi::event_type>
//     : lb::Enum<stormkit::wsi::event_type,
//                stormkit::wsi::event_type::none,
//                stormkit::wsi::event_type::closed,
//                stormkit::wsi::event_type::monitor_changed,
//                stormkit::wsi::event_type::resized,
//                stormkit::wsi::event_type::restored,
//                stormkit::wsi::event_type::minimized,
//                stormkit::wsi::event_type::key_down,
//                stormkit::wsi::event_type::key_up,
//                stormkit::wsi::event_type::mouse_button_down,
//                stormkit::wsi::event_type::mouse_button_up,
//                stormkit::wsi::event_type::mouse_moved,
//                stormkit::wsi::event_type::activate,
//                stormkit::wsi::event_type::deactivate> {};

namespace stormkit::lua::wsi {
    auto init_lua(sol::state& global_state) noexcept -> void;
} // namespace stormkit::lua::wsi
