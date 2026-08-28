// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.core;
import stormkit.math;
import stormkit.test;

#include <stormkit/test/test_macro.hpp>

using namespace stormkit::core;
using namespace std::literals;

namespace {
    auto _ = test::test_suite {
        "core.math.linear.matrix",
        {
          { "linear.matrix.as_view",
            [] static {
                auto a = math::imat4 { 0, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 4, 0, 0, 0 };
                EXPECTS(a[0] == 0);
                EXPECTS(a[1] == 1);
                EXPECTS((a[1, 2] == 2));

                const auto span  = math::as_view(a);
                auto       span2 = math::as_view_mut(a);

                EXPECTS(span[0] == 0);
                EXPECTS(span[1] == 1);

                span2[0] = 1;

                EXPECTS(span[0] == 1);
            } },
          { "linear.matrix.operator[]",
            [] static {
                auto a = math::imat4 { 0, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 4, 0, 0, 0 };
                EXPECTS(a[0] == 0);
                EXPECTS(a[1] == 1);
                EXPECTS(a[0] == 0);

                EXPECTS((a[1, 2] == 2));

                a[0] = 5;
            } },
          {
            "linear.matrix.determinant",
            [] static {
                const auto det_1 = determinant(math::imat2 { 2, 1, 4, 5 });
                EXPECTS(det_1 == 6);

                const auto det_2 = determinant(math::imat3 { 2, 1, 1, 1, 0, 1, 0, 3, 1 });
                EXPECTS(det_2 == -4);
            },
          }, {
            "linear.matrix.transpose",
            [] static {
                const auto a = math::imat4 { 0, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 4, 0, 0, 0 };

                const auto result = transpose(a);
                EXPECTS((result[0, 1] == 0));
                EXPECTS((result[1, 0] == 1));
                EXPECTS((result[1, 2] == 0));
                EXPECTS((result[2, 1] == 2));
                EXPECTS((result[2, 3] == 0));
                EXPECTS((result[3, 2] == 3));
                EXPECTS((result[3, 0] == 0));
                EXPECTS((result[0, 3] == 4));
            },
          }, {
            "linear.matrix.is_inversible",
            [] static {
                EXPECTS(math::is_inversible(math::imat2 { -3, 1, 5, 0 }));
                EXPECTS(not math::is_inversible(math::mat<int, 2, 3> { 2, 3, 4, 2, 1, 8 }));
            },
          }, {
            "linear.matrix.inverse",
            [] static {
                const auto a = math::fmat3 { 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 8.f };

                const auto result = math::inverse(a);
                EXPECTS(is(result[0, 0], -8.f / 3.f));
                EXPECTS(is(result[0, 1], 8.f / 3.f));
                EXPECTS(is(result[0, 2], -1.f));
                EXPECTS(is(result[1, 0], 10.f / 3.f));
                EXPECTS(is(result[1, 1], -13.f / 3.f));
                EXPECTS(is(result[1, 2], 2.f));
                EXPECTS(is(result[2, 0], -1.f));
                EXPECTS(is(result[2, 1], 2.f));
                EXPECTS(is(result[2, 2], -1.f));
            },
          }, {
            "linear.matrix.is_orthogonal",
            [] static {
                EXPECTS(math::is_orthogonal(math::imat3 { 0, 1, 0, 0, 0, 1, 1, 0, 0 }));
                EXPECTS(not math::is_orthogonal(math::mat<int, 2, 3> {}));
            },
          }, {
            "linear.matrix.mul.scalar",
            [] static {
                const auto a = math::imat4 { 22, 0, 0, 0, 0, 4, 0, 0, 0, 0, 8, 0, 0, 0, 0, 10 };
                const auto b = 2;

                const auto result = math::mul(a, b);
                EXPECTS((result[0, 0] == 44));
                EXPECTS((result[1, 1] == 8));
                EXPECTS((result[2, 2] == 16));
                EXPECTS((result[3, 3] == 20));
            },
          }, {
            "linear.matrix.div.scalar",
            [] static {
                const auto a = math::imat4 { 22, 0, 0, 0, 0, 4, 0, 0, 0, 0, 8, 0, 0, 0, 0, 10 };
                const auto b = 2;

                const auto result = math::div(a, b);
                EXPECTS((result[0, 0] == 11));
                EXPECTS((result[1, 1] == 2));
                EXPECTS((result[2, 2] == 4));
                EXPECTS((result[3, 3] == 5));
            },
          }, {
            "linear.matrix.mul.matrix",
            [] static {
                const auto a = math::imat4 { 22, 0, 0, 0, 0, 4, 0, 0, 0, 0, 8, 0, 0, 0, 0, 10 };
                const auto b = math::imat4 { 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 10, 0, 0, 0, 0, 26 };

                const auto result = math::mul(a, b);
                EXPECTS((result[0, 0] == 44));
                EXPECTS((result[1, 1] == 4));
                EXPECTS((result[2, 2] == 80));
                EXPECTS((result[3, 3] == 260));
            },
          }, {
            "linear.matrix.div.matrix",
            [] static {
                const auto a = math::fmat2 { 0, 1, 2, 3 };
                const auto b = math::fmat2 { 1, 2, 3, 4 };

                const auto result = math::div(a, b);
                EXPECTS(is(result[0], 3.f / 2.f));
                EXPECTS(is(result[1], -1.f / 2.f));
                EXPECTS(is(result[2], 1.f / 2.f));
                EXPECTS(is(result[3], 1.f / 2.f));
            },
          }, {
            "linear.matrix.translate",
            [] static {
                constexpr auto a = math::fmat4::identity();
                const auto     b = math::fvec3 { 3, 2, 3 };

                const auto result = math::translate(a, b);
                EXPECTS((result[0, 0] == 1));
                EXPECTS((result[1, 1] == 1));
                EXPECTS((result[2, 2] == 1));
                EXPECTS((result[0, 3] == 3));
                EXPECTS((result[1, 3] == 2));
                EXPECTS((result[2, 3] == 3));
                EXPECTS((result[3, 3] == 1));
            },
          }, {
            "linear.matrix.scale",
            [] static {
                const auto a = math::imat4::identity();
                const auto b = math::ivec3 { 3, 2, 3 };

                const auto result = math::scale(a, b);
                EXPECTS((result[0, 0] == 3));
                EXPECTS((result[1, 1] == 2));
                EXPECTS((result[2, 2] == 3));
                EXPECTS((result[3, 3] == 1));
            },
          }, { "linear.matrix.as<string>",
            [] static {
                auto a = math::imat4x3 {
                    { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 }
                };

                const auto as_string = math::as<string>(a);
                EXPECTS((as_string ==
                         // clang-format off
R"([mat   1,   2,   3
       4,   5,   6
       7,   8,   9
      10,  11,  12])"));
                // clang-format on
            } },
          },
    };
} // namespace
