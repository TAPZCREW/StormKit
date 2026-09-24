// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/api.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.typesafe.safecasts:as_cpo;

import std;

import stormkit.core.types;
import stormkit.core.meta.tag_invoke;
import stormkit.core.meta.concepts;
import stormkit.core.meta.type_manipulation;

export namespace stormkit { inline namespace core {
    struct underlying;
    struct empty;
    struct error;

    template<typename To>
    struct as_fn final {
        template<typename... Ts>
        using invoke_result = meta::tag_invoke_result<as_fn<To>, Ts..., source_location_arg>;

        template<typename... Ts>
        static constexpr auto TAG_INVOKABLE = meta::tag_invocable<as_fn<To>, Ts..., source_location_arg>;

        template<typename... Ts>
        static constexpr auto operator()(Ts&&...) noexcept = delete ("As caster not defined for these types!");

        template<typename... Ts>
        [[nodiscard]]
        static constexpr auto operator()(Ts&&... args) noexcept -> invoke_result<Ts...>
            requires(TAG_INVOKABLE<Ts...>);
    };

    template<typename To>
    inline constexpr auto as = as_fn<To> {};

    static_assert(meta::is<as_fn<usize>, meta::remove_const_of<decltype(as<usize>)>>);
    static_assert(meta::is<as_fn<usize>, meta::tag<as<usize>>>);

    template<typename To, typename From>
    [[nodiscard]]
    constexpr auto unchecked_narrow(From from) noexcept -> To
        requires(meta::is_narrowing<To, From> or (meta::arithmetic<To> and meta::arithmetic<From>));

    template<typename To, meta::plain::same_as<To> From>
    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<To>, From&& value, source_location_arg = std::source_location::current()) noexcept
      -> meta::forward_like<From, To>;
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
    constexpr auto as_fn<To>::operator()(Ts&&... args) noexcept -> invoke_result<Ts...>
        requires(TAG_INVOKABLE<Ts...>)
    {
        return meta::tag_invoke(as<To>, std::forward<Ts>(args)..., std::source_location::current());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To, typename From>
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto unchecked_narrow(From from) noexcept -> To
        requires(meta::is_narrowing<To, From> or (meta::arithmetic<To> and meta::arithmetic<From>))
    {
        return static_cast<To>(from);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To, meta::plain::same_as<To> From>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(as_fn<To>, From&& value, source_location_arg) noexcept -> meta::forward_like<From, To> {
        return std::forward<From>(value);
    }
}} // namespace stormkit::core
