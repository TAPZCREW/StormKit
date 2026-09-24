// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/api.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.typesafe.safecasts:view_of_cpo;

import std;
import stormkit.core.types;
import stormkit.core.meta.tag_invoke;
import stormkit.core.meta.concepts;
import stormkit.core.meta.type_manipulation;
import stormkit.core.meta.type_query;
import stormkit.core.meta.algorithms;

import stormkit.core.typesafe.ref_ptr;

namespace stdr = std::ranges;
namespace stdv = std::views;

export namespace stormkit { inline namespace core {
    template<bool MUTABLE>
    struct view_of_fn final {
        template<typename From, typename... Ts>
        [[nodiscard]]
        static constexpr auto operator()(From&& from,
                                         Ts&&... args) noexcept(not meta::tag_invocable<view_of_fn, From, Ts...>
                                                                or meta::noexcept_tag_invocable<view_of_fn, From, Ts...>)
          -> decltype(auto);

        template<typename Tag, typename From, typename... Ts>
        [[nodiscard]]
        static constexpr auto operator()(Tag,
                                         From&& from,
                                         Ts&&... args) noexcept(meta::noexcept_tag_invocable<view_of_fn, Tag, From, Ts...>)
          -> meta::tag_invoke_result<view_of_fn, Tag, From, Ts...>
            requires(meta::tag_invocable<view_of_fn, Tag, From, Ts...>);
    };

    inline constexpr auto view_of         = view_of_fn<false> {};
    inline constexpr auto mutable_view_of = view_of_fn<true> {};
}} // namespace stormkit::core

namespace stormkit { inline namespace core { namespace meta {
    template<auto ViewOf, stdr::input_range Range>
    using view_type_from_range = std::invoke_result_t<decltype(ViewOf), range_value_type<Range>&>;

    template<auto ViewOf, view_indirection... Ts>
    using view_type_from_pack = std::invoke_result_t<decltype(ViewOf), indirection_target_type<first_type<Ts...>>>;
}}} // namespace stormkit::core::meta

export namespace stormkit { inline namespace core {
    template<typename... Ts>
    [[nodiscard]]
    constexpr auto views_of(Ts&&... args) noexcept -> array<meta::view_type_from_pack<view_of, Ts...>, sizeof...(args)>;

    template<template<class...> typename To, typename... Ts>
    [[nodiscard]]
    constexpr auto views_of(Ts&&... args) noexcept -> To<meta::view_type_from_pack<view_of, Ts...>>;

    template<template<class...> typename To, stdr::input_range T>
    [[nodiscard]]
    constexpr auto views_of(const T& args) noexcept -> To<meta::view_type_from_range<view_of, T>>;

    template<typename... Ts>
    [[nodiscard]]
    constexpr auto mutable_views_of(Ts&&... args) noexcept
      -> array<meta::view_type_from_pack<mutable_view_of, Ts...>, sizeof...(args)>;

    template<template<class...> typename To, typename... Ts>
    [[nodiscard]]
    constexpr auto mutable_views_of(Ts&&... args) noexcept -> To<meta::view_type_from_pack<mutable_view_of, Ts...>>;

    template<template<class...> typename To, stdr::input_range T>
    [[nodiscard]]
    constexpr auto mutable_views_of(T&& args) noexcept -> To<meta::view_type_from_range<mutable_view_of, meta::to_plain_type<T>>>;
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    template<bool MUTABLE>
    template<typename From, typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto view_of_fn<
      MUTABLE>::operator()(From&& from, Ts&&... args) noexcept(not meta::tag_invocable<view_of_fn, From, Ts...>
                                                               or meta::noexcept_tag_invocable<view_of_fn, From, Ts...>)
      -> decltype(auto) {
        if constexpr (meta::tag_invocable<view_of_fn, From, Ts...>)
            return meta::tag_invoke(view_of_fn {}, std::forward<From>(from), std::forward<Ts>(args)...);
        else if constexpr (meta::plain::pointer<From> and sizeof...(args) == 0)
            return ref_ptr { *from };
        else if constexpr (sizeof...(args) == 0)
            return ref_of(std::forward<From>(from));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<bool MUTABLE>
    template<typename Tag, typename From, typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto view_of_fn<MUTABLE>::
      operator()(Tag tag, From&& from, Ts&&... args) noexcept(meta::noexcept_tag_invocable<view_of_fn, Tag, From, Ts...>)
        -> meta::tag_invoke_result<view_of_fn, Tag, From, Ts...>
        requires(meta::tag_invocable<view_of_fn, Tag, From, Ts...>)
    {
        return meta::tag_invoke(view_of_fn {}, tag, std::forward<From>(from), std::forward<Ts>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto views_of(Ts&&... args) noexcept -> array<meta::view_type_from_pack<false, Ts...>, sizeof...(args)> {
        return array { { view_of(std::forward<Ts>(args))... } };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class...> typename To, typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto views_of(Ts&&... args) noexcept -> To<meta::view_type_from_pack<false, Ts...>> {
        return To { { view_of(std::forward<Ts>(args))... } };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class...> typename To, stdr::input_range T>
    STORMKIT_FORCE_INLINE
    constexpr auto views_of(const T& args) noexcept -> To<meta::view_type_from_range<false, T>> {
        return args | stdv::transform(view_of) | stdr::to<To>();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto mutable_views_of(Ts&&... args) noexcept -> array<meta::view_type_from_pack<false, Ts...>, sizeof...(args)> {
        return array { { mutable_view_of(std::forward<Ts>(args))... } };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class...> typename To, typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto mutable_views_of(Ts&&... args) noexcept -> To<meta::view_type_from_pack<false, Ts...>> {
        return To { { mutable_view_of(std::forward<Ts>(args))... } };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class...> typename To, stdr::input_range T>
    STORMKIT_FORCE_INLINE
    constexpr auto mutable_views_of(T&& args) noexcept -> To<meta::view_type_from_range<false, meta::to_plain_type<T>>> {
        return std::forward<T>(args) | stdv::transform(mutable_view_of) | stdr::to<To>();
    }

    static_assert(meta::is<decltype(view_of(std::declval<std::unique_ptr<int>>())), ref_ptr<int>>);
    static_assert(meta::is<decltype(view_of(std::declval<std::unique_ptr<int>&>())), ref_ptr<int>>);
}} // namespace stormkit::core
