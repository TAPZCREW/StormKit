// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
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
// struct lb::Stack<stormkit::wsi::WindowFlag>: lb::Enum<stormkit::wsi::WindowFlag> {};

// template<>
// struct lb::Stack<stormkit::wsi::WM>
//     : lb::Enum<stormkit::wsi::WM,
//                stormkit::wsi::WM::WIN32,
//                stormkit::wsi::WM::WAYLAND,
//                stormkit::wsi::WM::X11,
//                stormkit::wsi::WM::ANDROID,
//                stormkit::wsi::WM::MACOS,
//                stormkit::wsi::WM::IOS,
//                stormkit::wsi::WM::TVOS,
//                stormkit::wsi::WM::SWITCH> {};

// template<>
// struct lb::Stack<stormkit::wsi::EventType>
//     : lb::Enum<stormkit::wsi::EventType,
//                stormkit::wsi::EventType::NONE,
//                stormkit::wsi::EventType::CLOSED,
//                stormkit::wsi::EventType::MONITOR_CHANGED,
//                stormkit::wsi::EventType::RESIZED,
//                stormkit::wsi::EventType::RESTORED,
//                stormkit::wsi::EventType::MINIMIZED,
//                stormkit::wsi::EventType::KEY_DOWN,
//                stormkit::wsi::EventType::KEY_UP,
//                stormkit::wsi::EventType::MOUSE_BUTTON_DOWN,
//                stormkit::wsi::EventType::MOUSE_BUTTON_UP,
//                stormkit::wsi::EventType::MOUSE_MOVED,
//                stormkit::wsi::EventType::ACTIVATE,
//                stormkit::wsi::EventType::DEACTIVATE> {};

namespace stormkit::lua::wsi {
    auto init_lua(sol::state& global_state) noexcept -> void;
} // namespace stormkit::lua::wsi
