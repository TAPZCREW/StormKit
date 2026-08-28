// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/lua/lua.hpp>

module stormkit.lua:image;

import std;

import stormkit.core;
import stormkit.image;

namespace stormkit::lua::image {
    auto init_lua(sol::state& global_state) noexcept -> void;
} // namespace stormkit::lua::image
