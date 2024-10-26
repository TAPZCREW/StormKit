// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.Core;

import Test;

using namespace stormkit::core;

#define expects(x) test::expects(x, #x)

static_assert(std::is_standard_layout_v<Integer<int>>);
static_assert(std::is_trivially_copyable_v<Integer<int>>);

static_assert(std::is_constructible_v<Integer<signed short>, signed short>);
static_assert(std::is_assignable_v<Integer<signed short>, signed short>);
static_assert(std::is_constructible_v<Integer<unsigned short>, unsigned short>);
static_assert(std::is_assignable_v<Integer<unsigned short>, unsigned short>);

static_assert(std::is_constructible_v<Integer<signed int>, signed int>);
static_assert(std::is_assignable_v<Integer<signed int>, signed int>);
static_assert(std::is_constructible_v<Integer<unsigned int>, unsigned int>);
static_assert(std::is_assignable_v<Integer<unsigned int>, unsigned int>);

static_assert(std::is_constructible_v<Integer<signed long>, signed long>);
static_assert(std::is_assignable_v<Integer<signed long>, signed long>);
static_assert(std::is_constructible_v<Integer<unsigned long>, unsigned long>);
static_assert(std::is_assignable_v<Integer<unsigned long>, unsigned long>);

static_assert(std::is_constructible_v<Integer<signed long long>, signed long long>);
static_assert(std::is_assignable_v<Integer<signed long long>, signed long long>);
static_assert(std::is_constructible_v<Integer<unsigned long long>, unsigned long long>);
static_assert(std::is_assignable_v<Integer<unsigned long long>, unsigned long long>);

namespace {

    auto _ = test::TestSuite { "Core.TypeSafe",
                               { { "Integer.constructor",
                                   [] static noexcept {
                                       // auto integer_1 = Integer { 0 };
                                       // expects(static_cast<int>(integer_1) == 0);
                                       //
                                       // auto integer_2 = Integer { 9 };
                                       // expects(!static_cast<int>(integer_2) == 9);
                                       //
                                       // auto integer_3 = Integer { -5 };
                                       // expects(static_cast<int>(integer_3) == -5);
                                   } },
                                 // { "Integer.assignement",
                                 //   [] static noexcept {
                                 //       auto bool_1 = Integer { true };
                                 //       bool_1 = false;
                                 //       expects(!static_cast<bool>(bool_1));
                                 //       bool_1 = true;
                                 //       expects(static_cast<bool>(bool_1));
                                 //   } },
                                 // { "Integer.negate",
                                 //   [] static noexcept {
                                 //       auto bool_1 = Integer { true };
                                 //       expects(!bool_1 == false);
                                 //
                                 //       auto bool_2 = Integer { false };
                                 //       expects(!bool_2 == true);
                                 //   } },
                                 // { "Integer.comparison",
                                 //   [] static noexcept {
                                 //       auto bool_1 = Integer { true };
                                 //       expects(bool_1 == true);
                                 //       expects(true == bool_1);
                                 //       expects(bool_1 != false);
                                 //       expects(false != bool_1);
                                 //       expects(bool_1 == Integer { true });
                                 //       expects(bool_1 != Integer { false });
                                 //
                                 //       auto bool_2 = Integer { false };
                                 //       expects(bool_2 == false);
                                 //       expects(false == bool_2);
                                 //       expects(bool_2 != true);
                                 //       expects(true != bool_2);
                                 //       expects(bool_2 == Integer { false });
                                 //       expects(bool_2 != Integer { true });
                                 //   } },
                                 { "Integer.format", [] static noexcept {
                                      // auto integer = Integer { 0 };
                                      // auto str = std::format("{}", integer);
                                      // expects(str == "0");
                                      // integer = 9;
                                      // str = std::format("{}", integer);
                                      // expects(str == "9");
                                  } } } };
} // namespace
