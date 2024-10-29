// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#if not defined(STD_STACKTRACE_SUPPORTED)
    #include <cpptrace/cpptrace.hpp>
#endif

#include <stormkit/Core/PlatformMacro.hpp>

module stormkit.Core;

import std;

import :Console;
import :String.Operations;

namespace stormkit::core {
    /////////////////////////////////////
    /////////////////////////////////////
    auto printStacktrace(int ignore_count) noexcept -> void {
        const auto thread_name = getCurrentThreadName();
        if (not std::empty(thread_name))
            std::println(std::cerr,
                         "================= CALLSTACK (thread name: {}, id: {}) =================",
                         thread_name,
                         std::this_thread::get_id());
        else
            std::println(std::cerr,
                         "================= CALLSTACK (thread id: {}) =================",
                         std::this_thread::get_id());
#if defined(STD_STACKTRACE_SUPPORTED)
        const auto st = std::stacktrace::current();
#else
        const auto st = cpptrace::stacktrace::current();
#endif
        auto i = 0;
        for (auto&& frame : st) {
            if (i < ignore_count) {
                i += 1;
                continue;
            }
            auto symbol = frame.symbol;
#ifdef STORMKIT_COMPILER_LIBCPP
            symbol = replace(symbol, "::__1::", "::");
#endif
            symbol
                = replace(symbol, "basic_string_view<char, std::char_traits<char>>", "string_view");
            symbol = replace(symbol,
                             "basic_string<char, std::char_traits<char>, std::allocator<char>>",
                             "string");

            if (frame.line.has_value() and frame.column.has_value())
                std::println(std::cerr,
                             "{}# {}{} at {}:{}:{}",
                             (i++ - ignore_count),
                             ConsoleStyle { .fg = ConsoleColor::Blue }
                                 | (frame.object_address == 0
                                        ? "inlined"
                                        : std::format("{:#010x}", frame.object_address)),
                             (frame.symbol == "")
                                 ? ""
                                 : std::format(" in {}",
                                               ConsoleStyle { .fg = ConsoleColor::Yellow }
                                                   | symbol),
                             ConsoleStyle { .fg = ConsoleColor::Green } | frame.filename,
                             ConsoleStyle { .fg = ConsoleColor::Blue } | frame.line.value(),
                             ConsoleStyle { .fg = ConsoleColor::Blue } | frame.column.value());
            else
                std::println(std::cerr,
                             "{}# {}{} at {}",
                             (i++ - ignore_count),
                             ConsoleStyle { .fg = ConsoleColor::Blue }
                                 | (frame.object_address == 0
                                        ? "inlined"
                                        : std::format("{:#010x}", frame.object_address)),
                             (frame.symbol == "")
                                 ? ""
                                 : std::format(" in {}",
                                               ConsoleStyle { .fg = ConsoleColor::Yellow }
                                                   | symbol),
                             ConsoleStyle { .fg = ConsoleColor::Green } | frame.filename);
        }
    }
} // namespace stormkit::core
