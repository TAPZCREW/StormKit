// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <version>

#include <stormkit/core/platform_macro.hpp>

#if defined(__cpp_lib_stacktrace) and __cpp_lib_stacktrace >= 202011L
    #define STD_STACKTRACE_SUPPORTED
#endif

#if not defined(STD_STACKTRACE_SUPPORTED)
    #include <cpptrace/cpptrace.hpp>
#endif

module stormkit.core;

import std;

import :console;
import :string.operations;

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    auto print_stacktrace(int ignore_count) noexcept -> void {
        const auto thread_name = get_current_thread_name();
        if (not std::empty(thread_name))
            std::println(get_stderr(),
                         "================= CALLSTACK (thread name: {}, id: {}) =================",
                         thread_name,
                         std::this_thread::get_id());
        else
            std::println(get_stderr(),
                         "================= CALLSTACK (thread id: {}) =================",
                         std::this_thread::get_id());
#ifdef STD_STACKTRACE_SUPPORTED
        const auto st = std::stacktrace::current();
        auto       i  = 0;
        for (auto&& frame : st) {
            if (i < ignore_count) {
                i += 1;
                continue;
            }
    #ifdef STORMKIT_COMPILER_MSSTL
            auto       splitted = split(frame.description(), '+');
            const auto address  = from_string<u64>(splitted[1].substr(2), 16)
                                   .transform_error([&splitted](auto&& err) noexcept {
                                       std::println(get_stderr(),
                                                    "Failed to parse {}, reason: {}",
                                                    splitted[0],
                                                    err.message());
                                       return 0;
                                   })
                                   .value();
            splitted                    = split(splitted[0], '!');
            const auto module           = (std::size(splitted) >= 1) ? splitted[0] : ""s;
            auto       formatted_symbol = (std::size(splitted) >= 2) ? splitted[1] : ""s;
    #elifdef STORMKIT_COMPILER_LIBCPP
            // clang-format off
            // e.g 0x5adc4b1dc9fc: __invoke<(lambda at src/gpu/core/device.cpp:401:22)>: /opt/llvm-git/include/c++/v1/__type_traits/invoke.h:179
            // e.g 0x730060b19a75: terminate: /home/arthapz/Development/AUR/llvm-git/src/llvm-project/libcxxabi/src/cxa_handlers.cpp:92
            // clang-format on
            const auto frame_str = std::to_string(frame);
            const auto splitted  = split(frame_str, ": ");
            const auto address   = from_string<u64>(splitted[0].substr(2), 16)
                                   .transform_error([&splitted](auto&& err) noexcept {
                                       std::println(get_stderr(),
                                                    "Failed to parse {}, reason: {}",
                                                    splitted[0],
                                                    err);
                                       return 0;
                                   })
                                   .value();

            auto formatted_symbol = (stdr::size(splitted) > 2)
                                      ? "\n    in " + (YELLOW_TEXT_STYLE | splitted[1]).render()
                                      : ""s;
            formatted_symbol      = replace(formatted_symbol, "::__1::", "::");
            formatted_symbol      = replace(formatted_symbol, "::$_0::", "::");
            formatted_symbol      = replace(formatted_symbol, "__invoke", "invoke");
            formatted_symbol      = replace(formatted_symbol, "__function", "function");
            formatted_symbol      = replace(formatted_symbol, "[abi:se210000]", "");
            formatted_symbol      = replace(formatted_symbol, "[abi:ne210000]", "");
            formatted_symbol      = replace(formatted_symbol,
                                       "basic_string_view<char, std::char_traits<char>>",
                                       "string_view");
            formatted_symbol      = replace(formatted_symbol,
                                       "basic_string<char, std::char_traits<char>, "
                                            "std::allocator<char>>",
                                       "string");
    #else
            const auto address          = 0;
            const auto formatted_symbol = ""s;

    #endif
            const auto object_address = (address == 0 ? "inlined"
                                                      : std::format("{:#010x}", address));

            // const auto formatted_symbol = (symbol == "")
            //                                 ? ""
            //                                 : std::format(" in{}", YELLOW_TEXT_STYLE | symbol);

            if (not std::ranges::empty(frame.source_file()) and frame.source_line() != 0) {
                std::println(get_stderr(),
                             "{}# {}{}\n    at {}:{}",
                             (i++ - ignore_count),
                             BLUE_TEXT_STYLE | object_address,
                             formatted_symbol,
                             GREEN_TEXT_STYLE | frame.source_file(),
                             BLUE_TEXT_STYLE | frame.source_line());
            } else if (not std::ranges::empty(frame.source_file())) {
                std::println(get_stderr(),
                             "{}# {}{}\n    at {}",
                             (i++ - ignore_count),
                             BLUE_TEXT_STYLE | object_address,
                             formatted_symbol,
                             GREEN_TEXT_STYLE | frame.source_file());
            } else {
                std::println(get_stderr(),
                             "{}# {}{}",
                             (i++ - ignore_count),
                             BLUE_TEXT_STYLE | object_address,
                             formatted_symbol);
            }
        }
#else
        const auto st = cpptrace::stacktrace::current();
        auto       i  = 0;
        for (auto&& frame : st) {
            if (i < ignore_count) {
                i += 1;
                continue;
            }
            auto symbol = frame.symbol;
    #ifdef STORMKIT_COMPILER_LIBCPP
            symbol = replace(symbol, "::$_0::", "::");
            symbol = replace(symbol, "::__1::", "::");
            symbol = replace(symbol, "__invoke", "invoke");
            symbol = replace(symbol, "__function", "function");
            symbol = replace(symbol, "[abi:se210000]", "");
            symbol = replace(symbol, "[abi:ne210000]", "");
    #endif
            symbol = replace(symbol,
                             "basic_string_view<char, std::char_traits<char>>",
                             "string_view");
            symbol = replace(symbol,
                             "basic_string<char, std::char_traits<char>, std::allocator<char>>",
                             "string");

            const auto object_address = (frame.object_address == 0
                                           ? "inlined"
                                           : std::format("{:#010x}", frame.object_address));

            const auto formatted_symbol = (frame.symbol == "")
                                            ? ""
                                            : std::format("\n    in {}",
                                                          YELLOW_TEXT_STYLE | symbol);

            if (frame.line.has_value() and frame.column.has_value()) {
                std::println(get_stderr(),
                             "{}# {}{}\n    at {}:{}:{}",
                             (i++ - ignore_count),
                             BLUE_TEXT_STYLE | object_address,
                             formatted_symbol,
                             GREEN_TEXT_STYLE | frame.filename,
                             BLUE_TEXT_STYLE | frame.line.value(),
                             BLUE_TEXT_STYLE | frame.column.value());
            } else
                std::println(get_stderr(),
                             "{}# {}{}\n    at {}",
                             (i++ - ignore_count),
                             BLUE_TEXT_STYLE | object_address,
                             formatted_symbol,
                             GREEN_TEXT_STYLE | frame.filename);
        }
#endif
        std::println("============================================================================="
                     "===============");
    }
}} // namespace stormkit::core
