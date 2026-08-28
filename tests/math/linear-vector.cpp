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
    auto _ = test::test_suite {
        "core.math.linear.vector",
        {
          { "linear.vector.as_view",
            [] static {
                auto       a     = math::ivec2 { 2, 3 };
                const auto span  = math::as_view(a);
                auto       span2 = math::as_view_mut(a);

                EXPECTS(span[0] == 2);
                EXPECTS(span[1] == 3);

                span2[0] = 1;

                EXPECTS(span[0] == 1);
            } },
          { "linear.vector.operator[]",
            [] static {
                auto a = math::ivec2 { 2, 3 };
                EXPECTS(a[0] == 2);
                const auto c = math::ivec3 { 1, 3, 3 };
                EXPECTS(c[1] == 3);
                const auto b = math::ivec4 { 5, 3, 9, 2 };
                EXPECTS(b[2] == 9);

                a[0] = 5;
            } },
          {
            "linear.vector.add",
            [] static {
                const auto a = math::ivec2 { 2, 3 };
                const auto b = math::ivec2 { 3, 2 };

                const auto result = add(a, b);
                EXPECTS(result.x == 5);
                EXPECTS(result.y == 5);
            },
          }, {
            "linear.vector.sub",
            [] static {
                const auto a = math::ivec2 { 2, 3 };
                const auto b = math::ivec2 { 3, 2 };

                const auto result = sub(a, b);
                EXPECTS(result.x == -1);
                EXPECTS(result.y == 1);
            },
          }, {
            "linear.vector.mul",
            [] static {
                const auto a = math::ivec2 { 10, 6 };

                const auto result = mul(a, 2);
                EXPECTS(result.x == 20);
                EXPECTS(result.y == 12);
            },
          }, {
            "linear.vector.div",
            [] static {
                const auto a = math::ivec2 { 10, 6 };

                const auto result = div(a, 2);
                EXPECTS(result.x == 5);
                EXPECTS(result.y == 3);
            },
          }, {
            "linear.vector.dot",
            [] static {
                const auto a = math::ivec2 { 2, 3 };
                const auto b = math::ivec2 { 3, 2 };

                EXPECTS(math::dot(a, b) == 12);
            },
          }, {
            "linear.vector.cross",
            [] static {
                const auto a = math::ivec3 { 2, 3, 2 };
                const auto b = math::ivec3 { 3, 2, 3 };

                const auto result = math::cross(a, b);
                EXPECTS(result.x == 5 and result.y == 0 and result.z == -5);
            },
          }, {
            "linear.vector.normalize",
            [] static {
                const auto a = math::fvec2 {
                    1,
                    2,
                };

                const auto result = math::normalize(a);
                EXPECTS(is(result.x, 1.f / std::sqrt(1.f + 4.f)));
                EXPECTS(result.y == 2.f / std::sqrt(1.f + 4.f));
            },
          }, },
    };
} // namespace
