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
    using stormkit::wsi::monitor;

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto bind_monitor(sol::state& global_state, sol::table& metatable) noexcept -> void {
        metatable["monitor_flag"] = global_state.create_table_with(
          sol::meta_function::as<string>,
          +[](monitor::Flags flags) { return as<string>(flags); },
          "none",
          monitor::Flags::none,
          "PRIMARY",
          monitor::Flags::PRIMARY);

        auto monitor                           = metatable.new_usertype<monitor>("monitor");
        monitor[sol::meta_function::as<string>] = +[](monitor::Flags flags) { return as<string>(flags); },
        monitor[sol::meta_function::is]  = &monitor::operator==;
        monitor[sol::meta_function::less_than] = +[](const monitor& first, const monitor& second) static noexcept {
            return first < second;
        };
        monitor["flags"]        = &monitor::flags;
        monitor["name"]         = &monitor::name;
        monitor["extents"]      = &monitor::extents;
        monitor["scale_factor"] = &monitor::scale_factor;
    }
} // namespace stormkit::lua::wsi
