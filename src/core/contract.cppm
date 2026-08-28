// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#ifndef STORMKIT_ASSERT
    #define STORMKIT_ASSERT 1
#endif

module stormkit.core.contract;

import std;

import stormkit.core.string.format;
import stormkit.core.typesafe.flags;
import stormkit.core.typesafe.safecasts;
import stormkit.core.console.style;
import stormkit.core.console.io;

namespace stormkit { inline namespace core {
    static auto mutex = std::mutex {};

    /////////////////////////////////////
    /////////////////////////////////////
    auto assert_base(bool cond, assert_type type, string_view message, const std::source_location& location) noexcept -> void {
        if constexpr (STORMKIT_ASSERT == 1) {
            constexpr auto ASSERTION_PREFIX = console_style {
                .fg        = console_color::BRIGHT_RED,
                .modifiers = style_modifier::BOLD | style_modifier::INVERSE
            } | "[Assertion]"sv;
            if (not cond) [[unlikely]] {
                auto lock = std::unique_lock { mutex };
                std::println(get_stderr(),
                             "{} {} failed in \n"
                             "    > file:     {}:{}:{}\n"
                             "      function: {}\n"
                             "      reason:   {}",
                             ASSERTION_PREFIX,
                             as_string(type),
                             console_style { .fg = console_color::GREEN } | location.file_name(),
                             console_style { .fg = console_color::BLUE } | location.line(),
                             console_style { .fg = console_color::BLUE } | location.column(),
                             console_style { .fg = console_color::YELLOW } | location.function_name(),
                             console_style { .fg = console_color::RED, .modifiers = style_modifier::BOLD } | message);
                std::fflush(get_stderr());
#if defined(__cpp_lib_debugging) and __cpp_lib_debugging >= 202311L
                if (std::is_debugger_present) { std::breakpoint(); }
#endif

                std::terminate();
            }
        }
    }
}} // namespace stormkit::core
