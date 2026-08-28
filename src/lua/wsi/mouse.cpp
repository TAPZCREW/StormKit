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
    using stormkit::wsi::MouseButton;

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto bind_mouse(sol::state& global_state, sol::table& metatable) noexcept -> void {
        metatable["mouse_button"] = global_state.create_table_with(
          sol::meta_function::as<string>,
          +[](MouseButton mouse_button) { return as<string>(mouse_button); },
          "LEFT",
          MouseButton::LEFT,
          "RIGHT",
          MouseButton::RIGHT,
          "MIDDLE",
          MouseButton::MIDDLE,
          "BUTTON_1",
          MouseButton::BUTTON_1,
          "BUTTON_2",
          MouseButton::BUTTON_2,
          "BUTTON_3",
          MouseButton::BUTTON_3,
          "BUTTON_4",
          MouseButton::BUTTON_4,
          "BUTTON_5",
          MouseButton::BUTTON_5,
          "BUTTON_6",
          MouseButton::BUTTON_6,
          "BUTTON_7",
          MouseButton::BUTTON_7,
          "BUTTON_8",
          MouseButton::BUTTON_8,
          "BUTTON_9",
          MouseButton::BUTTON_9,
          "BUTTON_10",
          MouseButton::BUTTON_10,
          "BUTTON_11",
          MouseButton::BUTTON_11,
          "BUTTON_12",
          MouseButton::BUTTON_12);
    }
} // namespace stormkit::lua::wsi
