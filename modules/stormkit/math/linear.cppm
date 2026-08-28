// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.math.linear;

import std;

import stormkit.core.meta;
import stormkit.core.typesafe;
import stormkit.core.functional;
import stormkit.core.types;
import stormkit.core.contract;

import stormkit.math.arithmetic;

namespace stdr = std::ranges;

// TODO improve template deduction
export namespace stormkit { inline namespace core { namespace math {
    namespace angle {
        template<meta::floating_point T>
        using euler = strong_type<T, struct EulerAngleTag, "euler", capabilities::arithmetic, ImplicitConvertionTag>;

        template<meta::floating_point T>
        using radian = strong_type<T, struct RadianAngleTag, "radian", capabilities::arithmetic, ImplicitConvertionTag>;

        template<meta::floating_point T>
        [[nodiscard]]
        constexpr auto radians(T degres) noexcept -> radian<T>;
    } // namespace angle

    template<core::meta::arithmetic T, usize... Sizes>
        requires(sizeof...(Sizes) >= 1)
    using TensorSpan = mdarray_view<T, std::extents<u8, Sizes...>>;

    template<typename T, usize N>
    using VectorSpan = TensorSpan<T, N>;

    template<typename T, usize M, usize N>
    using MatrixSpan = TensorSpan<T, M, N>;

    template<typename T, usize N>
    using SquareMatrixSpan = MatrixSpan<T, N, N>;

    namespace meta {
        template<typename T>
        concept IsTensorSpan = core::meta::IsSpecializationWithNTTPOf<T, TensorSpan>;
    }

    // template<core::meta::arithmetic T, usize... Sizes>
    // [[nodiscard]]
    // constexpr auto as_span(TensorSpan<T, Sizes...>& tensor) noexcept -> array_view<T, (Sizes *
    // ...)>;

    template<typename T, usize... Sizes>
    [[nodiscard]]
    constexpr auto as_span(const TensorSpan<const T, Sizes...>& tensor) noexcept -> array_view<const T, (Sizes * ...)>;

    template<typename T, usize... Sizes>
        requires(not core::meta::const_type<T>)
    [[nodiscard]]
    constexpr auto as_span_mut(TensorSpan<T, Sizes...> tensor) noexcept -> array_view<T, (Sizes * ...)>;

    template<usize... Sizes, stdr::contiguous_range T>
        requires(core::meta::arithmetic<core::meta::range_value_type<T>>)
    [[nodiscard]]
    constexpr auto as_mdspan(const T& data) noexcept -> TensorSpan<const core::meta::range_value_type<T>, Sizes...>;

    template<usize... Sizes, stdr::contiguous_range T>
        requires(core::meta::arithmetic<core::meta::range_value_type<T>> and not core::meta::const_type<core::meta::range_value_type<T>>)
    [[nodiscard]]
    constexpr auto as_mdspan_mut(T& data) noexcept -> TensorSpan<core::meta::range_value_type<T>, Sizes...>;

    template<meta::IsTensorSpan To, meta::IsTensorSpan From>
    [[nodiscard]]
    constexpr auto as(const From& data) noexcept -> To;

    template<typename T, usize... Sizes>
        requires(not core::meta::const_type<T>)
    constexpr auto add(const TensorSpan<const T, Sizes...>& a,
                       const TensorSpan<const T, Sizes...>& b,
                       TensorSpan<T, Sizes...>              out) noexcept -> void;

    template<typename T, usize... Sizes>
        requires(not core::meta::const_type<T>)
    constexpr auto sub(const TensorSpan<const T, Sizes...>& a,
                       const TensorSpan<const T, Sizes...>& b,
                       TensorSpan<T, Sizes...>              out) noexcept -> void;

    template<core::meta::arithmetic T, usize... Sizes>
        requires(not core::meta::const_type<T>)
    constexpr auto mul(const TensorSpan<const T, Sizes...>& a, T b, TensorSpan<T, Sizes...> out) noexcept -> void;

    template<core::meta::arithmetic T, usize... Sizes>
        requires(not core::meta::const_type<T>)
    constexpr auto div(const TensorSpan<const T, Sizes...>& a, T b, TensorSpan<T, Sizes...> out) noexcept -> void;
    // TODO WAIT FOR SUBMDSPAN
    // template<core::meta::arithmetic T, usize... Sizes>
    // constexpr auto normalize(TensorSpan<const T, Sizes...> a,
    //                           TensorSpan<T, Sizes...> out) noexcept -> void;

    /* vector */
    template<typename T, usize N>
        requires(not core::meta::const_type<T>)
    constexpr auto normalize(const VectorSpan<const T, N>& a, VectorSpan<T, N> out) noexcept -> void;

    template<typename T, usize N>
        requires(not core::meta::const_type<T>)
    constexpr auto transpose(const VectorSpan<const T, N>& a, VectorSpan<T, N> out) noexcept -> void;

    template<typename T, usize N>
    [[nodiscard]]
    constexpr auto dot(const VectorSpan<const T, N>& a, const VectorSpan<const T, N>& b) noexcept -> T;

    template<typename T>
        requires(not core::meta::const_type<T>)
    constexpr auto cross(const VectorSpan<const T, 3>& a, const VectorSpan<const T, 3>& b, VectorSpan<T, 3> out) noexcept -> void;

    /* matrix */
    template<typename T, usize N>
        requires(not core::meta::const_type<T>)
    [[nodiscard]]
    constexpr auto determinant(const SquareMatrixSpan<const T, N>& a) noexcept -> T;

    template<typename T, usize N>
        requires(not core::meta::const_type<T>)
    constexpr auto transpose(const SquareMatrixSpan<const T, N>& a, SquareMatrixSpan<T, N> out) noexcept -> void;

    template<typename T, usize M, usize N>
    [[nodiscard]]
    constexpr auto is_inversible(const MatrixSpan<T, M, N>& mat) noexcept -> bool;

    template<typename T, usize N>
        requires(not core::meta::const_type<T>)
    constexpr auto inverse(const SquareMatrixSpan<const T, N>& a, SquareMatrixSpan<T, N> out) noexcept -> void;

    template<typename T, usize M, usize N>
    [[nodiscard]]
    constexpr auto is_orthogonal(const MatrixSpan<T, M, N>& mat) noexcept -> bool;

    template<typename T, usize M, usize N, usize K>
        requires(not core::meta::const_type<T>)
    constexpr auto mul(const MatrixSpan<const T, M, N>& a, const MatrixSpan<const T, N, K>& b, MatrixSpan<T, M, K> out) noexcept
      -> void;

    template<typename T, usize M, usize N, usize K>
        requires(not core::meta::const_type<T>)
    constexpr auto div(const MatrixSpan<const T, M, N>&    a,
                       const SquareMatrixSpan<const T, N>& b,
                       SquareMatrixSpan<T, N>              out) noexcept -> void;

    template<typename T>
    constexpr auto translate(const SquareMatrixSpan<const T, 4>& mat,
                             const VectorSpan<const T, 3>&       translation,
                             SquareMatrixSpan<T, 4>              out) noexcept -> void;

    template<typename T>
    constexpr auto scale(const SquareMatrixSpan<const T, 4>& mat,
                         const VectorSpan<const T, 3>&       translation,
                         SquareMatrixSpan<T, 4>              out) noexcept -> void;

    template<core::meta::arithmetic T>
        requires(std::is_signed_v<T> and not core::meta::const_type<T>)
    constexpr auto rotate(const SquareMatrixSpan<const T, 4>& mat,
                          T                                   angle,
                          const VectorSpan<const T, 3>&       axis,
                          SquareMatrixSpan<T, 4>              out) noexcept -> void;

    /* graphics */
    template<core::meta::arithmetic T>
        requires(std::is_signed_v<T> and not core::meta::const_type<T>)
    constexpr auto orthographique(T left, T right, T bottom, T top, T near, T far, SquareMatrixSpan<T, 4> out) noexcept -> void;

    template<core::meta::arithmetic T>
        requires(std::is_signed_v<T> and not core::meta::const_type<T>)
    constexpr auto orthographique(T left, T right, T bottom, T top, SquareMatrixSpan<T, 4> out) noexcept -> void;

    template<core::meta::arithmetic T>
        requires(std::is_signed_v<T> and not core::meta::const_type<T>)
    constexpr auto perspective(angle::radian<T> fov_y, T aspect, T near, T far, SquareMatrixSpan<T, 4> out) noexcept -> void;

    template<typename T>
        requires(std::is_signed_v<T> and not core::meta::const_type<T>)
    constexpr auto look_at(VectorSpan<const T, 3> eye,
                           VectorSpan<const T, 3> center,
                           VectorSpan<const T, 3> up,
                           SquareMatrixSpan<T, 4> out) noexcept -> void;
}}} // namespace stormkit::core::math

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stdr = std::ranges;

namespace stormkit { inline namespace core { namespace math {
    namespace angle {
        ////////////////////////////////////////
        ////////////////////////////////////////
        template<meta::floating_point T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
        constexpr auto radians(T degres) noexcept -> radian<T> {
            static constexpr auto one_rad = std::numbers::pi_v<T> / T { 180 };
            return radian<T> { degres * one_rad };
        }
    } // namespace angle

    template<typename T, usize M, usize N>
    using MatData = array<T, M * N>;
    template<typename T, usize M>
    using SMatData = MatData<T, M, M>;

    template<typename T, usize N>
    using VecData = array<T, N>;

    template<typename T>
    using vec2data = VecData<T, 2>;

    template<typename T>
    using vec3data = VecData<T, 3>;

    template<typename T>
    using vec4data = VecData<T, 4>;

    // ////////////////////////////////////////
    // ////////////////////////////////////////
    // template<core::meta::arithmetic T, usize... Sizes>
    // [[nodiscard]]
    // constexpr auto as_span(const TensorSpan<T, Sizes...>& tensor) noexcept
    //   -> array_view<T, (Sizes * ...)> {
    //     return array_view<T, (Sizes * ...)> { tensor.data_handle(), (Sizes * ...) };
    // }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, usize... Sizes>
    [[nodiscard]]
    constexpr auto as_span(const TensorSpan<const T, Sizes...>& tensor) noexcept -> array_view<const T, (Sizes * ...)> {
        return array_view<const T, (Sizes * ...)> { tensor.data_handle(), (Sizes * ...) };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, usize... Sizes>
        requires(not core::meta::const_type<T>)
    [[nodiscard]]
    constexpr auto as_span_mut(TensorSpan<T, Sizes...> tensor) noexcept -> array_view<T, (Sizes * ...)> {
        return array_view<T, (Sizes * ...)> { tensor.data_handle(), (Sizes * ...) };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<usize... Sizes, stdr::contiguous_range T>
        requires(core::meta::arithmetic<core::meta::range_value_type<T>>)
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto as_mdspan(const T& data) noexcept -> TensorSpan<const core::meta::range_value_type<T>, Sizes...> {
        EXPECTS(stdr::size(data) == (Sizes * ...));
        return TensorSpan<const core::meta::range_value_type<T>, Sizes...> { stdr::data(data), Sizes... };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<usize... Sizes, stdr::contiguous_range T>
        requires(core::meta::arithmetic<core::meta::range_value_type<T>> and not core::meta::const_type<core::meta::range_value_type<T>>)
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto as_mdspan_mut(T& data) noexcept -> TensorSpan<core::meta::range_value_type<T>, Sizes...> {
        EXPECTS(stdr::size(data) == (Sizes * ...));
        return TensorSpan<core::meta::range_value_type<T>, Sizes...> { stdr::data(data), Sizes... };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::IsTensorSpan To, meta::IsTensorSpan From>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto as(const From& data) noexcept -> To {
        return To { data };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, usize... Sizes>
        requires(not core::meta::const_type<T>)
    STORMKIT_FORCE_INLINE
    constexpr auto add(const TensorSpan<const T, Sizes...>& a,
                       const TensorSpan<const T, Sizes...>& b,
                       TensorSpan<T, Sizes...>              out) noexcept -> void {
        EXPECTS(a.data_handle() != out.data_handle());
        EXPECTS(b.data_handle() != out.data_handle());
        const auto a_linear   = as_span(a);
        const auto b_linear   = as_span(b);
        auto       out_linear = as_span_mut(out);
        for (auto i = 0u; i < (Sizes * ...); ++i) out_linear[i] = a_linear[i] + b_linear[i];
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, usize... Sizes>
        requires(not core::meta::const_type<T>)
    STORMKIT_FORCE_INLINE
    constexpr auto sub(const TensorSpan<const T, Sizes...>& a,
                       const TensorSpan<const T, Sizes...>& b,
                       TensorSpan<T, Sizes...>              out) noexcept -> void {
        EXPECTS(a.data_handle() != out.data_handle());
        EXPECTS(b.data_handle() != out.data_handle());
        const auto a_linear   = as_span(a);
        const auto b_linear   = as_span(b);
        auto       out_linear = as_span_mut(out);
        for (auto i = 0u; i < (Sizes * ...); ++i) out_linear[i] = a_linear[i] - b_linear[i];
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T, usize... Sizes>
        requires(not core::meta::const_type<T>)
    STORMKIT_FORCE_INLINE
    constexpr auto mul(const TensorSpan<const T, Sizes...>& a, T b, TensorSpan<T, Sizes...> out) noexcept -> void {
        EXPECTS(a.data_handle() != out.data_handle());
        const auto a_linear   = as_span(a);
        auto       out_linear = as_span_mut(out);
        for (auto i = 0u; i < (Sizes * ...); ++i) out_linear[i] = a_linear[i] * b;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T, usize... Sizes>
        requires(not core::meta::const_type<T>)
    STORMKIT_FORCE_INLINE
    constexpr auto div(const TensorSpan<const T, Sizes...>& a, T b, TensorSpan<T, Sizes...> out) noexcept -> void {
        EXPECTS(a.data_handle() != out.data_handle());
        const auto a_linear   = as_span(a);
        auto       out_linear = as_span_mut(out);
        for (auto i = 0u; i < (Sizes * ...); ++i) out_linear[i] = a_linear[i] / b;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, usize N>
        requires(not core::meta::const_type<T>)
    STORMKIT_FORCE_INLINE
    constexpr auto normalize(const VectorSpan<const T, N>& a, VectorSpan<T, N> out) noexcept -> void {
        EXPECTS(a.data_handle() != out.data_handle());
        const auto sum = init_by<T>([&a](auto& out) noexcept {
            for (auto i = 0u; i < N; ++i) out += (a[i] * a[i]);
            out = unchecked_narrow<T>(std::sqrt(out));
        });

        for (auto i = 0u; i < N; ++i) out[i] = a[i] / sum;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, usize N>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto dot(const VectorSpan<const T, N>& a, const VectorSpan<const T, N>& b) noexcept -> T {
        auto out = T { 0 };
        for (auto i = 0u; i < N; ++i) out += (a[i] * b[i]);
        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
        requires(not core::meta::const_type<T>)
    STORMKIT_FORCE_INLINE
    constexpr auto cross(const VectorSpan<const T, 3>& a, const VectorSpan<const T, 3>& b, VectorSpan<T, 3> out) noexcept
      -> void {
        EXPECTS(a.data_handle() != out.data_handle());
        EXPECTS(b.data_handle() != out.data_handle());

        out[0] = a[1] * b[2] - a[2] * b[1];
        out[1] = a[2] * b[0] - a[0] * b[2];
        out[2] = a[0] * b[1] - a[1] * b[0];
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, usize M>
        requires(not core::meta::const_type<T>)
    constexpr auto cofactor(const SquareMatrixSpan<const T, M>& mat, SquareMatrixSpan<T, M> out) noexcept -> void {
        static constexpr auto N = M - 1u;

        for (auto i = 0u; i < M; ++i)
            for (auto j = 0u; j < M; ++j) {
                const auto submatrix = init_by<SMatData<T, N>>([&i, &j, &mat](auto& out) noexcept {
                    auto c = 0u;
                    for (auto k = 0u; k < M; ++k) {
                        for (auto l = 0u; l < M; ++l) {
                            if (k == i or l == j) continue;

                            out[c] = mat[k, l];

                            ++c;
                        }
                    }
                });

                out[i, j] = unchecked_narrow<T>(std::pow(-1, i + j)) * determinant(as_mdspan<N, N>(submatrix));
            }
    }

    // TODO rewrite without recursivity
    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, usize M>
        requires(not core::meta::const_type<T>)
    STORMKIT_PURE
    constexpr auto determinant(const SquareMatrixSpan<const T, M>& mat) noexcept -> T {
        if constexpr (M == 0) return 1;
        else if constexpr (M == 1)
            return mat[0, 0];
        else if constexpr (M == 2)
            return mat[0, 0] * mat[1, 1] - mat[0, 1] * mat[1, 0];
        else {
            static constexpr auto N = M - 1u;
            static constexpr auto i = 0u;

            auto result = T { 0 };
            for (auto j = 0u; j < M; ++j) {
                const auto submatrix = init_by<SMatData<T, N>>([&j, &mat](auto& out) noexcept {
                    auto c = 0u;
                    for (auto k = 0u; k < M; ++k) {
                        for (auto l = 0u; l < M; ++l) {
                            if (k == i or l == j) continue;

                            out[c] = mat[k, l];

                            ++c;
                        }
                    }
                });

                const auto det = determinant(as_mdspan<N, N>(submatrix));

                result += mat[i, j] * unchecked_narrow<T>(std::pow(-1, i + j)) * det;
            }
            return result;
        }
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, usize N>
        requires(not core::meta::const_type<T>)
    STORMKIT_FORCE_INLINE
    constexpr auto transpose(const SquareMatrixSpan<const T, N>& a, SquareMatrixSpan<T, N> out) noexcept -> void {
        EXPECTS(a.data_handle() != out.data_handle());

        for (auto i = 0u; i < N; ++i)
            for (auto j = 0u; j < N; ++j) { out[i, j] = a[j, i]; }
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, usize M, usize N>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto is_inversible(const MatrixSpan<const T, M, N>& mat) noexcept -> bool {
        if constexpr (M != N) return false;
        else if constexpr (M == 0)
            return false;
        else {
            const auto d = determinant(mat);
            return d != 0;
        }
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, usize N>
        requires(not core::meta::const_type<T>)
    constexpr auto inverse(const SquareMatrixSpan<const T, N>& a, SquareMatrixSpan<T, N> out) noexcept -> void {
        EXPECTS(a.data_handle() != out.data_handle());
        EXPECTS(is_inversible(a));

        const auto factor = init_by<SMatData<T, N>>([&a](auto& out) noexcept { cofactor(a, as_mdspan_mut<N, N>(out)); });

        const auto transposed = init_by<SMatData<T, N>>([&factor](auto& out) noexcept {
            transpose(as_mdspan<N, N>(factor), as_mdspan_mut<N, N>(out));
        });

        const auto transposed_mat = as_mdspan<N, N>(transposed);

        const auto one_over_determinant = T { 1 } / determinant(a);

        for (auto i = 0u; i < N; ++i)
            for (auto j = 0u; j < N; ++j) { out[i, j] = transposed_mat[i, j] * one_over_determinant; }
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, usize M, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto is_orthogonal(const MatrixSpan<const T, M, N>& mat) noexcept -> bool {
        if constexpr (M != N) return false;
        else {
            if (not is_inversible(mat)) return false;

            static constexpr auto IDENTITY = [] static consteval noexcept {
                auto matrix = MatData<T, M, N> {};
                auto write  = as_mdspan_mut<M, N>(matrix);

                for (auto i = 0u; i < M; ++i) { write[i, i] = T { 1 }; }

                return matrix;
            }();

            const auto transposed = init_by<SMatData<T, N>>([&mat](auto& out) noexcept {
                transpose(mat, as_mdspan_mut<M, N>(out));
            });

            const auto result = init_by<SMatData<T, N>>([&mat, &transposed](auto& out) noexcept {
                mul(mat, as_mdspan<M, N>(transposed), as_mdspan_mut<M, N>(out));
            });

            return result == IDENTITY;
        }
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, usize M, usize N, usize K>
        requires(not core::meta::const_type<T>)
    STORMKIT_FORCE_INLINE
    constexpr auto mul(const MatrixSpan<const T, M, N>& a, const MatrixSpan<const T, N, K>& b, MatrixSpan<T, M, K> out) noexcept
      -> void {
        for (auto i = 0u; i < M; ++i)
            for (auto j = 0u; j < K; ++j)
                for (auto k = 0u; k < N; ++k) out[i, j] += a[i, k] * b[j, k];
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, usize M, usize N>
        requires(not core::meta::const_type<T>)
    STORMKIT_FORCE_INLINE
    constexpr auto div(const MatrixSpan<const T, M, N>&    a,
                       const SquareMatrixSpan<const T, N>& b,
                       SquareMatrixSpan<T, N>              out) noexcept -> void {
        EXPECTS(a.data_handle() != out.data_handle());
        EXPECTS(b.data_handle() != out.data_handle());

        const auto inverted = init_by<SMatData<T, N>>([&b](auto& out) noexcept { inverse(b, as_mdspan_mut<N, N>(out)); });

        mul(a, as_mdspan<N, N>(inverted), out);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    constexpr auto translate(const SquareMatrixSpan<const T, 4>& a,
                             const VectorSpan<const T, 3>&       translation,
                             SquareMatrixSpan<T, 4>              out) noexcept -> void {
        EXPECTS(a.data_handle() != out.data_handle());
        EXPECTS(translation.data_handle() != out.data_handle());

        out[0, 3] = a[0, 3]
                    + translation[0]; // a[0, 0] * translation[0] + a[1, 0] * translation[1] + a[2, 0] * translation[2] + a[3, 0];
        out[1, 3] = a[1, 3]
                    + translation[1]; // a[0, 1] * translation[0] + a[1, 1] * translation[1] + a[2, 1] * translation[2] + a[3, 1];
        out[2, 3] = a[2, 3]
                    + translation[2]; // a[0, 2] * translation[0] + a[1, 2] * translation[1] + a[2, 2] * translation[2] + a[3, 2];
        // out[3, 3] = a[0, 3] * translation[0] + a[1, 3] * translation[1] + a[2, 3] * translation[2] + a[3, 3];
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    constexpr auto scale(const SquareMatrixSpan<const T, 4>& a,
                         const VectorSpan<const T, 3>&       scale,
                         SquareMatrixSpan<T, 4>              out) noexcept -> void {
        EXPECTS(a.data_handle() != out.data_handle());
        EXPECTS(scale.data_handle() != out.data_handle());

        out[0, 0] = a[0, 0] * scale[0];
        out[1, 0] = a[1, 0] * scale[0];
        out[2, 0] = a[2, 0] * scale[0];
        out[3, 0] = a[3, 0] * scale[0];

        out[0, 1] = a[0, 1] * scale[1];
        out[1, 1] = a[1, 1] * scale[1];
        out[2, 1] = a[2, 1] * scale[1];
        out[3, 1] = a[3, 1] * scale[1];

        out[0, 2] = a[0, 2] * scale[2];
        out[1, 2] = a[1, 2] * scale[2];
        out[2, 2] = a[2, 2] * scale[2];
        out[3, 2] = a[3, 2] * scale[2];

        out[0, 3] = a[0, 3];
        out[1, 3] = a[1, 3];
        out[2, 3] = a[2, 3];
        out[3, 3] = a[3, 3];
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T>
        requires(std::is_signed_v<T> and not core::meta::const_type<T>)
    STORMKIT_FORCE_INLINE
    constexpr auto rotate(const SquareMatrixSpan<const T, 4>& a,
                          angle::radian<T>                    angle,
                          const VectorSpan<const T, 3>&       axis,
                          SquareMatrixSpan<T, 4>              out) noexcept -> void {
        EXPECTS(a.data_handle() != out.data_handle());
        EXPECTS(axis.data_handle() != out.data_handle());

        const auto cos = unchecked_narrow<T>(std::cos(angle.get()));
        const auto sin = unchecked_narrow<T>(std::sin(angle.get()));

        const auto axis_norm = [&axis] noexcept {
            auto axis_norm = vec3data<T> {};
            normalize(axis, as_mdspan_mut<3>(axis_norm));
            return axis_norm;
        }();

        const auto temp =
          [&axis_norm, &cos] noexcept {
              auto temp = vec3data<T> {};
              mul<T>(as_mdspan<3>(axis_norm), T { 1 } - cos, as_mdspan_mut<3>(temp));
              return temp;
          }

        ();

        auto rotation_matrix  = SMatData<T, 4> {};
        auto rotation_matrix_ = as_mdspan_mut<4, 4>(rotation_matrix);

        rotation_matrix_[0, 0] = cos + temp[0] * axis_norm[0];
        rotation_matrix_[1, 0] = temp[0] * axis_norm[1] + sin * axis_norm[2];
        rotation_matrix_[2, 0] = temp[0] * axis_norm[2] - sin * axis_norm[1];

        rotation_matrix_[0, 1] = temp[1] * axis_norm[0] - sin * axis_norm[2];
        rotation_matrix_[1, 1] = cos + temp[1] * axis_norm[1];
        rotation_matrix_[2, 1] = temp[1] * axis_norm[2] + sin * axis_norm[0];

        rotation_matrix_[0, 2] = temp[2] * axis_norm[0] + sin * axis_norm[1];
        rotation_matrix_[1, 2] = temp[2] * axis_norm[1] - sin * axis_norm[0];
        rotation_matrix_[2, 2] = cos + temp[2] * axis_norm[2];

        // // TODO replace by mul when submdspan is available
        for (auto i = 0u; i < 3; ++i)
            for (auto j = 0u; j < 3; ++j)
                for (auto k = 0u; k < 3; ++k) out[j, i] += a[k, i] * rotation_matrix_[j, k];
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T>
        requires(std::is_signed_v<T> and not core::meta::const_type<T>)
    STORMKIT_FORCE_INLINE
    constexpr auto orthographique(T left, T right, T bottom, T top, T near, T far, SquareMatrixSpan<T, 4> out) noexcept -> void {
        out[0, 0] = core::as<T>(2) / (right - left);
        out[1, 1] = core::as<T>(2) / (top - bottom);
        out[2, 2] = core::as<T>(1) / (far - near);

        out[0, 3] = -(right + left) / (right - left);
        out[1, 3] = -(top + bottom) / (top - bottom);
        out[2, 3] = -near / (far - near);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T>
        requires(std::is_signed_v<T> and not core::meta::const_type<T>)
    STORMKIT_FORCE_INLINE
    constexpr auto orthographique(T left, T right, T bottom, T top, SquareMatrixSpan<T, 4> out) noexcept -> void {
        static constexpr auto far  = core::as<T>(100);
        static constexpr auto near = core::unchecked_narrow<T>(0.1);

        return orthographique(left, right, bottom, top, near, far, out);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<core::meta::arithmetic T>
        requires(std::is_signed_v<T> and not core::meta::const_type<T>)
    STORMKIT_FORCE_INLINE
    constexpr auto perspective(angle::radian<T> fov_y, T aspect, T near, T far, SquareMatrixSpan<T, 4> out) noexcept -> void {
        EXPECTS(not is(aspect, T { 0 }));
        EXPECTS(not is(near, far));

        const auto half_fov_y = unchecked_narrow<T>(std::tan(fov_y.get() / T { 2 }));

        stdr::fill(as_span_mut(out), T { 0 });
        out[0, 0] = T { 1 } / (aspect * half_fov_y);
        out[1, 1] = T { -1 } / half_fov_y;
        out[2, 2] = far / (far - near);
        out[2, 3] = T { 1 };
        out[3, 2] = -(far * near) / (far - near);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
        requires(std::is_signed_v<T> and not core::meta::const_type<T>)
    STORMKIT_FORCE_INLINE
    constexpr auto look_at(VectorSpan<const T, 3> eye,
                           VectorSpan<const T, 3> center,
                           VectorSpan<const T, 3> up,
                           SquareMatrixSpan<T, 4> out) noexcept -> void {
        EXPECTS(eye.data_handle() != out.data_handle());
        EXPECTS(center.data_handle() != out.data_handle());
        EXPECTS(up.data_handle() != out.data_handle());

        const auto z_temp = init_by<vec3data<T>>([&eye, &center](auto& out) noexcept {
            sub(center, eye, as_mdspan_mut<3>(out));
        });
        const auto z      = init_by<vec3data<T>>([&z_temp](auto& out) noexcept {
            normalize(as_mdspan<3>(z_temp), as_mdspan_mut<3>(out));
        });

        const auto x_temp = init_by<vec3data<T>>([&up, &z](auto& out) noexcept {
            cross(up, as_mdspan<3>(z), as_mdspan_mut<3>(out));
        });
        const auto x      = init_by<vec3data<T>>([&x_temp](auto& out) noexcept {
            normalize(as_mdspan<3>(x_temp), as_mdspan_mut<3>(out));
        });

        const auto y_temp = init_by<vec3data<T>>([&z, &x](auto& out) noexcept {
            cross(as_mdspan<3>(z), as_mdspan<3>(x), as_mdspan_mut<3>(out));
        });
        const auto y      = init_by<vec3data<T>>([&y_temp](auto& out) noexcept {
            normalize(as_mdspan<3>(y_temp), as_mdspan_mut<3>(out));
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

        out[3, 0] = -dot(as_mdspan<3>(x), eye);
        out[3, 1] = -dot(as_mdspan<3>(y), eye);
        out[3, 2] = -dot(as_mdspan<3>(z), eye);
        out[3, 3] = 1;
    }
}}} // namespace stormkit::core::math
