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
    using stormkit::wsi::window_manager;

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto bind_core(sol::state& global_state, sol::table& metatable) noexcept -> void {
        metatable["window_manager"] = global_state.create_table_with(
          sol::meta_function::as<string>,
          +[](window_manager wm) { return as<string>(wm); },
          "WIN32",
          window_manager::win32,
          "WAYLAND",
          window_manager::wayland,
          "X11",
          window_manager::x11,
          "ANDROID",
          window_manager::android,
          "MACOS",
          window_manager::macos,
          "IOS",
          window_manager::IOS,
          "TVOS",
          window_manager::TVOS,
          "SWITCH",
          window_manager::SWITCH);
    }
} // namespace stormkit::lua::wsi
