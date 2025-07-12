// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.core;
import stormkit.test;

#include <stormkit/test/test_macro.hpp>

using namespace stormkit::core;

namespace {
    auto _ = test::TestSuite {
        "Core.utils",
        { { "Allocation.safe",
            [] static noexcept {
                auto allocation = allocate<int>(5);
                EXPECTS(allocation.has_value());
                EXPECTS(*allocation.value() == 5);
            } },
          { "Allocation.unsafe",
            [] static noexcept {
                auto allocation = allocate_unsafe<int>(5);
                EXPECTS(*allocation == 5);
            } } }
    };
} // namespace
