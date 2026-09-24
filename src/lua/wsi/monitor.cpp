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
    using stormkit::wsi::Monitor;

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto bind_monitor(sol::state& global_state, sol::table& metatable) noexcept -> void {
        metatable["monitor_flag"] = global_state.create_table_with(
          sol::meta_function::as<string>,
          +[](Monitor::Flags flags) { return as<string>(flags); },
          "NONE",
          Monitor::Flags::NONE,
          "PRIMARY",
          Monitor::Flags::PRIMARY);

        auto monitor                           = metatable.new_usertype<Monitor>("monitor");
        monitor[sol::meta_function::as<string>] = +[](Monitor::Flags flags) { return as<string>(flags); },
        monitor[sol::meta_function::is]  = &Monitor::operator==;
        monitor[sol::meta_function::less_than] = +[](const Monitor& first, const Monitor& second) static noexcept {
            return first < second;
        };
        monitor["flags"]        = &Monitor::flags;
        monitor["name"]         = &Monitor::name;
        monitor["extents"]      = &Monitor::extents;
        monitor["scale_factor"] = &Monitor::scale_factor;
    }
} // namespace stormkit::lua::wsi
