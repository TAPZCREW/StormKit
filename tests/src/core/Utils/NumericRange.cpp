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
        "Core.utils",
        { { "Numerics.range.simple",
            [] {
                auto rng     = range(10);
                auto success = true;
                for (auto i = 0; i < 10; ++i) {
                    if (rng[i] != i) {
                        success = false;
                        break;
                    }
                }
                expects(success);
            } },
          { "Numerics.range.custom_start",
            [] {
                auto rng     = range(2, 10);
                auto success = true;
                for (auto i = 2; i < 10; ++i) {
                    if (rng[i - 2] != i) {
                        success = false;
                        break;
                    }
                }
                expects(success);
            } },
          { "Numerics.range.custom_negative_start",
            [] {
                auto rng     = range(-5, 10);
                auto success = true;
                for (auto i = -5; i < 10; ++i) {
                    if (rng[i + 5] != i) {
                        success = false;
                        break;
                    }
                }
                expects(success);
            } },
          { "Numerics.range.custom_step",
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
                expects(success);
            } },
          { "Numerics.range.custom_numeric_range",
            [] {
                auto rng     = range(NumericsRange { -4, 10, 2 });
                auto success = true;
                auto j       = -4;
                for (auto i : rng) {
                    if (i != j) {
                        success = false;
                        break;
                    }
                    j += 2;
                }
                expects(success);
            } },
          { "Numerics.multirange.simple",
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

                expects(success);
            } },
          { "Numerics.multirange.different_types",
            [] {
                auto success = true;
                auto i       = 0;
                auto j       = 0u;

                for (auto [v1, v2] : multi_range(10, 5u)) {
                    if (v1 != i or v2 != j or not std::same_as<decltype(v2), decltype(j)>) {
                        success = false;
                        break;
                    }

                    j += 1u;
                    if (j >= 5) {
                        j = 0u;
                        i += 1;
                    }
                }

                expects(success);
            } },
          { "Numerics.multirange.custom_start",
            [] {
                auto success = true;
                auto i       = 5;
                auto j       = 2;

                auto rng = multi_range(NumericsRange { 5, 10 }, NumericsRange { 2, 5 });
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

                expects(success);
            } },
          { "Numerics.multirange.custom_step",
            [] {
                auto success = true;
                auto i       = 0;
                auto j       = 0;

                auto rng = multi_range(NumericsRange { 0, 10, 5 }, NumericsRange { 0, 6, 2 });
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

                expects(success);
            } } }
    };
} // namespace
