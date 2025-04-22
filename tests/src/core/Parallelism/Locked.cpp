// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.core;

import Test;

using namespace stormkit::core;

#define expects(x) test::expects(x, #x)

namespace {
    auto _ = test::TestSuite {
        "Core.parallelism",
        {
          { "Locked.write",
              [] static noexcept {
                  static constexpr auto ITERATIONS = 1'000'000;
                  auto                  locked_int = Locked { 0 };
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

                  expects(locked_int.unsafe() == (ITERATIONS * 2));
              } },
          }
    };
} // namespace
