// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/api.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.typesafe.safecasts:is_cpo;

import std;

import stormkit.core.types;
import stormkit.core.meta.tag_invoke;
import stormkit.core.meta.concepts;
import stormkit.core.meta.type_manipulation;

export namespace stormkit { inline namespace core {
    struct equal;

    template<typename T>
    struct is_fn final {
        template<typename... Ts>
        static constexpr auto TAG_INVOKABLE = meta::tag_invocable<is_fn<T>, Ts...>;

        template<typename U>
        [[nodiscard]]
        static constexpr auto operator()(const U& value) noexcept -> bool;

        template<typename First, typename Second>
            requires(meta::same_as<T, equal>)
        [[nodiscard]]
        static constexpr auto operator()(const First& first, const Second& second) noexcept -> bool;
    };

    template<typename T>
    inline constexpr auto is_cpo = is_fn<T> {};

    template<typename First, typename Second>
    [[nodiscard]]
    constexpr auto is(const First& first, const Second& second) noexcept -> bool;

    template<typename T, typename U>
    [[nodiscard]]
    constexpr auto is(const U& value) noexcept -> bool;

    // template<typename T, typename U>
    // [[nodiscard]]
    // constexpr auto tag_invoke(cmeta::tag<is<T>>, const U& value) noexcept -> bool;

    template<typename T, meta::plain::unary_predicate<T> Predicate>
    [[nodiscard]]
    constexpr auto tag_invoke(is_fn<equal>, T&& value, Predicate&& predicate) noexcept -> bool;
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    template<typename U>
    STORMKIT_FORCE_INLINE
    constexpr auto is_fn<T>::operator()(const U& value) noexcept -> bool {
        static_assert(TAG_INVOKABLE<const U&>, "Is queryier not defined for these types!");
        return meta::tag_invoke(is_fn<T> {}, value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    template<typename First, typename Second>
        requires(meta::same_as<T, equal>)
    STORMKIT_FORCE_INLINE
    constexpr auto is_fn<T>::operator()(const First& first, const Second& second) noexcept -> bool {
        static_assert(TAG_INVOKABLE<const First&, const Second&> or meta::has_equality_operator<First, Second>,
                      "Is queryier not defined for these types!");

        if constexpr (TAG_INVOKABLE<const First&, const Second&>) return meta::tag_invoke(is_fn<T> {}, first, second);
        else
            return first == second;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename First, typename Second>
    STORMKIT_FORCE_INLINE
    constexpr auto is(const First& first, const Second& second) noexcept -> bool {
        return is_fn<equal> {}(first, second);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, typename U>
    STORMKIT_FORCE_INLINE
    constexpr auto is(const U& value) noexcept -> bool {
        return is_fn<T> {}(value);
    }

    // /////////////////////////////////////
    // /////////////////////////////////////
    // template<typename T, typename U>
    //     STORMKIT_FORCE_INLINE
    // constexpr auto tag_invoke(cmeta::tag<is<T>>, const U& value) noexcept -> bool {
    //     return
    // }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, meta::plain::unary_predicate<T> Predicate>
        STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(is_fn<equal>, T&& value, Predicate&& predicate) noexcept -> bool {
        return std::forward<Predicate>(predicate)(std::forward<T>(value));
    }
}} // namespace stormkit::core
