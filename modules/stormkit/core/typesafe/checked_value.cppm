// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/api.hpp>
#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.typesafe.checked_value;

import std;

import stormkit.core.types;

import stormkit.core.meta.concepts;
import stormkit.core.meta.type_manipulation;
import stormkit.core.meta.type_query;
import stormkit.core.contract;
import stormkit.core.string.format;

export namespace stormkit { inline namespace core {
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    struct checked_value final {
      private:
        static constexpr auto IS_ARITHMETIC = meta::arithmetic<T>;

        static constexpr auto BY_VALUE = meta::prefer_pass_by_value<T>;

      public:
        using value_type           = T;
        using reference_type       = T&;
        using const_reference_type = T&;
        using copy_param_type      = meta::in<T>;
        using checked_param_type   = meta::in<checked_value>;

        constexpr checked_value() noexcept(meta::noexcept_default_constructible<value_type>)
            requires(meta::default_constructible<value_type>);

        constexpr checked_value(copy_param_type value) noexcept(meta::noexcept_copyable<value_type>)
            requires(meta::copyable<value_type>);
        constexpr checked_value(value_type&& value) noexcept(meta::noexcept_movable<value_type>)
            requires(not BY_VALUE and meta::movable<value_type>);
        constexpr ~checked_value() noexcept(meta::noexcept_destructible<value_type>);

        constexpr checked_value(const checked_value&) noexcept(meta::noexcept_copyable<value_type>)
            requires(meta::copyable<value_type>);
        constexpr checked_value(checked_value&&) noexcept(meta::noexcept_movable<value_type>)
            requires(meta::movable<value_type>);

        constexpr auto operator=(const checked_value&) noexcept(meta::noexcept_copy_assignable<value_type>) -> checked_value&
            requires(meta::copy_assignable<value_type>);
        constexpr auto operator=(checked_value&&) noexcept(meta::noexcept_move_assignable<value_type>) -> checked_value&
            requires(meta::move_assignable<value_type>);

        constexpr auto operator=(copy_param_type value) noexcept(meta::noexcept_copy_assignable<value_type>) -> checked_value&
            requires(meta::copy_assignable<value_type>);
        constexpr auto operator=(value_type&& value) noexcept(meta::noexcept_move_assignable<value_type>) -> checked_value&
            requires(meta::move_assignable<value_type> and not BY_VALUE);

        constexpr operator reference_type() noexcept STORMKIT_LIFETIMEBOUND;
        constexpr operator const_reference_type() const noexcept STORMKIT_LIFETIMEBOUND;

        constexpr auto operator+(copy_param_type other) const noexcept -> checked_value
            requires(IS_ARITHMETIC);
        constexpr auto operator+(checked_param_type other) const noexcept -> checked_value
            requires(IS_ARITHMETIC);
        constexpr auto operator+=(copy_param_type other) noexcept -> checked_value&
            requires(IS_ARITHMETIC);
        constexpr auto operator+=(checked_param_type other) noexcept -> checked_value&
            requires(IS_ARITHMETIC);

        constexpr auto operator-(copy_param_type other) const noexcept -> checked_value
            requires(IS_ARITHMETIC);
        constexpr auto operator-(checked_param_type other) const noexcept -> checked_value
            requires(IS_ARITHMETIC);
        constexpr auto operator-=(copy_param_type other) noexcept -> checked_value&
            requires(IS_ARITHMETIC);
        constexpr auto operator-=(checked_param_type other) noexcept -> checked_value&
            requires(IS_ARITHMETIC);

        constexpr auto operator*(copy_param_type other) const noexcept -> checked_value
            requires(IS_ARITHMETIC);
        constexpr auto operator*(checked_param_type other) const noexcept -> checked_value
            requires(IS_ARITHMETIC);
        constexpr auto operator*=(copy_param_type other) noexcept -> checked_value&
            requires(IS_ARITHMETIC);
        constexpr auto operator*=(checked_param_type other) noexcept -> checked_value&
            requires(IS_ARITHMETIC);

        constexpr auto operator/(copy_param_type other) const noexcept -> checked_value
            requires(IS_ARITHMETIC);
        constexpr auto operator/(checked_param_type other) const noexcept -> checked_value
            requires(IS_ARITHMETIC);
        constexpr auto operator/=(copy_param_type other) noexcept -> checked_value&
            requires(IS_ARITHMETIC);
        constexpr auto operator/=(checked_param_type other) noexcept -> checked_value&
            requires(IS_ARITHMETIC);

        value_type value;
    };

    namespace meta {
        template<typename T>
        concept is_checked_value = plain::specialization_of_nttp_ttv<T, checked_value>;
    }

    template<meta::arithmetic T>
    using positive = checked_value<T, struct positive_tag, [](T value) static noexcept {
        if constexpr (meta::unsigned_type<T>) return true;
        else
            return value >= 0;
    }>;

    template<meta::arithmetic T>
    using negative = checked_value<T, struct negative_tag, [](T value) static noexcept {
        if constexpr (meta::signed_type<T>) return value < 0;
        else
            return false;
    }>;

    template<typename CharT, typename FormatContext, meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
    constexpr auto tag_invoke(format_as_fn<CharT>, meta::in<checked_value<T, Tag, CHECK_FN>> value, FormatContext& ctx)
      -> decltype(ctx.out());
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE
    constexpr checked_value<T, Tag, CHECK_FN>::checked_value() noexcept(meta::noexcept_default_constructible<value_type>)
        requires(meta::default_constructible<value_type>)
        : value {} {
        // FORMAT HERE
        expects(CHECK_FN(value), "Checked value predicated violation!");
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE
    constexpr checked_value<T, Tag, CHECK_FN>::checked_value(copy_param_type value_) noexcept(meta::noexcept_copyable<value_type>)
        requires(meta::copyable<value_type>)
        : value { value_ } {
        expects(CHECK_FN(value), "Checked value predicated violation!");
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE
    constexpr checked_value<T, Tag, CHECK_FN>::checked_value(value_type&& value_) noexcept(meta::noexcept_movable<value_type>)
        requires(not BY_VALUE and meta::movable<value_type>)
        : value { std::move(value_) } {
        expects(CHECK_FN(value), "Checked value predicated violation!");
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE
    constexpr checked_value<T, Tag, CHECK_FN>::~checked_value() noexcept(meta::noexcept_destructible<value_type>) = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE
    constexpr checked_value<T, Tag, CHECK_FN>::checked_value(const checked_value&
                                                               other) noexcept(meta::noexcept_copyable<value_type>)
        requires(meta::copyable<value_type>)
        : value { other.value } {
        expects(CHECK_FN(value), "Checked value predicated violation!");
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE
    constexpr checked_value<T, Tag, CHECK_FN>::checked_value(checked_value&& other) noexcept(meta::noexcept_movable<value_type>)
        requires(meta::movable<value_type>)
        : value { std::move(other.value) } {
        expects(CHECK_FN(value), "Checked value predicated violation!");
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE
    constexpr auto checked_value<T, Tag, CHECK_FN>::operator=(const checked_value&
                                                                other) noexcept(meta::noexcept_copy_assignable<value_type>)
      -> checked_value&
        requires(meta::copy_assignable<value_type>)
    {
        if (&other == this) [[unlikely]]
            return *this;

        value = other.value;
        expects(CHECK_FN(value), "Checked value predicated violation!");
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE
    constexpr auto checked_value<T, Tag, CHECK_FN>::operator=(checked_value&&
                                                                other) noexcept(meta::noexcept_move_assignable<value_type>)
      -> checked_value&
        requires(meta::move_assignable<value_type>)
    {
        if (&other == this) [[unlikely]]
            return *this;

        value = std::move(other.value);
        expects(CHECK_FN(value), "Checked value predicated violation!");
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE
    constexpr auto checked_value<T, Tag, CHECK_FN>::operator=(copy_param_type
                                                                value_) noexcept(meta::noexcept_copy_assignable<value_type>)
      -> checked_value&
        requires(meta::copy_assignable<value_type>)
    {
        value = value_;
        expects(CHECK_FN(value), "Checked value predicated violation!");
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE
    constexpr auto checked_value<T, Tag, CHECK_FN>::operator=(value_type&&
                                                                value_) noexcept(meta::noexcept_move_assignable<value_type>)
      -> checked_value&
        requires(meta::move_assignable<value_type> and not BY_VALUE)
    {
        value = std::move(value_);
        expects(CHECK_FN(value), "Checked value predicated violation!");
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE
    constexpr checked_value<T, Tag, CHECK_FN>::operator reference_type() noexcept {
        return value;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE
    constexpr checked_value<T, Tag, CHECK_FN>::operator const_reference_type() const noexcept {
        return value;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto checked_value<T, Tag, CHECK_FN>::operator+(copy_param_type other) const noexcept -> checked_value
        requires(IS_ARITHMETIC)
    {
        return { value + other };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto checked_value<T, Tag, CHECK_FN>::operator+(checked_param_type other) const noexcept -> checked_value
        requires(IS_ARITHMETIC)
    {
        return { value + other.value };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE
    constexpr auto checked_value<T, Tag, CHECK_FN>::operator+=(copy_param_type other) noexcept -> checked_value&
        requires(IS_ARITHMETIC)
    {
        value += other;
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE
    constexpr auto checked_value<T, Tag, CHECK_FN>::operator+=(checked_param_type other) noexcept -> checked_value&
        requires(IS_ARITHMETIC)
    {
        value += other.value;
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto checked_value<T, Tag, CHECK_FN>::operator-(copy_param_type other) const noexcept -> checked_value
        requires(IS_ARITHMETIC)
    {
        return { value - other };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto checked_value<T, Tag, CHECK_FN>::operator-(checked_param_type other) const noexcept -> checked_value
        requires(IS_ARITHMETIC)
    {
        return { value - other.value };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE
    constexpr auto checked_value<T, Tag, CHECK_FN>::operator-=(copy_param_type other) noexcept -> checked_value&
        requires(IS_ARITHMETIC)
    {
        value -= other;
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE
    constexpr auto checked_value<T, Tag, CHECK_FN>::operator-=(checked_param_type other) noexcept -> checked_value&
        requires(IS_ARITHMETIC)
    {
        value -= other.value;
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto checked_value<T, Tag, CHECK_FN>::operator*(copy_param_type other) const noexcept -> checked_value
        requires(IS_ARITHMETIC)
    {
        return { value * other };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto checked_value<T, Tag, CHECK_FN>::operator*(checked_param_type other) const noexcept -> checked_value
        requires(IS_ARITHMETIC)
    {
        return { value * other.value };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE
    constexpr auto checked_value<T, Tag, CHECK_FN>::operator*=(copy_param_type other) noexcept -> checked_value&
        requires(IS_ARITHMETIC)
    {
        value *= other;
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE
    constexpr auto checked_value<T, Tag, CHECK_FN>::operator*=(checked_param_type other) noexcept -> checked_value&
        requires(IS_ARITHMETIC)
    {
        value *= other.value;
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto checked_value<T, Tag, CHECK_FN>::operator/(copy_param_type other) const noexcept -> checked_value
        requires(IS_ARITHMETIC)
    {
        return { value / other };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto checked_value<T, Tag, CHECK_FN>::operator/(checked_param_type other) const noexcept -> checked_value
        requires(IS_ARITHMETIC)
    {
        return { value / other.value };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE
    constexpr auto checked_value<T, Tag, CHECK_FN>::operator/=(copy_param_type other) noexcept -> checked_value&
        requires(IS_ARITHMETIC)
    {
        value /= other;
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
        requires(meta::destructible<T> and meta::unary_predicate<decltype(CHECK_FN), T>)
    STORMKIT_FORCE_INLINE
    constexpr auto checked_value<T, Tag, CHECK_FN>::operator/=(checked_param_type other) noexcept -> checked_value&
        requires(IS_ARITHMETIC)
    {
        value /= other.value;
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename CharT, typename FormatContext, meta::is_decayed T, meta::is_decayed Tag, auto CHECK_FN>
    constexpr auto tag_invoke(format_as_fn<CharT>, meta::in<checked_value<T, Tag, CHECK_FN>> value, FormatContext& ctx)
      -> decltype(ctx.out()) {
        return std::format_to(ctx.out(), "{}", value.value);
    }
}} // namespace stormkit::core
