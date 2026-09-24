// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/api.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.typesafe.safecasts:into_cpo;

import std;

import stormkit.core.types;
import stormkit.core.meta.tag_invoke;
import stormkit.core.meta.concepts;

export namespace stormkit { inline namespace core {
    template<typename To>
    struct into_fn final {
        template<typename... Ts>
        using invoke_result = meta::tag_invoke_result<into_fn<To>, Ts...>;

        template<typename... Ts>
        static constexpr auto TAG_INVOKABLE = meta::tag_invocable<into_fn<To>, Ts...>;

        template<typename... Ts>
        static constexpr auto operator()(Ts&&...) noexcept = delete ("Into caster not defined for these types!");

        template<typename T, usize N>
        [[nodiscard]]
        static constexpr auto operator()(T (&values)[N]) noexcept -> invoke_result<decltype(values)>
            requires(TAG_INVOKABLE<T (&)[N]>);

        template<typename T, usize N>
        [[nodiscard]]
        static constexpr auto operator()(T (&&values)[N]) noexcept -> invoke_result<decltype(values)>
            requires(TAG_INVOKABLE<T (&&)[N]>);

        template<typename Tag, typename T, usize N>
        [[nodiscard]]
        static constexpr auto operator()(Tag, T (&values)[N]) noexcept -> invoke_result<Tag, decltype(values)>
            requires(TAG_INVOKABLE<Tag, T (&)[N]>);

        template<typename Tag, typename T, usize N>
        [[nodiscard]]
        static constexpr auto operator()(Tag, T (&&values)[N]) noexcept -> invoke_result<Tag, decltype(values)>
            requires(TAG_INVOKABLE<Tag, T (&&)[N]>);
    };

    template<typename To, typename T, usize N>
    constexpr auto into(T (&values)[N]) -> To;

    template<typename To, typename T, usize N>
    constexpr auto into(T (&&values)[N]) -> To;

    template<template<class> typename To, typename T, usize N>
    constexpr auto into(T (&values)[N]) -> decltype(auto);

    template<template<class> typename To, typename T, usize N>
    constexpr auto into(T (&&values)[N]) -> decltype(auto);

    template<template<class, usize> typename To, typename T, usize N>
    constexpr auto into(T (&values)[N]) -> decltype(auto);

    template<template<class, usize> typename To, typename T, usize N>
    constexpr auto into(T (&&values)[N]) -> decltype(auto);

    template<typename To, typename Tag, typename T, usize N>
    constexpr auto into(Tag, T (&values)[N]) -> decltype(auto);

    template<typename To, typename Tag, typename T, usize N>
    constexpr auto into(Tag, T (&&values)[N]) -> decltype(auto);

    template<template<class> typename To, typename Tag, typename T, usize N>
    constexpr auto into(Tag, T (&values)[N]) -> decltype(auto);

    template<template<class> typename To, typename Tag, typename T, usize N>
    constexpr auto into(Tag, T (&&values)[N]) -> decltype(auto);

    template<template<class, usize> typename To, typename Tag, typename T, usize N>
    constexpr auto into(Tag, T (&values)[N]) -> decltype(auto);

    template<template<class, usize> typename To, typename Tag, typename T, usize N>
    constexpr auto into(Tag, T (&&values)[N]) -> decltype(auto);
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To>
    template<typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto into_fn<To>::operator()(T (&values)[N]) noexcept -> invoke_result<decltype(values)>
        requires(TAG_INVOKABLE<T (&)[N]>)
    {
        return meta::tag_invoke(into_fn<To> {}, values);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To>
    template<typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto into_fn<To>::operator()(T (&&values)[N]) noexcept -> invoke_result<decltype(values)>
        requires(TAG_INVOKABLE<T (&&)[N]>)
    {
        return meta::tag_invoke(into_fn<To> {}, std::move(values));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To>
    template<typename Tag, typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto into_fn<To>::operator()(Tag tag, T (&values)[N]) noexcept -> invoke_result<Tag, decltype(values)>
        requires(TAG_INVOKABLE<Tag, T (&)[N]>)
    {
        return meta::tag_invoke(into_fn<To> {}, tag, values);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To>
    template<typename Tag, typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto into_fn<To>::operator()(Tag tag, T (&&values)[N]) noexcept -> invoke_result<Tag, decltype(values)>
        requires(TAG_INVOKABLE<Tag, T (&&)[N]>)
    {
        return meta::tag_invoke(into_fn<To> {}, tag, std::move(values));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To, typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto into(T (&values)[N]) -> To {
        return into_fn<To> {}(values);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To, typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto into(T (&&values)[N]) -> To {
        return into_fn<To> {}(std::move(values));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class> typename To, typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto into(T (&values)[N]) -> decltype(auto) {
        return into_fn<To<T>> {}(values);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class> typename To, typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto into(T (&&values)[N]) -> decltype(auto) {
        return into_fn<To<T>> {}(std::move(values));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class, usize> typename To, typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto into(T (&values)[N]) -> decltype(auto) {
        return into_fn<To<T, N>> {}(values);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class, usize> typename To, typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto into(T (&&values)[N]) -> decltype(auto) {
        return into_fn<To<T, N>> {}(std::move(values));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To, typename Tag, typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto into(Tag tag, T (&values)[N]) -> decltype(auto) {
        return into_fn<To> {}(tag, values);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To, typename Tag, typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto into(Tag tag, T (&&values)[N]) -> decltype(auto) {
        return into_fn<To> {}(tag, std::move(values));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class> typename To, typename Tag, typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto into(Tag tag, T (&values)[N]) -> decltype(auto) {
        return into_fn<To<T>> {}(tag, values);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class> typename To, typename Tag, typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto into(Tag tag, T (&&values)[N]) -> decltype(auto) {
        return into_fn<To<T>> {}(tag, std::move(values));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class, usize> typename To, typename Tag, typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto into(Tag tag, T (&values)[N]) -> decltype(auto) {
        return into_fn<To<Tag, N>> {}(tag, values);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class, usize> typename To, typename Tag, typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto into(Tag tag, T (&&values)[N]) -> decltype(auto) {
        return into_fn<To<Tag, N>> {}(tag, std::move(values));
    }
}} // namespace stormkit::core
