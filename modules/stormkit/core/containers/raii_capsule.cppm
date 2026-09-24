// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.containers.raii_capsule;

import std;

import stormkit.core.meta;
import stormkit.core.functional;

export namespace stormkit { inline namespace core {
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto RELEASE_VALUE = T {}>
    class RAIICapsule {
      public:
        using value_type     = T;
        using ReferenceType = value_type&;

        using value_type = value_type;
        using reference  = ReferenceType;

        template<typename... Ts>
        static constexpr auto create(Ts&&... args) noexcept -> RAIICapsule
            requires meta::is<std::invoke_result_t<decltype(Constructor), Ts...>, value_type>;
        template<typename... Ts>
        static constexpr auto create(Ts&&... args) noexcept -> decltype(auto)
            requires meta::specialization_of<std::invoke_result_t<decltype(Constructor), Ts...>, std::expected>;

        static constexpr auto take(value_type&& value) noexcept -> RAIICapsule;
        static constexpr auto empty() noexcept -> RAIICapsule;

        constexpr ~RAIICapsule() noexcept;

        constexpr RAIICapsule(RAIICapsule& other)                    = delete;
        constexpr auto operator=(RAIICapsule& other) -> RAIICapsule& = delete;

        constexpr RAIICapsule(RAIICapsule&& other) noexcept;
        constexpr auto operator=(RAIICapsule&& other) noexcept -> RAIICapsule&;

        constexpr      operator value_type() const noexcept;
        constexpr auto handle() noexcept -> ReferenceType;
        constexpr auto handle() const noexcept -> value_type;
        constexpr auto release() noexcept -> value_type;
        constexpr auto reset(T handle = RELEASE_VALUE) noexcept -> void;

      private:
        constexpr RAIICapsule() noexcept;
        constexpr RAIICapsule(value_type handle) noexcept;

        constexpr auto destroy() noexcept -> void;

        value_type m_handle = RELEASE_VALUE;
    };
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////
namespace stormkit { inline namespace core {

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto RELEASE_VALUE>
    template<typename... Ts>
    constexpr auto RAIICapsule<T, Constructor, Deleter, Tag, RELEASE_VALUE>::create(Ts&&... args) noexcept -> RAIICapsule
        requires meta::is<std::invoke_result_t<decltype(Constructor), Ts...>, value_type>
    {
        return RAIICapsule { Constructor(std::forward<Ts>(args)...) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto RELEASE_VALUE>
    template<typename... Ts>
    constexpr auto RAIICapsule<T, Constructor, Deleter, Tag, RELEASE_VALUE>::create(Ts&&... args) noexcept -> decltype(auto)
        requires meta::specialization_of<std::invoke_result_t<decltype(Constructor), Ts...>, std::expected>
    {
        return Constructor(std::forward<Ts>(args)...).transform(monadic::init<RAIICapsule>());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto RELEASE_VALUE>
    STORMKIT_FORCE_INLINE
    constexpr auto RAIICapsule<T, Constructor, Deleter, Tag, RELEASE_VALUE>::take(value_type&& value) noexcept -> RAIICapsule {
        return RAIICapsule { std::move(value) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto RELEASE_VALUE>
    STORMKIT_FORCE_INLINE
    constexpr auto RAIICapsule<T, Constructor, Deleter, Tag, RELEASE_VALUE>::empty() noexcept -> RAIICapsule {
        return RAIICapsule { RELEASE_VALUE };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto RELEASE_VALUE>
    STORMKIT_FORCE_INLINE
    constexpr RAIICapsule<T, Constructor, Deleter, Tag, RELEASE_VALUE>::~RAIICapsule() noexcept {
        destroy();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto RELEASE_VALUE>
    STORMKIT_FORCE_INLINE
    constexpr RAIICapsule<T, Constructor, Deleter, Tag, RELEASE_VALUE>::RAIICapsule(RAIICapsule&& other) noexcept {
        m_handle = other.release();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto RELEASE_VALUE>
    STORMKIT_FORCE_INLINE
    constexpr auto RAIICapsule<T, Constructor, Deleter, Tag, RELEASE_VALUE>::operator=(RAIICapsule&& other) noexcept
      -> RAIICapsule& {
        m_handle = other.release();
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto RELEASE_VALUE>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr RAIICapsule<T, Constructor, Deleter, Tag, RELEASE_VALUE>::operator value_type() const noexcept {
        return m_handle;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto RELEASE_VALUE>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto RAIICapsule<T, Constructor, Deleter, Tag, RELEASE_VALUE>::handle() noexcept -> ReferenceType {
        return m_handle;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto RELEASE_VALUE>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto RAIICapsule<T, Constructor, Deleter, Tag, RELEASE_VALUE>::handle() const noexcept -> value_type {
        return m_handle;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto RELEASE_VALUE>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto RAIICapsule<T, Constructor, Deleter, Tag, RELEASE_VALUE>::release() noexcept -> value_type {
        auto tmp = std::exchange(m_handle, RELEASE_VALUE);
        return tmp;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto RELEASE_VALUE>
    STORMKIT_FORCE_INLINE
    constexpr auto RAIICapsule<T, Constructor, Deleter, Tag, RELEASE_VALUE>::reset(value_type handle) noexcept -> void {
        destroy();
        m_handle = handle;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto RELEASE_VALUE>
    STORMKIT_FORCE_INLINE
    constexpr auto RAIICapsule<T, Constructor, Deleter, Tag, RELEASE_VALUE>::destroy() noexcept -> void {
        if (m_handle != RELEASE_VALUE) { Deleter(release()); }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto RELEASE_VALUE>
    STORMKIT_FORCE_INLINE
    constexpr RAIICapsule<T, Constructor, Deleter, Tag, RELEASE_VALUE>::RAIICapsule(value_type handle) noexcept {
        m_handle = handle;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto RELEASE_VALUE>
    STORMKIT_FORCE_INLINE
    constexpr RAIICapsule<T, Constructor, Deleter, Tag, RELEASE_VALUE>::RAIICapsule() noexcept = default;
}} // namespace stormkit::core
