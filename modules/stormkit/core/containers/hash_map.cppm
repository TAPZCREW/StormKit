// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.containers.hash_map;

import std;

import ankerl.unordered_dense;
import frozen;

import stormkit.core.types;
import stormkit.core.hash;

export namespace stormkit { inline namespace core {
    template<class Key,
             typename T,
             class Hash                 = ankerl::unordered_dense::hash<Key>,
             class KeyEqual             = std::equal_to<Key>,
             class AllocatorOrContainer = std::allocator<std::pair<Key, T>>>
    using hash_map = ankerl::unordered_dense::map<Key, T, Hash, KeyEqual, AllocatorOrContainer>;

    template<class Key,
             class Hash                 = ankerl::unordered_dense::hash<Key>,
             class KeyEqual             = std::equal_to<Key>,
             class AllocatorOrContainer = std::allocator<Key>>
    using hash_set = ankerl::unordered_dense::set<Key, Hash, KeyEqual, AllocatorOrContainer>;

    template<class Key, class Value, usize N, typename Hash = frozen::anna<Key>, class KeyEqual = std::equal_to<Key>>
    using static_hash_map = frozen::unordered_map<Key, Value, N, Hash, KeyEqual>;

    template<class Key, usize N, typename Hash = frozen::elsa<Key>, class KeyEqual = std::equal_to<Key>>
    using static_hash_set = frozen::unordered_set<Key, N, Hash, KeyEqual>;

    template<typename Key, typename Value, usize N>
    constexpr auto make_static_hash_map(std::pair<Key, Value> (&pairs)[N]) noexcept -> static_hash_map<Key, Value, N>;

    template<typename Key, typename Value, usize N>
    constexpr auto make_static_hash_map(std::pair<Key, Value> (&&pairs)[N]) noexcept -> static_hash_map<Key, Value, N>;
}} // namespace stormkit::core

export template<stormkit::core::meta::has_hasher T>
struct ankerl::unordered_dense::hash<T> {
    using is_avalanching = void;

    [[nodiscard]]
    constexpr auto operator()(const T& value) const noexcept -> stormkit::u64 {
        return std::bit_cast<stormkit::u64>(stormkit::hash_of(value));
    }
};

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Key, typename Value, usize N>
    constexpr auto make_static_hash_map(std::pair<Key, Value> (&pairs)[N]) noexcept -> static_hash_map<Key, Value, N> {
        return frozen::make_unordered_map<Key, Value, N>(pairs);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Key, typename Value, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto make_static_hash_map(std::pair<Key, Value> (&&pairs)[N]) noexcept -> static_hash_map<Key, Value, N> {
        return frozen::make_unordered_map<Key, Value, N>(std::move(pairs));
    }
}} // namespace stormkit::core
