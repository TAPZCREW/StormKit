// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#define SUBSTITUTION 0

export module stormkit.core.typesafe.flags;

import std;

import stormkit.core.types;
import stormkit.core.meta.concepts;
import stormkit.core.meta.type_query;

export {
    namespace stormkit { inline namespace core {
        namespace meta {
            template<enumeration T>
            inline constexpr auto FLAG_TRAIT = false;

            template<typename T>
            concept is_flag = FLAG_TRAIT<T>;
        } // namespace meta

        template<meta::is_flag T>
        [[nodiscard]]
        constexpr auto has_flag_bit(T value, T flag) noexcept -> bool;
    }} // namespace stormkit::core

    template<stormkit::meta::is_flag T>
    [[nodiscard]]
    constexpr auto operator|(T lhs, T rhs) noexcept -> T;

    template<stormkit::meta::is_flag T>
    [[nodiscard]]
    constexpr auto operator&(T lhs, T rhs) noexcept -> T;

    template<stormkit::meta::is_flag T>
    [[nodiscard]]
    constexpr auto operator^(T lhs, T rhs) noexcept -> T;

    template<stormkit::meta::is_flag T>
    [[nodiscard]]
    constexpr auto operator~(T lhs) noexcept -> T;

    template<stormkit::meta::is_flag T>
    constexpr auto operator|=(T& lhs, T rhs) noexcept -> T&;

    template<stormkit::meta::is_flag T>
    constexpr auto operator&=(T& lhs, T rhs) noexcept -> T&;

    template<stormkit::meta::is_flag T>
    constexpr auto operator^=(T& lhs, T rhs) noexcept -> T&;
}

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_flag T>
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto has_flag_bit(T value, T flag) noexcept -> bool {
        return (value & flag) == flag;
    }
}} // namespace stormkit::core

/////////////////////////////////////
/////////////////////////////////////
template<stormkit::meta::is_flag T>
STORMKIT_FORCE_INLINE STORMKIT_CONST
constexpr auto operator|(T lhs, T rhs) noexcept -> T {
    using type = stormkit::meta::underlying_type<T>;
    return static_cast<T>(static_cast<type>(lhs) | static_cast<type>(rhs));
}

/////////////////////////////////////
/////////////////////////////////////
template<stormkit::meta::is_flag T>
STORMKIT_FORCE_INLINE STORMKIT_CONST
constexpr auto operator&(T lhs, T rhs) noexcept -> T {
    using type = stormkit::meta::underlying_type<T>;
    return static_cast<T>(static_cast<type>(lhs) & static_cast<type>(rhs));
}

/////////////////////////////////////
/////////////////////////////////////
template<stormkit::meta::is_flag T>
STORMKIT_FORCE_INLINE STORMKIT_CONST
constexpr auto operator^(T lhs, T rhs) noexcept -> T {
    using type = stormkit::meta::underlying_type<T>;
    return static_cast<T>(static_cast<type>(lhs) ^ static_cast<type>(rhs));
}

/////////////////////////////////////
/////////////////////////////////////
template<stormkit::meta::is_flag T>
STORMKIT_FORCE_INLINE STORMKIT_CONST
constexpr auto operator~(T lhs) noexcept -> T {
    using type = stormkit::meta::underlying_type<T>;
    return static_cast<T>(~static_cast<type>(lhs));
}

/////////////////////////////////////
/////////////////////////////////////
template<stormkit::meta::is_flag T>
STORMKIT_FORCE_INLINE
constexpr auto operator|=(T& lhs, T rhs) noexcept -> T& {
    lhs = lhs | rhs;
    return lhs;
}

/////////////////////////////////////
/////////////////////////////////////
template<stormkit::meta::is_flag T>
STORMKIT_FORCE_INLINE
constexpr auto operator&=(T& lhs, T rhs) noexcept -> T& {
    lhs = lhs & rhs;
    return lhs;
}

/////////////////////////////////////
/////////////////////////////////////
template<stormkit::meta::is_flag T>
STORMKIT_FORCE_INLINE
constexpr auto operator^=(T& lhs, T rhs) noexcept -> T& {
    lhs = lhs ^ rhs;
    return lhs;
}
