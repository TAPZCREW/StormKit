// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#if not defined(STD_STACKTRACE_SUPPORTED)
    #include <cpptrace/cpptrace.hpp>
#endif

module stormkit.Core;

import std;

import :Console.Style;

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
            if (frame.line.has_value() and frame.column.has_value())
                std::println(std::cerr,
                             "{}# {} in {} at {}:{}:{}",
                             (i++ - ignore_count),
                             ConsoleStyle { .fg = ConsoleColor::Blue } | frame.object_address,
                             ConsoleStyle { .fg = ConsoleColor::Yellow } | frame.symbol,
                             ConsoleStyle { .fg = ConsoleColor::Green } | frame.filename,
                             ConsoleStyle { .fg = ConsoleColor::Blue } | frame.line.value(),
                             ConsoleStyle { .fg = ConsoleColor::Blue } | frame.column.value());
            else
                std::println(std::cerr,
                             "{}# {} in {} at {}",
                             (i++ - ignore_count),
                             ConsoleStyle { .fg = ConsoleColor::Blue } | frame.object_address,
                             ConsoleStyle { .fg = ConsoleColor::Yellow } | frame.symbol,
                             ConsoleStyle { .fg = ConsoleColor::Green } | frame.symbol);
        }
    }
} // namespace stormkit::core
