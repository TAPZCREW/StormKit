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
    using stormkit::wsi::mouse_button;

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto bind_mouse(sol::state& global_state, sol::table& metatable) noexcept -> void {
        metatable["mouse_button"] = global_state.create_table_with(
          sol::meta_function::as<string>,
          +[](mouse_button mouse_button) { return as<string>(mouse_button); },
          "LEFT",
          mouse_button::left,
          "RIGHT",
          mouse_button::right,
          "MIDDLE",
          mouse_button::middle,
          "BUTTON_1",
          mouse_button::button_1,
          "BUTTON_2",
          mouse_button::button_2,
          "BUTTON_3",
          mouse_button::button_3,
          "BUTTON_4",
          mouse_button::button_4,
          "BUTTON_5",
          mouse_button::button_5,
          "BUTTON_6",
          mouse_button::button_6,
          "BUTTON_7",
          mouse_button::button_7,
          "BUTTON_8",
          mouse_button::button_8,
          "BUTTON_9",
          mouse_button::button_9,
          "BUTTON_10",
          mouse_button::button_10,
          "BUTTON_11",
          mouse_button::button_11,
          "BUTTON_12",
          mouse_button::button_12);
    }
} // namespace stormkit::lua::wsi
