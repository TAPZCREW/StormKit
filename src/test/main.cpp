// Copyright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import stormkit.core;
import stormkit.test;

import std;

#include <stormkit/main/main_macro.hpp>

auto main(std::span<const std::string_view> args) noexcept -> int {
    test::parse_args(args);

    return test::runTests();
}
