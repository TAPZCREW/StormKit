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
    template<typename T, auto CONSTRUCTOR, auto DELETER, typename Tag, auto RELEASE_VALUE = T {}>
    class raii_capsule {
      public:
        using value_type     = value_type;
        using reference_type = value_type&;

        template<typename... Ts>
        static constexpr auto create(Ts&&... args) noexcept -> raii_capsule
            requires meta::is<std::invoke_result_t<decltype(CONSTRUCTOR), Ts...>, value_type>;
        template<typename... Ts>
        static constexpr auto create(Ts&&... args) noexcept -> decltype(auto)
            requires meta::specialization_of<std::invoke_result_t<decltype(CONSTRUCTOR), Ts...>, std::expected>;

        static constexpr auto take(value_type&& value) noexcept -> raii_capsule;
        static constexpr auto empty() noexcept -> raii_capsule;

        constexpr ~raii_capsule() noexcept;

        constexpr raii_capsule(raii_capsule& other)                    = delete;
        constexpr auto operator=(raii_capsule& other) -> raii_capsule& = delete;

        constexpr raii_capsule(raii_capsule&& other) noexcept;
        constexpr auto operator=(raii_capsule&& other) noexcept -> raii_capsule&;

        constexpr      operator value_type() const noexcept;
        constexpr auto handle() noexcept -> reference_type;
        constexpr auto handle() const noexcept -> value_type;
        constexpr auto release() noexcept -> value_type;
        constexpr auto reset(T handle = RELEASE_VALUE) noexcept -> void;

      private:
        constexpr raii_capsule() noexcept;
        constexpr raii_capsule(value_type handle) noexcept;

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
    template<typename T, auto CONSTRUCTOR, auto DELETER, typename Tag, auto RELEASE_VALUE>
    template<typename... Ts>
    constexpr auto raii_capsule<T, CONSTRUCTOR, DELETER, Tag, RELEASE_VALUE>::create(Ts&&... args) noexcept -> raii_capsule
        requires meta::is<std::invoke_result_t<decltype(CONSTRUCTOR), Ts...>, value_type>
    {
        return raii_capsule { CONSTRUCTOR(std::forward<Ts>(args)...) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto CONSTRUCTOR, auto DELETER, typename Tag, auto RELEASE_VALUE>
    template<typename... Ts>
    constexpr auto raii_capsule<T, CONSTRUCTOR, DELETER, Tag, RELEASE_VALUE>::create(Ts&&... args) noexcept -> decltype(auto)
        requires meta::specialization_of<std::invoke_result_t<decltype(CONSTRUCTOR), Ts...>, std::expected>
    {
        return CONSTRUCTOR(std::forward<Ts>(args)...).transform(monadic::init<raii_capsule>());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto CONSTRUCTOR, auto DELETER, typename Tag, auto RELEASE_VALUE>
    STORMKIT_FORCE_INLINE
    constexpr auto raii_capsule<T, CONSTRUCTOR, DELETER, Tag, RELEASE_VALUE>::take(value_type&& value) noexcept -> raii_capsule {
        return raii_capsule { std::move(value) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto CONSTRUCTOR, auto DELETER, typename Tag, auto RELEASE_VALUE>
    STORMKIT_FORCE_INLINE
    constexpr auto raii_capsule<T, CONSTRUCTOR, DELETER, Tag, RELEASE_VALUE>::empty() noexcept -> raii_capsule {
        return raii_capsule { RELEASE_VALUE };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto CONSTRUCTOR, auto DELETER, typename Tag, auto RELEASE_VALUE>
    STORMKIT_FORCE_INLINE
    constexpr raii_capsule<T, CONSTRUCTOR, DELETER, Tag, RELEASE_VALUE>::~raii_capsule() noexcept {
        destroy();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto CONSTRUCTOR, auto DELETER, typename Tag, auto RELEASE_VALUE>
    STORMKIT_FORCE_INLINE
    constexpr raii_capsule<T, CONSTRUCTOR, DELETER, Tag, RELEASE_VALUE>::raii_capsule(raii_capsule&& other) noexcept {
        m_handle = other.release();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto CONSTRUCTOR, auto DELETER, typename Tag, auto RELEASE_VALUE>
    STORMKIT_FORCE_INLINE
    constexpr auto raii_capsule<T, CONSTRUCTOR, DELETER, Tag, RELEASE_VALUE>::operator=(raii_capsule&& other) noexcept
      -> raii_capsule& {
        m_handle = other.release();
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto CONSTRUCTOR, auto DELETER, typename Tag, auto RELEASE_VALUE>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr raii_capsule<T, CONSTRUCTOR, DELETER, Tag, RELEASE_VALUE>::operator value_type() const noexcept {
        return m_handle;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto CONSTRUCTOR, auto DELETER, typename Tag, auto RELEASE_VALUE>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto raii_capsule<T, CONSTRUCTOR, DELETER, Tag, RELEASE_VALUE>::handle() noexcept -> reference_type {
        return m_handle;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto CONSTRUCTOR, auto DELETER, typename Tag, auto RELEASE_VALUE>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto raii_capsule<T, CONSTRUCTOR, DELETER, Tag, RELEASE_VALUE>::handle() const noexcept -> value_type {
        return m_handle;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto CONSTRUCTOR, auto DELETER, typename Tag, auto RELEASE_VALUE>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto raii_capsule<T, CONSTRUCTOR, DELETER, Tag, RELEASE_VALUE>::release() noexcept -> value_type {
        auto tmp = std::exchange(m_handle, RELEASE_VALUE);
        return tmp;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto CONSTRUCTOR, auto DELETER, typename Tag, auto RELEASE_VALUE>
    STORMKIT_FORCE_INLINE
    constexpr auto raii_capsule<T, CONSTRUCTOR, DELETER, Tag, RELEASE_VALUE>::reset(value_type handle) noexcept -> void {
        destroy();
        m_handle = handle;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto CONSTRUCTOR, auto DELETER, typename Tag, auto RELEASE_VALUE>
    STORMKIT_FORCE_INLINE
    constexpr auto raii_capsule<T, CONSTRUCTOR, DELETER, Tag, RELEASE_VALUE>::destroy() noexcept -> void {
        if (m_handle != RELEASE_VALUE) { DELETER(release()); }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto CONSTRUCTOR, auto DELETER, typename Tag, auto RELEASE_VALUE>
    STORMKIT_FORCE_INLINE
    constexpr raii_capsule<T, CONSTRUCTOR, DELETER, Tag, RELEASE_VALUE>::raii_capsule(value_type handle) noexcept {
        m_handle = handle;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto CONSTRUCTOR, auto DELETER, typename Tag, auto RELEASE_VALUE>
    STORMKIT_FORCE_INLINE
    constexpr raii_capsule<T, CONSTRUCTOR, DELETER, Tag, RELEASE_VALUE>::raii_capsule() noexcept = default;
}} // namespace stormkit::core
