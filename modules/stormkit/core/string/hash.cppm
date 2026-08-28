// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.string.hash;

import std;

import stormkit.core.types;
import stormkit.core.hash;
import stormkit.core.meta.concepts;
import stormkit.core.meta.type_query;
import stormkit.core.containers.hash_map;
import stormkit.core.typesafe.safecasts;

namespace stdr = std::ranges;

export namespace stormkit { inline namespace core {
    struct string_hash_fn {
        using is_transparent = void;
        using is_avalanching = void;

        template<typename T = u64>
        [[nodiscard]]
        static constexpr auto operator()(string_view value, T seed = 0) noexcept -> T;
    };

    template<class Value, usize SIZE, class Key = string>
    using static_string_hash_map = static_hash_map<Key, Value, SIZE, string_hash_fn, std::equal_to<>>;

    template<usize SIZE, class Value = string>
    using static_string_hash_set = static_hash_set<Value, SIZE, string_hash_fn, std::equal_to<>>;

    template<class Value, class Key = string>
    using string_hash_map = hash_map<Key, Value, string_hash_fn, std::equal_to<>>;

    template<class Value = string>
    using string_hash_set = hash_set<Value, string_hash_fn, std::equal_to<>>;

    template<meta::hash_type Ret>
    constexpr auto tag_invoke(hash_fn<Ret>, string_view value) noexcept -> Ret;

    namespace literals {
        constexpr auto operator""_hash32(czstring str, usize size) -> hash32;
        constexpr auto operator""_hash64(czstring str, usize size) -> hash64;
    } // namespace literals
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    template<typename T>
    class lehmer_128_hasher {
      public:
        STORMKIT_FORCE_INLINE
        explicit constexpr lehmer_128_hasher(T basis) noexcept
            : m_hash { basis } {}

        STORMKIT_FORCE_INLINE
        constexpr auto bytes(array_view<const byte, 4> bytes) noexcept -> void {
            bytes_4(bytes);
        }

        STORMKIT_FORCE_INLINE
        constexpr auto bytes_4(array_view<const byte, 4> bytes) noexcept -> void {
            const auto val = T(bytes[0]) << 0 | T(bytes[1]) << 8 | T(bytes[2]) << 16 | T(bytes[3]) << 24;
            dword(val);
        }

        STORMKIT_FORCE_INLINE
        constexpr auto dword(T val) noexcept -> void {
            m_hash = mix(val ^ m_hash);
        }

        STORMKIT_FORCE_INLINE
        constexpr auto hash() const noexcept -> T {
            return m_hash;
        }

      private:
        static constexpr T mix(T seed) noexcept {
            auto seed128 = static_cast<u128>(seed);
            // Pierre L’Ecuyer. 1999. Tables of linear congruential generators of different sizes
            // and good lattice structure. Mathematics of Computation of the American Mathematical
            // Society 68, 225 (1999), 249–260.
            // https://www.ams.org/journals/mcom/1999-68-225/S0025-5718-99-00996-5/S0025-5718-99-00996-5.pdf
            seed128 *= 4292484099903637661;
            if constexpr (meta::is<T, u64>) return static_cast<T>(seed128 >> 64);
            else if constexpr (meta::is<T, u32>)
                return static_cast<T>(seed128 >> 96);
            else
                std::unreachable();
        }

        T m_hash;
    };

    template<class Hasher>
    STORMKIT_FORCE_INLINE
    constexpr auto hash_selected_characters(u8 mask, Hasher& hasher, czstring s, usize size) noexcept -> void {
        if (std::popcount(mask) == 4) {
            auto dword = u64 { 0 };
            auto i     = i32 { 0 };

            if ((mask & (3 << 0)) == (3 << 0)) {
                auto w = u16 { 0 };
                if consteval {
                    w = as<u16>(s[0] | (s[1] << 8));
                } else {
                    // FIXME(strager): This is endian-dependent, but it makes GCC
                    // generate the code we want.
                    std::memcpy(&w, s, 2);
                }
                dword |= w;
                i += 2;
            } else {
                if (mask & (1 << 0)) {
                    dword = static_cast<u8>(s[0]);
                    i += 1;
                }
                if (mask & (1 << 1)) {
                    dword |= static_cast<u64>(static_cast<u8>(s[1])) << (i * 8);
                    i += 1;
                }
            }

            if ((mask & (3 << 2)) == (3 << 2)) {
                auto w = u16 { 0 };
                if consteval {
                    w = as<u16>(s[size - 2] | (s[size - 1] << 8));
                } else {
                    // FIXME(strager): This is endian-dependent, but it makes GCC
                    // generate the code we want.
                    std::memcpy(&w, &s[size - 2], 2);
                }
                dword |= as<u64>(w << (i * 8));
                i += 2;
            } else {
                if (mask & (1 << 2)) {
                    dword |= static_cast<u64>(static_cast<u8>(s[size - 2])) << (i * 8);
                    i += 1;
                }
                if (mask & (1 << 3)) {
                    dword |= static_cast<u64>(static_cast<u8>(s[size - 1])) << (i * 8);
                    i += 1;
                }
            }

            if (mask & (1 << 4)) {
                dword |= static_cast<u64>(static_cast<u8>(size)) << (i * 8);
                i += 1;
            }

            // assert(i == 4);

            hasher.dword(dword);
        } else {
            array<byte, 5> bytes;
            auto           i = usize { 0 };

            if (mask & (1 << 0)) bytes[i++] = static_cast<byte>(s[0]);
            if (mask & (1 << 1)) bytes[i++] = static_cast<byte>(s[1]);

            if (mask & (1 << 2)) bytes[i++] = static_cast<byte>(s[size - 2]);
            if (mask & (1 << 3)) bytes[i++] = static_cast<byte>(s[size - 1]);

            if (mask & (1 << 4)) bytes[i++] = static_cast<byte>(size);

            hasher.bytes(array_view<const byte, 4> { stdr::data(bytes), i });
        }
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    constexpr auto string_hash_fn::operator()(string_view value, T seed) noexcept -> T {
        auto mask   = u8 { 0b01111 };
        auto hasher = lehmer_128_hasher<T> { seed };
        hash_selected_characters(mask, hasher, stdr::data(value), stdr::size(value));
        return hasher.hash();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::hash_type Ret>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(hash_fn<Ret>, string_view value) noexcept -> Ret {
        return as<Ret>(string_hash_fn::operator()<meta::underlying_type<Ret>>(value));
    }

    namespace literals {
        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        constexpr auto operator""_hash32(czstring str, usize size) -> hash32 {
            return hash_of<hash32>(string_view { str, size });
        }

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        constexpr auto operator""_hash64(czstring str, usize size) -> hash64 {
            return hash_of(string_view { str, size });
        }
    } // namespace literals
}} // namespace stormkit::core
