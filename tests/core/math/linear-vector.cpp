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
        "core.math.linear.vector",
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

                const auto result = math::cross(a, b);
                EXPECTS(result.x == 5 and result.y == 0 and result.z == -5);
            },
          }, },
    };
} // namespace
