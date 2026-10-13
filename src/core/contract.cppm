// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#ifndef STORMKIT_ASSERT
    #define STORMKIT_ASSERT 1
#endif

module stormkit.core.contract;

import std;

import stormkit.core.stacktrace;
import stormkit.core.string.format;
import stormkit.core.typesafe.flags;
import stormkit.core.console.style;
import stormkit.core.console.io;

namespace stormkit { inline namespace core {
    static auto mutex = std::mutex {};

    /////////////////////////////////////
    /////////////////////////////////////
    auto assert_base(bool cond, assert_type type, string_view message, const std::source_location& location) noexcept -> void {
        if constexpr (STORMKIT_ASSERT == 1) {
            static constexpr auto assertion_prefix = console_style {
                .fg        = console_color::bright_red,
                .modifiers = style_modifier::bold | style_modifier::inverse
            } | "[Assertion]"sv;
            if (not cond) [[unlikely]] {
                auto lock = std::unique_lock { mutex };
                std::println(get_stderr(),
                             "{} {} failed in \n"
                             "    > file:     {}:{}:{}\n"
                             "      function: {}\n"
                             "      reason:   {}",
                             assertion_prefix,
                             as_string(type),
                             console_style { .fg = console_color::green } | location.file_name(),
                             console_style { .fg = console_color::blue } | location.line(),
                             console_style { .fg = console_color::blue } | location.column(),
                             console_style { .fg = console_color::yellow } | location.function_name(),
                             console_style { .fg = console_color::red, .modifiers = style_modifier::bold } | message);
                std::fflush(get_stderr());
#if defined(__cpp_lib_debugging) and __cpp_lib_debugging >= 202311L
                if (std::is_debugger_present()) { std::breakpoint(); }
#endif

                std::terminate();
            }
        }
    }
}} // namespace stormkit::core
