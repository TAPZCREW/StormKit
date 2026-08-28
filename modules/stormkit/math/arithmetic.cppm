// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/api.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.math.arithmetic;

import std;

import stormkit.core.meta.concepts;
import stormkit.core.types;
import stormkit.core.typesafe.safecasts;

export {
    namespace stormkit { inline namespace core { namespace math {
        template<meta::arithmetic T>
        [[nodiscard]]
        constexpr auto floor(T v) noexcept -> T;

        template<meta::arithmetic T>
        [[nodiscard]]
        constexpr auto log2(T v) noexcept -> T;

        template<meta::arithmetic T>
        [[nodiscard]]
        constexpr auto min(T a, T b) noexcept -> T;

        template<meta::arithmetic T>
        [[nodiscard]]
        constexpr auto max(T a, T b) noexcept -> T;

        template<meta::arithmetic T, meta::arithmetic U>
        [[nodiscard]]
        constexpr auto scale(U x, U rmin, U rmax, T tmin, T tmax) noexcept -> T;

        template<meta::integral T>
        [[nodiscard]]
        constexpr auto scale(T x, T rmin, T rmax, T tmin, T tmax) noexcept -> T;

        template<meta::floating_point T>
        [[nodiscard]]
        constexpr auto scale(T x, T rmin, T rmax, T tmin, T tmax) noexcept -> T;

        template<stormkit::core::meta::arithmetic T>
        [[nodiscard]]
        constexpr auto abs(T n) noexcept -> T;

        template<meta::arithmetic T>
        [[nodiscard]]
        constexpr auto is_positive(T value) noexcept -> bool;

        template<meta::arithmetic T>
        [[nodiscard]]
        constexpr auto is_negative(T value) noexcept -> bool;
    }}} // namespace stormkit::core::math
}

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core { namespace math {
    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::arithmetic T>
    STORMKIT_CONST STORMKIT_FORCE_INLINE
    constexpr auto floor(T v) noexcept -> T {
        // if consteval {
        //     if constexpr (meta::integral<T>) return as<T>(std::floor(v));
        //     else
        //         return as<T>(as<i64>(v));
        // } else {
        return as<T>(std::floor(v));
        // }
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::arithmetic T>
    STORMKIT_CONST STORMKIT_FORCE_INLINE
    constexpr auto log2(T v) noexcept -> T {
        if consteval {
            auto val_i = as<i32>(v);
            auto log_2 = std::bit_cast<f32>(((val_i >> 23) & 255) - 128);
            val_i &= ~(255 << 23);
            val_i += 127 << 23;
            const auto val_f = std::bit_cast<f32>(val_i);
            log_2 += ((-0.3358287811f) * val_f + 2.0f) * val_f - 0.65871759316667f;
            return as<T>(log_2);
        } else {
            return as<T>(std::log2(v));
        }
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::arithmetic T>
    STORMKIT_CONST STORMKIT_FORCE_INLINE
    constexpr auto min(T a, T b) noexcept -> T {
        return (a < b) ? a : b;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::arithmetic T>
    STORMKIT_CONST STORMKIT_FORCE_INLINE
    constexpr auto max(T a, T b) noexcept -> T {
        return (a > b) ? a : b;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T, core::meta::arithmetic U>
    STORMKIT_CONST STORMKIT_FORCE_INLINE
    constexpr auto scale(U x, U rmin, U rmax, T tmin, T tmax) noexcept -> T {
        return scale(as<T>(x), as<T>(rmin), as<T>(rmax), tmin, tmax);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::integral T>
    STORMKIT_CONST STORMKIT_FORCE_INLINE
    constexpr auto scale(T x, T rmin, T rmax, T tmin, T tmax) noexcept -> T {
        return (x - rmin) * (tmax - tmin) / (rmax - rmin) + tmin;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::floating_point T>
    STORMKIT_CONST STORMKIT_FORCE_INLINE
    constexpr auto scale(T x, T rmin, T rmax, T tmin, T tmax) noexcept -> T {
        if (x < rmin) return rmin;
        if (x > rmax) return rmax;
        return ((x - rmin) * (tmax - tmin) / (rmax - rmin)) + tmin;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<stormkit::core::meta::arithmetic T>
    STORMKIT_CONST STORMKIT_FORCE_INLINE
    constexpr auto abs(T n) noexcept -> T {
        if constexpr (not stormkit::meta::signed_type<T>) return n;
        else
            return as<T>(std::abs(n));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::arithmetic T>
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto is_positive(T value) noexcept -> bool {
        if constexpr (meta::unsigned_type<T>) return true;
        else
            return value >= 0;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::arithmetic T>
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto is_negative(T value) noexcept -> bool {
        if constexpr (meta::signed_type<T>) return value < 0;
        else
            return false;
    }

#ifndef STORMKIT_COMPILER_MSVC
    #ifndef STORMKIT_OS_WINDOWS
        #undef STORMKIT_CORE_API
        #define STORMKIT_CORE_API
    #endif

    #define INSTANCIATE(t)                                                  \
        template STORMKIT_CORE_API auto is_positive<t>(t) noexcept -> bool; \
        template STORMKIT_CORE_API auto is_negative<t>(t) noexcept -> bool; \
        template STORMKIT_CORE_API auto abs<t>(t) noexcept -> t;            \
        template STORMKIT_CORE_API auto min<t>(t, t) noexcept -> t;         \
        template STORMKIT_CORE_API auto max<t>(t, t) noexcept -> t;         \
        template STORMKIT_CORE_API auto log2<t>(t) noexcept -> t;           \
        template STORMKIT_CORE_API auto floor<t>(t) noexcept -> t;

    INSTANCIATE(u8);
    INSTANCIATE(i8);
    INSTANCIATE(u16);
    INSTANCIATE(i16);
    INSTANCIATE(u32);
    INSTANCIATE(i32);
    INSTANCIATE(u64);
    INSTANCIATE(i64);
    // INSTANCIATE(u128);
    // INSTANCIATE(i128);
    INSTANCIATE(f32);
    INSTANCIATE(f64);

    #undef INSTANCIATE
#endif

}}} // namespace stormkit::core::math
