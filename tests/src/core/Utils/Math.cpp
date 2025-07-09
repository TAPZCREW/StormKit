// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.core;

import Test;

using namespace stormkit::core;

#define expects(x) test::expects(x, #x)

namespace {
    auto _
        = test::TestSuite { "Core.utils",
                            { { "Math.scale", [] static noexcept {
                                   expects(math::scale(5, 0, 10, 0, 20) == 10);
                                   expects(math::scale<u32>(5, 0, 10, 0u, 20u) == 10u);
                                   expects(is(math::scale(5.f, 0.f, 10.f, 0.f, 20.f), 10.f));
                                   expects(is(math::scale<f64>(5.f, 0.f, 10.f, 0., 20.), 10.));
                                   expects(math::scale<u32>(5.f, 0.f, 10.f, 0u, 20u) == 10u);
                               } } } };
} // namespace
