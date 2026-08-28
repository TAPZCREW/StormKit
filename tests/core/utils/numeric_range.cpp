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
        "core.utils",
        { { "numeric.range.simple",
            [] {
                auto rng     = range(10);
                auto success = true;
                auto j       = 0;
                for (auto i : rng) {
                    if (i != j++) {
                        success = false;
                        break;
                    }
                }
                EXPECTS(success);
            } },
          { "numeric.range.custom_start",
            [] {
                auto rng     = range(2, 10);
                auto success = true;
                auto j       = 2;
                for (auto i : rng) {
                    if (i != j++) {
                        success = false;
                        break;
                    }
                }
                EXPECTS(success);
            } },
          { "numeric.range.custom_negative_start",
            [] {
                auto rng     = range(-5, 10);
                auto success = true;
                auto j       = -5;
                for (auto i : rng) {
                    if (i != j++) {
                        success = false;
                        break;
                    }
                }
                EXPECTS(success);
            } },
          { "numeric.range.custom_step",
            [] {
                auto rng     = range(-6, 10, 2);
                auto success = true;
                auto j       = -6;
                for (auto i : rng) {
                    if (i != j) {
                        success = false;
                        break;
                    }
                    j += 2;
                }
                EXPECTS(success);
            } },
          { "numeric.range.custom_numeric_range",
            [] {
                auto rng     = range(numeric_range { -4, 10, 2 });
                auto success = true;
                auto j       = -4;
                for (auto i : rng) {
                    if (i != j) {
                        success = false;
                        break;
                    }
                    j += 2;
                }
                EXPECTS(success);
            } },
          { "numeric.multirange.simple",
            [] {
                auto success = true;
                auto i       = 0;
                auto j       = 0;

                for (auto [v1, v2] : multi_range(10, 5)) {
                    if (v1 != i or v2 != j) {
                        success = false;
                        break;
                    }

                    j += 1;
                    if (j >= 5) {
                        j = 0;
                        i += 1;
                    }
                }

                EXPECTS(success);
            } },
          { "numeric.multirange.different_types",
            [] {
                auto success = true;
                auto i       = 0;
                auto j       = 0u;

                for (auto [v1, v2] : multi_range(10, 5u)) {
                    if (v1 != i or v2 != j or not meta::same_as<decltype(v2), decltype(j)>) {
                        success = false;
                        break;
                    }

                    j += 1u;
                    if (j >= 5) {
                        j = 0u;
                        i += 1;
                    }
                }

                EXPECTS(success);
            } },
          { "numeric.multirange.custom_start",
            [] {
                auto success = true;
                auto i       = 5;
                auto j       = 2;

                auto rng = multi_range(numeric_range { 5, 10 }, numeric_range { 2, 5 });
                for (auto [v1, v2] : rng) {
                    if (v1 != i or v2 != j) {
                        success = false;
                        break;
                    }

                    j += 1;
                    if (j >= 5) {
                        j = 2;
                        i += 1;
                    }
                }

                EXPECTS(success);
            } },
          { "numeric.multirange.custom_step",
            [] {
                auto success = true;
                auto i       = 0;
                auto j       = 0;

                auto rng = multi_range(numeric_range { 0, 10, 5 }, numeric_range { 0, 6, 2 });
                for (auto [v1, v2] : rng) {
                    if (v1 != i or v2 != j) {
                        success = false;
                        break;
                    }

                    j += 2;
                    if (j >= 5) {
                        j = 0;
                        i += 5;
                    }
                }

                EXPECTS(success);
            } } }
    };
} // namespace
