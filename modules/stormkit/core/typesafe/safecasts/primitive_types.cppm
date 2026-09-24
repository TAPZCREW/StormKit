// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/api.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.typesafe.safecasts:primitive_types;

import :as_cpo;
import :to_cpo;
import :is_cpo;

import stormkit.core.contract;
import stormkit.core.meta.type_query;

export namespace stormkit { inline namespace core {
    template<meta::floating_point First, meta::floating_point Second>
    [[nodiscard]]
    constexpr auto tag_invoke(is_fn<equal>, First first, Second second) noexcept -> bool;

    template<meta::integral First, meta::floating_point Second>
    [[nodiscard]]
    constexpr auto tag_invoke(is_fn<equal>, First first, Second second) noexcept -> bool;

    template<meta::floating_point First, meta::integral Second>
    [[nodiscard]]
    constexpr auto tag_invoke(is_fn<equal>, First first, Second second) noexcept -> bool;

    template<meta::integral First, meta::integral Second>
    [[nodiscard]]
    constexpr auto tag_invoke(is_fn<equal>, First first, Second second) noexcept -> bool;

    template<meta::arithmetic To, meta::arithmetic From>
    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<To>, From value, source_location_arg = std::source_location::current()) noexcept -> To
        requires(not meta::same_as<To, From>);

    template<meta::arithmetic From>
    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<byte>, From value, source_location_arg = std::source_location::current()) noexcept -> byte;

    template<meta::arithmetic To>
    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<To>, byte value, source_location_arg = std::source_location::current()) noexcept -> To;

    template<meta::enumeration From>
    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<byte>, From value, source_location_arg = std::source_location::current()) noexcept -> byte;

    template<meta::enumeration To>
    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<To>, byte value, source_location_arg = std::source_location::current()) noexcept -> To;

    template<meta::arithmetic To, meta::enumeration From>
    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<To>, From value, source_location_arg = std::source_location::current()) noexcept -> To;

    template<meta::enumeration From>
    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<underlying>, From value, source_location_arg = std::source_location::current()) noexcept
      -> std::underlying_type_t<From>;

    template<meta::enumeration To, meta::arithmetic From>
    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<To>, From value, source_location_arg = std::source_location::current()) noexcept -> To;
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    namespace details {
        /////////////////////////////////////
        /////////////////////////////////////
        template<typename To, typename From>
        STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
        constexpr auto is_safe_narrowing(const From& from) noexcept -> bool
            requires((meta::arithmetic<To> or meta::is<To, byte> or meta::enumeration<To>)
                     and (meta::arithmetic<From> or meta::is<From, byte> or meta::enumeration<From>))
        {
            if constexpr (meta::arithmetic<From> and meta::arithmetic<To>)
                return (static_cast<From>(static_cast<To>(from)) == from)
                       or (meta::signed_type<To> != meta::unsigned_type<From>
                           and ((static_cast<To>(from) < To {}) == (from < From {})));
            else
                return (static_cast<From>(static_cast<To>(from)) == from);
        }
    } // namespace details

    static constexpr auto
      NARROWING_ERROR_MSG = "Lossy narrowing detected, use unchecked_narrow() to explicitly force the conversion and possibly "
                            "lose information!";

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::floating_point First, meta::floating_point Second>
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto tag_invoke(is_fn<equal>, First first, Second second) noexcept -> bool {
        const auto second_ = as<First>(second);

        const auto diff = std::abs(first - second_);
        const auto a    = std::abs(first);
        const auto b    = std::abs(second_);
        const auto ab   = (a > b) ? b : a;
        return diff <= (ab * std::numeric_limits<First>::epsilon());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::integral First, meta::floating_point Second>
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto tag_invoke(is_fn<equal> tag, First first, Second second) noexcept -> bool {
        return tag_invoke(tag, second, first);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::floating_point First, meta::integral Second>
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto tag_invoke(is_fn<equal> tag, First first, Second second) noexcept -> bool {
        return tag_invoke(tag, first, as<First>(second));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::integral First, meta::integral Second>
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto tag_invoke(is_fn<equal>, First first, Second second) noexcept -> bool {
        if constexpr (meta::is_narrowing<First, Second>) {
            using SafeT        = meta::safe_narrow_type<First, Second>;
            const auto _first  = as<SafeT>(first);
            const auto _second = as<SafeT>(second);
            return _first == _second;
        } else
            return first == second;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::arithmetic To, meta::arithmetic From>
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto tag_invoke(as_fn<To>, From value, [[maybe_unused]] source_location_arg location) noexcept -> To
        requires(not meta::same_as<To, From>)
    {
        if constexpr (meta::is_narrowing<To, From>) expects(details::is_safe_narrowing<To>(value), NARROWING_ERROR_MSG, location);

        return static_cast<To>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::arithmetic From>
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto tag_invoke(as_fn<byte>, From value, [[maybe_unused]] source_location_arg location) noexcept -> byte {
        if constexpr (meta::is_narrowing<byte, From>)
            expects(details::is_safe_narrowing<byte>(value), NARROWING_ERROR_MSG, location);

        return static_cast<byte>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::arithmetic To>
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto tag_invoke(as_fn<To>, byte value, source_location_arg) noexcept -> To {
        return static_cast<To>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::enumeration From>
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto tag_invoke(as_fn<byte>, From value, [[maybe_unused]] source_location_arg location) noexcept -> byte {
        if constexpr (meta::is_narrowing<byte, From>)
            expects(details::is_safe_narrowing<byte>(value), NARROWING_ERROR_MSG, location);

        return static_cast<byte>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::enumeration To>
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto tag_invoke(as_fn<To>, byte value, source_location_arg) noexcept -> To {
        return static_cast<To>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::arithmetic To, meta::enumeration From>
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto tag_invoke(as_fn<To>, From value, [[maybe_unused]] source_location_arg location) noexcept -> To {
        using From_ = meta::to_plain_type<From>;

        if constexpr (meta::is_narrowing<To, From_>)
            expects(details::is_safe_narrowing<To>(value), NARROWING_ERROR_MSG, location);

        return static_cast<To>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::enumeration From>
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto tag_invoke(as_fn<underlying>, From value, source_location_arg) noexcept -> std::underlying_type_t<From> {
        return std::to_underlying(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::enumeration To, meta::arithmetic From>
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto tag_invoke(as_fn<To>, From value, [[maybe_unused]] source_location_arg location) noexcept -> To {
        if constexpr (meta::is_narrowing<std::underlying_type_t<To>, From>)
            expects(details::is_safe_narrowing<std::underlying_type_t<To>>(value), NARROWING_ERROR_MSG, location);

        return static_cast<To>(value);
    }

}} // namespace stormkit::core
