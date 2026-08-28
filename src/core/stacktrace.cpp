// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <version>

#include <stormkit/core/platform_macro.hpp>

#if defined(__cpp_lib_stacktrace) and __cpp_lib_stacktrace >= 202011L
    #define STD_STACKTRACE_SUPPORTED
#endif

module stormkit.core.stacktrace;

import std;

import stormkit.core.console;
import stormkit.core.string;
import stormkit.core.errors;
import stormkit.core.types;
import stormkit.core.typesafe.safecasts;
import stormkit.core.parallelism.threadutils;

namespace stdr = std::ranges;

using namespace std::literals;

namespace stormkit { inline namespace core {
    auto prettify(string_view str) -> string {
        auto out = string { str };
        out      = replace(out, "::__1::", "::");
        out      = replace(out, "::$_0::", "::");
        out      = replace(out, "__invoke", "invoke");
        out      = replace(out, "__function", "function");
        out      = replace(out, "[abi:se210000]", "");
        out      = replace(out, "[abi:ne210000]", "");
        out      = replace(out, "basic_string_view<char, std::char_traits<char>>", "string_view");
        out      = replace(out, "basic_string_view<char, std::char_traits<char> >", "string_view");
        out      = replace(out,
                           "basic_string<char, std::char_traits<char>, "
                           "std::allocator<char>>",
                           "string");
        out      = replace(out,
                           "basic_string<char, std::char_traits<char>, "
                           "std::allocator<char> >",
                           "string");
        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto print_stacktrace(u32 ignore_count) noexcept -> void {
        const auto thread_name = get_current_thread_name();
        const auto stderr      = get_stderr();

        if (not std::empty(thread_name))
            std::println(stderr,
                         "================= CALLSTACK (thread name: {}, id: {}) =================",
                         thread_name,
                         std::this_thread::get_id());
        else
            std::println(stderr, "================= CALLSTACK (thread id: {}) =================", std::this_thread::get_id());
#ifdef STD_STACKTRACE_SUPPORTED
        const auto st = std::stacktrace::current();
        auto       i  = 0;
        for (const auto& frame : st) {
            if (i < ignore_count) {
                i += 1;
                continue;
            }
    #ifdef STORMKIT_COMPILER_MSSTL
            const auto frame_str        = std::to_string(frame);
            auto       splitted         = split(frame_str, "+");
            const auto address          = as<u64>(splitted[1].substr(2), 16)
                                            .transform_error([stderr, &splitted](auto&& err) noexcept {
                                       std::println(stderr, "Failed to parse {}, reason: {}", splitted[0], err);
                                       return 0;
                                            })
                                            .value();
            splitted                    = split(splitted[0], "!");
            const auto formatted_symbol = prettify((stdr::size(splitted) >= 2)
                                                     ? "\n    in " + (YELLOW_TEXT_STYLE | splitted[1]).render()
                                                     : ""s);
    #elifdef STORMKIT_COMPILER_LIBCPP
            // clang-format off
            // e.g 0x5adc4b1dc9fc: __invoke<(lambda at src/gpu/core/device.cpp:401:22)>: /opt/llvm-git/include/c++/v1/__type_traits/invoke.h:179
            // e.g 0x730060b19a75: terminate: /home/arthapz/Development/AUR/llvm-git/src/llvm-project/libcxxabi/src/cxa_handlers.cpp:92
            // clang-format on
            const auto frame_str = std::to_string(frame);
            const auto splitted  = split(frame_str, ": ");
            const auto
              address = *try_as<u64>(splitted[0].substr(2), 16).transform_error([stderr, &splitted](auto&& err) noexcept {
                  std::println(stderr, "Failed to parse {}, reason: {}", splitted[0], err);
                  return 0;
              });

            const auto formatted_symbol = prettify((stdr::size(splitted) > 2)
                                                     ? "\n    in " + (YELLOW_TEXT_STYLE | splitted[1]).render()
                                                     : ""s);
    #else
            // TODO LIBSTDC++
            const auto address          = 0;
            const auto formatted_symbol = ""s;
    #endif
            const auto object_address = (address == 0 ? "inlined" : std::format("{:#010x}", address));

            if (not stdr::empty(frame.source_file()) and frame.source_line() != 0) {
                std::println(stderr,
                             "{}# {}{}\n    at {}:{}",
                             (i++ - ignore_count),
                             BLUE_TEXT_STYLE | object_address,
                             formatted_symbol,
                             GREEN_TEXT_STYLE | frame.source_file(),
                             BLUE_TEXT_STYLE | frame.source_line());
            } else if (not stdr::empty(frame.source_file())) {
                std::println(stderr,
                             "{}# {}{}\n    at {}",
                             (i++ - ignore_count),
                             BLUE_TEXT_STYLE | object_address,
                             formatted_symbol,
                             GREEN_TEXT_STYLE | frame.source_file());
            } else {
                std::println(stderr, "{}# {}{}", (i++ - ignore_count), BLUE_TEXT_STYLE | object_address, formatted_symbol);
            }
        }
#else
        auto _ = ignore_count;
        std::println(stderr, "std::stacktrace not supported!");
#endif
        std::println(stderr,
                     "============================================================================="
                     "===============");
    }
}} // namespace stormkit::core
