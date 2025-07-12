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
          }, },
    };
} // namespace
