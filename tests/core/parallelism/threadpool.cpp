// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.core;
import stormkit.test;

#include <stormkit/test/test_macro.hpp>

using namespace stormkit::core;

using namespace std::literals;

namespace stdr = std::ranges;
namespace stdv = std::views;

namespace {
    auto _ = test::test_suite {
        "core.parallelism",
        {
          { "thread_pool.post_task_future_void",
            [] static noexcept {
                auto thread_pool_ = thread_pool {};

                auto future = thread_pool_.post_task<void>([] static noexcept { std::println("Hello from a thread!"); });

                EXPECTS(future.valid());
                future.wait();
            } },
          { "thread_pool.post_task_future_int",
            [] static noexcept {
                auto thread_pool_ = thread_pool {};

                auto future = thread_pool_.post_task<int>([] static noexcept {
                    std::println("Hello from a thread!");
                    return 8;
                });

                EXPECTS(future.valid());
                future.wait();
                EXPECTS(future.get() == 8);
            } },
          { "thread_pool.post_task_no_future",
            [] static noexcept {
                auto thread_pool_ = thread_pool {};

                auto val = std::atomic_int { 5 };

                thread_pool_.post_task<void>(
                  [&val] noexcept {
                      std::println("Hello from a thread!");
                      val = 8;
                  },
                  thread_pool::NO_FUTURE);

                std::this_thread::sleep_for(1s);

                EXPECTS(val == 8);
            } },
          { "thread_pool.post_task_from_other_task",
            [] static noexcept {
                auto thread_pool_ = thread_pool {};

                auto val = std::atomic_int { 5 };

                thread_pool_.post_task<void>(
                  [&val, &thread_pool_] noexcept {
                      thread_pool_.post_task<void>(
                        [&val] noexcept {
                            std::println("Hello from a thread!");
                            val = 8;
                        },
                        thread_pool::NO_FUTURE);
                  },
                  thread_pool::NO_FUTURE);

                std::this_thread::sleep_for(1s);

                EXPECTS(val == 8);
            } },
          { "thread_pool.post_task_from_other_thread",
            [] static noexcept {
                auto thread_pool_ = thread_pool {};

                auto val = std::atomic_int { 5 };

                auto thread = std::jthread { [&val, &thread_pool_] noexcept {
                    thread_pool_.post_task<void>(
                      [&val] noexcept {
                          std::println("Hello from a thread!");
                          val = 8;
                      },
                      thread_pool::NO_FUTURE);
                } };

                std::this_thread::sleep_for(1s);

                if (thread.joinable()) thread.join();

                EXPECTS(val == 8);
            } },
          { "thread_pool.parallel_for",
            [] static noexcept {
                auto thread_pool_ = thread_pool {};

                auto values = dynarray<i32> { std::from_range, range(0, 1000000) };
                parallel_for(thread_pool_, values, [](auto& value) { value += value; });

                auto k = 0;
                for (auto v : values) EXPECTS(v == (k++ * 2));
            } },
          }
    };
} // namespace
