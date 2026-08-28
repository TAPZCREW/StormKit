// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/api.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.math.linear.matrix;

import std;

import stormkit.core.meta;
import stormkit.core.types;
import stormkit.core.typesafe;
import stormkit.core.hash;
import stormkit.core.string;
import stormkit.core.ranges;

import stormkit.math.linear;
import stormkit.math.linear.vector;

export {
    namespace stormkit { inline namespace core { namespace math {
        inline namespace matrix {
            // M => rows
            // N => columns
            template<core::meta::arithmetic T, usize M, usize N>
            struct alignas(array<T, M * N>) mat {
                using value_type   = T;
                using StorageType = array<value_type, M * N>;
                using SizeType    = usize;
                using ExtentType  = u8;

                using value_type   = value_type;
                using storage_type = StorageType;
                using size_type    = SizeType;
                using extent_type  = ExtentType;

                static constexpr auto EXTENTS = array<ExtentType, 2> { M, N };

                StorageType values;

                template<typename Self>
                [[nodiscard]]
                constexpr auto operator[](this Self&& self, SizeType i) noexcept -> core::meta::forward_like<Self, value_type>&;

                template<typename Self>
                [[nodiscard]]
                constexpr auto operator[](this Self&& self, SizeType i, SizeType j) noexcept
                  -> core::meta::forward_like<Self, value_type>&;

                template<typename Self>
                [[nodiscard]]
                constexpr auto begin(this Self& self) noexcept -> decltype(auto);
                [[nodiscard]]
                constexpr auto cbegin() const noexcept -> decltype(auto);

                template<typename Self>
                [[nodiscard]]
                constexpr auto end(this Self& self) noexcept -> decltype(auto);
                [[nodiscard]]
                constexpr auto cend() const noexcept -> decltype(auto);

                template<typename Self>
                [[nodiscard]]
                constexpr auto data(this Self& self) noexcept -> core::meta::forward_const_to<Self, value_type>*;

                [[nodiscard]]
                constexpr auto size() const noexcept -> SizeType;
                [[nodiscard]]
                static consteval auto max_size() noexcept -> SizeType;

                [[nodiscard]]
                static constexpr auto identity() noexcept -> mat<value_type, M, N>
                    requires(M == N);
            };

            template<typename T>
            using mat2x2 = mat<T, 2, 2>;
            template<typename T>
            using mat3x3 = mat<T, 3, 3>;
            template<typename T>
            using mat4x4 = mat<T, 4, 4>;
            template<typename T>
            using mat2x3 = mat<T, 2, 3>;
            template<typename T>
            using mat3x2 = mat<T, 3, 2>;
            template<typename T>
            using mat4x3 = mat<T, 4, 3>;
            template<typename T>
            using mat3x4 = mat<T, 3, 4>;
            template<typename T>
            using mat4x2 = mat<T, 4, 2>;
            template<typename T>
            using mat2x4 = mat<T, 2, 4>;

            using fmat2 = mat2x2<f32>;
            using imat2 = mat2x2<i32>;
            using umat2 = mat2x2<u32>;

            using fmat3 = mat3x3<f32>;
            using imat3 = mat3x3<i32>;
            using umat3 = mat3x3<u32>;

            using fmat4 = mat4x4<f32>;
            using imat4 = mat4x4<i32>;
            using umat4 = mat4x4<u32>;

            using fmat2x3 = mat2x3<f32>;
            using imat2x3 = mat2x3<i32>;
            using umat2x3 = mat2x3<u32>;

            using fmat3x2 = mat3x2<f32>;
            using imat3x2 = mat3x2<i32>;
            using umat3x2 = mat3x2<u32>;

            using fmat4x3 = mat4x3<f32>;
            using imat4x3 = mat4x3<i32>;
            using umat4x3 = mat4x3<u32>;

            using fmat3x4 = mat3x4<f32>;
            using imat3x4 = mat3x4<i32>;
            using umat3x4 = mat3x4<u32>;

            using fmat4x2 = mat4x2<f32>;
            using imat4x2 = mat4x2<i32>;
            using umat4x2 = mat4x2<u32>;

            using fmat2x4 = mat2x4<f32>;
            using imat2x4 = mat2x4<i32>;
            using umat2x4 = mat2x4<u32>;
        } // namespace matrix

        namespace meta {
            template<typename T>
            concept IsMat2 = core::meta::specialization_of<T, mat2x2>;

            template<typename T>
            concept IsMat3 = core::meta::specialization_of<T, mat3x3>;

            template<typename T>
            concept IsMat4 = core::meta::specialization_of<T, mat4x4>;

            template<typename T>
            concept IsMat = core::meta::IsSpecializationWithNTTPOf<T, mat>;

            template<typename T>
            concept IsSquareMat = IsMat<T> and T::EXTENTS[0] == T::EXTENTS[1];

            template<typename T, typename U>
            concept HasOneMatType = not(core::meta::std_mdspan<T> and core::meta::std_mdspan<U>)
                                    or meta::IsMat<T>
                                    or meta::IsMat<U>;
        } // namespace meta

        inline namespace matrix {
            template<meta::IsSquareMat T>
            [[nodiscard]]
            constexpr auto determinant(const T& mat) noexcept -> typename T::value_type;

            template<meta::IsSquareMat T>
            constexpr auto transpose(const T& mat) noexcept -> T;

            template<meta::IsMat T>
            [[nodiscard]]
            constexpr auto is_inversible(const T& mat) noexcept -> bool;

            template<meta::IsSquareMat T>
            constexpr auto inverse(const T& mat) noexcept -> T;

            template<meta::IsMat T>
            [[nodiscard]]
            constexpr auto is_orthogonal(const T& mat) noexcept -> bool;

            template<meta::IsMat T>
            [[nodiscard]]
            constexpr auto mul(const T& a, typename T::value_type b) noexcept -> T;

            template<meta::IsMat T>
            [[nodiscard]]
            constexpr auto div(const T& a, typename T::value_type b) noexcept -> T;

            template<meta::IsMat T>
            [[nodiscard]]
            constexpr auto mul(const T& a, const T& b) noexcept -> T;

            template<meta::IsMat T, meta::IsSquareMat U>
                requires(meta::same_as<typename T::value_type, typename U::value_type>)
            [[nodiscard]]
            constexpr auto div(const T& a, const U& b) noexcept -> U;

            template<typename T>
            [[nodiscard]]
            constexpr auto translate(const mat4x4<T>& mat, const vec3<T>& translation) noexcept -> mat4x4<T>;

            template<typename T>
            [[nodiscard]]
            constexpr auto scale(const mat4x4<T>& mat, const vec3<T>& scale) noexcept -> mat4x4<T>;

            template<typename T>
            [[nodiscard]]
            constexpr auto rotate(const mat4x4<T>& mat, angle::radian<T> angle, const vec3<T>& axis) noexcept -> mat4x4<T>;

            template<core::meta::arithmetic T>
            [[nodiscard]]
            constexpr auto orthographique(T left, T right, T bottom, T top, T near, T far) noexcept -> mat4x4<T>;

            template<core::meta::arithmetic T>
            [[nodiscard]]
            constexpr auto orthographique(T left, T right, T bottom, T top) noexcept -> mat4x4<T>;

            template<core::meta::arithmetic T>
            [[nodiscard]]
            constexpr auto perspective(angle::radian<T> fov_y, T aspect, T near, T far) noexcept -> mat4x4<T>;

            template<typename T>
            [[nodiscard]]
            constexpr auto look_at(const vec3<T>& eye, const vec3<T>& center, const vec3<T>& up) noexcept -> mat4x4<T>;

            template<meta::IsMat T>
            [[nodiscard]]
            constexpr auto as_view(const T& value) noexcept
              -> array_view<const typename T::value_type, T::EXTENTS[0] * T::EXTENTS[1]>;

            template<meta::IsMat T>
            [[nodiscard]]
            constexpr auto as_view_mut(T& value) noexcept -> array_view<typename T::value_type, T::EXTENTS[0] * T::EXTENTS[1]>;

            template<meta::IsMat T>
            [[nodiscard]]
            constexpr auto as_mdspan(const T& value) noexcept
              -> MatrixSpan<const typename T::value_type, T::EXTENTS[0], T::EXTENTS[1]>;

            template<meta::IsMat T>
                requires(not core::meta::const_type<T>)
            [[nodiscard]]
            constexpr auto as_mdspan(T& value) noexcept -> MatrixSpan<typename T::value_type, T::EXTENTS[0], T::EXTENTS[1]>;

            template<meta::IsMat T>
            auto to_string(const T& value) noexcept -> string;

            template<core::meta::hash_type Ret = hash32, meta::IsMat T>
            constexpr auto hasher(const T& value) noexcept -> Ret;

            template<meta::IsMat T, typename FormatContext>
            auto format_as(const T& value, FormatContext& ctx) noexcept -> decltype(ctx.out());
        } // namespace matrix
    }}} // namespace stormkit::core::math

    namespace std {
        template<stormkit::math::meta::IsMat T>
        constexpr range_format format_kind<T> = range_format::disabled;
    } // namespace std
}

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stdr = std::ranges;
namespace stdv = std::views;

namespace stormkit { inline namespace core { namespace math { inline namespace matrix {
    static_assert(sizeof(mat2x2<i32>) == sizeof(i32) * 2 * 2);
    static_assert(sizeof(mat2x2<i64>) == sizeof(i64) * 2 * 2);
    static_assert(sizeof(mat2x2<u32>) == sizeof(u32) * 2 * 2);
    static_assert(sizeof(mat2x2<u64>) == sizeof(u64) * 2 * 2);
    static_assert(sizeof(mat2x2<f32>) == sizeof(f32) * 2 * 2);
    static_assert(sizeof(mat2x2<f64>) == sizeof(f64) * 2 * 2);

    static_assert(sizeof(mat3x3<i32>) == sizeof(i32) * 3 * 3);
    static_assert(sizeof(mat3x3<i64>) == sizeof(i64) * 3 * 3);
    static_assert(sizeof(mat3x3<u32>) == sizeof(u32) * 3 * 3);
    static_assert(sizeof(mat3x3<u64>) == sizeof(u64) * 3 * 3);
    static_assert(sizeof(mat3x3<f32>) == sizeof(f32) * 3 * 3);
    static_assert(sizeof(mat3x3<f64>) == sizeof(f64) * 3 * 3);

    static_assert(sizeof(mat4x4<i32>) == sizeof(i32) * 4 * 4);
    static_assert(sizeof(mat4x4<i64>) == sizeof(i64) * 4 * 4);
    static_assert(sizeof(mat4x4<u32>) == sizeof(u32) * 4 * 4);
    static_assert(sizeof(mat4x4<u64>) == sizeof(u64) * 4 * 4);
    static_assert(sizeof(mat4x4<f32>) == sizeof(f32) * 4 * 4);
    static_assert(sizeof(mat4x4<f64>) == sizeof(f64) * 4 * 4);

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T, usize M, usize N>
    template<typename Self>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto mat<T, M, N>::operator[](this Self&& self, SizeType i) noexcept -> core::meta::forward_like<Self, value_type>& {
        return std::forward_like<Self&>(self.values[i]);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T, usize M, usize N>
    template<typename Self>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto mat<T, M, N>::operator[](this Self&& self, SizeType i, SizeType j) noexcept
      -> core::meta::forward_like<Self, value_type>& {
        return std::forward_like<Self&>(self.operator[](((i * EXTENTS[0]) + j)));
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T, usize M, usize N>
    template<typename Self>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto mat<T, M, N>::begin(this Self& self) noexcept -> decltype(auto) {
        return stdr::begin(self.values);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T, usize M, usize N>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto mat<T, M, N>::cbegin() const noexcept -> decltype(auto) {
        return stdr::cbegin(values);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T, usize M, usize N>
    template<typename Self>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto mat<T, M, N>::end(this Self& self) noexcept -> decltype(auto) {
        return stdr::end(self.values);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T, usize M, usize N>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto mat<T, M, N>::cend() const noexcept -> decltype(auto) {
        return stdr::cend(values);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T, usize M, usize N>
    template<typename Self>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto mat<T, M, N>::data(this Self& self) noexcept -> core::meta::forward_const_to<Self, value_type>* {
        return stdr::data(self.values);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T, usize M, usize N>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto mat<T, M, N>::size() const noexcept -> SizeType {
        return stdr::size(values);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T, usize M, usize N>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    consteval auto mat<T, M, N>::max_size() noexcept -> SizeType {
        return M * N;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T, usize M, usize N>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto mat<T, M, N>::identity() noexcept -> mat<value_type, M, N>
        requires(M == N)
    {
        auto matrix = mat<value_type, M, N> {};

        for (auto i = 0u; i < M; ++i) { matrix[i, i] = T { 1 }; }

        return matrix;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::IsSquareMat T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto determinant(const T& mat) noexcept -> typename T::value_type {
        return math::determinant(as_mdspan(mat));
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::IsSquareMat T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto transpose(const T& mat) noexcept -> T {
        auto out = T {};
        math::transpose(as_mdspan(mat), as_mdspan_mut(out));
        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::IsMat T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto is_inversible(const T& mat) noexcept -> bool {
        return math::is_inversible(as_mdspan(mat));
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::IsSquareMat T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto inverse(const T& mat) noexcept -> T {
        auto out = T {};
        math::inverse(as_mdspan(mat), as_mdspan_mut(out));
        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::IsMat T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto is_orthogonal(const T& mat) noexcept -> bool {
        return math::is_orthogonal(as_mdspan(mat));
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::IsMat T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto mul(const T& a, typename T::value_type b) noexcept -> T {
        auto out = T {};

        math::mul(as_mdspan(a), b, as_mdspan_mut(out));

        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::IsMat T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto div(const T& a, typename T::value_type b) noexcept -> T {
        auto out = T {};

        math::div(as_mdspan(a), b, as_mdspan_mut(out));

        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::IsMat T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto mul(const T& a, const T& b) noexcept -> T {
        auto out = T {};

        math::mul(as_mdspan(a), as_mdspan(b), as_mdspan_mut(out));

        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::IsMat T, meta::IsSquareMat U>
        requires(meta::same_as<typename T::value_type, typename U::value_type>)
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto div(const T& a, const U& b) noexcept -> U {
        auto out = T {};

        math::div(as_mdspan(a), as_mdspan(b), as_mdspan_mut(out));

        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto translate(const mat4x4<T>& mat, const vec3<T>& translation) noexcept -> mat4x4<T> {
        auto out = auto(mat);

        math::translate(as_mdspan(mat), as_mdspan(translation), as_mdspan_mut(out));

        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto scale(const mat4x4<T>& mat, const vec3<T>& scale_factors) noexcept -> mat4x4<T> {
        auto out = auto(mat);

        math::scale(as_mdspan(mat), as_mdspan(scale_factors), as_mdspan_mut(out));

        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto rotate(const mat4x4<T>& mat, angle::radian<T> angle, const vec3<T>& axis) noexcept -> mat4x4<T> {
        auto out = auto(mat);

        const auto cos = unchecked_narrow<T>(std::cos(angle.get()));
        const auto sin = unchecked_narrow<T>(std::sin(angle.get()));

        const auto axis_norm = normalize(axis);
        const auto temp      = mul(axis_norm, T { 1 } - cos);

        auto rotation_matrix  = mat4x4<T> {};
        rotation_matrix[0, 0] = cos + temp[0] * axis_norm[0];
        rotation_matrix[1, 0] = temp[0] * axis_norm[1] + sin * axis_norm[2];
        rotation_matrix[2, 0] = temp[0] * axis_norm[2] - sin * axis_norm[1];

        rotation_matrix[0, 1] = temp[1] * axis_norm[0] - sin * axis_norm[2];
        rotation_matrix[1, 1] = cos + temp[1] * axis_norm[1];
        rotation_matrix[2, 1] = temp[1] * axis_norm[2] + sin * axis_norm[0];

        rotation_matrix[0, 2] = temp[2] * axis_norm[0] + sin * axis_norm[1];
        rotation_matrix[1, 2] = temp[2] * axis_norm[1] - sin * axis_norm[0];
        rotation_matrix[2, 2] = cos + temp[2] * axis_norm[2];

        out[0, 0] = mat[0, 0] * rotation_matrix[0, 0] + mat[1, 0] * rotation_matrix[1, 0] + mat[2, 0] * rotation_matrix[2, 0];
        out[0, 1] = mat[0, 1] * rotation_matrix[0, 0] + mat[1, 1] * rotation_matrix[1, 0] + mat[2, 1] * rotation_matrix[2, 0];
        out[0, 2] = mat[0, 2] * rotation_matrix[0, 0] + mat[1, 2] * rotation_matrix[1, 0] + mat[2, 2] * rotation_matrix[2, 0];

        out[1, 0] = mat[0, 0] * rotation_matrix[0, 1] + mat[1, 0] * rotation_matrix[1, 1] + mat[2, 0] * rotation_matrix[2, 1];
        out[1, 1] = mat[0, 1] * rotation_matrix[0, 1] + mat[1, 1] * rotation_matrix[1, 1] + mat[2, 1] * rotation_matrix[2, 1];
        out[1, 2] = mat[0, 2] * rotation_matrix[0, 1] + mat[1, 2] * rotation_matrix[1, 1] + mat[2, 2] * rotation_matrix[2, 1];

        out[2, 0] = mat[0, 0] * rotation_matrix[0, 2] + mat[1, 0] * rotation_matrix[1, 2] + mat[2, 0] * rotation_matrix[2, 2];
        out[2, 1] = mat[0, 1] * rotation_matrix[0, 2] + mat[1, 1] * rotation_matrix[1, 2] + mat[2, 1] * rotation_matrix[2, 2];
        out[2, 2] = mat[0, 2] * rotation_matrix[0, 2] + mat[1, 2] * rotation_matrix[1, 2] + mat[2, 2] * rotation_matrix[2, 2];

        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto orthographique(T left, T right, T bottom, T top, T near, T far) noexcept -> mat4x4<T> {
        auto out = mat4x4<T>::identity();

        math::orthographique(left, right, top, bottom, near, far, as_mdspan_mut(out));

        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto orthographique(T left, T right, T bottom, T top) noexcept -> mat4x4<T> {
        auto out = mat4x4<T>::identity();

        math::orthographique(left, right, bottom, top, as_mdspan_mut(out));

        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto perspective(angle::radian<T> fov_y, T aspect, T near, T far) noexcept -> mat4x4<T> {
        auto out = mat4x4<T> {};

        math::perspective(fov_y, aspect, near, far, as_mdspan_mut(out));

        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto look_at(const vec3<T>& eye, const vec3<T>& center, const vec3<T>& up) noexcept -> mat4x4<T> {
        auto out = mat4x4<T> {};

        math::look_at(as_mdspan(eye), as_mdspan(center), as_mdspan(up), as_mdspan_mut(out));

        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::IsMat T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto as_view(const T& value) noexcept -> array_view<const typename T::value_type, T::EXTENTS[0] * T::EXTENTS[1]> {
        return array_view<const typename T::value_type, T::EXTENTS[0] * T::EXTENTS[1]> {
            stdr::data(value),
            T::EXTENTS[0] * T::EXTENTS[1]
        };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::IsMat T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto as_view_mut(T& value) noexcept -> array_view<typename T::value_type, T::EXTENTS[0] * T::EXTENTS[1]> {
        return array_view<typename T::value_type, T::EXTENTS[0] * T::EXTENTS[1]> {
            stdr::data(value),
            T::EXTENTS[0] * T::EXTENTS[1]
        };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::IsMat T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto as_mdspan(const T& value) noexcept -> MatrixSpan<const typename T::value_type, T::EXTENTS[0], T::EXTENTS[1]> {
        return MatrixSpan<const typename T::value_type, T::EXTENTS[0], T::EXTENTS[1]> { stdr::data(value), T::EXTENTS };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::IsMat T>
        requires(not core::meta::const_type<T>)
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto as_mdspan_mut(T& value) noexcept -> MatrixSpan<typename T::value_type, T::EXTENTS[0], T::EXTENTS[1]> {
        return MatrixSpan<typename T::value_type, T::EXTENTS[0], T::EXTENTS[1]> { stdr::data(value), T::EXTENTS };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::IsMat T>
    inline auto to_string(const T& value) noexcept -> string {
        return std::format("{}", value);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::hash_type Ret, meta::IsMat T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto hasher(const T& value) noexcept -> Ret {
        return hash<Ret>(as_view(value));
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::IsMat T, typename FormatContext>
    inline auto format_as(const T& mat, FormatContext& ctx) noexcept -> decltype(ctx.out()) {
        auto out = ctx.out();

        auto max_digit = 0u;
        if constexpr (stormkit::meta::signed_type<typename T::value_type>)
            for (auto v : as_view(mat)) {
                max_digit = std::max(max_digit, unchecked_narrow<i64>(v) == 0 ? 2 : unchecked_narrow<u32>(std::log10(v) + 2));
                if (v < 0) max_digit = max_digit + 1;
            }
        else
            for (auto v : as_view(mat)) max_digit = std::max(max_digit, unchecked_narrow<i64>(v) == 0 ? 2 : unchecked_narrow<u32>(std::log10(v) + 2));

        format_to(out, "[mat ");

        for (auto i : range<usize>(T::EXTENTS[0] * T::EXTENTS[1])) {
            const auto row = i / T::EXTENTS[1];
            const auto col = i % T::EXTENTS[1];

            if (row != 0 && col == 0) format_to(out, "     ");

            if constexpr (stormkit::meta::integral<typename T::value_type>) {
                if (col < T::EXTENTS[1] - 1) format_to(out, "{:>{}}, ", mat[i], max_digit);
                else {
                    if (row < T::EXTENTS[0] - 1) format_to(out, "{:>{}}\n", mat[i], max_digit);
                    else
                        format_to(out, "{:>{}}]", mat[i], max_digit);
                }
            } else {
                if (col < T::EXTENTS[1] - 1) format_to(out, "{:>{}.5f}, ", mat[i], max_digit);
                else {
                    if (row < T::EXTENTS[0] - 1) format_to(out, "{:>{}.5f}\n", mat[i], max_digit);
                    else
                        format_to(out, "{:>{}.5f}]", mat[i], max_digit);
                }
            }
        }

        return out;
    }

#ifndef STORMKIT_OS_WINDOWS
    #undef STORMKIT_CORE_API
    #define STORMKIT_CORE_API
#endif

#define DETERMINANT_INSTANCIATE(mat_type) \
    template STORMKIT_CORE_API auto determinant<mat_type>(const mat_type&) noexcept -> typename mat_type::value_type

    DETERMINANT_INSTANCIATE(fmat2);
    DETERMINANT_INSTANCIATE(fmat3);
    DETERMINANT_INSTANCIATE(fmat4);
    DETERMINANT_INSTANCIATE(umat2);
    DETERMINANT_INSTANCIATE(umat3);
    DETERMINANT_INSTANCIATE(umat4);
    DETERMINANT_INSTANCIATE(imat2);
    DETERMINANT_INSTANCIATE(imat3);
    DETERMINANT_INSTANCIATE(imat4);

#undef DETERMINANT_INSTANCIATE

#define TRANSPOSE_INSTANCIATE(mat_type) template STORMKIT_CORE_API auto transpose<mat_type>(const mat_type&) noexcept -> mat_type

    TRANSPOSE_INSTANCIATE(fmat2);
    TRANSPOSE_INSTANCIATE(fmat3);
    TRANSPOSE_INSTANCIATE(fmat4);
    TRANSPOSE_INSTANCIATE(umat2);
    TRANSPOSE_INSTANCIATE(umat3);
    TRANSPOSE_INSTANCIATE(umat4);
    TRANSPOSE_INSTANCIATE(imat2);
    TRANSPOSE_INSTANCIATE(imat3);
    TRANSPOSE_INSTANCIATE(imat4);

#undef TRANSPOSE_INSTANCIATE

#define IS_INVERSIBLE_INSTANCIATE(mat_type) \
    template STORMKIT_CORE_API auto is_inversible<mat_type>(const mat_type&) noexcept -> bool

    IS_INVERSIBLE_INSTANCIATE(fmat2);
    IS_INVERSIBLE_INSTANCIATE(fmat2x3);
    IS_INVERSIBLE_INSTANCIATE(fmat2x4);
    IS_INVERSIBLE_INSTANCIATE(fmat3);
    IS_INVERSIBLE_INSTANCIATE(fmat3x2);
    IS_INVERSIBLE_INSTANCIATE(fmat3x4);
    IS_INVERSIBLE_INSTANCIATE(fmat4);
    IS_INVERSIBLE_INSTANCIATE(fmat4x2);
    IS_INVERSIBLE_INSTANCIATE(fmat4x3);
    IS_INVERSIBLE_INSTANCIATE(umat2);
    IS_INVERSIBLE_INSTANCIATE(umat2x3);
    IS_INVERSIBLE_INSTANCIATE(umat2x4);
    IS_INVERSIBLE_INSTANCIATE(umat3);
    IS_INVERSIBLE_INSTANCIATE(umat3x2);
    IS_INVERSIBLE_INSTANCIATE(umat3x4);
    IS_INVERSIBLE_INSTANCIATE(umat4);
    IS_INVERSIBLE_INSTANCIATE(umat4x2);
    IS_INVERSIBLE_INSTANCIATE(umat4x3);
    IS_INVERSIBLE_INSTANCIATE(imat2);
    IS_INVERSIBLE_INSTANCIATE(imat2x3);
    IS_INVERSIBLE_INSTANCIATE(imat2x4);
    IS_INVERSIBLE_INSTANCIATE(imat3);
    IS_INVERSIBLE_INSTANCIATE(imat3x2);
    IS_INVERSIBLE_INSTANCIATE(imat3x4);
    IS_INVERSIBLE_INSTANCIATE(imat4);
    IS_INVERSIBLE_INSTANCIATE(imat4x2);
    IS_INVERSIBLE_INSTANCIATE(imat4x3);

#undef IS_INVERSIBLE_INSTANCIATE

#define INVERSE_INSTANCIATE(mat_type) template STORMKIT_CORE_API auto inverse<mat_type>(const mat_type&) noexcept -> mat_type

    INVERSE_INSTANCIATE(fmat2);
    INVERSE_INSTANCIATE(fmat3);
    INVERSE_INSTANCIATE(fmat4);
    INVERSE_INSTANCIATE(umat2);
    INVERSE_INSTANCIATE(umat3);
    INVERSE_INSTANCIATE(umat4);
    INVERSE_INSTANCIATE(imat2);
    INVERSE_INSTANCIATE(imat3);
    INVERSE_INSTANCIATE(imat4);

#undef INVERSE_INSTANCIATE

#define IS_ORTHOGONAL_INSTANCIATE(mat_type) \
    template STORMKIT_CORE_API auto is_orthogonal<mat_type>(const mat_type&) noexcept -> bool

    IS_ORTHOGONAL_INSTANCIATE(fmat2);
    IS_ORTHOGONAL_INSTANCIATE(fmat2x3);
    IS_ORTHOGONAL_INSTANCIATE(fmat2x4);
    IS_ORTHOGONAL_INSTANCIATE(fmat3);
    IS_ORTHOGONAL_INSTANCIATE(fmat3x2);
    IS_ORTHOGONAL_INSTANCIATE(fmat3x4);
    IS_ORTHOGONAL_INSTANCIATE(fmat4);
    IS_ORTHOGONAL_INSTANCIATE(fmat4x2);
    IS_ORTHOGONAL_INSTANCIATE(fmat4x3);
    IS_ORTHOGONAL_INSTANCIATE(umat2);
    IS_ORTHOGONAL_INSTANCIATE(umat2x3);
    IS_ORTHOGONAL_INSTANCIATE(umat2x4);
    IS_ORTHOGONAL_INSTANCIATE(umat3);
    IS_ORTHOGONAL_INSTANCIATE(umat3x2);
    IS_ORTHOGONAL_INSTANCIATE(umat3x4);
    IS_ORTHOGONAL_INSTANCIATE(umat4);
    IS_ORTHOGONAL_INSTANCIATE(umat4x2);
    IS_ORTHOGONAL_INSTANCIATE(umat4x3);
    IS_ORTHOGONAL_INSTANCIATE(imat2);
    IS_ORTHOGONAL_INSTANCIATE(imat2x3);
    IS_ORTHOGONAL_INSTANCIATE(imat2x4);
    IS_ORTHOGONAL_INSTANCIATE(imat3);
    IS_ORTHOGONAL_INSTANCIATE(imat3x2);
    IS_ORTHOGONAL_INSTANCIATE(imat3x4);
    IS_ORTHOGONAL_INSTANCIATE(imat4);
    IS_ORTHOGONAL_INSTANCIATE(imat4x2);
    IS_ORTHOGONAL_INSTANCIATE(imat4x3);

#undef IS_ORTHOGONAL_INSTANCIATE

#define MUL_INSTANCIATE(mat_type) \
    template STORMKIT_CORE_API auto mul<mat_type>(const mat_type&, typename mat_type::value_type) noexcept -> mat_type

    MUL_INSTANCIATE(fmat2);
    MUL_INSTANCIATE(fmat2x3);
    MUL_INSTANCIATE(fmat2x4);
    MUL_INSTANCIATE(fmat3);
    MUL_INSTANCIATE(fmat3x2);
    MUL_INSTANCIATE(fmat3x4);
    MUL_INSTANCIATE(fmat4);
    MUL_INSTANCIATE(fmat4x2);
    MUL_INSTANCIATE(fmat4x3);
    MUL_INSTANCIATE(umat2);
    MUL_INSTANCIATE(umat2x3);
    MUL_INSTANCIATE(umat2x4);
    MUL_INSTANCIATE(umat3);
    MUL_INSTANCIATE(umat3x2);
    MUL_INSTANCIATE(umat3x4);
    MUL_INSTANCIATE(umat4);
    MUL_INSTANCIATE(umat4x2);
    MUL_INSTANCIATE(umat4x3);
    MUL_INSTANCIATE(imat2);
    MUL_INSTANCIATE(imat2x3);
    MUL_INSTANCIATE(imat2x4);
    MUL_INSTANCIATE(imat3);
    MUL_INSTANCIATE(imat3x2);
    MUL_INSTANCIATE(imat3x4);
    MUL_INSTANCIATE(imat4);
    MUL_INSTANCIATE(imat4x2);
    MUL_INSTANCIATE(imat4x3);

#undef MUL_INSTANCIATE

#define DIV_INSTANCIATE(mat_type) \
    template STORMKIT_CORE_API auto div<mat_type>(const mat_type&, typename mat_type::value_type) noexcept -> mat_type

    DIV_INSTANCIATE(fmat2);
    DIV_INSTANCIATE(fmat2x3);
    DIV_INSTANCIATE(fmat2x4);
    DIV_INSTANCIATE(fmat3);
    DIV_INSTANCIATE(fmat3x2);
    DIV_INSTANCIATE(fmat3x4);
    DIV_INSTANCIATE(fmat4);
    DIV_INSTANCIATE(fmat4x2);
    DIV_INSTANCIATE(fmat4x3);
    DIV_INSTANCIATE(umat2);
    DIV_INSTANCIATE(umat2x3);
    DIV_INSTANCIATE(umat2x4);
    DIV_INSTANCIATE(umat3);
    DIV_INSTANCIATE(umat3x2);
    DIV_INSTANCIATE(umat3x4);
    DIV_INSTANCIATE(umat4);
    DIV_INSTANCIATE(umat4x2);
    DIV_INSTANCIATE(umat4x3);
    DIV_INSTANCIATE(imat2);
    DIV_INSTANCIATE(imat2x3);
    DIV_INSTANCIATE(imat2x4);
    DIV_INSTANCIATE(imat3);
    DIV_INSTANCIATE(imat3x2);
    DIV_INSTANCIATE(imat3x4);
    DIV_INSTANCIATE(imat4);
    DIV_INSTANCIATE(imat4x2);
    DIV_INSTANCIATE(imat4x3);

#undef DIV_INSTANCIATE

#define TRANSLATE_INSTANCIATE(type) \
    template STORMKIT_CORE_API auto translate<type>(const mat4x4<type>&, const vec3<type>&) noexcept -> mat4x4<type>

    TRANSLATE_INSTANCIATE(f32);
    TRANSLATE_INSTANCIATE(u32);
    TRANSLATE_INSTANCIATE(i32);

#undef TRANSLATE_INSTANCIATE

#define SCALE_INSTANCIATE(type) \
    template STORMKIT_CORE_API auto scale<type>(const mat4x4<type>&, const vec3<type>&) noexcept -> mat4x4<type>

    SCALE_INSTANCIATE(f32);
    SCALE_INSTANCIATE(u32);
    SCALE_INSTANCIATE(i32);

#undef SCALE_INSTANCIATE

#define ROTATE_INSTANCIATE(type)                                                                                       \
    template STORMKIT_CORE_API auto rotate<type>(const mat4x4<type>&, angle::radian<type>, const vec3<type>&) noexcept \
      -> mat4x4<type>

    ROTATE_INSTANCIATE(f32);
    // ROTATE_INSTANCIATE(i32);

#undef ROTATE_INSTANCIATE

#define ORTHOGRAPHIQUE_INSTANCIATE(type)                                                                   \
    template STORMKIT_CORE_API auto orthographique<type>(type, type, type, type) noexcept -> mat4x4<type>; \
    template STORMKIT_CORE_API auto orthographique<type>(type, type, type, type, type, type) noexcept -> mat4x4<type>

    ORTHOGRAPHIQUE_INSTANCIATE(f32);
    ORTHOGRAPHIQUE_INSTANCIATE(i32);

#undef ORTHOGRAPHIQUE_INSTANCIATE

#define PERSPECTIVE_INSTANCIATE(type) \
    template STORMKIT_CORE_API auto perspective<type>(angle::radian<type>, type, type, type) noexcept -> mat4x4<type>

    PERSPECTIVE_INSTANCIATE(f32);
    // PERSPECTIVE_INSTANCIATE(i32);

#undef PERSPECTIVE_INSTANCIATE

#define LOOK_AT_INSTANCIATE(type)                                                                                   \
    template STORMKIT_CORE_API auto look_at<type>(const vec3<type>&, const vec3<type>&, const vec3<type>&) noexcept \
      -> mat4x4<type>

    LOOK_AT_INSTANCIATE(f32);
    LOOK_AT_INSTANCIATE(i32);

#undef LOOK_AT_INSTANCIATE
}}}} // namespace stormkit::core::math::matrix
