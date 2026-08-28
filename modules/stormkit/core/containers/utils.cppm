// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <version>

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.containers.utils;

import std;

import stormkit.core.meta;
import stormkit.core.types;
import stormkit.core.typesafe;

namespace stdr = std::ranges;

export namespace stormkit { inline namespace core {
    namespace meta {
        template<stdr::input_range... Inputs>
        using common_range_value_type = range_value_type<first_type<Inputs...>>;
    } // namespace meta

    // template<template<class> typename Container = std::inplace_vector, typename T>
    template<template<class> typename Container = dynarray, typename T>
    constexpr auto filled_with(usize size, T value) noexcept -> Container<T>;

    template<usize N, typename T>
    constexpr auto filled_with(T value) noexcept -> array<T, N>;

    template<stdr::range Out, stdr::input_range... Inputs>
    constexpr auto merge(Out& output, const Inputs&... ranges) noexcept -> void;

    template<template<class...> typename Out = dynarray, stdr::input_range... Inputs>
    constexpr auto concat(const Inputs&... inputs) noexcept -> Out<meta::common_range_value_type<Inputs...>>;

    template<stdr::range Out, stdr::input_range... Inputs>
    constexpr auto move_and_merge(Out& output, Inputs&&... ranges) noexcept -> void;

    template<template<class...> typename Out = dynarray, stdr::input_range... Inputs>
    constexpr auto move_and_concat(Inputs&&... inputs) noexcept -> Out<meta::common_range_value_type<Inputs...>>;

    // using std::to_array;

    // template<stdr::input_range T>
    // constexpr auto to_dynarray(T&& range) noexcept -> dynarray<stdr::range_value_t<T>>;

    // template<typename... Ts>
    //     requires(sizeof...(Ts) > 0)
    // constexpr auto into_array(Ts&&... args) noexcept -> array<meta::first_type<Ts...>, sizeof...(Ts)>;

    // template<typename T, meta::is<T>... Ts>
    //     requires(sizeof...(Ts) > 0)
    // constexpr auto into_array_of(Ts&&... args) noexcept -> array<T, sizeof...(Ts)>;

    // template<typename... Ts>
    //     requires(sizeof...(Ts) > 0)
    // constexpr auto into_dynarray(Ts&&... args) noexcept -> dynarray<meta::first_type<Ts...>>;

    // template<typename T, meta::is<T>... Ts>
    //     requires(sizeof...(Ts) > 0)
    // constexpr auto into_dynarray_of(Ts&&... args) noexcept -> dynarray<T>;

    // template<meta::convertible_to<string_view> T>
    // constexpr auto as_view(T& range) noexcept -> string_view;

    // template<typename T>
    //     requires(not stdr::range<T>)
    // constexpr auto as_view(T& value) noexcept -> array_view<T>;

    // template<stdr::contiguous_range T>
    // constexpr auto as_view(T& range) noexcept -> array_view<meta::forward_const_to<T, stdr::range_value_t<T>>>;
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    ////////////////////////////////////////
    ////////////////////////////////////////
    template<template<class> typename Container, typename T>
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto filled_with(usize size, T value) noexcept -> Container<T> {
        auto out = Container<T> {};
        out.resize(size);
        std::ranges::fill(out, value);
        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<usize N, typename T>
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto filled_with(T value) noexcept -> array<T, N> {
        auto out = array<T, N> {};
        std::ranges::fill(out, value);
        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<stdr::range Out, stdr::input_range... Inputs>
    STORMKIT_FORCE_INLINE
    constexpr auto merge(Out& output, const Inputs&... ranges) noexcept -> void {
        static_assert(stdr::output_range<Out, meta::range_value_type<Inputs...[0]>>);
        output.reserve(std::size(output) + (stdr::size(ranges) + ...));
        (stdr::copy(ranges, std::back_inserter(output)), ...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<template<class...> typename Out, stdr::input_range... Inputs>
    STORMKIT_FORCE_INLINE
    STORMKIT_PURE
    constexpr auto concat(const Inputs&... inputs) noexcept -> Out<meta::common_range_value_type<Inputs...>> {
        auto output = Out<meta::common_range_value_type<Inputs...>> {};
        merge(output, inputs...);

        return output;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<stdr::range Out, stdr::input_range... Inputs>
    STORMKIT_FORCE_INLINE
    constexpr auto move_and_merge(Out& output, Inputs&&... inputs) noexcept -> void {
        static_assert(stdr::output_range<Out, meta::range_value_type<Inputs...[0]>>);
        output.reserve(std::size(output) + (stdr::size(inputs) + ...));
        (stdr::move(std::forward<Inputs>(inputs), std::back_inserter(output)), ...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<template<class...> typename Out, stdr::input_range... Inputs>
    STORMKIT_FORCE_INLINE
    STORMKIT_PURE
    constexpr auto move_and_concat(Inputs&&... inputs) noexcept -> Out<meta::common_range_value_type<Inputs...>> {
        auto output = Out<meta::common_range_value_type<Inputs...>> {};
        move_and_merge(output, std::forward<Inputs>(inputs)...);

        return output;
    }

    // /////////////////////////////////////
    // /////////////////////////////////////
    // template<stdr::input_range T>
    // STORMKIT_FORCE_INLINE
    // STORMKIT_PURE
    // constexpr auto to_dynarray(T&& range) noexcept -> dynarray<stdr::range_value_t<T>> {
    //     return std::forward<T>(range) | stdr::to<dynarray<stdr::range_value_t<T>>>();
    // }

    // /////////////////////////////////////
    // /////////////////////////////////////
    // template<typename... Ts>
    //     requires(sizeof...(Ts) > 0)
    // STORMKIT_FORCE_INLINE
    // STORMKIT_PURE
    // constexpr auto into_array(Ts&&... args) noexcept -> array<meta::first_type<Ts...>, sizeof...(Ts)> {
    //     static_assert((not meta::lvalue_ref<Ts> and ...),
    //                   "lvalue reference can't be passed to into_ functions as it take "
    //                   "ownership");
    //     return array { std::move(args)... };
    // }

    // /////////////////////////////////////
    // /////////////////////////////////////
    // template<typename T, meta::is<T>... Ts>
    //     requires(sizeof...(Ts) > 0)
    // STORMKIT_FORCE_INLINE
    // STORMKIT_PURE
    // constexpr auto into_array_of(Ts&&... args) noexcept -> array<T, sizeof...(Ts)> {
    //     return array<T, sizeof...(Ts)> { std::forward<Ts>(args)... };
    // }

    // /////////////////////////////////////
    // /////////////////////////////////////
    // template<typename... Ts>
    //     requires(sizeof...(Ts) > 0)
    // STORMKIT_FORCE_INLINE STORMKIT_PURE
    // constexpr auto into_dynarray(Ts&&... args) noexcept -> dynarray<meta::first_type<Ts...>> {
    //     static_assert((not meta::lvalue_ref<Ts> and ...),
    //                   "lvalue reference can't be passed to into_ functions as it take "
    //                   "ownership");
    //     return dynarray { std::move(args)... };
    // }

    // /////////////////////////////////////
    // /////////////////////////////////////
    // template<typename T, meta::is<T>... Ts>
    //     requires(sizeof...(Ts) > 0)
    // STORMKIT_FORCE_INLINE STORMKIT_PURE
    // constexpr auto into_dynarray_of(Ts&&... args) noexcept -> dynarray<T> {
    //     return dynarray<T> { std::forward<Ts>(args)... };
    // }

    // /////////////////////////////////////
    // /////////////////////////////////////
    // template<meta:: T>
    // STORMKIT_FORCE_INLINE
    // STORMKIT_PURE
    // constexpr auto as_view(T& range) noexcept -> string_view {
    //     return string_view { range };
    // }

    // /////////////////////////////////////
    // /////////////////////////////////////
    // template<typename T>
    //     requires(not stdr::range<T>)
    // STORMKIT_FORCE_INLINE STORMKIT_PURE
    // constexpr auto as_view(T& value) noexcept -> array_view<T> {
    //     return { &value, 1 };
    // }

    // /////////////////////////////////////
    // /////////////////////////////////////
    // template<stdr::contiguous_range T>
    // STORMKIT_FORCE_INLINE STORMKIT_PURE
    // constexpr auto as_view(T& range) noexcept -> array_view<meta::forward_const_to<T, stdr::range_value_t<T>>> {
    //     return { range };
    // }
}} // namespace stormkit::core
