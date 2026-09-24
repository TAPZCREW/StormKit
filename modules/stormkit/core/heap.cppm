// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.heap;

import std;

import stormkit.core.meta;
import stormkit.core.types;

export namespace stormkit { inline namespace core {
    struct allocation_error {
        string_view type;
        usize       size;
    };

    template<typename T>
    using heap_ptr = std::unique_ptr<T>;
    template<typename T>
    using shared_heap_ptr = std::shared_ptr<T>;
    using std::weak_ptr;

    template<typename T, class... Ts>
    auto allocate(Ts&&... args) noexcept(meta::noexcept_constructible_from<T, Ts...>)
      -> std::expected<heap_ptr<T>, allocation_error>;

    template<typename T, class... Ts>
    auto allocate_unsafe(Ts&&... args) noexcept(meta::noexcept_constructible_from<T, Ts...>) -> heap_ptr<T>;

    template<typename T, class... Ts>
    auto allocate_shared(Ts&&... args) noexcept(meta::noexcept_constructible_from<T, Ts...>)
      -> std::expected<shared_heap_ptr<T>, allocation_error>;

    template<typename T, class... Ts>
    auto allocate_shared_unsafe(Ts&&... args) noexcept(meta::noexcept_constructible_from<T, Ts...>) -> shared_heap_ptr<T>;
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, class... Ts>
    STORMKIT_FORCE_INLINE
    auto allocate(Ts&&... args) noexcept(meta::noexcept_constructible_from<T, Ts...>)
      -> std::expected<heap_ptr<T>, allocation_error> {
        auto value = heap_ptr<T> { new (std::nothrow) T(std::forward<Ts>(args)...) };
        if (not value) [[unlikely]]
            return std::unexpected(allocation_error { .type = typeid(T).name(), .size = sizeof(T) });
        return std::expected<heap_ptr<T>, allocation_error> { std::in_place, std::move(value) };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, class... Ts>
    STORMKIT_FORCE_INLINE
    auto allocate_unsafe(Ts&&... args) noexcept(meta::noexcept_constructible_from<T, Ts...>) -> heap_ptr<T> {
        return heap_ptr<T> { new (std::nothrow) T(std::forward<Ts>(args)...) };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, class... Ts>
    STORMKIT_FORCE_INLINE
    auto allocate_shared(Ts&&... args) noexcept(meta::noexcept_constructible_from<T, Ts...>)
      -> std::expected<shared_heap_ptr<T>, allocation_error> {
        auto value = shared_heap_ptr<T> { new (std::nothrow) T(std::forward<Ts>(args)...) };
        if (not value) [[unlikely]]
            return std::unexpected(allocation_error { .type = typeid(T).name(), .size = sizeof(T) });
        return std::expected<shared_heap_ptr<T>, allocation_error> { std::in_place, std::move(value) };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, class... Ts>
    STORMKIT_FORCE_INLINE
    auto allocate_shared_unsafe(Ts&&... args) noexcept(meta::noexcept_constructible_from<T, Ts...>) -> shared_heap_ptr<T> {
        return shared_heap_ptr<T> { new (std::nothrow) T(std::forward<Ts>(args)...) };
    }
}} // namespace stormkit::core
