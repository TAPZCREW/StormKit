// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.math.linear.tensor;

import std;

import stormkit.core.meta;
import stormkit.core.types;
import stormkit.core.ranges;
import stormkit.core.contract;
import stormkit.core.typesafe.safecasts;

namespace cmeta = stormkit::core::meta;
namespace stdr  = std::ranges;

export namespace stormkit::math {
    template<cmeta::arithmetic T, usize... EXTENTS>
        requires(sizeof...(EXTENTS) >= 1)
    using tensor_view = mdarray_view<T, std::extents<usize, EXTENTS...>>;

    template<cmeta::arithmetic T, usize... EXTENTS_>
    struct alignas(array<T, (EXTENTS_ * ...)>) tensor_base {
        using value_type   = T;
        using storage_type = array<value_type, (EXTENTS_ * ...)>;
        using size_type    = usize;
        using rank_type    = usize;

        static constexpr auto EXTENTS = array<size_type, sizeof...(EXTENTS_)> { EXTENTS_... };

        storage_type storage;

        template<typename Self>
        [[nodiscard]]
        constexpr auto operator[](this Self& self, size_type i) noexcept -> cmeta::forward_const_to<Self, value_type>&;

        template<typename Self>
        [[nodiscard]]
        constexpr auto operator[](this Self& self, cmeta::convertible_to<size_type> auto... indices) noexcept
          -> cmeta::forward_const_to<Self, value_type>&
            requires(sizeof...(EXTENTS_) > 1 and sizeof...(indices) == sizeof...(EXTENTS_));

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
        constexpr auto data(this Self& self) noexcept -> cmeta::forward_const_to<Self, value_type>*;

        [[nodiscard]]
        constexpr auto size() const noexcept -> size_type;
        [[nodiscard]]
        static consteval auto max_size() noexcept -> size_type;
        [[nodiscard]]
        static consteval auto order() noexcept -> size_type;

        [[nodiscard]]
        constexpr auto extent(rank_type rank) const noexcept -> size_type;
        [[nodiscard]]
        static constexpr auto static_extent(rank_type rank) noexcept -> size_type;
    };

    template<cmeta::arithmetic T, typename Interface, usize... EXTENTS_>
    struct tensor: tensor_base<T, EXTENTS_...>, Interface {
        template<typename Self>
        constexpr operator tensor_view<cmeta::forward_const_to<Self, T>, EXTENTS_...>(this Self& self) noexcept;
        template<typename Self>
        constexpr operator array_view<cmeta::forward_const_to<Self, T>, (EXTENTS_ * ...)>(this Self& self) noexcept;
    };

    namespace meta {
        template<typename T>
        concept is_tensor = cmeta::specialization_of_nttp_ttv<T, tensor>;

        template<typename T>
        concept is_tensor_view = cmeta::specialization_of_nttp_tv<T, tensor_view>;

        template<typename T>
        concept is_tensor_or_view = is_tensor<T> or is_tensor_view<T>;
    } // namespace meta

    inline constexpr struct as_linear_type {
    } as_linear {};

    template<cmeta::arithmetic T, typename Interface, usize... EXTENTS>
    constexpr auto tag_invoke(cmeta::tag<mutable_view_of>, tensor<T, Interface, EXTENTS...>& value) noexcept
      -> tensor_view<T, EXTENTS...>;

    template<cmeta::arithmetic T, typename Interface, usize... EXTENTS>
    constexpr auto tag_invoke(cmeta::tag<view_of>, const tensor<T, Interface, EXTENTS...>& value) noexcept
      -> tensor_view<const T, EXTENTS...>;

    template<cmeta::arithmetic T, typename Interface, usize... EXTENTS>
    constexpr auto tag_invoke(cmeta::tag<mutable_view_of>, as_linear_type, tensor<T, Interface, EXTENTS...>& value) noexcept
      -> array_view<T, (EXTENTS * ...)>;

    template<cmeta::arithmetic T, typename Interface, usize... EXTENTS>
    constexpr auto tag_invoke(cmeta::tag<view_of>, as_linear_type, const tensor<T, Interface, EXTENTS...>& value) noexcept
      -> array_view<const T, (EXTENTS * ...)>;

    template<cmeta::arithmetic T, usize... EXTENTS>
    constexpr auto tag_invoke(cmeta::tag<mutable_view_of>, tensor_view<const T, EXTENTS...> value) noexcept
      -> tensor_view<T, EXTENTS...>;

    template<cmeta::arithmetic T, usize... EXTENTS>
    constexpr auto tag_invoke(cmeta::tag<view_of>, tensor_view<const T, EXTENTS...> value) noexcept
      -> tensor_view<const T, EXTENTS...>;

    template<cmeta::arithmetic T, usize... EXTENTS>
    constexpr auto tag_invoke(cmeta::tag<mutable_view_of>, as_linear_type, tensor_view<T, EXTENTS...> value) noexcept
      -> array_view<T, (EXTENTS * ...)>;

    template<cmeta::arithmetic T, usize... EXTENTS>
    constexpr auto tag_invoke(cmeta::tag<view_of>, as_linear_type, tensor_view<const T, EXTENTS...> value) noexcept
      -> array_view<const T, (EXTENTS * ...)>;

    template<meta::is_tensor T>
    [[nodiscard]]
    constexpr auto add(const T& a, const T& b) noexcept -> T;

    template<cmeta::arithmetic T, usize... EXTENTS>
    constexpr auto add(tensor_view<const T, EXTENTS...> a,
                       tensor_view<const T, EXTENTS...> b,
                       tensor_view<T, EXTENTS...>       out) noexcept -> void;

    template<meta::is_tensor T>
    [[nodiscard]]
    constexpr auto sub(const T& a, const T& b) noexcept -> T;

    template<cmeta::arithmetic T, usize... EXTENTS>
    constexpr auto sub(tensor_view<const T, EXTENTS...> a,
                       tensor_view<const T, EXTENTS...> b,
                       tensor_view<T, EXTENTS...>       out) noexcept -> void;

    template<meta::is_tensor T>
    [[nodiscard]]
    constexpr auto mul(const T& a, cmeta::value_type<T> b) noexcept -> T;

    template<cmeta::arithmetic T, usize... EXTENTS>
    constexpr auto mul(tensor_view<const T, EXTENTS...> a, T b, tensor_view<T, EXTENTS...> out) noexcept -> void;

    template<meta::is_tensor T>
    [[nodiscard]]
    constexpr auto div(const T& a, cmeta::value_type<T> b) noexcept -> T;

    template<cmeta::arithmetic T, usize... EXTENTS>
    constexpr auto div(tensor_view<const T, EXTENTS...> a, T b, tensor_view<T, EXTENTS...> out) noexcept -> void;
} // namespace stormkit::math

export template<cmeta::arithmetic T, typename Interface, stormkit::core::usize... EXTENTS_>
constexpr auto std::format_kind<stormkit::math::tensor<T, Interface, EXTENTS_...>> = std::range_format::disabled;

namespace stormkit::math {
    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize... EXTENTS_>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto tensor_base<T, EXTENTS_...>::operator[](this Self& self, size_type i) noexcept
      -> cmeta::forward_const_to<Self, value_type>& {
        return self.storage[i];
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize... EXTENTS_>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto tensor_base<T, EXTENTS_...>::operator[](this Self& self,
                                                           cmeta::convertible_to<size_type> auto... indices) noexcept
      -> cmeta::forward_const_to<Self, value_type>&
        requires(sizeof...(EXTENTS_) > 1 and sizeof...(indices) == sizeof...(EXTENTS_))
    {
        auto indices_ = std::array { as<size_type>(indices)... };
        if constexpr (sizeof...(EXTENTS_) == 2) return self.operator[](indices_[0] * EXTENTS[1] + indices_[1]);
        else {
            static_assert(false, "not implemented yet!");
            // auto id  = indices_[0];
            // auto acc = 1;
            // for (auto i : range(sizeof...(indices))) {
            //     acc *= EXTENTS[i];
            //     id += (indices_[i] * acc);
            // }

            // return self.operator[](id);
        }
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize... EXTENTS_>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto tensor_base<T, EXTENTS_...>::begin(this Self& self) noexcept -> decltype(auto) {
        return stdr::begin(self.storage);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize... EXTENTS_>
    STORMKIT_FORCE_INLINE
    constexpr auto tensor_base<T, EXTENTS_...>::cbegin() const noexcept -> decltype(auto) {
        return stdr::cbegin(storage);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize... EXTENTS_>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto tensor_base<T, EXTENTS_...>::end(this Self& self) noexcept -> decltype(auto) {
        return stdr::end(self.storage);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize... EXTENTS_>
    STORMKIT_FORCE_INLINE
    constexpr auto tensor_base<T, EXTENTS_...>::cend() const noexcept -> decltype(auto) {
        return stdr::cend(storage);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize... EXTENTS_>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto tensor_base<T, EXTENTS_...>::data(this Self& self) noexcept -> cmeta::forward_const_to<Self, value_type>* {
        return stdr::data(self.storage);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize... EXTENTS_>
    STORMKIT_CONST STORMKIT_FORCE_INLINE
    constexpr auto tensor_base<T, EXTENTS_...>::size() const noexcept -> size_type {
        return stdr::size(storage);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize... EXTENTS_>
    STORMKIT_CONST STORMKIT_FORCE_INLINE
    consteval auto tensor_base<T, EXTENTS_...>::max_size() noexcept -> size_type {
        return (EXTENTS_ * ...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize... EXTENTS_>
    STORMKIT_CONST STORMKIT_FORCE_INLINE
    consteval auto tensor_base<T, EXTENTS_...>::order() noexcept -> size_type {
        return sizeof...(EXTENTS_);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize... EXTENTS_>
    STORMKIT_CONST STORMKIT_FORCE_INLINE
    constexpr auto tensor_base<T, EXTENTS_...>::extent(rank_type rank) const noexcept -> size_type {
        expects(rank < sizeof...(EXTENTS_));
        return EXTENTS[rank];
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize... EXTENTS_>
    STORMKIT_CONST STORMKIT_FORCE_INLINE
    constexpr auto tensor_base<T, EXTENTS_...>::static_extent(rank_type rank) noexcept -> size_type {
        expects(rank < sizeof...(EXTENTS_));
        return EXTENTS[rank];
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, typename Interface, usize... EXTENTS_>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr tensor<T, Interface, EXTENTS_...>::operator tensor_view<cmeta::forward_const_to<Self, T>,
                                                                      EXTENTS_...>(this Self& self) noexcept {
        return view_of(self);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, typename Interface, usize... EXTENTS_>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr tensor<T, Interface, EXTENTS_...>::operator array_view<cmeta::forward_const_to<Self, T>,
                                                                     (EXTENTS_ * ...)>(this Self& self) noexcept {
        return view_of(as_linear, self);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, typename Interface, usize... EXTENTS>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(cmeta::tag<mutable_view_of>, tensor<T, Interface, EXTENTS...>& value) noexcept
      -> tensor_view<T, EXTENTS...> {
        return tensor_view<T, EXTENTS...> { stdr::data(value.storage) };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, typename Interface, usize... EXTENTS>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(cmeta::tag<view_of>, const tensor<T, Interface, EXTENTS...>& value) noexcept
      -> tensor_view<const T, EXTENTS...> {
        return tensor_view<const T, EXTENTS...> { stdr::data(value.storage) };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, typename Interface, usize... EXTENTS>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(cmeta::tag<mutable_view_of>, as_linear_type, tensor<T, Interface, EXTENTS...>& value) noexcept
      -> array_view<T, (EXTENTS * ...)> {
        return array_view<T, (EXTENTS * ...)> { stdr::data(value.storage), (EXTENTS * ...) };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, typename Interface, usize... EXTENTS>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(cmeta::tag<view_of>, as_linear_type, const tensor<T, Interface, EXTENTS...>& value) noexcept
      -> array_view<const T, (EXTENTS * ...)> {
        return array_view<const T, (EXTENTS * ...)> { stdr::data(value.storage), (EXTENTS * ...) };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize... EXTENTS>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(cmeta::tag<mutable_view_of>, tensor_view<const T, EXTENTS...> value) noexcept
      -> tensor_view<T, EXTENTS...> {
        return value;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize... EXTENTS>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(cmeta::tag<view_of>, tensor_view<const T, EXTENTS...> value) noexcept
      -> tensor_view<const T, EXTENTS...> {
        return value;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize... EXTENTS>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(cmeta::tag<mutable_view_of>, as_linear_type, tensor_view<T, EXTENTS...> value) noexcept
      -> array_view<T, (EXTENTS * ...)> {
        return array_view<T, (EXTENTS * ...)> { value.data_handle(), (EXTENTS * ...) };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize... EXTENTS>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(cmeta::tag<view_of>, as_linear_type, tensor_view<const T, EXTENTS...> value) noexcept
      -> array_view<const T, (EXTENTS * ...)> {
        return array_view<const T, (EXTENTS * ...)> { value.data_handle(), (EXTENTS * ...) };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_tensor T>
    STORMKIT_FORCE_INLINE
    constexpr auto add(const T& a, const T& b) noexcept -> T {
        auto out = T {};
        add(view_of(a), view_of(b), mutable_view_of(out));
        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize... EXTENTS>
    STORMKIT_FORCE_INLINE
    constexpr auto add(tensor_view<const T, EXTENTS...> a,
                       tensor_view<const T, EXTENTS...> b,
                       tensor_view<T, EXTENTS...>       out) noexcept -> void {
        EXPECTS(a.data_handle() != out.data_handle());
        EXPECTS(b.data_handle() != out.data_handle());

        const auto a_linear   = view_of(as_linear, a);
        const auto b_linear   = view_of(as_linear, b);
        auto       out_linear = mutable_view_of(as_linear, out);
        for (auto i = 0u; i < (EXTENTS * ...); ++i) out_linear[i] = a_linear[i] + b_linear[i];
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_tensor T>
    STORMKIT_FORCE_INLINE
    constexpr auto sub(const T& a, const T& b) noexcept -> T {
        auto out = T {};
        sub(view_of(a), view_of(b), mutable_view_of(out));
        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize... EXTENTS>
    constexpr auto sub(tensor_view<const T, EXTENTS...> a,
                       tensor_view<const T, EXTENTS...> b,
                       tensor_view<T, EXTENTS...>       out) noexcept -> void {
        EXPECTS(a.data_handle() != out.data_handle());
        EXPECTS(b.data_handle() != out.data_handle());

        const auto a_linear   = view_of(as_linear, a);
        const auto b_linear   = view_of(as_linear, b);
        auto       out_linear = mutable_view_of(as_linear, out);
        for (auto i = 0u; i < (EXTENTS * ...); ++i) out_linear[i] = a_linear[i] - b_linear[i];
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_tensor T>
    STORMKIT_FORCE_INLINE
    constexpr auto mul(const T& a, cmeta::value_type<T> b) noexcept -> T {
        auto out = T {};
        mul(view_of(a), b, mutable_view_of(out));
        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize... EXTENTS>
    constexpr auto mul(tensor_view<const T, EXTENTS...> a, T b, tensor_view<T, EXTENTS...> out) noexcept -> void {
        EXPECTS(a.data_handle() != out.data_handle());

        const auto a_linear   = view_of(as_linear, a);
        auto       out_linear = mutable_view_of(as_linear, out);
        for (auto i = 0u; i < (EXTENTS * ...); ++i) out_linear[i] = a_linear[i] * b;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_tensor T>
    STORMKIT_FORCE_INLINE
    constexpr auto div(const T& a, cmeta::value_type<T> b) noexcept -> T {
        auto out = T {};
        div(view_of(a), b, mutable_view_of(out));
        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::arithmetic T, usize... EXTENTS>
    constexpr auto div(tensor_view<const T, EXTENTS...> a, T b, tensor_view<T, EXTENTS...> out) noexcept -> void {
        EXPECTS(a.data_handle() != out.data_handle());
        EXPECTS(not is(b, T { 0 }));

        const auto a_linear   = view_of(as_linear, a);
        auto       out_linear = mutable_view_of(as_linear, out);
        for (auto i = 0u; i < (EXTENTS * ...); ++i) out_linear[i] = a_linear[i] / b;
    }

} // namespace stormkit::math
