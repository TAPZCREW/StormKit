// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.math.combinatoric;

import std;

import stormkit.core.types;
import stormkit.core.typesafe;
import stormkit.core.meta;

import stormkit.math.arithmetic;

namespace stdr = std::ranges;

export namespace stormkit { inline namespace core { namespace math {
    template<stormkit::core::meta::arithmetic T>
    constexpr auto factoriel(T n) noexcept -> T;
}}} // namespace stormkit::core::math

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core { namespace math {
    constexpr auto FTABLE = array<u64, 14> {
        1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800, 39916800, 479001600, 6227020800
    };

    /////////////////////////////////////
    /////////////////////////////////////
    template<stormkit::core::meta::arithmetic T>
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto factoriel(T n) noexcept -> T {
        if constexpr (stormkit::meta::integral<T>) {
            const auto i = [](auto n) static noexcept {
                if constexpr (stormkit::meta::signed_type<T>) return abs(static_cast<usize>(n));
                else
                    return static_cast<usize>(n);
            }(n);

            if constexpr (sizeof(T) <= 4) {
                expects(FTABLE[i], "factoriel<i8> with n >= 5 result in integer overflow");
                return static_cast<T>(FTABLE[i]);
            } else {
                if (i < stdr::size(FTABLE)) return static_cast<T>(FTABLE[static_cast<usize>(n)]);
            }
        }

        auto res = T { 1 };
        for (auto i = T { 2 }; i <= n; ++i) res *= i;
        return res;
    }
}}} // namespace stormkit::core::math
