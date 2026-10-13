// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <cstdio>

module stormkit.log;

import std;

import stormkit.core;

using namespace std::literals;

namespace stdr = std::ranges;

namespace stormkit::log {
    namespace {
        constexpr auto style_map = make_static_hash_map<severity, console_style>({
          { severity::info,    console_style { .fg = console_color::green, .modifiers = style_modifier::inverse }   },
          { severity::warning, console_style { .fg = console_color::magenta, .modifiers = style_modifier::inverse } },
          { severity::error,   console_style { .fg = console_color::yellow, .modifiers = style_modifier::inverse }  },
          { severity::fatal,   console_style { .fg = console_color::red, .modifiers = style_modifier::inverse }     },
          { severity::debug,   console_style { .fg = console_color::cyan, .modifiers = style_modifier::inverse }    },
        });

        constexpr auto format_string             = "{}[ {:^7} | {:%S} ]{} {}"sv;
        constexpr auto format_string_with_module = "{}[ {:^7} | {} | {:%S} ]{} {}"sv;
    } // namespace

    ////////////////////////////////////////
    ////////////////////////////////////////
    console_logger::console_logger(clock_type::time_point start) noexcept : logger { std::move(start) } {
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    console_logger::console_logger(clock_type::time_point start, severity log_level) noexcept
        : logger { std::move(start), log_level } {
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto console_logger::write(severity severity_, const module& module, std::string_view str) noexcept -> void {
        const auto now      = clock_type::now();
        const auto time     = std::chrono::duration_cast<std::chrono::seconds>(now - start_time_);
        const auto is_error = severity_ == severity::error or severity_ == severity::fatal;
        const auto out      = (is_error) ? get_stderr() : get_stdout();
        const auto style    = style_map.at(severity_);

        if (stdr::empty(module.name)) std::println(out, format_string, style, severity_, time, ecma48::reset, str);
        else
            std::println(out, format_string_with_module, style, severity_, module.name, time, ecma48::reset, str);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto console_logger::flush() noexcept -> void {
        std::fflush(get_stdout());
        std::fflush(get_stderr());
    }
} // namespace stormkit::log
