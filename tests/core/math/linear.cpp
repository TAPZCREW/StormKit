// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.core;
import stormkit.test;

#include <stormkit/test/test_macro.hpp>

using namespace stormkit::core;
using namespace std::literals;

namespace {
    auto _ = test::TestSuite {
        "core.math.linear",
        {
          {
            "linear.vector.dot",
            [] static {
                const auto a = math::vec2i { 2, 3 };
                const auto b = math::vec2i { 3, 2 };

                EXPECTS(math::dot(a, b) == 12);
            },
          }, {
            "linear.vector.cross",
            [] static {
                const auto a = math::vec3i { 2, 3, 2 };
                const auto b = math::vec3i { 3, 2, 3 };

                const auto cross_result = math::cross(a, b);
                EXPECTS(cross_result.x == 5 and cross_result.y == 0 and cross_result.z == -5);
            },
          }, },
    };
} // namespace
