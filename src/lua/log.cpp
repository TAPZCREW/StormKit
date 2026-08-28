// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/lua/lua.hpp>

module stormkit.lua;

import std;

import stormkit.core;
import stormkit.log;

LOGGER("lua")

namespace stormkit::lua::log {
    auto init_lua(sol::state& global_state) noexcept -> void {
        auto log_table    = global_state["log"].get_or_create<sol::table>();
        log_table["info"] = [&global_state](string_view str, sol::variadic_args args) noexcept {
            const auto format = sol::protected_function { global_state["format"] };
            const auto result = luacall(format, str, std::move(args));
            const auto out    = sol::object { result }.as<string>();
            ilog("{}", out);
        };
        log_table["debug"] = [&global_state](string_view str, sol::variadic_args args) noexcept {
            const auto format = sol::protected_function { global_state["format"] };
            const auto result = luacall(format, str, std::move(args));
            const auto out    = sol::object { result }.as<string>();
            dlog("{}", out);
        };
        log_table["error"] = [&global_state](string_view str, sol::variadic_args args) noexcept {
            const auto format = sol::protected_function { global_state["format"] };
            const auto result = luacall(format, str, std::move(args));
            const auto out    = sol::object { result }.as<string>();
            elog("{}", out);
        };
        log_table["fatal"] = [&global_state](string_view str, sol::variadic_args args) noexcept {
            const auto format = sol::protected_function { global_state["format"] };
            const auto result = luacall(format, str, std::move(args));
            const auto out    = sol::object { result }.as<string>();
            flog("{}", out);
        };
        log_table["warning"] = [&global_state](string_view str, sol::variadic_args args) noexcept {
            const auto format = sol::protected_function { global_state["format"] };
            const auto result = luacall(format, str, std::move(args));
            const auto out    = sol::object { result }.as<string>();
            wlog("{}", out);
        };
    }
} // namespace stormkit::lua::log
