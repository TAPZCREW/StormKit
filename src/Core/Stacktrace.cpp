// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <version>

#if defined(__cpp_lib_stacktrace) and __cpp_lib_stacktrace >= 202011L
    #define STD_STACKTRACE_SUPPORTED
#endif

#if not defined(STD_STACKTRACE_SUPPORTED)
    #include <cpptrace/cpptrace.hpp>
#endif

#include <stormkit/Core/PlatformMacro.hpp>

module stormkit.Core;

import std;

import :Console;
import :String.Operations;

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    auto printStacktrace([[maybe_unused]] int ignore_count) noexcept -> void {
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
        auto i = 0;
        for (auto&& frame : st) {
            if (i < ignore_count) {
                i += 1;
                continue;
            }
#if defined(STORMKIT_COMPILER_MSSTL)
            auto splitted = split(frame.description(), '+');
            const auto address = *fromString<UInt>(splitted[1].substr(2), 16);
            splitted = split(splitted[0], '!');
            const auto module = (std::size(splitted) >= 1) ? splitted[0] : "";
            auto symbol = (std::size(splitted) >= 2) ? splitted[1] : "";
#else
            
#endif
#ifdef STORMKIT_COMPILER_LIBCPP
            symbol = replace(symbol, "::__1::", "::");
#endif
            const auto object_address
                = (address == 0 ? "inlined"
                                : std::format("{:#010x}", address));
            
            const auto formatted_symbol = (symbol == "") ? std::format(" on {}", module)
                                                         : std::format(" in {} on {}", YellowTextStyle | symbol, module);

            if (frame.source_file() != "" and frame.source_line() != 0) {
                std::println(std::cerr,
                             "{}# {}{} at {}:{}",
                             (i++ - ignore_count),
                             BlueTextStyle | object_address,
                             formatted_symbol,
                             BlueTextStyle | frame.source_file(),
                             BlueTextStyle | frame.source_line());
            } else {
                std::println(std::cerr,
                             "{}# {}{}",
                             (i++ - ignore_count),
                             BlueTextStyle | object_address,
                             formatted_symbol);
            }
        }
#else
        const auto st = cpptrace::stacktrace::current();
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

            const auto object_address
                = (frame.object_address == 0 ? "inlined"
                                             : std::format("{:#010x}", frame.object_address));

            const auto formatted_symbol
                = (frame.symbol == "") ? "" : std::format(" in {}", YellowTextStyle | symbol);

            if (frame.line.has_value() and frame.column.has_value()) {
                std::println(std::cerr,
                             "{}# {}{} at {}:{}:{}",
                             (i++ - ignore_count),
                             BlueTextStyle | object_address,
                             formatted_symbol,
                             GreenTextStyle | frame.filename,
                             BlueTextStyle | frame.line.value(),
                             BlueTextStyle | frame.column.value());
            } else
                std::println(std::cerr,
                             "{}# {}{} at {}",
                             (i++ - ignore_count),
                             BlueTextStyle | object_address,
                             formatted_symbol,
                             GreenTextStyle | frame.filename);
        }
#endif
    }
}} // namespace stormkit::core
