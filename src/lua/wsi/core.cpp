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
    using stormkit::wsi::WM;

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto bind_core(sol::state& global_state, sol::table& metatable) noexcept -> void {
        metatable["window_manager"] = global_state.create_table_with(
          sol::meta_function::as<string>,
          +[](WM wm) { return as<string>(wm); },
          "WIN32",
          WM::WIN32,
          "WAYLAND",
          WM::WAYLAND,
          "X11",
          WM::X11,
          "ANDROID",
          WM::ANDROID,
          "MACOS",
          WM::MACOS,
          "IOS",
          WM::IOS,
          "TVOS",
          WM::TVOS,
          "SWITCH",
          WM::SWITCH);
    }
} // namespace stormkit::lua::wsi
