// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.Core;

import Test;

using namespace stormkit::core;

#define expects(x) test::expects(x, #x)

namespace {
    auto _ = test::TestSuite {
        "Core.Utils",
        { { "Allocation.safe",
            [] static noexcept {
                auto allocation = makeUnique<int>(5);
                expects(allocation.has_value());
                expects(*allocation.value() == 5);
            } },
          { "Allocation.unsafe",
            [] static noexcept {
                auto allocation = makeUniqueUnsafe<int>(5);
                expects(*allocation == 5);
            } } }
    };
} // namespace
