// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/api.hpp>
#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.math.linear.matrix;

import std;

import stormkit.core.meta;
import stormkit.core.types;
import stormkit.core.typesafe;
import stormkit.core.ranges;
import stormkit.core.contract;
import stormkit.core.functional;
import stormkit.core.string.format;

import stormkit.math.arithmetic;
import stormkit.math.linear.tensor;
import stormkit.math.linear.vector;
import stormkit.math.linear.angle;

namespace cmeta = stormkit::core::meta;
namespace stdr  = std::ranges;
namespace stdv  = std::views;

using namespace std::literals;
using namespace stormkit::core::literals;

export namespace stormkit::math {
    template<typename T, usize M, usize N>
    using mat_view = tensor_view<T, M, N>;

    template<typename T, usize N>
    using square_mat_view = mat_view<T, N, N>;

    // M => rows
    // N => columns
    template<cmeta::arithmetic T, usize M, usize N>
    struct mat_interface {
        [[nodiscard]]
        static constexpr auto identity() noexcept -> decltype(auto)
            requires(M == N);

        template<typename Self>
        [[nodiscard]]
        constexpr auto row(this Self& self, usize id) noexcept -> array_view<cmeta::forward_const_to<Self, T>, M>;
    };

    template<cmeta::arithmetic T, usize M, usize N>
    using mat = tensor<T, mat_interface<T, M, N>, M, N>;

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

    namespace meta {
        template<typename T>
        concept is_mat = is_tensor<T> and T::order() == 2;

        template<typename T>
        concept is_mat_view = is_tensor_view<T> and T::rank() == 2;

        template<typename T>
        concept is_square_mat = is_mat<T> and (T::EXTENTS[0] == T::EXTENTS[1]);

        template<typename T>
        concept is_square_mat_view = is_mat_view<T> and T::static_extent(0) == T::static_extent(1);

        template<typename T>
        concept is_mat_2 = is_square_mat<T> and T::static_extent(0) == 2;

        template<typename T>
        concept is_mat_3 = is_square_mat<T> and T::static_extent(0) == 3;

        template<typename T>
        concept is_mat_4 = is_square_mat<T> and T::static_extent(0) == 4;

        template<typename T>
        concept is_mat_or_view = is_mat<T> or is_mat_view<T>;

        template<typename T>
        concept is_square_mat_or_view = is_square_mat<T> or is_square_mat_view<T>;

        namespace plain {
            template<typename T>
            concept is_mat_or_view = meta::is_mat_or_view<cmeta::to_plain_type<T>>;

            template<typename T>
            concept is_square_mat_or_view = meta::is_square_mat_or_view<cmeta::to_plain_type<T>>;
        } // namespace plain
    } // namespace meta

    template<typename CharT, cmeta::arithmetic T, usize M, usize N>
    constexpr auto tag_invoke(format_as_fn<CharT>, const mat<T, M, N>& matrix, cmeta::format_context auto& ctx) noexcept
      -> decltype(ctx.out());

    template<typename CharT, cmeta::arithmetic T, usize M, usize N>
    constexpr auto tag_invoke(format_as_fn<CharT>, mat_view<const T, M, N> matrix, cmeta::format_context auto& ctx) noexcept
      -> decltype(ctx.out());

    template<meta::is_square_mat_or_view T>
    [[nodiscard]]
    constexpr auto determinant(const T& matrix) noexcept -> cmeta::remove_const_of<cmeta::value_type<T>>;

    template<cmeta::arithmetic T, usize M, usize N>
    [[nodiscard]]
    constexpr auto transpose(const mat<T, M, N>& matrix) noexcept -> mat<T, N, M>;

    template<cmeta::arithmetic T, usize M, usize N>
    constexpr auto transpose(mat_view<const T, M, N> matrix, mat_view<T, N, M> out) noexcept -> void;

    template<cmeta::arithmetic T, usize M>
    [[nodiscard]]
    constexpr auto cofactor(const mat<T, M, M>& matrix) noexcept -> mat<T, M, M>;

    template<typename T, usize M>
    constexpr auto cofactor(square_mat_view<const T, M> matrix, square_mat_view<T, M> out) noexcept -> void;

    template<meta::is_mat_or_view T>
    [[nodiscard]]
    constexpr auto is_inversible(const T& matrix) noexcept -> bool;

    template<cmeta::arithmetic T, usize M>
    [[nodiscard]]
    constexpr auto inverse(const mat<T, M, M>& matrix) noexcept -> mat<T, M, M>;

    template<cmeta::arithmetic T, usize N>
    constexpr auto inverse(square_mat_view<const T, N> a, square_mat_view<T, N> out) noexcept -> void;

    template<meta::is_mat_or_view T>
    [[nodiscard]]
    constexpr auto is_orthogonal(const T& matrix) noexcept -> bool;

    template<cmeta::arithmetic T, usize M, usize N, usize K>
    [[nodiscard]]
    constexpr auto mul(const mat<T, M, N>& a, const mat<T, N, K>& b) noexcept -> mat<T, M, K>;

    template<cmeta::arithmetic T, usize M, usize N, usize K>
    constexpr auto mul(mat_view<const T, M, N> a, mat_view<const T, N, K> b, mat_view<T, M, K> out) noexcept -> void;

    template<cmeta::arithmetic T, usize M, usize N>
    [[nodiscard]]
    constexpr auto div(const mat<T, M, N>& a, const mat<T, N, N>& b) noexcept -> mat<T, N, N>;

    template<cmeta::arithmetic T, usize M, usize N>
    constexpr auto div(mat_view<const T, M, N> a, square_mat_view<const T, N> b, square_mat_view<T, N> out) noexcept -> void;

    template<cmeta::arithmetic T>
    [[nodiscard]]
    constexpr auto translate(const mat<T, 4, 4>& matrix, const vec<T, 3>& translation) noexcept -> mat<T, 4, 4>;

    template<cmeta::arithmetic T>
    constexpr auto translate(square_mat_view<const T, 4> matrix,
                             vec_view<const T, 3>        translation,
                             square_mat_view<T, 4>       out) noexcept -> void;

    template<cmeta::arithmetic T>
    [[nodiscard]]
    constexpr auto scale(const mat<T, 4, 4>& matrix, const vec<T, 3>& translation) noexcept -> mat<T, 4, 4>;

    template<cmeta::arithmetic T>
    constexpr auto scale(square_mat_view<const T, 4> matrix, vec_view<const T, 3> translation, square_mat_view<T, 4> out) noexcept
      -> void;

    template<cmeta::arithmetic T>
    [[nodiscard]]
    constexpr auto rotate(const mat<T, 4, 4>& matrix, T angle, const vec<T, 3>& axis) noexcept -> mat<T, 4, 4>
        requires(std::is_signed_v<T>);

    template<cmeta::arithmetic T>
    constexpr auto rotate(square_mat_view<const T, 4> matrix,
                          radian<T>                   angle,
                          vec_view<const T, 3>        axis,
                          square_mat_view<T, 4>       out) noexcept -> void
        requires(std::is_signed_v<T>);

    template<cmeta::arithmetic T>
    [[nodiscard]]
    constexpr auto orthographique(T left, T right, T bottom, T top, T near, T far) noexcept -> mat<T, 4, 4>
        requires(std::is_signed_v<T>);

    template<cmeta::arithmetic T>
    constexpr auto orthographique(T left, T right, T bottom, T top, T near, T far, square_mat_view<T, 4> out) noexcept -> void
        requires(std::is_signed_v<T>);

    template<cmeta::arithmetic T>
    [[nodiscard]]
    constexpr auto orthographique(T left, T right, T bottom, T top) noexcept -> mat<T, 4, 4>
        requires(std::is_signed_v<T>);

    template<cmeta::arithmetic T>
    constexpr auto orthographique(T left, T right, T bottom, T top, square_mat_view<T, 4> out) noexcept -> void
        requires(std::is_signed_v<T>);

    template<cmeta::arithmetic T>
    [[nodiscard]]
    constexpr auto perspective(radian<T> fov_y, T aspect, T near, T far) noexcept -> mat<T, 4, 4>
        requires(std::is_signed_v<T>);

    template<cmeta::arithmetic T>
    constexpr auto perspective(radian<T> fov_y, T aspect, T near, T far, square_mat_view<T, 4> out) noexcept -> void
        requires(std::is_signed_v<T>);

    template<cmeta::arithmetic T>
    [[nodiscard]]
    constexpr auto look_at(const vec<T, 3>& eye, const vec<T, 3>& center, const vec<T, 3>& up) noexcept -> mat<T, 4, 4>
        requires(std::is_signed_v<T>);

    template<cmeta::arithmetic T>
    constexpr auto look_at(vec_view<const T, 3>  eye,
                           vec_view<const T, 3>  center,
                           vec_view<const T, 3>  up,
                           square_mat_view<T, 4> out) noexcept -> void
        requires(std::is_signed_v<T>);
} // namespace stormkit::math

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::math {
    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize M, usize N>
    constexpr auto mat_interface<T, M, N>::identity() noexcept -> decltype(auto)
        requires(M == N)
    {
        auto matrix = mat<T, M, N> {};
        for (auto i : range(M)) matrix[i, i] = T { 1 };
        return matrix;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize M, usize N>
    template<typename Self>
    constexpr auto mat_interface<T, M, N>::row(this Self& self, usize id) noexcept
      -> array_view<cmeta::forward_const_to<Self, T>, M> {
        return array_view<cmeta::forward_const_to<Self, T>, M> { self.begin() + id * M, N };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename CharT, cmeta::arithmetic T, usize M, usize N>
    constexpr auto tag_invoke(format_as_fn<CharT>, const mat<T, M, N>& matrix, cmeta::format_context auto& ctx) noexcept
      -> decltype(ctx.out()) {
        return tag_invoke(format_as_fn<CharT> {}, view_of(matrix), ctx);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename CharT, cmeta::arithmetic T, usize M, usize N>
    constexpr auto tag_invoke(format_as_fn<CharT>, mat_view<const T, M, N> matrix, cmeta::format_context auto& ctx) noexcept
      -> decltype(ctx.out()) {
        auto i               = 0;
        auto chunk_by_extent = [&i](auto, auto) mutable {
            const auto insert = i++ < (N - 1);
            if (not insert) i = 0;

            return insert;
        };
        const auto matrix_as_linear = view_of(as_linear, matrix);

        format_to(ctx, "[mat");
        format_to(ctx, "{}", M);
        format_to(ctx, "x");
        format_to(ctx, "{}", N);
        format_to(ctx, ":");

        auto l         = 0_u32;
        auto max_digit = 0_u32;
        for (auto v : matrix_as_linear)
            max_digit = std::max(max_digit, unchecked_narrow<i64>(v) == 0 ? 2_u32 : unchecked_narrow<u32>(std::log10(v) + 2));

        for (auto&& slice : matrix_as_linear | stdv::chunk_by(chunk_by_extent)) {
            if constexpr (cmeta::integral<T>) {
                format_to(ctx,
                          "{}|{:n:>{}}|{}",
                          (l > 0) ? "         "sv : " "sv,
                          slice,
                          max_digit,
                          (l != (M - 1)) ? "\n"sv : "]"sv);
            } else {
                format_to(ctx,
                          "{}| {:n:>{}.5f}|{}",
                          (l > 0) ? "         "sv : " "sv,
                          slice,
                          max_digit,
                          (l != (M - 1)) ? "\n"sv : "]"sv);
            }
            ++l;
        }

        return ctx.out();
    }

    // TODO rewrite without recursivity
    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_square_mat_or_view T>
       STORMKIT_PURE
    constexpr auto determinant(const T& matrix) noexcept -> cmeta::remove_const_of<cmeta::value_type<T>> {
        using value_type = cmeta::value_type<T>;

        static constexpr auto M = matrix.extent(0);

        if constexpr (M == 0) return 1;
        else if constexpr (M == 1)
            return matrix[0, 0];
        else if constexpr (M == 2)
            return matrix[0, 0] * matrix[1, 1] - matrix[0, 1] * matrix[1, 0];
        else if constexpr (M == 3)
            return matrix[0, 0] * (matrix[1, 1] * matrix[2, 2] - matrix[2, 1] * matrix[1, 2])
                   - matrix[1, 0] * (matrix[0, 1] * matrix[2, 2] - matrix[2, 1] * matrix[0, 2])
                   + matrix[2, 0] * (matrix[0, 1] * matrix[1, 2] - matrix[1, 1] * matrix[0, 2]);
        else if constexpr (M == 4) {
            const auto subfactor_0 = matrix[2, 2] * matrix[3, 3] - matrix[3, 2] * matrix[2, 3];
            const auto subfactor_1 = matrix[2, 1] * matrix[3, 3] - matrix[3, 1] * matrix[2, 3];
            const auto subfactor_2 = matrix[2, 1] * matrix[3, 2] - matrix[3, 1] * matrix[2, 2];
            const auto subfactor_3 = matrix[2, 0] * matrix[3, 3] - matrix[3, 0] * matrix[2, 3];
            const auto subfactor_4 = matrix[2, 0] * matrix[3, 2] - matrix[3, 0] * matrix[2, 2];
            const auto subfactor_5 = matrix[2, 0] * matrix[3, 1] - matrix[3, 0] * matrix[2, 1];

            const auto det_cof_0 = matrix[1, 1] * subfactor_0 - matrix[1, 2] * subfactor_1 + matrix[1, 3] * subfactor_2;
            const auto det_cof_1 = -(matrix[1, 0] * subfactor_0 - matrix[1, 2] * subfactor_3 + matrix[1, 3] * subfactor_4);
            const auto det_cof_2 = matrix[1, 0] * subfactor_1 - matrix[1, 1] * subfactor_3 + matrix[1, 3] * subfactor_5;
            const auto det_cof_3 = -(matrix[1, 0] * subfactor_2 - matrix[1, 1] * subfactor_4 + matrix[1, 2] * subfactor_5);

            return matrix[0, 0] * det_cof_0 + matrix[0, 1] * det_cof_1 + matrix[0, 2] * det_cof_2 + matrix[0, 3] * det_cof_3;
        } else {
            auto matrix_ = mat<value_type, M, M> {};
            stdr::copy(view_of(as_linear, matrix), stdr::begin(matrix_));

            static constexpr auto swap = [](auto row, auto col) static noexcept {
                const auto row_data = init_by<array<value_type, M>>([&row](auto& row_data) noexcept {
                    stdr::copy(row, stdr::begin(row_data));
                });

                stdr::copy(col, stdr::begin(row));
                stdr::copy(row_data, stdr::begin(col));
            };

            for (auto col : range(M)) {
                const auto found = init_by<bool>([&matrix_, col](auto& found) noexcept {
                    found = false;
                    for (auto row : range(col, M)) {
                        if (not is(matrix_[row, col], value_type { 0 })) {
                            found = true;
                            if (row != col) { swap(matrix_.row(row), matrix_.row(col)); }
                        }
                    }
                });
                expects(found, "Did not found a non-zero row!");

                auto i = 0;
                for (auto row : range(col + 1, M)) {
                    for (;;) {
                        const auto del = matrix_[row, col] / matrix_[col, col];
                        if not consteval { std::println("del: {}", del); }
                        for (auto j : range(col, M)) matrix_[row, j] -= del * matrix_[col, j];

                        if (is(matrix_[row, col], value_type { 0 })) break;

                        swap(matrix_.row(row), matrix_.row(col));
                    }
                }
            }

            const auto res = init_by<value_type>([&matrix_](auto& res) noexcept {
                res = -1;
                for (auto i : range(M)) res *= matrix_[i, i];
            });

            return res;
        }
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize M, usize N>
       STORMKIT_PURE
    constexpr auto transpose(const mat<T, M, N>& matrix) noexcept -> mat<T, N, M> {
        auto out = mat<T, N, M> {};
        transpose(view_of(matrix), mutable_view_of(out));
        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize M, usize N>
    constexpr auto transpose(mat_view<const T, M, N> matrix, mat_view<T, N, M> out) noexcept -> void {
        EXPECTS(matrix.data_handle() != out.data_handle());

        for (auto [i, j] : multi_range(N, M)) out[i, j] = matrix[j, i];
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize M>
    STORMKIT_FORCE_INLINE
    constexpr auto cofactor(const mat<T, M, M>& matrix) noexcept -> mat<T, M, M> {
        auto out = mat<T, M, M> {};
        cofactor(view_of(matrix), mutable_view_of(out));
        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, usize M>
    constexpr auto cofactor(square_mat_view<const T, M> matrix, square_mat_view<T, M> out) noexcept -> void {
        static constexpr auto N = M - 1u;

        for (auto [i, j] : multi_range(M, M)) {
            const auto submatrix = init_by<mat<T, N, N>>([i, j, &matrix](auto& out) noexcept {
                auto c = 0u;
                for (auto [k, l] : multi_range(M, M)) {
                    if (k == i or l == j) continue;

                    out[c] = matrix[k, l];

                    ++c;
                }
            });

            out[i, j] = unchecked_narrow<T>(std::pow(-1, i + j)) * determinant(submatrix);
        }
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_mat_or_view T>
       STORMKIT_PURE
    constexpr auto is_inversible(const T& matrix) noexcept -> bool {
        static constexpr auto M = matrix.extent(0);
        static constexpr auto N = matrix.extent(1);

        if constexpr (M != N or M == 0) return false;
        else {
            const auto d = determinant(matrix);
            return d != 0;
        }
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize M>
    STORMKIT_FORCE_INLINE
    constexpr auto inverse(const mat<T, M, M>& matrix) noexcept -> mat<T, M, M> {
        auto out = mat<T, M, M> {};
        inverse(view_of(matrix), mutable_view_of(out));
        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize N>
    constexpr auto inverse(square_mat_view<const T, N> matrix, square_mat_view<T, N> out) noexcept -> void {
        EXPECTS(matrix.data_handle() != out.data_handle());
        EXPECTS(is_inversible(matrix));

        if constexpr (N == 2) {
            const auto one_over_determinant = T { 1 } / determinant(matrix);

            out[0, 0] = matrix[1, 1] * one_over_determinant;
            out[0, 1] = -matrix[0, 1] * one_over_determinant;
            out[1, 0] = -matrix[1, 0] * one_over_determinant;
            out[1, 1] = matrix[0, 0] * one_over_determinant;
        } else if constexpr (N == 3) {
            const auto a = vec4<T> { matrix[0, 0], matrix[0, 1], matrix[0, 2], T { 0 } };
            const auto b = vec4<T> { matrix[1, 0], matrix[1, 1], matrix[1, 2], T { 0 } };
            const auto c = vec4<T> { matrix[2, 0], matrix[2, 1], matrix[2, 2], T { 0 } };

            const auto i0 = cross(b, c);
            const auto i1 = cross(c, a);
            const auto i2 = cross(a, b);

            const auto inverse = init_by<mat<T, N, N>>([&i0, &i1, &i2](auto& inverse) {
                auto inverse_ = mat<T, N, N> {};
                stdr::copy(i0, stdr::begin(inverse_.row(0)));
                stdr::copy(i1, stdr::begin(inverse_.row(1)));
                stdr::copy(i2, stdr::begin(inverse_.row(2)));

                transpose(view_of(inverse_), mutable_view_of(inverse));
            });

            const auto one_over_determinant = T { 1 } / determinant(matrix);
            mul(view_of(inverse), one_over_determinant, out);
            // } else if constexpr (N == 4) {
            // TODO
        } else {
            const auto one_over_determinant = T { 1 } / determinant(matrix);
            const auto factor               = init_by<mat<T, N, N>>([matrix](auto& factor) noexcept {
                cofactor(matrix, mutable_view_of(factor));
            });
            const auto transposed           = init_by<mat<T, N, N>>([factor = view_of(factor)](auto& transposed) noexcept {
                transpose(factor, mutable_view_of(transposed));
            });

            mul(view_of(transposed), one_over_determinant, out);
        }
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_mat_or_view U>
    constexpr auto is_orthogonal(const U& matrix) noexcept -> bool {
        using T                 = cmeta::value_type<U>;
        static constexpr auto M = matrix.extent(0);
        static constexpr auto N = matrix.extent(1);

        if constexpr (M != N) return false;
        else {
            if (not is_inversible(matrix)) return false;

            static constexpr auto IDENTITY = [] static consteval noexcept {
                auto matrix = mat<T, M, N> {};
                for (auto i : range(M)) matrix[i, i] = T { 1 };
                return matrix;
            }();

            const auto transposed = init_by<mat<T, M, N>>([matrix = view_of(matrix)](auto& transposed) noexcept {
                transpose(matrix, mutable_view_of(transposed));
            });
            const auto result     = init_by<mat<T, M, N>>([matrix     = view_of(matrix),
                                                           transposed = view_of(transposed)](auto& result) noexcept {
                mul(matrix, transposed, mutable_view_of(result));
            });

            return std::memcmp(stdr::data(result), stdr::data(IDENTITY), M * N * sizeof(T));
        }
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize M, usize N, usize K>
    STORMKIT_FORCE_INLINE
    constexpr auto mul(const mat<T, M, N>& a, const mat<T, N, K>& b) noexcept -> mat<T, M, K> {
        auto out = mat<T, M, K> {};
        mul(view_of(a), view_of(b), mutable_view_of(out));
        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize M, usize N, usize K>
    constexpr auto mul(mat_view<const T, M, N> a, mat_view<const T, N, K> b, mat_view<T, M, K> out) noexcept -> void {
        for (auto [i, j, k] : multi_range(M, K, N)) out[i, j] += a[i, k] * b[k, j];
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize M, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto div(const mat<T, M, N>& a, const mat<T, N, N>& b) noexcept -> mat<T, N, N> {
        auto out = mat<T, N, N> {};
        div(view_of(a), view_of(b), mutable_view_of(out));
        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize M, usize N>
       STORMKIT_FORCE_INLINE
    constexpr auto div(mat_view<const T, M, N> a, square_mat_view<const T, N> b, square_mat_view<T, N> out) noexcept -> void {
        EXPECTS(a.data_handle() != out.data_handle());
        EXPECTS(b.data_handle() != out.data_handle());

        const auto d = determinant(b);
        EXPECTS(not is(d, 0));

        const auto b_inverted = init_by<mat<T, N, N>>([b](auto& inverted) noexcept { inverse(b, mutable_view_of(inverted)); });

        mul(a, view_of(b_inverted), out);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T>
    STORMKIT_FORCE_INLINE
    constexpr auto translate(const mat<T, 4, 4>& matrix, const vec<T, 3>& translation) noexcept -> mat<T, 4, 4> {
        auto out = mat<T, 4, 4> {};
        translate(view_of(matrix), view_of(translation), mutable_view_of(out));
        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T>
    constexpr auto translate(square_mat_view<const T, 4> matrix,
                             vec_view<const T, 3>        translation,
                             square_mat_view<T, 4>       out) noexcept -> void {
        EXPECTS(matrix.data_handle() != out.data_handle());
        EXPECTS(translation.data_handle() != out.data_handle());

        auto out_linear = mutable_view_of(as_linear, out);
        stdr::copy(view_of(as_linear, matrix), stdr::begin(out_linear));

        out[0, 3] = matrix[0, 3] + translation[0];
        out[1, 3] = matrix[1, 3] + translation[1];
        out[2, 3] = matrix[2, 3] + translation[2];
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T>
    STORMKIT_FORCE_INLINE
    constexpr auto scale(const mat<T, 4, 4>& matrix, const vec<T, 3>& scale_factor) noexcept -> mat<T, 4, 4> {
        auto out = mat<T, 4, 4> {};
        scale(view_of(matrix), view_of(scale_factor), mutable_view_of(out));
        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T>
    constexpr auto scale(square_mat_view<const T, 4> matrix, vec_view<const T, 3> scale, square_mat_view<T, 4> out) noexcept
      -> void {
        EXPECTS(matrix.data_handle() != out.data_handle());
        EXPECTS(scale.data_handle() != out.data_handle());

        out[0, 0] = matrix[0, 0] * scale[0];
        out[1, 0] = matrix[1, 0] * scale[0];
        out[2, 0] = matrix[2, 0] * scale[0];
        out[3, 0] = matrix[3, 0] * scale[0];

        out[0, 1] = matrix[0, 1] * scale[1];
        out[1, 1] = matrix[1, 1] * scale[1];
        out[2, 1] = matrix[2, 1] * scale[1];
        out[3, 1] = matrix[3, 1] * scale[1];

        out[0, 2] = matrix[0, 2] * scale[2];
        out[1, 2] = matrix[1, 2] * scale[2];
        out[2, 2] = matrix[2, 2] * scale[2];
        out[3, 2] = matrix[3, 2] * scale[2];

        out[0, 3] = matrix[0, 3];
        out[1, 3] = matrix[1, 3];
        out[2, 3] = matrix[2, 3];
        out[3, 3] = matrix[3, 3];
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T>
    STORMKIT_FORCE_INLINE
    constexpr auto rotate(const mat<T, 4, 4>& matrix, radian<T> angle, const vec<T, 3>& axis) noexcept -> mat<T, 4, 4>
        requires(std::is_signed_v<T>)
    {
        auto out = mat<T, 4, 4> {};
        rotate(view_of(matrix), angle, view_of(axis), mutable_view_of(out));
        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T>
    constexpr auto rotate(square_mat_view<const T, 4> matrix,
                          radian<T>                   angle,
                          vec_view<const T, 3>        axis,
                          square_mat_view<T, 4>       out) noexcept -> void
        requires(std::is_signed_v<T>)
    {
        EXPECTS(matrix.data_handle() != out.data_handle());
        EXPECTS(axis.data_handle() != out.data_handle());

        const auto cos = unchecked_narrow<T>(std::cos(angle.get()));
        const auto sin = unchecked_narrow<T>(std::sin(angle.get()));

        const auto axis_norm = init_by<vec3<T>>([axis](auto& axis_norm) noexcept {
            normalize(axis, mutable_view_of(axis_norm));
        });

        const auto temp = init_by<vec3<T>>([axis_norm = view_of(axis_norm), cos](auto& temp) noexcept {
            normalize(axis_norm), mutable_view_of(temp);
            mul<T>(axis_norm, T { 1 } - cos, mutable_view_of(temp));
        });

        const auto rotation_matrix = init_by<mat<T, 4, 4>>([&](auto& rotation_matrix) noexcept {
            rotation_matrix[0, 0] = cos + temp[0] * axis_norm[0];
            rotation_matrix[1, 0] = temp[0] * axis_norm[1] + sin * axis_norm[2];
            rotation_matrix[2, 0] = temp[0] * axis_norm[2] - sin * axis_norm[1];

            rotation_matrix[0, 1] = temp[1] * axis_norm[0] - sin * axis_norm[2];
            rotation_matrix[1, 1] = cos + temp[1] * axis_norm[1];
            rotation_matrix[2, 1] = temp[1] * axis_norm[2] + sin * axis_norm[0];

            rotation_matrix[0, 2] = temp[2] * axis_norm[0] + sin * axis_norm[1];
            rotation_matrix[1, 2] = temp[2] * axis_norm[1] - sin * axis_norm[0];
            rotation_matrix[2, 2] = cos + temp[2] * axis_norm[2];
        });

        // // TODO replace by mul when submdspan is available
        for (auto [i, j, k] : multi_range(3_usize, 3_usize, 3_usize)) out[j, i] += matrix[k, i] * rotation_matrix[j, k];
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T>
       STORMKIT_FORCE_INLINE
    constexpr auto orthographique(T left, T right, T bottom, T top) noexcept -> mat<T, 4, 4>
        requires(std::is_signed_v<T>)
    {
        auto out = mat<T, 4, 4> {};
        orthographique(left, right, bottom, top, mutable_view_of(out));
        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T>
       STORMKIT_FORCE_INLINE
    constexpr auto orthographique(T left, T right, T bottom, T top, square_mat_view<T, 4> out) noexcept -> void
        requires(std::is_signed_v<T>)
    {
        static constexpr auto far  = core::as<T>(100);
        static constexpr auto near = core::unchecked_narrow<T>(0.1);

        return orthographique(left, right, bottom, top, near, far, out);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T>
       STORMKIT_FORCE_INLINE
    constexpr auto orthographique(T left, T right, T bottom, T top, T near, T far) noexcept -> mat<T, 4, 4>
        requires(std::is_signed_v<T>)
    {
        auto out = mat<T, 4, 4> {};
        orthographique(left, right, bottom, top, near, far, mutable_view_of(out));
        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T>
    constexpr auto orthographique(T left, T right, T bottom, T top, T near, T far, square_mat_view<T, 4> out) noexcept -> void
        requires(std::is_signed_v<T>)
    {
        out[0, 0] = core::as<T>(2) / (right - left);
        out[1, 1] = core::as<T>(2) / (top - bottom);
        out[2, 2] = core::as<T>(1) / (far - near);

        out[0, 3] = -(right + left) / (right - left);
        out[1, 3] = -(top + bottom) / (top - bottom);
        out[2, 3] = -near / (far - near);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T>
    constexpr auto perspective(radian<T> fov_y, T aspect, T near, T far) noexcept -> mat<T, 4, 4>
        requires(std::is_signed_v<T>)
    {
        auto out = mat<T, 4, 4> {};
        perspective(fov_y, aspect, near, far, mutable_view_of(out));
        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T>
    constexpr auto perspective(radian<T> fov_y, T aspect, T near, T far, square_mat_view<T, 4> out) noexcept -> void
        requires(std::is_signed_v<T>)
    {
        EXPECTS(not is(aspect, T { 0 }));
        EXPECTS(not is(near, far));

        const auto half_fov_y = unchecked_narrow<T>(std::tan(fov_y.get() / T { 2 }));

        stdr::fill(mutable_view_of(as_linear, out), T { 0 });
        out[0, 0] = T { 1 } / (aspect * half_fov_y);
        out[1, 1] = T { -1 } / half_fov_y;
        out[2, 2] = far / (far - near);
        out[2, 3] = T { 1 };
        out[3, 2] = -(far * near) / (far - near);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T>
    constexpr auto look_at(const vec3<T>& eye, const vec3<T>& center, const vec3<T>& up) noexcept -> mat<T, 4, 4>
        requires(std::is_signed_v<T>)
    {
        auto out = mat<T, 4, 4> {};
        look_at(eye, center, up, mutable_view_of(out));
        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T>
    constexpr auto look_at(vec_view<const T, 3>  eye,
                           vec_view<const T, 3>  center,
                           vec_view<const T, 3>  up,
                           square_mat_view<T, 4> out) noexcept -> void
        requires(std::is_signed_v<T>)
    {
        EXPECTS(eye.data_handle() != out.data_handle());
        EXPECTS(center.data_handle() != out.data_handle());
        EXPECTS(up.data_handle() != out.data_handle());

        const auto z_temp = init_by<vec3<T>>([eye, center](auto& out) noexcept { sub(center, eye, mutable_view_of(out)); });
        const auto z      = init_by<vec3<T>>([z_temp = view_of(z_temp)](auto& out) noexcept {
            normalize(z_temp, mutable_view_of(out));
        });

        const auto x_temp = init_by<vec3<T>>([up, z = view_of(z)](auto& out) noexcept { cross(up, z, mutable_view_of(out)); });
        const auto x      = init_by<vec3<T>>([x_temp = view_of(x_temp)](auto& out) noexcept {
            normalize(x_temp, mutable_view_of(out));
        });

        const auto y_temp = init_by<vec3<T>>([z = view_of(z), x = view_of(x)](auto& out) noexcept {
            cross(z, x, mutable_view_of(out));
        });
        const auto y      = init_by<vec3<T>>([y_temp = view_of(y_temp)](auto& out) noexcept {
            normalize(y_temp, mutable_view_of(out));
        });

        out[0, 0] = x[0];
        out[0, 1] = y[0];
        out[0, 2] = z[0];
        out[0, 3] = 0;

        out[1, 0] = x[1];
        out[1, 1] = y[1];
        out[1, 2] = z[1];
        out[1, 3] = 0;

        out[2, 0] = x[2];
        out[2, 1] = y[2];
        out[2, 2] = z[2];
        out[2, 3] = 0;

        out[3, 0] = -dot(x, eye);
        out[3, 1] = -dot(y, eye);
        out[3, 2] = -dot(z, eye);
        out[3, 3] = 1;
    }
} // namespace stormkit::math
