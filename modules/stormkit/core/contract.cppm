// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/api.hpp>
#include <stormkit/core/platform_macro.hpp>

#ifndef STORMKIT_ASSERT
    #define STORMKIT_ASSERT 1
#endif

export module stormkit.core.contract;

import std;

import stormkit.core.stacktrace;
import stormkit.core.types;

namespace stdr = std::ranges;

export namespace stormkit { inline namespace core {
    enum class assert_type {
        ASSERTION,
        PRE_CONDITION,
        POST_CONDITION,
    };

    STORMKIT_CORE_API
    auto assert_base(bool                        cond,
                     assert_type                 type,
                     string_view                 message,
                     const std::source_location& location = std::source_location::current()) noexcept -> void;

    consteval auto consteval_assert_base(bool cond, assert_type type, string_view message) noexcept -> void;

    constexpr auto assert(bool                        cond,
                          string_view                 message,
                          const std::source_location& location = std::source_location::current()) noexcept -> void;

    constexpr auto assert(bool cond, const std::source_location& location = std::source_location::current()) noexcept -> void;

    constexpr auto expects(bool                        cond,
                           string_view                 message,
                           const std::source_location& location = std::source_location::current()) noexcept -> void;

    constexpr auto expects(bool cond, const std::source_location& location = std::source_location::current()) noexcept -> void;

    constexpr auto ensures(bool                        cond,
                           string_view                 message,
                           const std::source_location& location = std::source_location::current()) noexcept -> void;

    constexpr auto ensures(bool cond, const std::source_location& location = std::source_location::current()) noexcept -> void;
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

using namespace std::literals;

namespace stormkit { inline namespace core {
    struct static_string {
        array<char, 512> buff;
        usize            size;

        consteval auto view() noexcept -> string_view { return { std::data(buff), size }; }
    };

    auto consteval_failure(static_string) -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto as_string(assert_type value) noexcept -> string_view {
        using enum assert_type;
        switch (value) {
            case ASSERTION: return "Contract check";
            case PRE_CONDITION: return "Pre condition check";
            case POST_CONDITION: return "Post condition check";
            default: break;
        };

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    consteval auto generate_consteval_message(assert_type type, string_view message) noexcept -> static_string {
        auto       result = static_string {};
        const auto str    = "[ASSERTION]"s + as_string(type) + ": " + message;
        stdr::copy(str, stdr::begin(result.buff));
        result.size = stdr::size(str);
        return result;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    consteval auto consteval_assert_base(bool cond, assert_type type, string_view message) noexcept -> void {
        if (not cond) [[unlikely]] { consteval_failure(generate_consteval_message(type, message)); }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto assert(bool cond, string_view message, [[maybe_unused]] const std::source_location& location) noexcept
      -> void {
        if consteval {
            consteval_assert_base(cond, assert_type::ASSERTION, message);
        } else {
            assert_base(cond, assert_type::ASSERTION, message, location);
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto assert(bool cond, const std::source_location& location) noexcept -> void {
        assert(cond, "", location);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto expects(bool cond, string_view message, const std::source_location& location) noexcept -> void {
        if consteval {
            consteval_assert_base(cond, assert_type::PRE_CONDITION, message);
        } else {
            assert_base(cond, assert_type::PRE_CONDITION, message, location);
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto expects(bool cond, const std::source_location& location) noexcept -> void {
        expects(cond, "", location);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto ensures(bool cond, string_view message, const std::source_location& location) noexcept -> void {
        if consteval {
            consteval_assert_base(cond, assert_type::POST_CONDITION, message);
        } else {
            assert_base(cond, assert_type::POST_CONDITION, message, location);
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto ensures(bool cond, const std::source_location& location) noexcept -> void {
        ensures(cond, "", location);
    }
}} // namespace stormkit::core
