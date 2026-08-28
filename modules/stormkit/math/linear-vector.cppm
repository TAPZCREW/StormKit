// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/api.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.math.linear.vector;

import std;

import stormkit.core.meta;
import stormkit.core.typesafe.safecasts;
import stormkit.core.hash;
import stormkit.core.types;
import stormkit.core.string;

import stormkit.math.linear;

export namespace stormkit { inline namespace core { namespace math {
    inline namespace vector {
        template<core::meta::arithmetic T>
        struct alignas(array<T, 2>) vec2 {
            using value_type  = T;
            using SizeType   = usize;
            using ExtentType = u8;

            using value_type  = value_type;
            using size_type   = SizeType;
            using extent_type = ExtentType;

            static constexpr auto EXTENT = array<ExtentType, 1> { 2uz };

            value_type x;
            value_type y;

            template<typename Self>
            constexpr auto operator[](this Self& self, usize i) noexcept -> core::meta::forward_const_to<Self, value_type>&;

            template<core::meta::arithmetic U>
            constexpr auto to() const noexcept -> vec2<U>;
        };

        using fvec2 = vec2<f32>;
        using ivec2 = vec2<i32>;
        using uvec2 = vec2<u32>;

        template<core::meta::arithmetic T>
        struct alignas(array<T, 3>) vec3 {
            using value_type  = T;
            using SizeType   = usize;
            using ExtentType = u8;

            using value_type  = value_type;
            using size_type   = SizeType;
            using extent_type = ExtentType;

            static constexpr auto EXTENT = array<ExtentType, 1> { 3uz };

            value_type x;
            value_type y;
            value_type z;

            template<typename Self>
            constexpr auto operator[](this Self& self, usize i) noexcept -> core::meta::forward_const_to<Self, value_type>&;

            template<core::meta::arithmetic U>
            constexpr auto to() const noexcept -> vec3<U>;
        };

        using fvec3 = vec3<f32>;
        using ivec3 = vec3<i32>;
        using uvec3 = vec3<u32>;

        template<core::meta::arithmetic T>
        struct alignas(array<T, 4>) vec4 {
            using value_type  = T;
            using SizeType   = usize;
            using ExtentType = u8;

            using value_type  = value_type;
            using size_type   = SizeType;
            using extent_type = ExtentType;

            static constexpr auto EXTENT = array<ExtentType, 1> { 4uz };

            value_type x;
            value_type y;
            value_type z;
            value_type w;

            template<typename Self>
            constexpr auto operator[](this Self& self, usize i) noexcept -> core::meta::forward_const_to<Self, value_type>&;

            template<core::meta::arithmetic U>
            constexpr auto to() const noexcept -> vec4<U>;
        };

        using fvec4 = vec4<f32>;
        using ivec4 = vec4<i32>;
        using uvec4 = vec4<u32>;

    } // namespace vector

    namespace meta {
        template<typename T>
        concept IsVec2 = core::meta::specialization_of<T, vec2>;
        template<typename T>
        concept IsVec3 = core::meta::specialization_of<T, vec3>;
        template<typename T>
        concept IsVec4 = core::meta::specialization_of<T, vec4>;

        template<typename T>
        concept IsVec = IsVec2<T> || IsVec3<T> || IsVec4<T>;

        template<typename T, typename U>
        concept HasOneVecType = not(core::meta::std_mdspan<T> and core::meta::std_mdspan<U>)
                                or meta::IsVec<T>
                                or meta::IsVec<U>;
    } // namespace meta

    inline namespace vector {
        template<meta::IsVec T>
        [[nodiscard]]
        constexpr auto add(const T& a, const T& b) noexcept -> T;

        template<meta::IsVec T>
        [[nodiscard]]
        constexpr auto sub(const T& a, const T& b) noexcept -> T;

        template<meta::IsVec T>
        [[nodiscard]]
        constexpr auto mul(const T& a, typename T::value_type b) noexcept -> T;

        template<meta::IsVec T>
        [[nodiscard]]
        constexpr auto div(const T& a, typename T::value_type b) noexcept -> T;

        template<meta::IsVec T>
        [[nodiscard]]
        constexpr auto dot(const T& a, const T& b) noexcept -> typename T::value_type;

        template<typename T>
        [[nodiscard]]
        constexpr auto cross(const vec3<T>& a, const vec3<T>& b) noexcept -> vec3<T>;

        template<meta::IsVec T>
        [[nodiscard]]
        constexpr auto normalize(const T& a) noexcept -> T;

        template<meta::IsVec T>
        [[nodiscard]]
        constexpr auto as_view(const T& value) noexcept -> array_view<const typename T::value_type, T::EXTENT[0]>;

        template<meta::IsVec T>
        [[nodiscard]]
        constexpr auto as_view_mut(T& value) noexcept -> array_view<typename T::value_type, T::EXTENT[0]>;

        template<meta::IsVec T>
        [[nodiscard]]
        constexpr auto as_mdspan(const T& value) noexcept -> VectorSpan<const typename T::value_type, T::EXTENT[0]>;

        template<meta::IsVec T>
            requires(not core::meta::const_type<T>)
        [[nodiscard]]
        constexpr auto as_mdspan_mut(T& value) noexcept -> VectorSpan<typename T::value_type, T::EXTENT[0]>;

        template<core::meta::arithmetic T>
        auto to_string(const vec2<T>& value) noexcept -> string;

        template<core::meta::arithmetic T>
        auto to_string(const vec3<T>& value) noexcept -> string;

        template<core::meta::arithmetic T>
        auto to_string(const vec4<T>& value) noexcept -> string;

        template<core::meta::hash_type Ret = hash32, core::meta::arithmetic T>
        constexpr auto hasher(const vec2<T>& value) noexcept -> Ret;

        template<core::meta::hash_type Ret = hash32, core::meta::arithmetic T>
        constexpr auto hasher(const vec3<T>& value) noexcept -> Ret;

        template<core::meta::hash_type Ret = hash32, core::meta::arithmetic T>
        constexpr auto hasher(const vec4<T>& value) noexcept -> Ret;

        template<core::meta::arithmetic T, typename FormatContext>
        auto format_as(const vec2<T>& value, FormatContext& ctx) noexcept -> decltype(ctx.out());

        template<core::meta::arithmetic T, typename FormatContext>
        auto format_as(const vec3<T>& value, FormatContext& ctx) noexcept -> decltype(ctx.out());

        template<core::meta::arithmetic T, typename FormatContext>
        auto format_as(const vec4<T>& value, FormatContext& ctx) noexcept -> decltype(ctx.out());
    } // namespace vector
}}} // namespace stormkit::core::math

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stdr = std::ranges;

namespace stormkit { inline namespace core { namespace math { inline namespace vector {
    static_assert(sizeof(uvec2) == sizeof(u32) * 2);
    static_assert(sizeof(vec2<u64>) == sizeof(u64) * 2);
    static_assert(sizeof(ivec2) == sizeof(i32) * 2);
    static_assert(sizeof(vec2<i64>) == sizeof(i64) * 2);
    static_assert(sizeof(fvec2) == sizeof(f32) * 2);
    static_assert(sizeof(vec2<f64>) == sizeof(f64) * 2);

    static_assert(sizeof(uvec3) == sizeof(u32) * 3);
    static_assert(sizeof(vec3<u64>) == sizeof(u64) * 3);
    static_assert(sizeof(ivec3) == sizeof(i32) * 3);
    static_assert(sizeof(vec3<i64>) == sizeof(i64) * 3);
    static_assert(sizeof(fvec3) == sizeof(f32) * 3);
    static_assert(sizeof(vec3<f64>) == sizeof(f64) * 3);

    static_assert(sizeof(uvec4) == sizeof(u32) * 4);
    static_assert(sizeof(vec4<u64>) == sizeof(u64) * 4);
    static_assert(sizeof(ivec4) == sizeof(i32) * 4);
    static_assert(sizeof(vec4<i64>) == sizeof(i64) * 4);
    static_assert(sizeof(fvec4) == sizeof(f32) * 4);
    static_assert(sizeof(vec4<f64>) == sizeof(f64) * 4);

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto vec2<T>::operator[](this Self& self, usize i) noexcept -> core::meta::forward_const_to<Self, value_type>& {
        static constexpr auto members = array { &vec2::x, &vec2::y };

        return std::forward_like<Self&>(self.*members[i]);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T>
    template<core::meta::arithmetic U>
    STORMKIT_CONST STORMKIT_FORCE_INLINE
    constexpr auto vec2<T>::to() const noexcept -> vec2<U> {
        return { core::as<U>(x), core::as<U>(y) };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T>
    template<typename Self>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto vec3<T>::operator[](this Self& self, usize i) noexcept -> core::meta::forward_const_to<Self, value_type>& {
        // WORKAROUND FOR ICE ON CLANG ON WINDOWS
#if defined(STORMKIT_OS_WINDOWS) and defined(STORMKIT_COMPILER_CLANG)
        if (i == 0) return std::forward_like<Self&>(self.x);
        else if (i == 1)
            return std::forward_like<Self&>(self.y);
        else if (i == 2)
            return std::forward_like<Self&>(self.z);

        std::unreachable();
#else
        static constexpr auto members = array { &vec3::x, &vec3::y, &vec3::z };
        return std::forward_like<Self&>(self.*members[i]);
#endif
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T>
    template<core::meta::arithmetic U>
    STORMKIT_CONST STORMKIT_FORCE_INLINE
    constexpr auto vec3<T>::to() const noexcept -> vec3<U> {
        return { core::as<U>(x), core::as<U>(y), core::as<U>(z) };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto vec4<T>::operator[](this Self& self, usize i) noexcept -> core::meta::forward_const_to<Self, value_type>& {
        static constexpr auto members = array { &vec4::x, &vec4::y, &vec4::z, &vec4::w };

        return std::forward_like<Self&>(self.*members[i]);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T>
    template<core::meta::arithmetic U>
    STORMKIT_CONST STORMKIT_FORCE_INLINE
    constexpr auto vec4<T>::to() const noexcept -> vec4<U> {
        return { core::as<U>(x), core::as<U>(y), core::as<U>(z), core::as<U>(w) };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::IsVec T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto normalize(const T& a) noexcept -> T {
        auto out = T {};

        math::normalize(as_mdspan(a), as_mdspan_mut(out));

        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::IsVec T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto add(const T& a, const T& b) noexcept -> T {
        auto out = T {};

        math::add(as_mdspan(a), as_mdspan(b), as_mdspan_mut(out));

        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::IsVec T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto sub(const T& a, const T& b) noexcept -> T {
        auto out = T {};

        math::sub(as_mdspan(a), as_mdspan(b), as_mdspan_mut(out));

        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::IsVec T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto mul(const T& a, typename T::value_type b) noexcept -> T {
        auto out = T {};

        math::mul(as_mdspan(a), b, as_mdspan_mut(out));

        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::IsVec T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto div(const T& a, typename T::value_type b) noexcept -> T {
        auto out = T {};

        math::div(as_mdspan(a), b, as_mdspan_mut(out));

        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::IsVec T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto dot(const T& a, const T& b) noexcept -> typename T::value_type {
        return math::dot(as_mdspan(a), as_mdspan(b));
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto cross(const vec3<T>& a, const vec3<T>& b) noexcept -> vec3<T> {
        auto out = vec3<T> {};

        math::cross(as_mdspan(a), as_mdspan(b), as_mdspan_mut(out));

        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::IsVec T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto as_view(const T& value) noexcept -> array_view<const typename T::value_type, T::EXTENT[0]> {
        return array_view<const typename T::value_type, T::EXTENT[0]> { &value.x, T::EXTENT[0] };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::IsVec T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto as_view_mut(T& value) noexcept -> array_view<typename T::value_type, T::EXTENT[0]> {
        return array_view<typename T::value_type, T::EXTENT[0]> { &value.x, T::EXTENT[0] };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::IsVec T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto as_mdspan(const T& value) noexcept -> VectorSpan<const typename T::value_type, T::EXTENT[0]> {
        return VectorSpan<const typename T::value_type, T::EXTENT[0]> { &value.x, T::EXTENT };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::IsVec T>
        requires(not core::meta::const_type<T>)
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto as_mdspan_mut(T& value) noexcept -> VectorSpan<typename T::value_type, T::EXTENT[0]> {
        return VectorSpan<typename T::value_type, T::EXTENT[0]> { &value.x, T::EXTENT };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T>
    STORMKIT_FORCE_INLINE
    auto to_string(const vec2<T>& value) noexcept -> string {
        return std::format("{}", value);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T>
    STORMKIT_FORCE_INLINE
    auto to_string(const vec3<T>& value) noexcept -> string {
        return std::format("{}", value);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T>
    STORMKIT_FORCE_INLINE
    auto to_string(const vec4<T>& value) noexcept -> string {
        return std::format("{}", value);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::hash_type Ret, core::meta::arithmetic T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto hasher(const vec2<T>& value) noexcept -> Ret {
        return hash<Ret>(value.x, value.y);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::hash_type Ret, core::meta::arithmetic T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto hasher(const vec3<T>& value) noexcept -> Ret {
        return hash<Ret>(value.x, value.y, value.z);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::hash_type Ret, core::meta::arithmetic T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto hasher(const vec4<T>& value) noexcept -> Ret {
        return hash<Ret>(value.x, value.y, value.z, value.w);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T, typename FormatContext>
    STORMKIT_FORCE_INLINE
    inline auto format_as(const vec2<T>& value, FormatContext& ctx) noexcept -> decltype(ctx.out()) {
        return std::format_to(ctx.out(), "[vec2 x: {}, y: {}]", value.x, value.y);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T, typename FormatContext>
    STORMKIT_FORCE_INLINE
    inline auto format_as(const vec3<T>& value, FormatContext& ctx) noexcept -> decltype(ctx.out()) {
        return std::format_to(ctx.out(), "[vec3 x: {}, y: {}, z: {}]", value.x, value.y, value.z);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T, typename FormatContext>
    STORMKIT_FORCE_INLINE
    inline auto format_as(const vec4<T>& value, FormatContext& ctx) noexcept -> decltype(ctx.out()) {
        return std::format_to(ctx.out(), "[vec4 x: {}, y: {}, z: {}, w: {}]", value.x, value.y, value.z, value.w);
    }

#ifndef STORMKIT_OS_WINDOWS
    #undef STORMKIT_CORE_API
    #define STORMKIT_CORE_API
#endif

#define ADD_INSTANCIATE(vec_type) \
    template STORMKIT_CORE_API auto add<vec_type>(const vec_type&, const vec_type&) noexcept -> vec_type

    ADD_INSTANCIATE(fvec2);
    ADD_INSTANCIATE(fvec3);
    ADD_INSTANCIATE(fvec4);
    ADD_INSTANCIATE(uvec2);
    ADD_INSTANCIATE(uvec3);
    ADD_INSTANCIATE(uvec4);
    ADD_INSTANCIATE(ivec2);
    ADD_INSTANCIATE(ivec3);
    ADD_INSTANCIATE(ivec4);

#undef ADD_INSTANCIATE

#define SUB_INSTANCIATE(vec_type) \
    template STORMKIT_CORE_API auto sub<vec_type>(const vec_type&, const vec_type&) noexcept -> vec_type

    SUB_INSTANCIATE(fvec2);
    SUB_INSTANCIATE(fvec3);
    SUB_INSTANCIATE(fvec4);
    SUB_INSTANCIATE(uvec2);
    SUB_INSTANCIATE(uvec3);
    SUB_INSTANCIATE(uvec4);
    SUB_INSTANCIATE(ivec2);
    SUB_INSTANCIATE(ivec3);
    SUB_INSTANCIATE(ivec4);

#undef SUB_INSTANCIATE

#define MUL_INSTANCIATE(vec_type) \
    template STORMKIT_CORE_API auto mul<vec_type>(const vec_type&, typename vec_type::value_type) noexcept -> vec_type

    MUL_INSTANCIATE(fvec2);
    MUL_INSTANCIATE(fvec3);
    MUL_INSTANCIATE(fvec4);
    MUL_INSTANCIATE(uvec2);
    MUL_INSTANCIATE(uvec3);
    MUL_INSTANCIATE(uvec4);
    MUL_INSTANCIATE(ivec2);
    MUL_INSTANCIATE(ivec3);
    MUL_INSTANCIATE(ivec4);

#undef MUL_INSTANCIATE

#define DIV_INSTANCIATE(vec_type) \
    template STORMKIT_CORE_API auto div<vec_type>(const vec_type&, typename vec_type::value_type) noexcept -> vec_type

    DIV_INSTANCIATE(fvec2);
    DIV_INSTANCIATE(fvec3);
    DIV_INSTANCIATE(fvec4);
    DIV_INSTANCIATE(uvec2);
    DIV_INSTANCIATE(uvec3);
    DIV_INSTANCIATE(uvec4);
    DIV_INSTANCIATE(ivec2);
    DIV_INSTANCIATE(ivec3);
    DIV_INSTANCIATE(ivec4);

#undef DIV_INSTANCIATE

#define DOT_INSTANCIATE(vec_type) \
    template STORMKIT_CORE_API auto dot<vec_type>(const vec_type&, const vec_type&) noexcept -> typename vec_type::value_type

    DOT_INSTANCIATE(fvec2);
    DOT_INSTANCIATE(fvec3);
    DOT_INSTANCIATE(fvec4);
    DOT_INSTANCIATE(uvec2);
    DOT_INSTANCIATE(uvec3);
    DOT_INSTANCIATE(uvec4);
    DOT_INSTANCIATE(ivec2);
    DOT_INSTANCIATE(ivec3);
    DOT_INSTANCIATE(ivec4);

#undef DOT_INSTANCIATE

#define CROSS_INSTANCIATE(type) \
    template STORMKIT_CORE_API auto cross<type>(const vec3<type>&, const vec3<type>&) noexcept -> vec3<type>

    CROSS_INSTANCIATE(f32);
    CROSS_INSTANCIATE(u32);
    CROSS_INSTANCIATE(i32);

#undef CROSS_INSTANCIATE

#define NORMALIZE_INSTANCIATE(vec_type) template STORMKIT_CORE_API auto normalize<vec_type>(const vec_type&) noexcept -> vec_type

    NORMALIZE_INSTANCIATE(fvec2);
    NORMALIZE_INSTANCIATE(fvec3);
    NORMALIZE_INSTANCIATE(fvec4);
    NORMALIZE_INSTANCIATE(uvec2);
    NORMALIZE_INSTANCIATE(uvec3);
    NORMALIZE_INSTANCIATE(uvec4);
    NORMALIZE_INSTANCIATE(ivec2);
    NORMALIZE_INSTANCIATE(ivec3);
    NORMALIZE_INSTANCIATE(ivec4);

#undef NORMALIZE_INSTANCIATE
}}}} // namespace stormkit::core::math::vector
