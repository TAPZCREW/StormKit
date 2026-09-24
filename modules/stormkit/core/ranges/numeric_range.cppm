// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#if defined(__cpp_lib_ranges_cartesian_product) and __cpp_lib_ranges_cartesian_product >= 202207L
    #define STD_CARTESIAN_PRODUCT_SUPPORTED
#else
    #include <stormkit/polyfill/cartesian_product_view.h>
#endif

export module stormkit.core.ranges.numeric_range;

import std;

import stormkit.core.types;

import stormkit.core.meta;
import stormkit.core.typesafe.safecasts;
import stormkit.core.coroutines;

export namespace stormkit { inline namespace core {
    template<typename T>
    struct numeric_range {
        T begin = 0;
        T end;
        T step = 1;
    };

    template<meta::arithmetic T>
    [[nodiscard]]
    constexpr auto range(T end) noexcept -> decltype(auto);

    template<meta::arithmetic T>
    [[nodiscard]]
    constexpr auto range(T begin, T end) noexcept -> decltype(auto);

    template<meta::arithmetic T>
    [[nodiscard]]
    constexpr auto range(T begin, T end, T step) noexcept -> decltype(auto);

    template<meta::arithmetic T>
    [[nodiscard]]
    constexpr auto range(const numeric_range<T>& nrange) noexcept -> decltype(auto);

    template<meta::arithmetic... Ts>
    [[nodiscard]]
    constexpr auto multi_range(Ts... args) noexcept -> decltype(auto);

    template<meta::arithmetic... Ts>
    [[nodiscard]]
    constexpr auto multi_range(const numeric_range<Ts>&... args) noexcept -> decltype(auto);
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stdr = std::ranges;
namespace stdv = std::views;

namespace stormkit { inline namespace core {
    template<meta::arithmetic T>
    STORMKIT_FORCE_INLINE
    constexpr auto range(const numeric_range<T>& nrange) noexcept -> decltype(auto) {
        return stdv::iota(nrange.begin, nrange.end) | stdv::stride(nrange.step);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::arithmetic T>
    STORMKIT_FORCE_INLINE
    constexpr auto range(T begin, T end, T step) noexcept -> decltype(auto) {
        return range(numeric_range<T> { .begin = begin, .end = end, .step = step });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::arithmetic T>
    STORMKIT_FORCE_INLINE
    constexpr auto range(T begin, T end) noexcept -> decltype(auto) {
        return stdv::iota(begin, end);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::arithmetic T>
    STORMKIT_FORCE_INLINE
    constexpr auto range(T end) noexcept -> decltype(auto) {
        return range(T { 0 }, end);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::arithmetic... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto multi_range(Ts... ends) noexcept -> decltype(auto) {
#ifdef STD_CARTESIAN_PRODUCT_SUPPORTED
        return stdv::cartesian_product(range(ends)...);
#else
        return cartesian_product(range(ends)...);
#endif
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::arithmetic... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto multi_range(const numeric_range<Ts>&... nranges) noexcept -> decltype(auto) {
#ifdef STD_CARTESIAN_PRODUCT_SUPPORTED
        return stdv::cartesian_product(range(nranges)...);
#else
        return cartesian_product(range(nranges)...);
#endif
    }
}} // namespace stormkit::core
