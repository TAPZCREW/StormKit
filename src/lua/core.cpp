// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/lua/lua.hpp>

module stormkit.lua;

import std;

import stormkit.core;

namespace stormkit::lua::core {
    extern auto bind_math(sol::state& global_state) noexcept -> void;
    extern auto bind_color(sol::state& global_state) noexcept -> void;

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto init_lua(sol::state& global_state) noexcept -> void {
        bind_color(global_state);
        bind_math(global_state);
    }
} // namespace stormkit::lua::core
