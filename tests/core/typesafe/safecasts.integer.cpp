// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.core;
import stormkit.test;

#include <stormkit/test/test_macro.hpp>

using namespace stormkit::core;
using namespace stormkit::literals;

struct Foo {
    int b;
};

template<stormkit::meta::integral T>
constexpr auto tag_invoke(as_fn<T>, const Foo& value, source_location_arg) noexcept -> T {
    return static_cast<T>(value.b);
}

namespace bar {
    struct Foo {
        int b;
    };

    template<stormkit::meta::integral T>
    constexpr auto tag_invoke(as_fn<T>, const Foo& value, source_location_arg) noexcept -> T {
        return static_cast<T>(value.b);
    }
} // namespace bar

namespace {
    [[maybe_unused]]
    const auto i8_1 = i8 { 1 };
    [[maybe_unused]]
    const auto i8_2 = i8 { 2 };

    [[maybe_unused]]
    const auto u8_1 = u8 { 1 };
    [[maybe_unused]]
    const auto u8_2 = u8 { 2 };

    [[maybe_unused]]
    const auto i16_1 = i16 { 1 };
    [[maybe_unused]]
    const auto i16_2 = i16 { 2 };

    [[maybe_unused]]
    const auto u16_1 = u16 { 1 };
    [[maybe_unused]]
    const auto u16_2 = u16 { 2 };

    [[maybe_unused]]
    const auto i32_1 = i32 { 1 };
    [[maybe_unused]]
    const auto i32_2 = i32 { 2 };

    [[maybe_unused]]
    const auto u32_1 = u32 { 1 };
    [[maybe_unused]]
    const auto u32_2 = u32 { 2 };

    [[maybe_unused]]
    const auto i64_1 = i64 { 1 };
    [[maybe_unused]]
    const auto i64_2 = i64 { 2 };

    [[maybe_unused]]
    const auto u64_1 = u64 { 1 };
    [[maybe_unused]]
    const auto u64_2 = u64 { 2 };

    [[maybe_unused]]
    const auto i128_1 = i128 { 1 };
    [[maybe_unused]]
    const auto i128_2 = i128 { 2 };

    [[maybe_unused]]
    const auto u128_1 = u128 { 1 };
    [[maybe_unused]]
    const auto u128_2 = u128 { 2 };

    auto _ = test::test_suite {
        "core.typesafe.safecasts",
        {
          { "arithmetic.integrals",
            [] static noexcept {

            } },
          { "arithmetic.is.integral",
            [] static noexcept {
                EXPECTS(is(7, 7));
                EXPECTS(is(19, 19.f));
            } },
          { "arithmetic.as.integral",
            [] static noexcept {
                static_assert(meta::is_sign_narrowing<i32, u32>);
                static_assert(meta::is_sign_narrowing<u32, i32>);
                static_assert(not meta::is_sign_narrowing<i8, u32>);
                static_assert(meta::is_sign_narrowing<u32, i8>);
                static_assert(meta::is_narrowing<i16, i32>);
                static_assert(not meta::is_narrowing<i32, i16>);
                static_assert(meta::is_narrowing<i32, u32>);
                static_assert(meta::is_narrowing<u32, i32>);

                EXPECTS(as<i8>(127) == i8 { 127 });
                EXPECTS(as<i8>(-80) != i8 { -81 });

                EXPECTS(as<i32>(8u) == 8);
                EXPECTS(as<i32>(1820) != 7);

                EXPECTS(as<u32>(14) == 14u);

                EXPECTS(is(as<f32>(19), 19));
                EXPECTS(as<i8>(1_i16) == 1);
                EXPECTS(as<i8>(1_i32) == 1);
                EXPECTS(as<i8>(1_i64) == 1);
                EXPECTS(as<i8>(1_i128) == 1);
                EXPECTS(as<i8>(1_u8) == 1);
                EXPECTS(as<i8>(1_u16) == 1);
                EXPECTS(as<i8>(1_u32) == 1);
                EXPECTS(as<i8>(1_u64) == 1);
                EXPECTS(as<i8>(1_u128) == 1);

                EXPECTS(as<u8>(1_i8) == 1);
                EXPECTS(as<u8>(1_i16) == 1);
                EXPECTS(as<u8>(1_i32) == 1);
                EXPECTS(as<u8>(1_i64) == 1);
                EXPECTS(as<u8>(1_i128) == 1);
                EXPECTS(as<u8>(1_u16) == 1);
                EXPECTS(as<u8>(1_u32) == 1);
                EXPECTS(as<u8>(1_u64) == 1);
                EXPECTS(as<u8>(1_u128) == 1);

                EXPECTS(as<i16>(1_i8) == 1);
                EXPECTS(as<i16>(1_i32) == 1);
                EXPECTS(as<i16>(1_i64) == 1);
                EXPECTS(as<i16>(1_i128) == 1);
                EXPECTS(as<i16>(1_u8) == 1);
                EXPECTS(as<i16>(1_u16) == 1);
                EXPECTS(as<i16>(1_u32) == 1);
                EXPECTS(as<i16>(1_u64) == 1);
                EXPECTS(as<i16>(1_u128) == 1);

                EXPECTS(as<u16>(1_i8) == 1);
                EXPECTS(as<u16>(1_i16) == 1);
                EXPECTS(as<u16>(1_i32) == 1);
                EXPECTS(as<u16>(1_i64) == 1);
                EXPECTS(as<u16>(1_i128) == 1);
                EXPECTS(as<u16>(1_u8) == 1);
                EXPECTS(as<u16>(1_u32) == 1);
                EXPECTS(as<u16>(1_u64) == 1);
                EXPECTS(as<u16>(1_u128) == 1);

                EXPECTS(as<i32>(1_i8) == 1);
                EXPECTS(as<i32>(1_i16) == 1);
                EXPECTS(as<i32>(1_i64) == 1);
                EXPECTS(as<i32>(1_i128) == 1);
                EXPECTS(as<i32>(1_u8) == 1);
                EXPECTS(as<i32>(1_u16) == 1);
                EXPECTS(as<i32>(1_u32) == 1);
                EXPECTS(as<i32>(1_u64) == 1);
                EXPECTS(as<i32>(1_u128) == 1);

                EXPECTS(as<u32>(1_i8) == 1);
                EXPECTS(as<u32>(1_i16) == 1);
                EXPECTS(as<u32>(1_i32) == 1);
                EXPECTS(as<u32>(1_i64) == 1);
                EXPECTS(as<u32>(1_i128) == 1);
                EXPECTS(as<u32>(1_u8) == 1);
                EXPECTS(as<u32>(1_u16) == 1);
                EXPECTS(as<u32>(1_u64) == 1);
                EXPECTS(as<u32>(1_u128) == 1);

                EXPECTS(as<i64>(1_i8) == 1);
                EXPECTS(as<i64>(1_i16) == 1);
                EXPECTS(as<i64>(1_i32) == 1);
                EXPECTS(as<i64>(1_i128) == 1);
                EXPECTS(as<i64>(1_u8) == 1);
                EXPECTS(as<i64>(1_u16) == 1);
                EXPECTS(as<i64>(1_u32) == 1);
                EXPECTS(as<i64>(1_u64) == 1);
                EXPECTS(as<i64>(1_u128) == 1);

                EXPECTS(as<u64>(1_i8) == 1);
                EXPECTS(as<u64>(1_i16) == 1);
                EXPECTS(as<u64>(1_i32) == 1);
                EXPECTS(as<u64>(1_i64) == 1);
                EXPECTS(as<u64>(1_i128) == 1);
                EXPECTS(as<u64>(1_u8) == 1);
                EXPECTS(as<u64>(1_u16) == 1);
                EXPECTS(as<u64>(1_u32) == 1);
                EXPECTS(as<u64>(1_u128) == 1);

                EXPECTS(as<i128>(1_i8) == 1);
                EXPECTS(as<i128>(1_i16) == 1);
                EXPECTS(as<i128>(1_i32) == 1);
                EXPECTS(as<i128>(1_i64) == 1);
                EXPECTS(as<i128>(1_u8) == 1);
                EXPECTS(as<i128>(1_u16) == 1);
                EXPECTS(as<i128>(1_u32) == 1);
                EXPECTS(as<i128>(1_u64) == 1);
                EXPECTS(as<i128>(1_u128) == 1);

                EXPECTS(as<u128>(1_i8) == 1);
                EXPECTS(as<u128>(1_i16) == 1);
                EXPECTS(as<u128>(1_i32) == 1);
                EXPECTS(as<u128>(1_i64) == 1);
                EXPECTS(as<u128>(1_i128) == 1);
                EXPECTS(as<u128>(1_u8) == 1);
                EXPECTS(as<u128>(1_u16) == 1);
                EXPECTS(as<u128>(1_u32) == 1);
                EXPECTS(as<u128>(1_u64) == 1);
            } },
          { "arithmetic.unchecked_narrow.integral",
            [] static noexcept {
                EXPECTS(unchecked_narrow<i8>(1_i16) == 1);
                EXPECTS(unchecked_narrow<i8>(1_i32) == 1);
                EXPECTS(unchecked_narrow<i8>(1_i64) == 1);
                EXPECTS(unchecked_narrow<i8>(1_i128) == 1);
                EXPECTS(unchecked_narrow<i8>(1_u8) == 1);
                EXPECTS(unchecked_narrow<i8>(1_u16) == 1);
                EXPECTS(unchecked_narrow<i8>(1_u32) == 1);
                EXPECTS(unchecked_narrow<i8>(1_u64) == 1);
                EXPECTS(unchecked_narrow<i8>(1_u128) == 1);

                EXPECTS(unchecked_narrow<u8>(1_i8) == 1);
                EXPECTS(unchecked_narrow<u8>(1_i16) == 1);
                EXPECTS(unchecked_narrow<u8>(1_i32) == 1);
                EXPECTS(unchecked_narrow<u8>(1_i64) == 1);
                EXPECTS(unchecked_narrow<u8>(1_i128) == 1);
                EXPECTS(unchecked_narrow<u8>(1_u16) == 1);
                EXPECTS(unchecked_narrow<u8>(1_u32) == 1);
                EXPECTS(unchecked_narrow<u8>(1_u64) == 1);
                EXPECTS(unchecked_narrow<u8>(1_u128) == 1);

                EXPECTS(unchecked_narrow<i16>(1_i8) == 1);
                EXPECTS(unchecked_narrow<i16>(1_i32) == 1);
                EXPECTS(unchecked_narrow<i16>(1_i64) == 1);
                EXPECTS(unchecked_narrow<i16>(1_i128) == 1);
                EXPECTS(unchecked_narrow<i16>(1_u8) == 1);
                EXPECTS(unchecked_narrow<i16>(1_u16) == 1);
                EXPECTS(unchecked_narrow<i16>(1_u32) == 1);
                EXPECTS(unchecked_narrow<i16>(1_u64) == 1);
                EXPECTS(unchecked_narrow<i16>(1_u128) == 1);

                EXPECTS(unchecked_narrow<u16>(1_i8) == 1);
                EXPECTS(unchecked_narrow<u16>(1_i16) == 1);
                EXPECTS(unchecked_narrow<u16>(1_i32) == 1);
                EXPECTS(unchecked_narrow<u16>(1_i64) == 1);
                EXPECTS(unchecked_narrow<u16>(1_i128) == 1);
                EXPECTS(unchecked_narrow<u16>(1_u8) == 1);
                EXPECTS(unchecked_narrow<u16>(1_u32) == 1);
                EXPECTS(unchecked_narrow<u16>(1_u64) == 1);
                EXPECTS(unchecked_narrow<u16>(1_u128) == 1);

                EXPECTS(unchecked_narrow<i32>(1_i8) == 1);
                EXPECTS(unchecked_narrow<i32>(1_i16) == 1);
                EXPECTS(unchecked_narrow<i32>(1_i64) == 1);
                EXPECTS(unchecked_narrow<i32>(1_i128) == 1);
                EXPECTS(unchecked_narrow<i32>(1_u8) == 1);
                EXPECTS(unchecked_narrow<i32>(1_u16) == 1);
                EXPECTS(unchecked_narrow<i32>(1_u32) == 1);
                EXPECTS(unchecked_narrow<i32>(1_u64) == 1);
                EXPECTS(unchecked_narrow<i32>(1_u128) == 1);

                EXPECTS(unchecked_narrow<u32>(1_i8) == 1);
                EXPECTS(unchecked_narrow<u32>(1_i16) == 1);
                EXPECTS(unchecked_narrow<u32>(1_i32) == 1);
                EXPECTS(unchecked_narrow<u32>(1_i64) == 1);
                EXPECTS(unchecked_narrow<u32>(1_i128) == 1);
                EXPECTS(unchecked_narrow<u32>(1_u8) == 1);
                EXPECTS(unchecked_narrow<u32>(1_u16) == 1);
                EXPECTS(unchecked_narrow<u32>(1_u64) == 1);
                EXPECTS(unchecked_narrow<u32>(1_u128) == 1);

                EXPECTS(unchecked_narrow<i64>(1_i8) == 1);
                EXPECTS(unchecked_narrow<i64>(1_i16) == 1);
                EXPECTS(unchecked_narrow<i64>(1_i32) == 1);
                EXPECTS(unchecked_narrow<i64>(1_i128) == 1);
                EXPECTS(unchecked_narrow<i64>(1_u8) == 1);
                EXPECTS(unchecked_narrow<i64>(1_u16) == 1);
                EXPECTS(unchecked_narrow<i64>(1_u32) == 1);
                EXPECTS(unchecked_narrow<i64>(1_u64) == 1);
                EXPECTS(unchecked_narrow<i64>(1_u128) == 1);

                EXPECTS(unchecked_narrow<u64>(1_i8) == 1);
                EXPECTS(unchecked_narrow<u64>(1_i16) == 1);
                EXPECTS(unchecked_narrow<u64>(1_i32) == 1);
                EXPECTS(unchecked_narrow<u64>(1_i64) == 1);
                EXPECTS(unchecked_narrow<u64>(1_i128) == 1);
                EXPECTS(unchecked_narrow<u64>(1_u8) == 1);
                EXPECTS(unchecked_narrow<u64>(1_u16) == 1);
                EXPECTS(unchecked_narrow<u64>(1_u32) == 1);
                EXPECTS(unchecked_narrow<u64>(1_u128) == 1);

                EXPECTS(unchecked_narrow<i128>(1_i8) == 1);
                EXPECTS(unchecked_narrow<i128>(1_i16) == 1);
                EXPECTS(unchecked_narrow<i128>(1_i32) == 1);
                EXPECTS(unchecked_narrow<i128>(1_i64) == 1);
                EXPECTS(unchecked_narrow<i128>(1_u8) == 1);
                EXPECTS(unchecked_narrow<i128>(1_u16) == 1);
                EXPECTS(unchecked_narrow<i128>(1_u32) == 1);
                EXPECTS(unchecked_narrow<i128>(1_u64) == 1);
                EXPECTS(unchecked_narrow<i128>(1_u128) == 1);

                EXPECTS(unchecked_narrow<u128>(1_i8) == 1);
                EXPECTS(unchecked_narrow<u128>(1_i16) == 1);
                EXPECTS(unchecked_narrow<u128>(1_i32) == 1);
                EXPECTS(unchecked_narrow<u128>(1_i64) == 1);
                EXPECTS(unchecked_narrow<u128>(1_i128) == 1);
                EXPECTS(unchecked_narrow<u128>(1_u8) == 1);
                EXPECTS(unchecked_narrow<u128>(1_u16) == 1);
                EXPECTS(unchecked_narrow<u128>(1_u32) == 1);
                EXPECTS(unchecked_narrow<u128>(1_u64) == 1);
            } },
          }
    };
} // namespace
