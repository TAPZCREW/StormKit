// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.math.geometry;

import std;

import stormkit.core.meta;
import stormkit.core.typesafe.safecasts;
import stormkit.core.typesafe.checked_value;
import stormkit.core.types;
import stormkit.core.hash;

import stormkit.math.arithmetic;
import stormkit.math.extent;
import stormkit.math.linear.vector;

export namespace stormkit { inline namespace core { namespace math {
    template<core::meta::arithmetic T>
    struct rect {
        using value_type  = T;
        using value_type = T;

        value_type           x      = value_type { 0 };
        value_type           y      = value_type { 0 };
        Positive<value_type> width  = value_type { 0 };
        Positive<value_type> height = value_type { 0 };

        constexpr auto position() const noexcept -> vec2<value_type>;
        constexpr auto extent() const noexcept -> extent2<value_type>;

        template<core::meta::arithmetic U>
        constexpr auto to() const noexcept -> rect<U>;
    };

    using irect = rect<i32>;
    using urect = rect<u32>;
    using frect = rect<f32>;

    template<core::meta::arithmetic T>
    rect(T, T, T, T) -> rect<T>;

    template<core::meta::arithmetic T>
    struct bounding_rect {
        using value_type  = T;
        using value_type = T;

        value_type left   = value_type { 0 };
        value_type top    = value_type { 0 };
        value_type right  = value_type { 0 };
        value_type bottom = value_type { 0 };

        constexpr auto topleft() const noexcept -> vec2<value_type>;
        constexpr auto bottomright() const noexcept -> vec2<value_type>;

        template<core::meta::arithmetic U>
        constexpr auto to() const noexcept -> rect<U>;
    };

    using ibounding_rect = bounding_rect<i32>;
    using ubounding_rect = bounding_rect<u32>;
    using fbounding_rect = bounding_rect<f32>;

    template<core::meta::arithmetic T>
    auto to_string(const rect<T>& value) noexcept -> string;

    template<core::meta::arithmetic T>
    auto to_string(const bounding_rect<T>& value) noexcept -> string;

    template<core::meta::hash_type Ret = hash32, core::meta::arithmetic T>
    constexpr auto hasher(const rect<T>& value) noexcept -> Ret;

    template<core::meta::hash_type Ret = hash32, core::meta::arithmetic T>
    constexpr auto hasher(const bounding_rect<T>& value) noexcept -> Ret;

    template<core::meta::arithmetic T, typename FormatContext>
    auto format_as(const rect<T>& value, FormatContext& ctx) -> decltype(ctx.out());

    template<core::meta::arithmetic T, typename FormatContext>
    auto format_as(const bounding_rect<T>& value, FormatContext& ctx) -> decltype(ctx.out());

    template<typename T>
    constexpr auto to_bounding_rect(const rect<T>& value) noexcept -> bounding_rect<T>;

    template<typename T>
    constexpr auto to_rect(const bounding_rect<T>& value) noexcept -> rect<T>;

    template<typename T>
    constexpr auto AABB(const rect<T>& rect1, const rect<T>& rect2) noexcept -> bool;

    template<typename T>
    constexpr auto AABB(const vec2<T>& pos, const bounding_rect<T>& rect) noexcept -> bool;

    template<typename T>
    constexpr auto AABB(const vec2<T>& pos, const rect<T>& rect) noexcept -> bool;
}}} // namespace stormkit::core::math

namespace stormkit { inline namespace core { namespace math {
    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T>
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto rect<T>::position() const noexcept -> vec2<T> {
        return { x, y };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T>
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto rect<T>::extent() const noexcept -> extent2<T> {
        return { width.value, height.value };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T>
    template<core::meta::arithmetic U>
    STORMKIT_FORCE_INLINE
    STORMKIT_PURE
    constexpr auto rect<T>::to() const noexcept -> rect<U> {
        return { as<U>(x), as<U>(y), as<U>(width), as<U>(height) };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T>
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto bounding_rect<T>::topleft() const noexcept -> vec2<T> {
        return { left, top };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T>
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto bounding_rect<T>::bottomright() const noexcept -> vec2<T> {
        return { right, bottom };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T>
    template<core::meta::arithmetic U>
    STORMKIT_FORCE_INLINE
    STORMKIT_PURE
    constexpr auto bounding_rect<T>::to() const noexcept -> rect<U> {
        return { as<U>(left), as<U>(top), as<U>(right), as<U>(bottom) };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T>
    STORMKIT_FORCE_INLINE
    inline auto to_string(const rect<T>& value) noexcept -> string {
        return std::format("{}", value);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T>
    STORMKIT_FORCE_INLINE
    inline auto to_string(const bounding_rect<T>& value) noexcept -> string {
        return std::format("{}", value);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::hash_type Ret, core::meta::arithmetic T>
    STORMKIT_FORCE_INLINE
    constexpr auto hasher(const rect<T>& value) noexcept -> Ret {
        return hash<Ret>(value.x, value.y, value.width, value.height);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::hash_type Ret, core::meta::arithmetic T>
    STORMKIT_FORCE_INLINE
    constexpr auto hasher(const bounding_rect<T>& value) noexcept -> Ret {
        return hash(value.left, value.top, value.right, value.bottom);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T, typename FormatContext>
    STORMKIT_FORCE_INLINE
    inline auto format_as(const rect<T>& point, FormatContext& ctx) -> decltype(ctx.out()) {
        return std::format_to(ctx.out(),
                              "[rect x = {}, y = {}, width = {}, height = {}]",
                              point.x,
                              point.y,
                              point.width,
                              point.height);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T, typename FormatContext>
    STORMKIT_FORCE_INLINE
    inline auto format_as(const bounding_rect<T>& point, FormatContext& ctx) -> decltype(ctx.out()) {
        return std::format_to(ctx.out(),
                              "[bounding_rect left = {}, top = {}, right = {}, bottom = {}]",
                              point.left,
                              point.top,
                              point.right,
                              point.bottom);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto to_bounding_rect(const rect<T>& _rect) noexcept -> bounding_rect<T> {
        return { _rect.x, _rect.y, _rect.x + _rect.width, _rect.y + _rect.height };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto to_rect(const bounding_rect<T>& _rect) noexcept -> rect<T> {
        return { _rect.left, _rect.top, _rect.right - _rect.left, _rect.bottom - _rect.top };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto AABB(const bounding_rect<T>& rect1, const bounding_rect<T>& rect2) noexcept -> bool {
        return rect1.left < rect2.right and rect1.right > rect2.left and rect1.top < rect2.bottom and rect1.bottom > rect2.top;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto AABB(const rect<T>& rect1, const rect<T>& rect2) noexcept -> bool {
        return AABB(to_bounding_rect(rect1), to_bounding_rect(rect2));
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto AABB(const vec2<T>& pos, const bounding_rect<T>& rect) noexcept -> bool {
        return pos.x >= rect.left and pos.x <= rect.right and pos.y >= rect.top and pos.y <= rect.bottom;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto AABB(const vec2<T>& pos, const rect<T>& rect) noexcept -> bool {
        return AABB(pos, to_bounding_rect(rect));
    }
}}} // namespace stormkit::core::math
