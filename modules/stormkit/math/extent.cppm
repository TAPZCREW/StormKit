// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.math.extent;

import std;

import stormkit.core.contract;
import stormkit.core.meta;
import stormkit.core.hash;
import stormkit.core.ranges;
import stormkit.core.types;
import stormkit.core.typesafe;
import stormkit.core.string.format;

namespace cmeta = stormkit::core::meta;

using namespace stormkit;
using namespace std::literals;

namespace stdr = std::ranges;

export {
    namespace stormkit::math {
        template<core::meta::arithmetic T, usize N>
        struct extent;

        template<core::meta::arithmetic T>
        struct alignas(array<T, 2>) extent<T, 2> {
            using value_type    = T;
            using ordering_type = meta::arithmetic_ordering_type<T>;
            using size_type     = usize;

            static constexpr auto RANK = size_type { 2 };

            value_type width  = 0;
            value_type height = 0;

            template<typename Self>
            constexpr auto operator[](this Self& self, size_type index) noexcept -> cmeta::forward_const_to<Self, value_type>&;
        };

        template<core::meta::arithmetic T>
        using extent2 = extent<T, 2>;

        using fextent2 = extent2<f32>;
        using uextent2 = extent2<u32>;
        using iextent2 = extent2<i32>;

        template<core::meta::arithmetic T>
        struct alignas(array<T, 3>) extent<T, 3> {
            using value_type    = T;
            using ordering_type = meta::arithmetic_ordering_type<T>;
            using size_type     = usize;

            static constexpr auto RANK = size_type { 3 };

            value_type width  = 0;
            value_type height = 0;
            value_type depth  = 1;

            template<typename Self>
            constexpr auto operator[](this Self& self, size_type index) noexcept -> cmeta::forward_const_to<Self, value_type>&;
        };

        template<core::meta::arithmetic T>
        using extent3 = extent<T, 3>;

        using fextent3 = extent3<f32>;
        using uextent3 = extent3<u32>;
        using iextent3 = extent3<i32>;

        template<core::meta::arithmetic T>
        extent(T, T) -> extent<T, 2>;

        template<core::meta::arithmetic T>
        extent(T, T, T) -> extent<T, 3>;

        namespace meta {
            template<typename T>
            concept is_extent = cmeta::specialization_of_nttp_tv<T, extent>;

            template<typename T>
            concept is_extent2 = is_extent<T> and requires(T) { requires T::RANK == cmeta::size_type<T> { 2 }; };

            template<typename T>
            concept is_extent3 = is_extent<T> and requires(T) { requires T::RANK == cmeta::size_type<T> { 3 }; };
        } // namespace meta

        template<meta::is_extent Extent>
        [[nodiscard]]
        constexpr auto operator<=>(const Extent& first, const Extent& second) noexcept -> typename Extent::ordering_type;

        template<meta::is_extent Extent>
        [[nodiscard]]
        constexpr auto operator==(const Extent& first, const Extent& second) noexcept -> bool;

        template<meta::is_extent Extent>
        [[nodiscard]]
        constexpr auto operator*(const Extent& extent, typename Extent::value_type factor) noexcept -> Extent;

        template<meta::is_extent Extent>
        [[nodiscard]]
        constexpr auto operator/(const Extent& extent, typename Extent::value_type factor) noexcept -> Extent;

        template<meta::is_extent Extent>
        [[nodiscard]]
        constexpr auto operator*=(Extent& extent, typename Extent::value_type factor) noexcept -> Extent&;

        template<meta::is_extent Extent>
        [[nodiscard]]
        constexpr auto operator/=(Extent& extent, typename Extent::value_type factor) noexcept -> Extent&;

        template<typename CharT, cmeta::arithmetic T>
        constexpr auto tag_invoke(format_as_fn<CharT>, const extent2<T>& matrix, cmeta::format_context auto& ctx) noexcept
          -> decltype(ctx.out());

        template<typename CharT, cmeta::arithmetic T>
        constexpr auto tag_invoke(format_as_fn<CharT>, const extent3<T>& matrix, cmeta::format_context auto& ctx) noexcept
          -> decltype(ctx.out());

        template<meta::is_extent To, meta::is_extent From>
        constexpr auto tag_invoke(as_fn<To>, const From& extent, source_location_arg = std::source_location::current()) noexcept
          -> To;
    } // namespace stormkit::math
}

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::math {
    /////////////////////////////////////
    /////////////////////////////////////
    template<core::meta::arithmetic T>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto extent<T, 2>::operator[](this Self& self, size_type index) noexcept
      -> cmeta::forward_const_to<Self, value_type>& {
        EXPECTS(index < 2);

        static constexpr auto accessors = std::array { &extent2<T>::width, &extent2<T>::height };

        return std::forward_like<Self&>(self.*accessors[index]);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<core::meta::arithmetic T>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto extent<T, 3>::operator[](this Self& self, size_type index) noexcept
      -> cmeta::forward_const_to<Self, value_type>& {
        EXPECTS(index < 3);

        static constexpr auto accessors = std::array { &extent3<T>::width, &extent3<T>::height, &extent3<T>::depth };

        return std::forward_like<Self&>(self.*accessors[index]);
    }

    // /////////////////////////////////////
    // /////////////////////////////////////
    // template<core::meta::arithmetic T>
    // template<usize NEW_RANK>
    // STORMKIT_PURE
    // constexpr auto extent<T, 2>::to() const noexcept -> extent<T, NEW_RANK> {
    //     using Out        = extent<value_type, NEW_RANK>;
    //     using Array      = array<value_type, RANK>;
    //     using OtherArray = array<typename Out::value_type, Out::RANK>;

    //    auto out = Out {};

    //    auto& values       = *std::bit_cast<Array*>(this);
    //    auto& other_values = *std::bit_cast<OtherArray*>(&out);

    //    if constexpr (NEW_RANK > RANK) {
    //        stdr::copy(values, stdr::begin(other_values));
    //    } else
    //        stdr::copy_n(stdr::begin(values), NEW_RANK, stdr::begin(other_values));

    //    return out;
    // }

    // /////////////////////////////////////
    // /////////////////////////////////////
    // template<core::meta::arithmetic T>
    // template<core::meta::arithmetic U>
    // STORMKIT_PURE
    // constexpr auto extent<T, 3>::narrow_to() const noexcept -> extent<U, 3> {
    //     return { .width = narrow<U>(width), .height = narrow<U>(height), .depth = narrow<U>(depth) };
    // }

    // /////////////////////////////////////
    // /////////////////////////////////////
    // template<core::meta::arithmetic T>
    // template<core::meta::arithmetic U>
    // STORMKIT_PURE
    // constexpr auto extent<T, 3>::to(const std::source_location& location) const noexcept -> extent<U, 3> {
    //     return { .width = as<U>(width, location), .height = as<U>(height, location), .depth = as<U>(depth, location) };
    // }

    // /////////////////////////////////////
    // /////////////////////////////////////
    // template<core::meta::arithmetic T>
    // template<usize NEW_RANK>
    // STORMKIT_PURE
    // constexpr auto extent<T, 3>::to() const noexcept -> extent<T, NEW_RANK> {
    //     using Out        = extent<value_type, NEW_RANK>;
    //     using Array      = array<value_type, RANK>;
    //     using OtherArray = array<typename Out::value_type, Out::RANK>;

    //    auto out = Out {};

    //    auto& values       = *std::bit_cast<Array*>(this);
    //    auto& other_values = *std::bit_cast<OtherArray*>(&out);

    //    if constexpr (NEW_RANK > RANK) {
    //        stdr::copy(values, stdr::begin(other_values));
    //    } else
    //        stdr::copy_n(stdr::begin(values), NEW_RANK, stdr::begin(other_values));

    //    return out;
    // }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_extent Extent>
    STORMKIT_CONST
    constexpr auto operator<=>(const Extent& first, const Extent& second) noexcept -> typename Extent::ordering_type {
        using ordering_type = cmeta::ordering_type<Extent>;

        for (auto i : range(Extent::RANK))
            if (auto ret = first[i] <=> second[i]; ret != 0) return ret;

        return ordering_type::equivalent;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_extent Extent>
    STORMKIT_PURE
    constexpr auto operator==(const Extent& first, const Extent& second) noexcept -> bool {
        for (auto i : range(Extent::RANK))
            if (first[i] != second[i]) return false;

        return true;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_extent Extent>
    STORMKIT_CONST
    constexpr auto operator*(const Extent& extent, typename Extent::value_type factor) noexcept -> Extent {
        auto out = auto(extent);
        out *= factor;
        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_extent Extent>
    STORMKIT_CONST
    constexpr auto operator/(const Extent& extent, typename Extent::value_type factor) noexcept -> Extent {
        auto out = auto(extent);
        out /= factor;
        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_extent Extent>
    constexpr auto operator*=(Extent& extent, typename Extent::value_type factor) noexcept -> Extent& {
        using value_type = cmeta::value_type<Extent>;

        for (auto i : range(Extent::RANK)) extent[i] *= factor;

        return extent;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_extent Extent>
    constexpr auto operator/=(Extent& extent, typename Extent::value_type factor) noexcept -> Extent& {
        using value_type = cmeta::value_type<Extent>;

        for (auto i : range(Extent::RANK)) extent[i] /= factor;

        return extent;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename CharT, cmeta::arithmetic T>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(format_as_fn<CharT>, const extent2<T>& extent, cmeta::format_context auto& ctx) noexcept
      -> decltype(ctx.out()) {
        return std::format_to(ctx.out(), "[extent2 .width = {}, .height = {}]", extent.width, extent.height);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename CharT, cmeta::arithmetic T>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(format_as_fn<CharT>, const extent3<T>& extent, cmeta::format_context auto& ctx) noexcept
      -> decltype(ctx.out()) {
        return std::format_to(ctx.out(),
                              "[extent2 .width = {}, .height = {}, .depth = {}]",
                              extent.width,
                              extent.height,
                              extent.depth);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_extent To, meta::is_extent From>
    constexpr auto tag_invoke(as_fn<To>, const From& extent, source_location_arg) noexcept -> To {
        using to_value_type   = cmeta::value_type<To>;
        using from_value_type = cmeta::value_type<From>;

        auto out = To {};
        if constexpr (To::RANK > From::RANK)
            for (auto i : range(From::RANK)) out[i] = as<to_value_type>(extent[i]);
        else
            for (auto i : range(To::RANK)) out[i] = as<to_value_type>(extent[i]);

        return out;
    }
} // namespace stormkit::math

static_assert(sizeof(math::uextent2) == sizeof(array<u32, 2>));
static_assert(sizeof(math::uextent3) == sizeof(array<u32, 3>));

static_assert(sizeof(math::iextent2) == sizeof(array<i32, 2>));
static_assert(sizeof(math::iextent3) == sizeof(array<i32, 3>));

static_assert(sizeof(math::extent2<u16>) == sizeof(array<u16, 2>));
static_assert(sizeof(math::extent3<u16>) == sizeof(array<u16, 3>));

static_assert(sizeof(math::extent2<i16>) == sizeof(array<i16, 2>));
static_assert(sizeof(math::extent3<i16>) == sizeof(array<i16, 3>));

static_assert(sizeof(math::fextent2) == sizeof(array<f32, 2>));
static_assert(sizeof(math::fextent3) == sizeof(array<f32, 3>));

static_assert(math::fextent2::RANK == 2);
static_assert(math::fextent3::RANK == 3);
static_assert(math::fextent3::RANK != 2);
static_assert(math::fextent2::RANK != 3);

static_assert(math::meta::is_extent<math::fextent2>);
static_assert(math::meta::is_extent<math::fextent3>);
static_assert(math::meta::is_extent2<math::fextent2>);
static_assert(not math::meta::is_extent2<math::fextent3>);
static_assert(math::meta::is_extent3<math::fextent3>);
static_assert(not math::meta::is_extent3<math::fextent2>);

static_assert(as<math::iextent3>(math::fextent2 { 1.f, 2.f }).width == 1);
