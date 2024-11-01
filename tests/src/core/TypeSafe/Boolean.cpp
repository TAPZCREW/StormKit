// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.Core;

import Test;

using namespace stormkit::core;
using namespace std::literals;

#define expects(x) test::expects(x, #x)
static_assert(std::is_standard_layout_v<Boolean>);
static_assert(std::is_trivially_copyable_v<Boolean>);
static_assert(not std::is_constructible_v<Boolean, int>);
static_assert(std::is_constructible_v<Boolean, bool>);
static_assert(std::is_assignable_v<Boolean, bool>);

namespace {
    auto _ = test::TestSuite {
        "Core.TypeSafe",
        { { "Boolean.constructor",
            [] static noexcept {
                auto bool_1 = Boolean { true };
                expects(static_cast<bool>(bool_1));

                bindBack([](auto foo) {}, 1)();

                auto bool_2 = Boolean { false };
                expects(!static_cast<bool>(bool_2));
            } },
          { "Boolean.assignement",
            [] static noexcept {
                auto bool_1 = Boolean { true };
                bool_1      = false;
                expects(!static_cast<bool>(bool_1));
                bool_1 = true;
                expects(static_cast<bool>(bool_1));
            } },
          { "Boolean.negate",
            [] static noexcept {
                auto bool_1 = Boolean { true };
                expects(!bool_1 == false);

                auto bool_2 = Boolean { false };
                expects(!bool_2 == true);
            } },
          { "Boolean.comparison",
            [] static noexcept {
                auto bool_1 = Boolean { true };
                expects(bool_1 == true);
                expects(true == bool_1);
                expects(bool_1 != false);
                expects(false != bool_1);
                expects(bool_1 == Boolean { true });
                expects(bool_1 != Boolean { false });

                auto bool_2 = Boolean { false };
                expects(bool_2 == false);
                expects(false == bool_2);
                expects(bool_2 != true);
                expects(true != bool_2);
                expects(bool_2 == Boolean { false });
                expects(bool_2 != Boolean { true });
            } },
          { "Boolean.format",
            [] static noexcept {
                auto bool_1 = Boolean { true };
                auto str    = std::format("{}", bool_1);
                expects(str == "true"s);
                bool_1 = false;
                str    = std::format("{}", bool_1);
                expects(str == "false"s);
            } } }
    };
} // namespace
