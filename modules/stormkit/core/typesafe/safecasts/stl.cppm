// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/api.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.typesafe.safecasts:stl;

import std;

import :as_cpo;
import :to_cpo;
import :is_cpo;
import :into_cpo;
import :view_of_cpo;

import stormkit.core.types;
import stormkit.core.contract;
import stormkit.core.errors;
import stormkit.core.typesafe.ref_ptr;
import stormkit.core.meta.type_query;
import stormkit.core.meta.algorithms;

namespace stdr = std::ranges;
namespace stdv = std::views;

namespace stormkit { inline namespace core { namespace meta::details {
    template<typename T, usize EXTENT>
    consteval auto get_byte_extent_value_of() -> usize {
        if constexpr (EXTENT == std::dynamic_extent) return EXTENT;
        else if constexpr (is<remove_const_of<T>, void>)
            return EXTENT;
        else
            return EXTENT * sizeof(T);
    }

    template<auto>
    struct require_constant;

    template<typename R>
    constexpr auto get_constexpr_size() -> usize {
        if constexpr (std::is_bounded_array_v<R>) return std::extent_v<R>;
        else if constexpr (std_array<R>)
            return std::tuple_size_v<R>;
        else if constexpr (std_span<R>)
            return R::extent;
        else if constexpr (stdr::sized_range<R> && requires { typename require_constant<R::size()>; })
            return R::size();
        else if constexpr (stdr::contiguous_range<R>)
            return std::dynamic_extent;
        else
            return 1;
    }

}}} // namespace stormkit::core::meta::details

export namespace stormkit { inline namespace core {
    namespace meta {
        template<typename T, typename CharT = char>
        concept has_as_string_view = requires(const T& value) {
            { as<basic_string_view<CharT>>(value) } -> same_as<basic_string_view<CharT>>;
        };

        template<typename T, typename CharT = char>
        concept has_as_string = requires(const T& value) {
            { as<basic_string<CharT>>(value) } -> same_as<basic_string<CharT>>;
        };

        namespace plain {
            template<typename T, typename CharT = char>
            concept has_as_string_view = apply_to<T, meta::has_as_string_view, CharT>;

            template<typename T, typename CharT = char>
            concept has_as_string = apply_to<T, meta::has_as_string, CharT>;
        } // namespace plain
    } // namespace meta

    inline constexpr struct as_bytes_type final {
    } as_bytes;

    ////////////////////////////////////////////////////////////////////
    ///                      VARIANT                                 ///
    ////////////////////////////////////////////////////////////////////
    template<typename T, meta::std_variant Variant>
    [[nodiscard]]
    constexpr auto tag_invoke(is_fn<T>, const Variant& variant) noexcept -> bool;

    template<typename To, meta::plain::apply_to<meta::std_variant> From>
    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<To>, From&& variant, source_location_arg = std::source_location::current()) noexcept
      -> meta::forward_like<From, To>;

    ////////////////////////////////////////////////////////////////////
    ///                      EXPECTED                                ///
    ////////////////////////////////////////////////////////////////////
    template<meta::same_as_any_of<empty, error> T, meta::std_expected Expected>
    [[nodiscard]]
    constexpr auto tag_invoke(is_fn<T>, const Expected& expected) noexcept -> bool;

    template<typename T, meta::std_expected Expected>
    [[nodiscard]]
    constexpr auto tag_invoke(is_fn<T>, const Expected& expected) noexcept -> bool
        requires(meta::same_as<T, meta::value_type<Expected>>);

    template<typename T, meta::std_expected Expected>
    [[nodiscard]]
    constexpr auto tag_invoke(is_fn<T>, const Expected& expected) noexcept -> bool
        requires(meta::same_as<T, std::unexpected<meta::error_type<Expected>>>);

    template<typename To, meta::plain::apply_to<meta::std_expected> Expected>
    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<To>, Expected&& expected, source_location_arg = std::source_location::current()) noexcept
      -> meta::forward_like<Expected, To>
        requires(meta::same_as<To, meta::value_type<Expected>>);

    template<typename To, meta::plain::apply_to<meta::std_expected> Expected>
    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<To>, Expected&& expected, source_location_arg = std::source_location::current()) noexcept
      -> meta::forward_like<Expected, To>
        requires(meta::same_as<To, std::unexpected<meta::error_type<Expected>>>);

    ////////////////////////////////////////////////////////////////////
    ///                      OPTIONAL                                ///
    ////////////////////////////////////////////////////////////////////
    template<typename T, meta::std_optional Optional>
    [[nodiscard]]
    constexpr auto tag_invoke(is_fn<T>, Optional& optional) noexcept -> bool;

    template<typename To, meta::plain::apply_to<meta::std_optional> From>
    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<To>, From&& optional, source_location_arg = std::source_location::current()) noexcept
      -> meta::forward_like<From, To>;

    ////////////////////////////////////////////////////////////////////
    ///                           CONTAINERS                     ///
    ////////////////////////////////////////////////////////////////////
    // into<dynarray>({0, 1, 2, 3, 4, 5})
    template<typename To, typename T, usize N>
    constexpr auto tag_invoke(into_fn<To>, T (&values)[N]) noexcept -> meta::replace_first<T, To>;

    template<typename To, typename T, usize N>
    constexpr auto tag_invoke(into_fn<To>, T (&&values)[N]) noexcept -> meta::replace_first<T, To>;

    template<typename To, meta::explicitly_convertible_to<byte> T, usize N>
    [[nodiscard]]
    constexpr auto tag_invoke(into_fn<To>, as_bytes_type, T (&values)[N]) noexcept -> meta::replace_first<byte, To>;

    template<typename To, meta::explicitly_convertible_to<byte> T, usize N>
    [[nodiscard]]
    constexpr auto tag_invoke(into_fn<To>, as_bytes_type, T (&&values)[N]) noexcept -> meta::replace_first<byte, To>;

    ////////////////////////////////////////////////////////////////////
    ///                           VIEWS                              ///
    ////////////////////////////////////////////////////////////////////
    // view_of(vector);
    template<stdr::contiguous_range From>
    [[nodiscard]]
    constexpr auto tag_invoke(meta::tag<mutable_view_of>, From& value) noexcept
      -> array_view<meta::range_value_type<From>, meta::details::get_constexpr_size<From>()>
        requires(not meta::constant_range<From>);

    template<stdr::contiguous_range From>
    [[nodiscard]]
    constexpr auto tag_invoke(meta::tag<view_of>, const From& value) noexcept
      -> array_view<const meta::range_value_type<From>, meta::details::get_constexpr_size<From>()>;

    template<stdr::contiguous_range From>
    [[nodiscard]]
    constexpr auto tag_invoke(meta::tag<mutable_view_of>, as_bytes_type, From& value) noexcept
      -> array_view<byte, meta::details::get_byte_extent_value_of<From, meta::details::get_constexpr_size<From>()>()>
        requires(/*not meta::std_span<From> and */ not meta::constant_range<From>);

    template<stdr::contiguous_range From>
    [[nodiscard]]
    constexpr auto tag_invoke(meta::tag<view_of>, as_bytes_type, const From& value) noexcept
      -> array_view<const byte, meta::details::get_byte_extent_value_of<From, meta::details::get_constexpr_size<From>()>()>;

    // view_of(as_bytes, value);
    template<meta::standard_layout From>
    [[nodiscard]]
    constexpr auto tag_invoke(meta::tag<mutable_view_of>, as_bytes_type, From& value) noexcept
      -> array_view<byte, meta::details::get_byte_extent_value_of<From, 1>()>
        requires(not stdr::input_range<From>);

    template<meta::standard_layout From>
    [[nodiscard]]
    constexpr auto tag_invoke(meta::tag<view_of>, as_bytes_type, const From& value) noexcept
      -> array_view<const byte, meta::details::get_byte_extent_value_of<From, 1>()>
        requires(not stdr::input_range<From>);

    template<typename From>
    [[nodiscard]]
    constexpr auto bytes_of(From& value) noexcept -> decltype(auto);

    template<typename From>
    [[nodiscard]]
    constexpr auto mutable_bytes_of(const From& value) noexcept -> decltype(auto);

    ////////////////////////////////////////////////////////////////////
    ///                           STRING                             ///
    ////////////////////////////////////////////////////////////////////
    [[nodiscard]]
    constexpr auto tag_invoke(meta::tag<as<string>>,
                              string_view value,
                              source_location_arg = std::source_location::current()) noexcept -> string;

    template<meta::has_as_string_view From>
    [[nodiscard]]
    constexpr auto tag_invoke(meta::tag<as<string>>, From&& value, source_location_arg = std::source_location::current()) noexcept
      -> string
        requires(not meta::plain::is<From, string>);

    template<meta::plain::integral From>
    [[nodiscard]]
    constexpr auto tag_invoke(meta::tag<as<string>>,
                              From value,
                              i32  base           = 10,
                              source_location_arg = std::source_location::current()) noexcept -> string;

    template<meta::plain::floating_point From>
    [[nodiscard]]
    constexpr auto tag_invoke(meta::tag<as<string>>,
                              From              value,
                              std::chars_format fmt = std::chars_format::general,
                              source_location_arg   = std::source_location::current()) noexcept -> string;

    template<meta::plain::integral To>
    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<To>,
                              string_view value,
                              i32         base    = 10,
                              source_location_arg = std::source_location::current()) noexcept -> To;

    template<meta::plain::floating_point To>
    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<To>,
                              string_view       value,
                              std::chars_format fmt = std::chars_format::general,
                              source_location_arg   = std::source_location::current()) noexcept -> To;

    template<meta::plain::integral From>
    [[nodiscard]]
    constexpr auto tag_invoke(to_fn<string>,
                              From value,
                              i32  base           = 10,
                              source_location_arg = std::source_location::current()) noexcept -> system_result<string>;

    template<meta::plain::floating_point From>
    [[nodiscard]]
    constexpr auto tag_invoke(to_fn<string>,
                              From              value,
                              std::chars_format fmt = std::chars_format::general,
                              source_location_arg   = std::source_location::current()) noexcept -> system_result<string>;

    template<meta::plain::integral To>
    [[nodiscard]]
    constexpr auto tag_invoke(to_fn<To>,
                              string_view value,
                              i32         base    = 10,
                              source_location_arg = std::source_location::current()) noexcept -> system_result<To>;

    template<meta::plain::floating_point To>
    [[nodiscard]]
    constexpr auto tag_invoke(to_fn<To>,
                              string_view       value,
                              std::chars_format fmt = std::chars_format::general,
                              source_location_arg   = std::source_location::current()) noexcept -> system_result<To>;

    // view_of(as_bytes, array_view<T>{});
    // template<meta::std_span From>
    // [[nodiscard]]
    // constexpr auto tag_invoke(view_of,
    //                           as_writable_bytes,
    //                           From value,
    //                           source_location_arg = std::source_location::current()) noexcept
    //   -> array_view<byte, get_byte_extent_value_of<meta::value_type<From>, get_constexpr_size<From>()>()>
    //     requires(not meta::const_type<meta::range_value_type<From>>);

    // template<meta::std_span From>
    // [[nodiscard]]
    // constexpr auto tag_invoke(view_of, as_bytes_type, From value, source_location_arg = std::source_location::current())
    // noexcept
    //   -> array_view<const byte, get_byte_extent_value_of<meta::value_type<From>, get_constexpr_size<From>()>()>;

}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    ////////////////////////////////////////////////////////////////////
    ///                      VARIANT                                 ///
    ////////////////////////////////////////////////////////////////////
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, meta::std_variant Variant>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(is_fn<T>, const Variant& value) noexcept -> bool {
        if constexpr (meta::same_as<T, empty>) {
            if constexpr (requires { variant_contains_type<std::monostate>(std::declval<Variant>()) == true; })
                return std::get_if<std::monostate>(&value) != nullptr;
            else
                return false;
        } else {
            return meta::variant_type_find_if(value,
                                              [&value]<typename It>(const It&) noexcept -> bool {
                                                  if (value.index() == It::INDEX)
                                                      return meta::same_as<T, std::variant_alternative_t<It::INDEX, Variant>>;
                                                  return false;
                                              })
                   != std::variant_npos;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To, meta::plain::apply_to<meta::std_variant> Variant>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(as_fn<To>, Variant&& value, source_location_arg location) noexcept
      -> meta::forward_like<Variant, To> {
        auto ptr = raw_ptr<meta::forward_const_to<Variant, To>> { nullptr };
        meta::variant_type_find_if(std::forward<Variant>(value), [&ptr, &value]<typename It>(const It&) noexcept -> bool {
            if constexpr (meta::same_as<typename It::type, To>) {
                if (std::forward<Variant>(value).index() == It::INDEX) ptr = &std::get<It::index>(value);
                return true;
            }

            return false;
        });

        ensures(ptr != nullptr, "Bad variant access!", location);
        return std::forward_like<Variant>(*ptr);
    }

    ////////////////////////////////////////////////////////////////////
    ///                      EXPECTED                                ///
    ////////////////////////////////////////////////////////////////////
    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::same_as_any_of<empty, error> T, meta::std_expected Expected>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(is_fn<T>, const Expected& value) noexcept -> bool {
        return not value.has_value();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, meta::std_expected Expected>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(is_fn<T>, const Expected& value) noexcept -> bool
        requires(meta::same_as<T, meta::value_type<Expected>>)
    {
        return value.has_value();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, meta::std_expected Expected>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(is_fn<T>, const Expected& value) noexcept -> bool
        requires(meta::same_as<T, std::unexpected<meta::error_type<Expected>>>)
    {
        return not value.has_value();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To, meta::plain::apply_to<meta::std_expected> Expected>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(as_fn<To>, Expected&& value, source_location_arg location) noexcept
      -> meta::forward_like<Expected, To>
        requires(meta::same_as<To, meta::value_type<Expected>>)
    {
        ensures(value.has_value(), "Bad expected access!", location);

        return std::forward_like<Expected>(value.value());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To, meta::plain::apply_to<meta::std_expected> Expected>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(as_fn<To>, Expected&& value, source_location_arg location) noexcept
      -> meta::forward_like<Expected, To>
        requires(meta::same_as<To, std::unexpected<meta::error_type<Expected>>>)
    {
        ensures(not value.has_value(), "Bad expected access!", location);

        return std::forward_like<Expected>(value.error());
    }

    ////////////////////////////////////////////////////////////////////
    ///                      OPTIONAL                                ///
    ////////////////////////////////////////////////////////////////////
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, meta::std_optional Optional>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(is_fn<T>, const Optional& value) noexcept -> bool {
        if (not value.has_value()) return meta::same_as<T, empty>;
        return meta::is<T, meta::value_type<Optional>>;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To, meta::plain::apply_to<meta::std_optional> Optional>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(as_fn<To>, Optional&& value, source_location_arg location) noexcept
      -> meta::forward_like<Optional, To> {
        ensures(is<To>(value), "Bad optional access!", location);

        return std::forward_like<Optional>(value.value());
    }

    ////////////////////////////////////////////////////////////////////
    ///                               CONTAINERS                     ///
    ////////////////////////////////////////////////////////////////////
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To, typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(into_fn<To>, T (&values)[N]) noexcept -> meta::replace_first<T, To> {
        return []<std::size_t... I>(T(&values)[N], std::index_sequence<I...>) -> meta::replace_first<T, To> {
            return { { values[I]... } };
        }(values, std::make_index_sequence<N> {});
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To, typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(into_fn<To>, T (&&values)[N]) noexcept -> meta::replace_first<T, To> {
        return []<std::size_t... I>(T(&&values)[N], std::index_sequence<I...>) -> meta::replace_first<T, To> {
            return { { std::move(values[I])... } };
        }(std::move(values), std::make_index_sequence<N> {});
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To, meta::explicitly_convertible_to<byte> T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(into_fn<To>, as_bytes_type, T (&values)[N]) noexcept -> meta::replace_first<byte, To> {
        return []<std::size_t... I>(T(&values)[N], std::index_sequence<I...>) -> meta::replace_first<byte, To> {
            return { { as<byte>(values[I])... } };
        }(values, std::make_index_sequence<N> {});
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To, meta::explicitly_convertible_to<byte> T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(into_fn<To>, as_bytes_type, T (&&values)[N]) noexcept -> meta::replace_first<byte, To> {
        return []<std::size_t... I>(T(&&values)[N], std::index_sequence<I...>) -> meta::replace_first<byte, To> {
            return { { as<byte>(std::move(values[I]))... } };
        }(std::move(values), std::make_index_sequence<N> {});
    }

    ////////////////////////////////////////////////////////////////////
    ///                           VIEWS                              ///
    ////////////////////////////////////////////////////////////////////
    /////////////////////////////////////
    /////////////////////////////////////
    template<stdr::contiguous_range From>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(meta::tag<mutable_view_of>, From& value) noexcept
      -> array_view<meta::range_value_type<From>, meta::details::get_constexpr_size<From>()>
        requires(not meta::constant_range<From>)
    {
        return array_view<meta::range_value_type<From>, meta::details::get_constexpr_size<From>()> { value };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<stdr::contiguous_range From>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(meta::tag<view_of>, const From& value) noexcept
      -> array_view<const meta::range_value_type<From>, meta::details::get_constexpr_size<From>()> {
        return array_view<const meta::range_value_type<From>, meta::details::get_constexpr_size<From>()> { value };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<stdr::contiguous_range From>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(meta::tag<mutable_view_of>, as_bytes_type, From& value) noexcept
      -> array_view<byte, meta::details::get_byte_extent_value_of<From, meta::details::get_constexpr_size<From>()>()>
        requires(/*not meta::std_span<From> and */ not meta::constant_range<From>)
    {
        return array_view<byte, meta::details::get_byte_extent_value_of<From, meta::details::get_constexpr_size<From>()>()> {
            static_cast<byte*>(static_cast<void*>(stdr::data(value))),
            sizeof(meta::range_value_type<From>) * stdr::size(value)
        };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<stdr::contiguous_range From>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(meta::tag<view_of>, as_bytes_type, const From& value) noexcept
      -> array_view<const byte, meta::details::get_byte_extent_value_of<From, meta::details::get_constexpr_size<From>()>()> {
        return array_view<const byte,
                          meta::details::get_byte_extent_value_of<From, meta::details::get_constexpr_size<From>()>()> {
            static_cast<const byte*>(static_cast<const void*>(stdr::data(value))),
            sizeof(meta::range_value_type<From>) * stdr::size(value)
        };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::standard_layout From>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(meta::tag<mutable_view_of>, as_bytes_type, From& value) noexcept
      -> array_view<byte, meta::details::get_byte_extent_value_of<From, 1>()>
        requires(not stdr::input_range<From>)
    {
        return array_view<byte, meta::details::get_byte_extent_value_of<From, 1>()> {
            static_cast<byte*>(static_cast<void*>(&value)),
            sizeof(From)
        };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::standard_layout From>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(meta::tag<view_of>, as_bytes_type, const From& value) noexcept
      -> array_view<const byte, meta::details::get_byte_extent_value_of<From, 1>()>
        requires(not stdr::input_range<From>)
    {
        return array_view<const byte, meta::details::get_byte_extent_value_of<From, 1>()> {
            static_cast<const byte*>(static_cast<const void*>(&value)),
            sizeof(From)
        };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename From>
    STORMKIT_FORCE_INLINE
    constexpr auto bytes_of(From& value) noexcept -> decltype(auto) {
        return view_of(as_bytes, value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename From>
    STORMKIT_FORCE_INLINE
    constexpr auto mutable_bytes_of(const From& value) noexcept -> decltype(auto) {
        return mutable_view_of(as_bytes, value);
    }

    ////////////////////////////////////////////////////////////////////
    ///                           STRING                             ///
    ////////////////////////////////////////////////////////////////////
    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(meta::tag<as<string>>, string_view value, source_location_arg) noexcept -> string {
        return string { value };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::has_as_string_view From>
        STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(meta::tag<as<string>>, From&& value, source_location_arg) noexcept -> string
        requires(not meta::plain::is<From, string>)
    {
        return string { as<string_view>(std::forward<From>(value)) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::plain::integral From>
    constexpr auto tag_invoke(meta::tag<as<string>>, From value, i32 base, source_location_arg) noexcept -> string {
        auto out = std::string {};
        out.resize(16);
        auto&& [ptr, errc] = std::to_chars(stdr::data(out), stdr::data(out) + stdr::size(out), value, base);
        if consteval {
            ensures(errc != std::errc {}, "Failed to convert to string");
        } else {
            ensures(errc != std::errc {},
                    std::format("Failed to convert {} {} to string, reason: {}!", value, base, error_code::from_stderrc(errc)));
        }

        const auto size = std::distance(stdr::data(out), ptr);
        out.resize(as<usize>(size));

        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::plain::floating_point From>
    constexpr auto tag_invoke(meta::tag<as<string>>, From value, std::chars_format fmt, source_location_arg) noexcept -> string {
        auto out = std::string {};
        out.resize(16, '\0');
        auto&& [ptr, errc] = std::to_chars(stdr::data(out), stdr::data(out) + stdr::size(out), value, fmt);
        if consteval {
            ensures(errc != std::errc {}, "Failed to convert to string");
        } else {
            ensures(errc != std::errc {},
                    std::format("Failed to convert {} (fmt: {}) to string, reason: {}!",
                                value,
                                fmt,
                                error_code::from_stderrc(errc)));
        }

        const auto size = std::distance(stdr::data(out), ptr);
        out.resize(as<usize>(size));

        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::plain::integral To>
    constexpr auto tag_invoke(as_fn<To>, string_view value, i32 base, source_location_arg) noexcept -> To {
        auto out         = To { 0 };
        auto&& [_, errc] = std::from_chars(stdr::data(value), stdr::data(value) + stdr::size(value), out, base);
        if consteval {
            ensures(errc != std::errc {}, "Failed to convert from string");
        } else {
            ensures(errc != std::errc {},
                    std::format("Failed to convert from string {} (base: {}), reason: {}!",
                                value,
                                base,
                                error_code::from_stderrc(errc)));
        }

        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::plain::floating_point To>
    constexpr auto tag_invoke(as_fn<To>, string_view value, std::chars_format fmt, source_location_arg) noexcept -> To {
        auto out         = To { 0. };
        auto&& [_, errc] = std::from_chars(stdr::data(value), stdr::data(value) + stdr::size(value), out, fmt);
        if consteval {
            ensures(errc != std::errc {}, "Failed to convert from string");
        } else {
            ensures(errc != std::errc {},
                    std::format("Failed to convert from string {} (fmt: {}), reason: {}!",
                                value,
                                fmt,
                                error_code::from_stderrc(errc)));
        }

        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::plain::integral From>
    constexpr auto tag_invoke(to_fn<string>, From&& value, i32 base, source_location_arg) noexcept -> system_result<string> {
        auto out = system_result<string> { std::in_place };
        out->resize(16);
        auto&& [ptr, errc] = std::to_chars(stdr::data(*out), stdr::data(*out) + stdr::size(*out), value, base);
        if (errc != std::errc {}) [[unlikely]]
            out = std::unexpected<system_code> { std::in_place, error_code::from_stderrc(std::move(errc)) };
        else {
            const auto size = std::distance(stdr::data(*out), ptr);
            out->resize(as<usize>(size));
        }

        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::plain::floating_point From>
    constexpr auto tag_invoke(to_fn<string>, From&& value, std::chars_format fmt, source_location_arg) noexcept
      -> system_result<string> {
        auto out = system_result<string> { std::in_place };
        out->resize(16, '\0');

        auto&& [ptr, errc] = std::to_chars(stdr::data(*out), stdr::data(*out) + stdr::size(*out), value, fmt);
        if (errc != std::errc {}) [[unlikely]]
            out = std::unexpected<system_code> { std::in_place, error_code::from_stderrc(std::move(errc)) };
        else {
            const auto size = std::distance(stdr::data(*out), ptr);
            out->resize(size);
        }

        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::plain::integral To>
    constexpr auto tag_invoke(to_fn<To>, string_view value, i32 base, source_location_arg) noexcept -> system_result<To> {
        auto out         = system_result<To> { std::in_place };
        auto&& [_, errc] = std::from_chars(stdr::data(value), stdr::data(value) + stdr::size(value), *out, base);
        if (errc != std::errc {}) [[unlikely]]
            out = std::unexpected<system_code> { std::in_place, error_code::from_stderrc(std::move(errc)) };

        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::plain::floating_point To>
    constexpr auto tag_invoke(to_fn<To>, string_view value, std::chars_format fmt, source_location_arg) noexcept
      -> system_result<To> {
        auto out         = system_result<To> { std::in_place };
        auto&& [_, errc] = std::from_chars(stdr::data(value), stdr::data(value) + stdr::size(value), *out, fmt);
        if (errc != std::errc {}) [[unlikely]]
            out = std::unexpected<system_code> { std::in_place, error_code::from_stderrc(std::move(errc)) };

        return out;
    }

    // static_assert(tag_invoke(std::declval<into_fn<array<int, 2>>>(), std::declval<int (&)[2]>());
    static_assert(meta::tag_invocable<into_fn<array<int, 2>>, int (&)[2]>);
    static_assert(stdr::size(into<array>({ 0, 1 })) == 2);
}} // namespace stormkit::core
