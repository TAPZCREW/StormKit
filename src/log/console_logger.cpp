// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
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
        constexpr auto STYLE_MAP = make_static_hash_map<severity, console_style>({
          { severity::INFO,    console_style { .fg = console_color::GREEN, .modifiers = style_modifier::INVERSE }   },
          { severity::WARNING, console_style { .fg = console_color::MAGENTA, .modifiers = style_modifier::INVERSE } },
          { severity::ERROR,   console_style { .fg = console_color::YELLOW, .modifiers = style_modifier::INVERSE }  },
          { severity::FATAL,   console_style { .fg = console_color::RED, .modifiers = style_modifier::INVERSE }     },
          { severity::DEBUG,   console_style { .fg = console_color::CYAN, .modifiers = style_modifier::INVERSE }    },
        });

        constexpr auto FORMAT_STRING             = "{}[ {:<7} | {:%S} ]{} {}"sv;
        constexpr auto FORMAT_STRING_WITH_MODULE = "{}[ {:<7} | {} | {:%S} ]{} {}"sv;
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
        const auto time     = std::chrono::duration_cast<std::chrono::seconds>(now - m_start_time);
        const auto is_error = severity_ == severity::ERROR or severity_ == severity::FATAL;
        const auto out      = (is_error) ? get_stderr() : get_stdout();
        const auto style    = STYLE_MAP.at(severity_);

        if (stdr::empty(module.name)) std::println(out, FORMAT_STRING, style, severity_, time, ecma48::RESET, str);
        else
            std::println(out, FORMAT_STRING_WITH_MODULE, style, severity_, module.name, time, ecma48::RESET, str);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto console_logger::flush() noexcept -> void {
        std::fflush(get_stdout());
        std::fflush(get_stderr());
    }
} // namespace stormkit::log
