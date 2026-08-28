// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.core;
import stormkit.test;

#include <stormkit/test/test_macro.hpp>

using namespace stormkit::core;

namespace {
    auto _ = test::test_suite {
        "core.parallelism",
        {
          { "locked.write",
            [] static noexcept {
                static constexpr auto ITERATIONS = 1'000'000;
                auto                  locked_int = locked { 0 };
                const auto            func       = [&locked_int] noexcept {
                    for (auto foo = 0; foo != ITERATIONS; ++foo) {
                        auto integer = locked_int.write();
                        *integer += 1;
                    }
                };

                auto future_1 = std::async(std::launch::async, func);
                auto future_2 = std::async(std::launch::async, func);

                future_1.wait();
                future_2.wait();

                EXPECTS(locked_int.unsafe() == (ITERATIONS * 2));
            } },
          { "locked.write_closure",
            [] static noexcept {
                static constexpr auto ITERATIONS = 1'000'000;
                auto                  locked_int = locked { 0 };
                const auto            func       = [&locked_int] noexcept {
                    for (auto foo = 0; foo != ITERATIONS; ++foo) {
                        locked_int.write([](auto& value) static noexcept { value += 1; });
                    }
                };

                auto future_1 = std::async(std::launch::async, func);
                auto future_2 = std::async(std::launch::async, func);

                future_1.wait();
                future_2.wait();

                EXPECTS(locked_int.unsafe() == (ITERATIONS * 2));
            } },
          { "locked.move",
            [] static noexcept {
                auto locked_int = locked { 0 };

                auto locked_int2 = std::move(locked_int);
            } },
          }
    };
} // namespace
