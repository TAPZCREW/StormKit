// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.string.algorithms;

import std;

import stormkit.core.contract;
import stormkit.core.meta;
import stormkit.core.types;
import stormkit.core.typesafe;
import stormkit.core.string.safecasts;

namespace stdr = std::ranges;
namespace stdv = std::views;

export namespace stormkit { inline namespace core {
    // namespace meta {
    //     template<typename T>
    //     concept has_as_string = requires(const T& value) {
    //         { as_string(value) } -> is<string_view>;
    //     };
    // } // namespace meta

    [[nodiscard]]
    constexpr auto split(string_view str, string_view delim) noexcept -> dynarray<string_view>;
    [[nodiscard]]
    constexpr auto to_lower(string_view str) noexcept -> string;
    [[nodiscard]]
    constexpr auto to_upper(string_view str) noexcept -> string;
    [[nodiscard]]
    auto to_lower(string_view str, const std::locale& locale) noexcept -> string;
    [[nodiscard]]
    auto to_upper(string_view str, const std::locale& locale) noexcept -> string;

    [[nodiscard]]
    constexpr auto replace(string_view in, string_view pattern, string_view replacement) noexcept -> string;

    template<meta::char_type T>
    constexpr auto is_text(T c) noexcept -> bool;
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto to_upper(char c) -> char {
        if (c >= 'a' && c <= 'z') return c - 32;
        return c;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto to_lower(char c) -> char {
        if (c >= 'A' && c <= 'Z') return c + 32;
        return c;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    constexpr auto split(string_view str, string_view delim) noexcept -> dynarray<string_view> {
        return str
               | stdv::split(delim)
               | stdv::transform([](auto&& subrange) { return string_view { stdr::cbegin(subrange), stdr::cend(subrange) }; })
               | stdr::to<dynarray<string_view>>();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto to_lower(string_view str) noexcept -> string {
        auto result = string { str };
        for (auto& c : result) c = to_lower(c);
        return result;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    inline auto to_lower(string_view str, const std::locale& locale) noexcept -> string {
        auto  result = string { str };
        auto& facet  = std::use_facet<std::ctype<typename string_view::value_type>>(locale);
        facet.tolower(&result[0], &result[0] + stdr::size(result));

        return result;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto to_upper(string_view str) noexcept -> string {
        auto result = string { str };
        for (auto& c : result) c = to_upper(c);
        return result;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    inline auto to_upper(string_view str, const std::locale& locale) noexcept -> string {
        auto  result = string { str };
        auto& facet  = std::use_facet<std::ctype<typename string_view::value_type>>(locale);
        facet.toupper(&result[0], &result[0] + stdr::size(result));

        return result;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    inline constexpr auto replace(string_view in, string_view pattern, string_view replacement) noexcept -> string {
        return in
               | stdv::split(pattern)
               | stdv::transform([replacement](auto&& substr) noexcept {
                     auto out = string {};
                     out.reserve(stdr::size(replacement) + stdr::size(substr));
                     out += replacement;
                     out += string_view { stdr::cbegin(substr), stdr::cend(substr) };
                     return out;
                 })
               | stdv::join
               | stdv::drop(stdr::size(replacement))
               | stdr::to<string>();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::char_type T>
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto is_text(T c) noexcept -> bool {
        return (c >= 32 and c <= 126) or (c >= 128);
    }
}} // namespace stormkit::core
