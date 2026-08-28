// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.meta.algorithms;

import std;

import stormkit.core.types;

import stormkit.core.meta.concepts;

namespace stormkit { inline namespace core { namespace meta::details {
    template<typename T>
    struct lazy_type {
        using type = T;
    };
    template<bool, template<class...> typename, typename, typename>
    struct lazy_conditional;

    template<template<class...> typename lazy_type, typename Then, typename OrElse>
    struct lazy_conditional<false, lazy_type, Then, OrElse> {
        using type = lazy_type<OrElse>::type;
    };

    template<template<class...> typename lazy_type, typename Then, typename OrElse>
    struct lazy_conditional<true, lazy_type, Then, OrElse> {
        using type = lazy_type<Then>::type;
    };

#if not(defined(__cpp_pack_indexing) and __cpp_pack_indexing >= 202311L)
    template<usize AT, typename... Ts>
    struct nth_type_impl;

    template<usize AT, typename Head, typename... Ts>
    struct nth_type_impl<AT, Head, Ts...>: nth_type_impl<AT - 1, Ts...> {};

    template<typename Head, typename... Ts>
    struct nth_type_impl<0, Head, Ts...> {
        using type = Head;
    };
#endif
}}} // namespace stormkit::core::meta::details

export namespace stormkit { inline namespace core { namespace meta {
    template<bool COND, typename Then, typename OrElse>
    using lazy_conditional = details::lazy_conditional<COND, details::lazy_type, Then, OrElse>::type;

    template<bool COND, typename Then, typename OrElse>
    using conditional = std::conditional_t<COND, Then, OrElse>;

    template<typename T, template<typename...> concept C, typename... Ts>
    concept negate = not C<T, Ts...>;

    template<typename T, template<typename> concept... C>
    concept all_of = (C<T> and ...);

    template<typename T, template<typename> concept... C>
    concept any_of = (C<T> or ...);

    template<typename Predicate, template<typename...> class Variant, typename... Ts>
    constexpr auto variant_type_find_if(const Variant<Ts...>&, Predicate&& predicate) noexcept -> usize;

    template<typename T, template<typename...> class Variant, typename... Ts>
    constexpr auto variant_contains_type(const Variant<Ts...>) noexcept -> bool;

#if defined(__cpp_pack_indexing) and __cpp_pack_indexing >= 202311L
    template<usize AT, typename... Ts>
    using nth_type = Ts...[AT];
#else
    template<usize AT, typename... Ts>
    using nth_type = typename details::nth_type_impl<AT, Ts...>::type;
#endif

    template<typename... Ts>
    using first_type = nth_type<0, Ts...>;

    template<typename... Ts>
    using last_type = nth_type<sizeof...(Ts) - 1, Ts...>;
}}} // namespace stormkit::core::meta

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core { namespace meta {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename... Ts, typename Predicate>
    STORMKIT_FORCE_INLINE
    constexpr auto variant_type_find_if_impl(Predicate&& predicate) noexcept -> usize {
        auto found = std::variant_npos;
        [&]<usize... Indices>(std::index_sequence<Indices...>) noexcept {
            if constexpr ((requires {
                              { std::forward<Predicate>(predicate).template operator()<Indices, Ts> } -> boolean_testable;
                          } and ...))
                (((std::forward<Predicate>(predicate).template operator()<Indices, Ts>()) and (found = Indices, true)) or ...);
            else
                static_assert(false, "Type not found");
        }(std::index_sequence_for<Ts...>());
        return found;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Predicate, template<typename...> class Variant, typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto variant_type_find_if(const Variant<Ts...>&, Predicate&& predicate) noexcept -> usize {
        return variant_type_find_if_impl<Ts...>(std::forward<Predicate>(predicate));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, template<typename...> class Variant, typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto variant_contains_type(const Variant<Ts...>&) noexcept -> bool {
        if constexpr (meta::is_any_of<T, Ts...>) return true;
        else
            return false;
    }
}}} // namespace stormkit::core::meta
