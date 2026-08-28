// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.string.safecasts;

import std;

import stormkit.core.types;
import stormkit.core.errors;
import stormkit.core.typesafe.safecasts;
import stormkit.core.contract;
import stormkit.core.meta.concepts;
import stormkit.core.meta.type_manipulation;
import stormkit.core.meta.tag_invoke;

namespace stdr = std::ranges;

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

    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<string>, string_view value, source_location_arg = std::source_location::current()) noexcept
      -> string;

    template<meta::has_as_string_view From>
    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<string>, From&& value, source_location_arg = std::source_location::current()) noexcept
      -> string
        requires(not meta::plain::is<From, string>);

    template<meta::plain::integral From>
    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<string>,
                              From value,
                              i32  base           = 10,
                              source_location_arg = std::source_location::current()) noexcept -> string;

    template<meta::plain::floating_point From>
    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<string>,
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
    constexpr auto tag_invoke(try_as_fn<string>,
                              From value,
                              i32  base           = 10,
                              source_location_arg = std::source_location::current()) noexcept -> system_result<string>;

    template<meta::plain::floating_point From>
    [[nodiscard]]
    constexpr auto tag_invoke(try_as_fn<string>,
                              From              value,
                              std::chars_format fmt = std::chars_format::general,
                              source_location_arg   = std::source_location::current()) noexcept -> system_result<string>;

    template<meta::plain::integral To>
    [[nodiscard]]
    constexpr auto tag_invoke(try_as_fn<To>,
                              string_view value,
                              i32         base    = 10,
                              source_location_arg = std::source_location::current()) noexcept -> system_result<To>;

    template<meta::plain::floating_point To>
    [[nodiscard]]
    constexpr auto tag_invoke(try_as_fn<To>,
                              string_view       value,
                              std::chars_format fmt = std::chars_format::general,
                              source_location_arg   = std::source_location::current()) noexcept -> system_result<To>;
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(as_fn<string>, string_view value, source_location_arg) noexcept -> string {
        return string { value };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::has_as_string_view From>
        STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(as_fn<string>, From&& value, source_location_arg) noexcept -> string
        requires(not meta::plain::is<From, string>)
    {
        return string { as<string_view>(std::forward<From>(value)) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::plain::integral From>
    constexpr auto tag_invoke(as_fn<string>, From value, i32 base, source_location_arg) noexcept -> string {
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
    constexpr auto tag_invoke(as_fn<string>, From value, std::chars_format fmt, source_location_arg) noexcept -> string {
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
    constexpr auto tag_invoke(try_as_fn<string>, From&& value, i32 base, source_location_arg) noexcept -> system_result<string> {
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
    constexpr auto tag_invoke(try_as_fn<string>, From&& value, std::chars_format fmt, source_location_arg) noexcept
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
    constexpr auto tag_invoke(try_as_fn<To>, string_view value, i32 base, source_location_arg) noexcept -> system_result<To> {
        auto out         = system_result<To> { std::in_place };
        auto&& [_, errc] = std::from_chars(stdr::data(value), stdr::data(value) + stdr::size(value), *out, base);
        if (errc != std::errc {}) [[unlikely]]
            out = std::unexpected<system_code> { std::in_place, error_code::from_stderrc(std::move(errc)) };

        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::plain::floating_point To>
    constexpr auto tag_invoke(try_as_fn<To>, string_view value, std::chars_format fmt, source_location_arg) noexcept
      -> system_result<To> {
        auto out         = system_result<To> { std::in_place };
        auto&& [_, errc] = std::from_chars(stdr::data(value), stdr::data(value) + stdr::size(value), *out, fmt);
        if (errc != std::errc {}) [[unlikely]]
            out = std::unexpected<system_code> { std::in_place, error_code::from_stderrc(std::move(errc)) };

        return out;
    }
}} // namespace stormkit::core
