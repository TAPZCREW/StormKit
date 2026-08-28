// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.random;

import std;

import stormkit.core.typesafe;
import stormkit.core.meta;
import stormkit.core.types;

export namespace stormkit { inline namespace core {
    auto seed(u32 seed) noexcept -> void;

    template<meta::floating_point T>
    [[nodiscard]]
    auto rand(T min, T max) noexcept -> T;

    template<meta::integral T>
    [[nodiscard]]
    auto rand(T min, T max) noexcept -> T;
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    inline auto random_generator() noexcept -> std::default_random_engine& {
        thread_local auto generator = std::default_random_engine {};
        return generator;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto seed(u32 seed) noexcept -> void {
        random_generator().seed(seed);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::floating_point T>
    STORMKIT_FORCE_INLINE
    inline auto rand(T min, T max) noexcept -> T {
        std::uniform_real_distribution<T> dis(min, max);
        return dis(random_generator());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::integral T>
    STORMKIT_FORCE_INLINE
    inline auto rand(T min, T max) noexcept -> T {
        std::uniform_int_distribution<T> dis(min, max);
        return dis(random_generator());
    }
}} // namespace stormkit::core
