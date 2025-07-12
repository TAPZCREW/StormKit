// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.core;
import stormkit.test;

#include <stormkit/test/test_macro.hpp>

using namespace stormkit::core;

// static_assert(std::is_standard_layout_v<Integer<int>>);
// static_assert(std::is_trivially_copyable_v<Integer<int>>);

// static_assert(std::is_constructible_v<Integer<signed short>, signed short>);
// static_assert(std::is_assignable_v<Integer<signed short>, signed short>);
// static_assert(std::is_constructible_v<Integer<unsigned short>, unsigned short>);
// static_assert(std::is_assignable_v<Integer<unsigned short>, unsigned short>);

// static_assert(std::is_constructible_v<Integer<signed int>, signed int>);
// static_assert(std::is_assignable_v<Integer<signed int>, signed int>);
// static_assert(std::is_constructible_v<Integer<unsigned int>, unsigned int>);
// static_assert(std::is_assignable_v<Integer<unsigned int>, unsigned int>);

// static_assert(std::is_constructible_v<Integer<signed long>, signed long>);
// static_assert(std::is_assignable_v<Integer<signed long>, signed long>);
// static_assert(std::is_constructible_v<Integer<unsigned long>, unsigned long>);
// static_assert(std::is_assignable_v<Integer<unsigned long>, unsigned long>);

// static_assert(std::is_constructible_v<Integer<signed long long>, signed long long>);
// static_assert(std::is_assignable_v<Integer<signed long long>, signed long long>);
// static_assert(std::is_constructible_v<Integer<unsigned long long>, unsigned long long>);
// static_assert(std::is_assignable_v<Integer<unsigned long long>, unsigned long long>);

namespace {

    auto _ = test::TestSuite {
        "Core.typesafe",
        { { "Integer.constructor",
            [] static noexcept {
                // auto integer_1 = Integer { 0 };
                // EXPECTS(static_cast<int>(integer_1) == 0);
                //
                // auto integer_2 = Integer { 9 };
                // EXPECTS(!static_cast<int>(integer_2) == 9);
                //
                // auto integer_3 = Integer { -5 };
                // EXPECTS(static_cast<int>(integer_3) == -5);
            } },
          // { "Integer.assignement",
          //   [] static noexcept {
          //       auto bool_1 = Integer { true };
          //       bool_1 = false;
          //       EXPECTS(!static_cast<bool>(bool_1));
          //       bool_1 = true;
          //       EXPECTS(static_cast<bool>(bool_1));
          //   } },
          // { "Integer.negate",
          //   [] static noexcept {
          //       auto bool_1 = Integer { true };
          //       EXPECTS(!bool_1 == false);
          //
          //       auto bool_2 = Integer { false };
          //       EXPECTS(!bool_2 == true);
          //   } },
          // { "Integer.comparison",
          //   [] static noexcept {
          //       auto bool_1 = Integer { true };
          //       EXPECTS(bool_1 == true);
          //       EXPECTS(true == bool_1);
          //       EXPECTS(bool_1 != false);
          //       EXPECTS(false != bool_1);
          //       EXPECTS(bool_1 == Integer { true });
          //       EXPECTS(bool_1 != Integer { false });
          //
          //       auto bool_2 = Integer { false };
          //       EXPECTS(bool_2 == false);
          //       EXPECTS(false == bool_2);
          //       EXPECTS(bool_2 != true);
          //       EXPECTS(true != bool_2);
          //       EXPECTS(bool_2 == Integer { false });
          //       EXPECTS(bool_2 != Integer { true });
          //   } },
          { "Integer.format",
            [] static noexcept {
                // auto integer = Integer { 0 };
                // auto str = std::format("{}", integer);
                // EXPECTS(str == "0");
                // integer = 9;
                // str = std::format("{}", integer);
                // EXPECTS(str == "9");
            } } }
    };
} // namespace
