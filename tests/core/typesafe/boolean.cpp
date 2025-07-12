// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.core;
import stormkit.test;

#include <stormkit/test/test_macro.hpp>

using namespace stormkit::core;
using namespace std::literals;

static_assert(std::is_standard_layout_v<Boolean>);
static_assert(std::is_trivially_copyable_v<Boolean>);
static_assert(not std::is_constructible_v<Boolean, int>);
static_assert(std::is_constructible_v<Boolean, bool>);
static_assert(std::is_assignable_v<Boolean, bool>);

namespace {
    auto _ = test::TestSuite {
        "Core.typesafe",
        { { "Boolean.constructor",
            [] static noexcept {
                auto bool_1 = Boolean { true };
                EXPECTS(static_cast<bool>(bool_1));

                bind_back([](auto) {}, 1)();

                auto bool_2 = Boolean { false };
                EXPECTS(!static_cast<bool>(bool_2));
            } },
          { "Boolean.assignement",
            [] static noexcept {
                auto bool_1 = Boolean { true };
                bool_1      = false;
                EXPECTS(!static_cast<bool>(bool_1));
                bool_1 = true;
                EXPECTS(static_cast<bool>(bool_1));
            } },
          { "Boolean.negate",
            [] static noexcept {
                auto bool_1 = Boolean { true };
                EXPECTS(!bool_1 == false);

                auto bool_2 = Boolean { false };
                EXPECTS(!bool_2 == true);
            } },
          { "Boolean.comparison",
            [] static noexcept {
                auto bool_1 = Boolean { true };
                EXPECTS(bool_1 == true);
                EXPECTS(true == bool_1);
                EXPECTS(bool_1 != false);
                EXPECTS(false != bool_1);
                EXPECTS(bool_1 == Boolean { true });
                EXPECTS(bool_1 != Boolean { false });

                auto bool_2 = Boolean { false };
                EXPECTS(bool_2 == false);
                EXPECTS(false == bool_2);
                EXPECTS(bool_2 != true);
                EXPECTS(true != bool_2);
                EXPECTS(bool_2 == Boolean { false });
                EXPECTS(bool_2 != Boolean { true });
            } },
          { "Boolean.format",
            [] static noexcept {
                auto bool_1 = Boolean { true };
                auto str    = std::format("{}", bool_1);
                EXPECTS(str == "true"s);
                bool_1 = false;
                str    = std::format("{}", bool_1);
                EXPECTS(str == "false"s);
            } } }
    };
} // namespace
