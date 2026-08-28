// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.containers.safecasts;

import std;

import stormkit.core.types;
import stormkit.core.errors;
import stormkit.core.typesafe.safecasts;
import stormkit.core.typesafe.ref_ptr;
import stormkit.core.contract;
import stormkit.core.meta.concepts;
import stormkit.core.meta.type_manipulation;
import stormkit.core.meta.tag_invoke;
import stormkit.core.meta.type_query;
import stormkit.core.meta.algorithms;

namespace stdr = std::ranges;

template<typename T, stormkit::usize EXTENT>
consteval auto get_byte_extent_value_of() -> stormkit::usize {
    if constexpr (EXTENT == std::dynamic_extent) return EXTENT;
    else if constexpr (stormkit::meta::is<stormkit::meta::remove_const_of<T>, void>)
        return EXTENT;
    else
        return EXTENT * sizeof(T);
}

template<auto>
struct require_constant;

template<class R>
constexpr auto get_constexpr_size() -> stormkit::usize {
    if constexpr (std::is_bounded_array_v<R>) return std::extent_v<R>;
    else if constexpr (stormkit::meta::std_array<R>)
        return std::tuple_size_v<R>;
    else if constexpr (stormkit::meta::std_span<R>)
        return R::extent;
    else if constexpr (stdr::sized_range<R> && requires { typename require_constant<R::size()>; })
        return R::size();
    else if constexpr (stdr::contiguous_range<R>)
        return std::dynamic_extent;
    else
        return 1;
}

export namespace stormkit { inline namespace core {
    inline constexpr struct as_bytes_type final {
    } as_bytes = {};

    inline constexpr struct from_bytes_type final {
    } from_bytes = {};

    inline constexpr struct as_ref_ptrs_type final {
    } as_ref_ptrs = {};

    ////////////////////////////////////////////////////////////////////
    ///                           GENERIC                            ///
    ////////////////////////////////////////////////////////////////////
    // into<dynarray>({0, 1, 2, 3, 4, 5})
    template<template<class> typename To, typename T, usize N>
    constexpr auto tag_invoke(into_nttp_fn<To>, T (&values)[N]) noexcept -> To<meta::to_plain_type<T>>;

    template<template<class> typename To, typename T, usize N>
    constexpr auto tag_invoke(into_nttp_fn<To>, T (&&values)[N]) noexcept -> To<meta::to_plain_type<T>>;

    // into<array>({0, 1, 2, 3, 4, 5})
    template<template<class, usize> typename To, typename T, usize N>
    constexpr auto tag_invoke(into_nttp_v_fn<To>, T (&values)[N]) noexcept -> To<meta::to_plain_type<T>, N>;

    template<template<class, usize> typename To, typename T, usize N>
    constexpr auto tag_invoke(into_nttp_v_fn<To>, T (&&values)[N]) noexcept -> To<meta::to_plain_type<T>, N>;

    ////////////////////////////////////////////////////////////////////
    ///                           BYTES                              ///
    ////////////////////////////////////////////////////////////////////
    // as<array_view>(as_bytes, array_view<T>{});
    template<meta::std_span From>
    [[nodiscard]]
    constexpr auto tag_invoke(as_nttp_v_fn<array_view>,
                              as_bytes_type,
                              From value,
                              source_location_arg = std::source_location::current()) noexcept
      -> array_view<meta::forward_const_to<meta::value_type<From>, byte>,
                    get_byte_extent_value_of<meta::value_type<From>, get_constexpr_size<From>()>()>;

    // as<array_view>(as_bytes, dynarray<T>{});
    template<stdr::contiguous_range From>
        requires(not meta::std_span<From>)
    [[nodiscard]]
    constexpr auto tag_invoke(as_nttp_v_fn<array_view>,
                              as_bytes_type,
                              From& value,
                              source_location_arg = std::source_location::current()) noexcept
      -> array_view<meta::forward_const_to<From, byte>, get_byte_extent_value_of<From, get_constexpr_size<From>()>()>;

    // as<array_view>(as_bytes, value);
    template<typename From>
        requires(not stdr::contiguous_range<From>)
    [[nodiscard]]
    constexpr auto tag_invoke(as_nttp_v_fn<array_view>,
                              as_bytes_type,
                              From& value,
                              source_location_arg = std::source_location::current()) noexcept
      -> array_view<meta::forward_const_to<From, byte>, get_byte_extent_value_of<From, 1>()>;

    // as<T>(from_bytes, bytes);
    template<typename To, usize EXTENT>
        requires(EXTENT == std::dynamic_extent or EXTENT == sizeof(To))
    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<To>,
                              from_bytes_type,
                              array_view<const byte, EXTENT> value,
                              source_location_arg = std::source_location::current()) noexcept -> To;

    // as<std::vector<T>>(from_bytes, bytes);
    template<stdr::contiguous_range To, usize EXTENT>
        requires(not meta::std_span<To>)
    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<To>,
                              from_bytes_type,
                              array_view<const byte, EXTENT> value,
                              usize                          count,
                              source_location_arg = std::source_location::current()) noexcept -> To;

    // as<std::array<T, 5>>(from_bytes, bytes);
    template<meta::std_array To, usize EXTENT>
        requires(EXTENT != std::dynamic_extent and (EXTENT / sizeof(meta::value_type<To>)) == get_constexpr_size<To>()
                 or EXTENT == std::dynamic_extent)
    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<To>,
                              from_bytes_type,
                              array_view<const byte, EXTENT> value,
                              source_location_arg = std::source_location::current()) noexcept -> dynarray<To>;

    // into<array>(as_bytes, {0, 1, 2, 3, 4});
    template<meta::explicitly_convertible_to<byte> T, usize EXTENT>
    [[nodiscard]]
    constexpr auto tag_invoke(into_nttp_v_fn<array>, as_bytes_type, array<T, EXTENT> value) noexcept -> array<byte, EXTENT>;

    template<meta::explicitly_convertible_to<byte> T, usize EXTENT>
    [[nodiscard]]
    constexpr auto tag_invoke(into_nttp_fn<dynarray>, as_bytes_type, array<T, EXTENT> value) noexcept -> dynarray<byte>;

    ////////////////////////////////////////////////////////////////////
    ///                           INDIRECTIONS                       ///
    ////////////////////////////////////////////////////////////////////
    namespace meta::details {
        template<typename T>
        concept view_pointer = not meta::lvalue_ref<T> and meta::plain::view_pointer<T>;

        template<typename T>
        concept owning_pointer = meta::lvalue_ref<T> and meta::plain::owning_pointer<T>;

        template<typename T>
        concept reference = meta::lvalue_ref<T> and not meta::owning_pointer<T>;
    } // namespace meta::details

    namespace meta {
        template<typename T>
        concept not_dangling_indirection = details::view_pointer<T> or details::owning_pointer<T> or details::reference<T>;
    }

    // into<dynarray>(as_ref_ptrs, ref_a, ref_b, ptr_a, ref_ptr_b)
    template<template<class...> typename To, meta::not_dangling_indirection... Ts>
    constexpr auto tag_invoke(into_nttp_fn<To>, as_ref_ptrs_type, Ts&&... values) noexcept
      -> To<ref_ptr<const meta::indirection_target_type<meta::first_type<Ts...>>>>
        requires(meta::const_type<meta::indirection_target_type<Ts>> or ...);

    template<template<class...> typename To, meta::not_dangling_indirection... Ts>
    constexpr auto tag_invoke(into_nttp_fn<To>, as_ref_ptrs_type, Ts&&... values) noexcept
      -> To<ref_ptr<meta::indirection_target_type<meta::first_type<Ts...>>>>
        requires(not meta::const_type<meta::indirection_target_type<Ts>> and ...);

    // into<array>(as_ref_ptrs, ref_a, ref_b, ptr_a, ref_ptr_b)
    template<template<class, usize> typename To, meta::not_dangling_indirection... Ts>
    constexpr auto tag_invoke(into_nttp_v_fn<To>, as_ref_ptrs_type, Ts&&... values) noexcept
      -> To<ref_ptr<const meta::indirection_target_type<meta::first_type<Ts...>>>, sizeof...(Ts)>
        requires(meta::const_type<meta::indirection_target_type<Ts>> or ...);

    template<template<class, usize> typename To, meta::not_dangling_indirection... Ts>
    constexpr auto tag_invoke(into_nttp_v_fn<To>, as_ref_ptrs_type, Ts&&... values) noexcept
      -> To<ref_ptr<meta::indirection_target_type<meta::first_type<Ts...>>>, sizeof...(Ts)>
        requires(not meta::const_type<meta::indirection_target_type<Ts>> and ...);

    // as<dynarray>(as_ref_ptrs, array)
    template<template<class...> typename To, stdr::input_range Range>
    constexpr auto tag_invoke(as_nttp_fn<To>, as_ref_ptrs_type, Range& range, source_location_arg)
      -> To<ref_ptr<meta::forward_const_to<Range, meta::range_value_type<Range>>>>;

}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    ////////////////////////////////////////////////////////////////////
    ///                           GENERIC                            ///
    ////////////////////////////////////////////////////////////////////
    namespace details {
        template<template<class, usize> typename To, typename T, usize N, usize... I>
        constexpr auto to_sized_range(T (&values)[N], std::index_sequence<I...>) -> To<meta::to_plain_type<T>, N> {
            return { { values[I]... } };
        }

        template<template<class, usize> typename To, typename T, usize N, usize... I>
        constexpr auto to_sized_range(T (&&values)[N], std::index_sequence<I...>) -> To<meta::to_plain_type<T>, N> {
            return { { std::move(values[I])... } };
        }
    } // namespace details

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class> typename To, typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(into_nttp_fn<To>, T (&values)[N]) noexcept -> To<meta::to_plain_type<T>> {
        return To<meta::to_plain_type<T>> { std::from_range, values };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class> typename To, typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(into_nttp_fn<To>, T (&&values)[N]) noexcept -> To<meta::to_plain_type<T>> {
        return To<meta::to_plain_type<T>> { std::from_range, std::move(values) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class, usize> typename To, typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(into_nttp_v_fn<To>, T (&values)[N]) noexcept -> To<meta::to_plain_type<T>, N> {
        return details::to_sized_range<To>(values, std::make_index_sequence<N> {});
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class, usize> typename To, typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(into_nttp_v_fn<To>, T (&&values)[N]) noexcept -> To<meta::to_plain_type<T>, N> {
        return details::to_sized_range<To>(std::move(values), std::make_index_sequence<N> {});
    }

    ////////////////////////////////////////////////////////////////////
    ///                           BYTES                              ///
    ////////////////////////////////////////////////////////////////////
    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::std_span From>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(as_nttp_v_fn<array_view>, as_bytes_type, From value, source_location_arg) noexcept
      -> array_view<meta::forward_const_to<meta::value_type<From>, byte>,
                    get_byte_extent_value_of<meta::value_type<From>, get_constexpr_size<From>()>()> {
        return { std::bit_cast<meta::forward_const_to<From, byte>*>(stdr::data(value)),
                 get_byte_extent_value_of<From, get_constexpr_size<From>()>() };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<stdr::contiguous_range From>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(as_nttp_v_fn<array_view>, as_bytes_type, From& value, source_location_arg) noexcept
      -> array_view<meta::forward_const_to<From, byte>, get_byte_extent_value_of<From, get_constexpr_size<From>()>()> {
        return { std::bit_cast<meta::forward_const_to<From, byte>*>(stdr::data(value)),
                 get_byte_extent_value_of<From, get_constexpr_size<From>()>() };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename From>
        requires(not stdr::contiguous_range<From>)
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(as_nttp_v_fn<array_view>, as_bytes_type, From& value, source_location_arg) noexcept
      -> array_view<meta::forward_const_to<From, byte>, get_byte_extent_value_of<From, 1>()> {
        return array_view<meta::forward_const_to<From, byte>, get_byte_extent_value_of<From, 1>()> {
            std::bit_cast<meta::forward_const_to<From, byte>*>(&value),
            get_byte_extent_value_of<From, 1>()
        };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To, usize EXTENT>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(as_fn<To>, from_bytes_type, array_view<const byte, EXTENT> value, source_location_arg) noexcept
      -> To {
        if constexpr (EXTENT == std::dynamic_extent) expects(stdr::size(value) == sizeof(To), "Invalid cast of bytes to type T!");

        return std::bit_cast<To>(*stdr::data(value));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<stdr::contiguous_range To, usize EXTENT>
        requires(not meta::std_span<To>)
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(as_fn<To>,
                              from_bytes_type,
                              array_view<const byte, EXTENT> value,
                              usize                          count,
                              source_location_arg) noexcept -> To {
        using To_value_type           = meta::value_type<To>;
        static constexpr auto TO_SIZE = sizeof(To_value_type);

        if constexpr (EXTENT != std::dynamic_extent) {
            expects((EXTENT / TO_SIZE) == count);
            return To { std::from_range, std::bit_cast<array<To_value_type, EXTENT / TO_SIZE>>(*stdr::data(value)) };
        } else {
            expects((stdr::size(value) / TO_SIZE) == count);

            auto out = To {};
            out.reserve(count);

            for (auto it : range(count)) out.emplace_back(std::bit_cast<To>(*(stdr::data(value) + (it * TO_SIZE))));

            return out;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::std_array To, usize EXTENT>
        requires(EXTENT != std::dynamic_extent and (EXTENT / sizeof(meta::value_type<To>)) == get_constexpr_size<To>()
                 or EXTENT == std::dynamic_extent)
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(as_fn<To>, from_bytes_type, array_view<const byte, EXTENT> value, source_location_arg) noexcept
      -> To {
        using To_value_type           = meta::value_type<To>;
        static constexpr auto TO_SIZE = sizeof(To_value_type);
        static constexpr auto COUNT   = get_constexpr_size<To>();

        if constexpr (EXTENT != std::dynamic_extent) expects((EXTENT / TO_SIZE) == COUNT);
        else
            expects((stdr::data(value) / TO_SIZE) == COUNT);
        return std::bit_cast<To>(*stdr::data(value));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::explicitly_convertible_to<byte> T, usize EXTENT>
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto tag_invoke(into_nttp_v_fn<array>, as_bytes_type, array<T, EXTENT> values) noexcept -> array<byte, EXTENT> {
        auto out = array<byte, EXTENT> {};
        stdr::transform(values, stdr::begin(out), [](auto&& value) static noexcept { return static_cast<byte>(value); });
        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::explicitly_convertible_to<byte> T, usize EXTENT>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(into_nttp_fn<dynarray>, as_bytes_type, array<T, EXTENT> values) noexcept -> dynarray<byte> {
        auto out = dynarray<byte> {};
        out.resize(EXTENT);
        stdr::transform(values, stdr::begin(out), [](auto&& value) static noexcept { return static_cast<byte>(value); });
        return out;
    }

    ////////////////////////////////////////////////////////////////////
    ///                           INDIRECTIONS                       ///
    ////////////////////////////////////////////////////////////////////
    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class...> typename To, meta::not_dangling_indirection... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(into_nttp_fn<To>, as_ref_ptrs_type, Ts&&... values) noexcept
      -> To<ref_ptr<const meta::indirection_target_type<meta::first_type<Ts...>>>>
        requires(meta::const_type<meta::indirection_target_type<Ts>> or ...)
    {
        using type = meta::indirection_target_type<meta::first_type<Ts...>>;

        return To<ref_ptr<const type>> { std::forward<Ts>(values)... };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class...> typename To, meta::not_dangling_indirection... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(into_nttp_fn<To>, as_ref_ptrs_type, Ts&&... values) noexcept
      -> To<ref_ptr<meta::indirection_target_type<meta::first_type<Ts...>>>>
        requires(not meta::const_type<meta::indirection_target_type<Ts>> and ...)
    {
        using type = meta::indirection_target_type<meta::first_type<Ts...>>;

        return To<ref_ptr<type>> { std::forward<Ts>(values)... };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class, usize> typename To, meta::not_dangling_indirection... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(into_nttp_v_fn<To>, as_ref_ptrs_type, Ts&&... values) noexcept
      -> To<ref_ptr<const meta::indirection_target_type<meta::first_type<Ts...>>>, sizeof...(Ts)>
        requires(meta::const_type<meta::indirection_target_type<Ts>> or ...)
    {
        using type = meta::indirection_target_type<meta::first_type<Ts...>>;

        return To<ref_ptr<const type>, sizeof...(Ts)> { std::forward<Ts>(values)... };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class, usize> typename To, meta::not_dangling_indirection... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(into_nttp_v_fn<To>, as_ref_ptrs_type, Ts&&... values) noexcept
      -> To<ref_ptr<meta::indirection_target_type<meta::first_type<Ts...>>>, sizeof...(Ts)>
        requires(not meta::const_type<meta::indirection_target_type<Ts>> and ...)
    {
        using type = meta::indirection_target_type<meta::first_type<Ts...>>;

        return To<ref_ptr<type>, sizeof...(Ts)> { std::forward<Ts>(values)... };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class...> typename To, stdr::input_range Range>
    constexpr auto tag_invoke(as_nttp_fn<To>, as_ref_ptrs_type, Range& range, source_location_arg)
      -> To<ref_ptr<meta::forward_const_to<Range, meta::range_value_type<Range>>>> {
        using type = meta::forward_const_to<Range, meta::range_value_type<Range>>;

        auto out = To<ref_ptr<type>> {};
        out.reserve(stdr::size(range));

        if constexpr (requires { out.push_back(std::declval<ref_ptr<type>>); }) stdr::copy(range, std::back_inserter(out));
        else
            stdr::copy(range, std::inserter(out, stdr::end(out)));

        return out;
    }
}} // namespace stormkit::core
