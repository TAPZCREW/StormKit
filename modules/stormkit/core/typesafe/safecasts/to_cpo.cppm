// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/api.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.typesafe.safecasts:to_cpo;

import std;

import :as_cpo;

import stormkit.core.types;
import stormkit.core.meta.tag_invoke;
import stormkit.core.meta.concepts;
import stormkit.core.meta.type_manipulation;
import stormkit.core.meta.type_query;

namespace stdr = std::ranges;

export namespace stormkit { inline namespace core {
    template<typename To>
    struct to_fn final {
        template<typename... Ts>
        using invoke_result = meta::tag_invoke_result<to_fn<To>, Ts..., source_location_arg>;

        template<typename... Ts>
        static constexpr auto TAG_INVOKABLE = meta::tag_invocable<to_fn<To>, Ts..., source_location_arg>;

        template<typename... Ts>
        static constexpr auto AS_TAG_INVOKABLE = meta::tag_invocable<to_fn<To>, Ts..., source_location_arg>;

        template<typename... Ts>
        static constexpr auto operator()(Ts&&...) noexcept = delete ("Try to caster not defined for these types!");

        template<typename... Ts>
        [[nodiscard]]
        static constexpr auto operator()(Ts&&... args) noexcept -> decltype(auto)
            requires(TAG_INVOKABLE<Ts...> or AS_TAG_INVOKABLE<Ts...>);
    };

    template<typename To>
    inline constexpr auto to_cpo = to_fn<To> {};

    template<typename To, typename... Ts>
    constexpr auto to(Ts&&... args) noexcept -> decltype(auto);

    template<template<class> typename To, stdr::input_range T>
    constexpr auto to(T&& range) noexcept -> To<meta::remove_const_of<meta::range_value_type<T>>>;
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To>
    template<typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto to_fn<To>::operator()(Ts&&... args) noexcept -> decltype(auto)
        requires(TAG_INVOKABLE<Ts...> or AS_TAG_INVOKABLE<Ts...>)
    {
        if constexpr (TAG_INVOKABLE<Ts...>)
            return meta::tag_invoke(to_fn<To> {}, std::forward<Ts>(args)..., std::source_location::current());
        else
            return meta::tag_invoke(as_fn<To> {}, std::forward<Ts>(args)..., std::source_location::current());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To, typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto to(Ts&&... args) noexcept -> decltype(auto) {
        return to_fn<To> {}(std::forward<Ts>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class> typename To, stdr::input_range T>
    STORMKIT_FORCE_INLINE
    constexpr auto to(T&& range) noexcept -> To<meta::remove_const_of<meta::range_value_type<T>>> {
        return to_fn<To<meta::remove_const_of<meta::range_value_type<T>>>> {}(std::forward<T>(range));
    }
}} // namespace stormkit::core
