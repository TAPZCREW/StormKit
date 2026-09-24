// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/api.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.math.linear.angle;

import std;

import stormkit.core.meta.concepts;
import stormkit.core.typesafe.strong_type;

namespace cmeta = stormkit::core::meta;

export namespace stormkit::math {
    template<cmeta::floating_point T>
    using euler = strong_type<T, struct euler_tag, "euler", capabilities::arithmetic>;

    template<cmeta::floating_point T>
    using radian = strong_type<T, struct radian_tag, "radian", capabilities::arithmetic>;

    template<cmeta::floating_point T>
    [[nodiscard]]
    constexpr auto radians(T degres) noexcept -> radian<T>;
} // namespace stormkit::math

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::math {
    ////////////////////////////////////////
    ////////////////////////////////////////
    template<cmeta::floating_point T>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto radians(T degres) noexcept -> radian<T> {
        static constexpr auto one_rad = std::numbers::pi_v<T> / T { 180 };
        return radian<T> { degres * one_rad };
    }
} // namespace stormkit::math
