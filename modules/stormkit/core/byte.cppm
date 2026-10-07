// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
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
import stormkit.core.typesafe.safecasts;

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

    template<meta::integral T>
    [[nodiscard]]
    constexpr auto low_byte(T value, std::endian endianess) noexcept -> T;

    template<std::endian Endianess = std::endian::native, meta::integral T>
    [[nodiscard]]
    constexpr auto low_byte(T value) noexcept -> T;

    template<meta::integral T>
    [[nodiscard]]
    constexpr auto high_byte(T value, std::endian endianess) noexcept -> T;

    template<std::endian Endianess = std::endian::native, meta::integral T>
    [[nodiscard]]
    constexpr auto high_byte(T value) noexcept -> T;
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
        auto bytes = mutable_bytes_of(value);
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

            return std::bit_cast<T>(repr);
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::integral T>
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto low_byte(T value, std::endian endianess) noexcept -> T {
        if constexpr (sizeof(value) == 1) return value;
        else {
            if (endianess == std::endian::little) return value & 0xFF;
            else if (endianess == std::endian::big)
                return value & (0xFF << (sizeof(T) / 2));
            else {
                ensures(false);
                std::unreachable();
            }
        }
    }

    template<std::endian Endianess = std::endian::native, meta::integral T>
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto low_byte(T value) noexcept -> T {
        if constexpr (sizeof(value) == 1) return value;
        else {
            if constexpr (Endianess == std::endian::little) return value & 0xFF;
            else if constexpr (Endianess == std::endian::big)
                return value & (0xFF << (sizeof(T) / 2));
            else {
                static_assert(false);
                std::unreachable();
            }
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::integral T>
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto high_byte(T value, std::endian endianess) noexcept -> T {
        if constexpr (sizeof(value) == 1) return value;
        else {
            if (endianess == std::endian::little) return (value >> (sizeof(T) / 2)) & 0xFF;
            else if (endianess == std::endian::big)
                return value & 0xFF;
            else {
                ensures(false);
                std::unreachable();
            }
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<std::endian Endianess = std::endian::native, meta::integral T>
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto high_byte(T value) noexcept -> T {
        if constexpr (sizeof(value) == 1) return value;
        else {
            if constexpr (Endianess == std::endian::little) return (value >> (sizeof(T) / 2)) & 0xFF;
            else if constexpr (Endianess == std::endian::big)
                return value & 0xFF;
            else {
                static_assert(false);
                std::unreachable();
            }
        }
    }
}} // namespace stormkit::core
