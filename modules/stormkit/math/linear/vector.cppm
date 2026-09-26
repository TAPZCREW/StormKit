// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/api.hpp>
#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/macro_utils.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.math.linear.vector;

import std;

import stormkit.core.meta;
import stormkit.core.types;
import stormkit.core.typesafe;
import stormkit.core.ranges;
import stormkit.core.contract;
import stormkit.core.functional;
import stormkit.core.string.format;

import stormkit.math.linear.tensor;

namespace cmeta = stormkit::core::meta;

using namespace stormkit::literals;

#define GEN_SWIZZLE2(a, b)                                                                       \
    template<typename Self>                                                                      \
    STORMKIT_FORCE_INLINE constexpr auto a##b(this Self& self) -> cmeta::remove_const_of<Self> { \
        return cmeta::remove_const_of<Self> {                                                    \
            .storage = { self.a(), self.b() }                                                    \
        };                                                                                       \
    }
#define GEN_SWIZZLE3(a, b, c)                                                                       \
    template<typename Self>                                                                         \
    STORMKIT_FORCE_INLINE constexpr auto a##b##c(this Self& self) -> cmeta::remove_const_of<Self> { \
        return cmeta::remove_const_of<Self> {                                                       \
            .storage = { self.a(), self.b(), self.c() }                                             \
        };                                                                                          \
    }
#define GEN_SWIZZLE4(a, b, c, d)                                                                       \
    template<typename Self>                                                                            \
    STORMKIT_FORCE_INLINE constexpr auto a##b##c##d(this Self& self) -> cmeta::remove_const_of<Self> { \
        return cmeta::remove_const_of<Self> {                                                          \
            .storage = { self.a(), self.b(), self.c(), self.d() }                                      \
        };                                                                                             \
    }

export namespace stormkit::math {
    template<typename T, usize N>
    using vec_view = tensor_view<T, N>;

    template<typename T, usize N>
    struct vec_interface;

    template<typename T>
    struct vec_interface<T, 1> {
        template<typename Self>
        constexpr auto x(this Self& self) -> cmeta::forward_const_to<Self, T>&;
    };

    template<typename T>
    struct vec_interface<T, 2>: vec_interface<T, 1> {
        template<typename Self>
        constexpr auto y(this Self& self) -> cmeta::forward_const_to<Self, T>&;

        STORMKIT_FOR_EACH_COMBINATION(GEN_SWIZZLE2, (x, y), (x, y))
    };

    template<typename T>
    struct vec_interface<T, 3>: vec_interface<T, 2> {
        template<typename Self>
        constexpr auto z(this Self& self) -> cmeta::forward_const_to<Self, T>&;

        STORMKIT_FOR_EACH_COMBINATION(GEN_SWIZZLE3, (x, y, z), (x, y, z), (x, y, z))
    };

    template<typename T>
    struct vec_interface<T, 4>: vec_interface<T, 3> {
        template<typename Self>
        constexpr auto w(this Self& self) -> cmeta::forward_const_to<Self, T>&;

        STORMKIT_FOR_EACH_COMBINATION(GEN_SWIZZLE4, (x, y, z, w), (x, y, z, w), (x, y, z, w), (x, y, z, w))
    };

    template<cmeta::arithmetic T, usize N>
    using vec = tensor<T, vec_interface<T, N>, N>;

    template<cmeta::arithmetic T>
    using vec1  = vec<T, 1>;
    using fvec1 = vec1<f32>;
    using ivec1 = vec1<i32>;
    using uvec1 = vec1<u32>;

    template<cmeta::arithmetic T>
    using vec2  = vec<T, 2>;
    using fvec2 = vec2<f32>;
    using ivec2 = vec2<i32>;
    using uvec2 = vec2<u32>;

    template<cmeta::arithmetic T>
    using vec3  = vec<T, 3>;
    using fvec3 = vec3<f32>;
    using ivec3 = vec3<i32>;
    using uvec3 = vec3<u32>;

    template<cmeta::arithmetic T>
    using vec4  = vec<T, 4>;
    using fvec4 = vec4<f32>;
    using ivec4 = vec4<i32>;
    using uvec4 = vec4<u32>;

    namespace meta {
        template<typename T>
        concept is_vec = is_tensor<T> and T::order() == 1;

        template<typename T>
        concept is_vec_view = is_tensor_view<T> and T::rank() == 1;

        template<typename T>
        concept is_vec_2 = is_vec<T> and T::static_extent(0) == 2;

        template<typename T>
        concept is_vec_3 = is_vec<T> and T::static_extent(0) == 3;

        template<typename T>
        concept is_vec_4 = is_vec<T> and T::static_extent(0) == 4;

        template<typename T>
        concept is_vec_or_view = is_vec<T> or is_vec_view<T>;
    } // namespace meta

    template<typename CharT, cmeta::arithmetic T, usize N>
    constexpr auto tag_invoke(format_as_fn<CharT>, const vec<T, N>& matrix, cmeta::format_context auto& ctx) noexcept
      -> decltype(ctx.out());

    template<typename CharT, cmeta::arithmetic T, usize N>
    constexpr auto tag_invoke(format_as_fn<CharT>, vec_view<const T, N> matrix, cmeta::format_context auto& ctx) noexcept
      -> decltype(ctx.out());

    template<cmeta::arithmetic T, usize N>
    [[nodiscard]]
    constexpr auto normalize(const vec<T, N>& vector) noexcept -> vec<T, N>;

    template<cmeta::arithmetic T, usize N>
    constexpr auto normalize(vec_view<const T, N> a, vec_view<T, N> out) noexcept -> void;

    template<meta::is_vec_or_view T>
    [[nodiscard]]
    constexpr auto dot(const T& a, const T& b) noexcept -> cmeta::value_type<T>;

    template<cmeta::arithmetic T>
    [[nodiscard]]
    constexpr auto cross(const vec<T, 3>& a, const vec<T, 3>& b) noexcept -> vec<T, 3>;

    template<cmeta::arithmetic T>
    constexpr auto cross(vec_view<const T, 3> a, vec_view<const T, 3> b, vec_view<T, 3> out) noexcept -> void;

    template<cmeta::arithmetic T>
    [[nodiscard]]
    constexpr auto cross(const vec<T, 4>& a, const vec<T, 4>& b) noexcept -> vec<T, 4>;

    template<cmeta::arithmetic T>
    constexpr auto cross(vec_view<const T, 4> a, vec_view<const T, 4> b, vec_view<T, 4> out) noexcept -> void;
} // namespace stormkit::math

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::math {
    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    template<typename Self>
       STORMKIT_FORCE_INLINE
    constexpr auto vec_interface<T, 1>::x(this Self& self) -> cmeta::forward_const_to<Self, T>& {
        return self.operator[](0);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    template<typename Self>
       STORMKIT_FORCE_INLINE
    constexpr auto vec_interface<T, 2>::y(this Self& self) -> cmeta::forward_const_to<Self, T>& {
        return self.operator[](1);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    template<typename Self>
       STORMKIT_FORCE_INLINE
    constexpr auto vec_interface<T, 3>::z(this Self& self) -> cmeta::forward_const_to<Self, T>& {
        return self.operator[](2);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    template<typename Self>
       STORMKIT_FORCE_INLINE
    constexpr auto vec_interface<T, 4>::w(this Self& self) -> cmeta::forward_const_to<Self, T>& {
        return self.operator[](3);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename CharT, cmeta::arithmetic T, usize N>
       STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(format_as_fn<CharT>, const vec<T, N>& vector, cmeta::format_context auto& ctx) noexcept
      -> decltype(ctx.out()) {
        return tag_invoke(format_as_fn<CharT> {}, view_of(vector), ctx);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename CharT, cmeta::arithmetic T, usize N>
    constexpr auto tag_invoke(format_as_fn<CharT>, vec_view<const T, N> vector, cmeta::format_context auto& ctx) noexcept
      -> decltype(ctx.out()) {
        format_to(ctx, "[vec");
        format_to(ctx, "{}", N);
        format_to(ctx, ":");

        if constexpr (N >= 1) {
            format_to(ctx, " .x = ");
            format_to(ctx, "{}", vector[0]);
        }
        if constexpr (N >= 2) {
            format_to(ctx, " .y = ");
            format_to(ctx, "{}", vector[1]);
        }
        if constexpr (N >= 3) {
            format_to(ctx, " .z = ");
            format_to(ctx, "{}", vector[2]);
        }
        if constexpr (N >= 4) {
            format_to(ctx, " .w = ");
            format_to(ctx, "{}", vector[3]);
        }
        if constexpr (N >= 5) {
            for (auto i : range(4_usize, N)) {
                format_to(ctx, " .[");
                format_to(ctx, "{}", i);
                format_to(ctx, "] = ");
                format_to(ctx, "{}", vector[i]);
            }
        }

        format_to(ctx, "]");

        return ctx.out();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize N>
       STORMKIT_FORCE_INLINE
    constexpr auto normalize(const vec<T, N>& vector) noexcept -> vec<T, N> {
        auto out = vec<T, N> {};
        normalize(view_of(vector), mutable_view_of(out));
        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize N>
    constexpr auto normalize(vec_view<const T, N> vector, vec_view<T, N> out) noexcept -> void {
        EXPECTS(vector.data_handle() != out.data_handle());

        const auto sum = init_by<T>([vector](auto& sum) noexcept {
            for (auto i : range(N)) sum += (vector[i] * vector[i]);
            sum = unchecked_narrow<T>(std::sqrt(sum));
        });

        for (auto i : range(N)) out[i] = vector[i] / sum;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_vec_or_view T>
    constexpr auto dot(const T& a, const T& b) noexcept -> cmeta::value_type<T> {
        static constexpr auto N = a.extent(0);

        auto out = cmeta::value_type<T> { 0 };
        for (auto i : range(N)) out += (a[i] * b[i]);
        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T>
       STORMKIT_FORCE_INLINE
    constexpr auto cross(const vec<T, 3>& a, const vec<T, 3>& b) noexcept -> vec<T, 3> {
        auto out = vec<T, 3> {};
        cross(view_of(a), view_of(b), mutable_view_of(out));
        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T>
    constexpr auto cross(vec_view<const T, 3> a, vec_view<const T, 3> b, vec_view<T, 3> out) noexcept -> void {
        EXPECTS(a.data_handle() != out.data_handle());
        EXPECTS(b.data_handle() != out.data_handle());

        out[0] = a[1] * b[2] - a[2] * b[1];
        out[1] = a[2] * b[0] - a[0] * b[2];
        out[2] = a[0] * b[1] - a[1] * b[0];
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T>
       STORMKIT_FORCE_INLINE
    constexpr auto cross(const vec<T, 4>& a, const vec<T, 4>& b) noexcept -> vec<T, 4> {
        auto out = vec<T, 4> {};
        cross(view_of(a), view_of(b), mutable_view_of(out));
        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T>
    constexpr auto cross(vec_view<const T, 4> a, vec_view<const T, 4> b, vec_view<T, 4> out) noexcept -> void {
        EXPECTS(a.data_handle() != out.data_handle());
        EXPECTS(b.data_handle() != out.data_handle());

        out[0] = a[1] * b[2] - a[2] * b[1];
        out[1] = a[2] * b[0] - a[0] * b[2];
        out[2] = a[0] * b[1] - a[1] * b[0];
        out[3] = T { 0 };
    }
} // namespace stormkit::math
