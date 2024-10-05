// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <cstdio>

module stormkit.Core;

import std;

namespace stormkit::core {
    /////////////////////////////////////
    /////////////////////////////////////
    auto printStacktrace([[maybe_unused]] int ignore_count) noexcept -> void {
        const auto thread_name = getCurrentThreadName();
        if (not std::empty(thread_name))
            std::println(stderr,
                         "================= CALLSTACK (thread name: {}, id: {}) =================",
                         thread_name,
                         std::this_thread::get_id());
        else
            std::println(stderr,
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
            std::println("{}# {}", (i++ - ignore_count), frame);
        }
#endif
    }
}
