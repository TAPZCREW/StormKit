// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#include <stormkit/core/contract_macro.hpp>

export module stormkit.core.byte;

import std;

import stormkit.core.meta;
import stormkit.core.contract;
import stormkit.core.types;
import stormkit.core.containers.safecasts;

namespace stdr = std::ranges;
namespace stdp = std::pmr;

using namespace stormkit::core::literals;

export namespace stormkit { inline namespace core {
    template<typename T>
    constexpr auto zero_bytes(T& value) noexcept -> void;

    template<typename T>
    constexpr auto zeroed() noexcept -> T;

    template<typename T>
    [[nodiscard]]
    constexpr auto byte_swap(const T& value) noexcept -> T;
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    constexpr auto zero_bytes(T& value) noexcept -> void {
        auto bytes = as<array_view>(as_bytes, value);
        stdr::fill(bytes, byte { 0 });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    constexpr auto zeroed() noexcept -> T {
        auto data = T {};
        zero_bytes(data);
        return data;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    constexpr auto byte_swap(const T& value) noexcept -> T {
        if constexpr (meta::integral<T>) return std::byteswap(value);
        else {
            auto repr = std::bit_cast<array<byte, sizeof(value)>>(value);

            stdr::reverse(repr);

            return std::launder(std::bit_cast<T>(repr));
        }
    }
}} // namespace stormkit::core
