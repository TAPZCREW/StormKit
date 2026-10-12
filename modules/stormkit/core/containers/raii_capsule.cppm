// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.containers.raii_capsule;

import std;

import stormkit.core.contract;
import stormkit.core.meta;
import stormkit.core.functional;

export namespace stormkit { inline namespace core {
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto Release_value = T {}>
    class raii_capsule {
      public:
        using value_type     = T;
        using reference_type = value_type&;

        template<typename... Ts>
        static constexpr auto create(Ts&&... args) noexcept -> raii_capsule<T, Constructor, Deleter, Tag, Release_value>
            requires meta::is<std::invoke_result_t<decltype(Constructor), Ts...>, value_type>;
        template<typename... Ts>
        static constexpr auto create(Ts&&... args) noexcept -> decltype(auto)
            requires meta::specialization_of<std::invoke_result_t<decltype(Constructor), Ts...>, std::expected>;

        static constexpr auto take(value_type&& value) noexcept -> raii_capsule<T, Constructor, Deleter, Tag, Release_value>;
        static constexpr auto empty() noexcept -> raii_capsule<T, Constructor, Deleter, Tag, Release_value>;

        constexpr ~raii_capsule() noexcept;

        constexpr raii_capsule(raii_capsule& other)                    = delete;
        constexpr auto operator=(raii_capsule& other) -> raii_capsule& = delete;

        constexpr raii_capsule(raii_capsule&& other) noexcept;
        constexpr auto operator=(raii_capsule&& other) noexcept -> raii_capsule&;

        constexpr      operator value_type() const noexcept;
        constexpr auto handle() noexcept -> reference_type;
        constexpr auto handle() const noexcept -> value_type;
        constexpr auto release() noexcept -> value_type;
        constexpr auto reset(value_type handle = Release_value) noexcept -> void;

      private:
        constexpr raii_capsule() noexcept;
        constexpr raii_capsule(value_type handle) noexcept;

        constexpr auto destroy() noexcept -> void;

        value_type m_handle = Release_value;
    };
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////
namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto Release_value>
    template<typename... Ts>
    constexpr auto raii_capsule<T, Constructor, Deleter, Tag, Release_value>::create(Ts&&... args) noexcept
      -> raii_capsule<T, Constructor, Deleter, Tag, Release_value>
        requires meta::is<std::invoke_result_t<decltype(Constructor), Ts...>, value_type>
    {
        return raii_capsule<T, Constructor, Deleter, Tag, Release_value> { Constructor(std::forward<Ts>(args)...) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto Release_value>
    template<typename... Ts>
    constexpr auto raii_capsule<T, Constructor, Deleter, Tag, Release_value>::create(Ts&&... args) noexcept -> decltype(auto)
        requires meta::specialization_of<std::invoke_result_t<decltype(Constructor), Ts...>, std::expected>
    {
        return Constructor(std::forward<Ts>(args)...)
          .transform(monadic::init<raii_capsule<T, Constructor, Deleter, Tag, Release_value>>());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto Release_value>
    STORMKIT_FORCE_INLINE
    constexpr auto raii_capsule<T, Constructor, Deleter, Tag, Release_value>::take(value_type&& value) noexcept
      -> raii_capsule<T, Constructor, Deleter, Tag, Release_value> {
        return raii_capsule<T, Constructor, Deleter, Tag, Release_value> { std::move(value) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto Release_value>
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto raii_capsule<T, Constructor, Deleter, Tag, Release_value>::empty() noexcept
      -> raii_capsule<T, Constructor, Deleter, Tag, Release_value> {
        return raii_capsule<T, Constructor, Deleter, Tag, Release_value> { Release_value };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto Release_value>
    STORMKIT_FORCE_INLINE
    constexpr raii_capsule<T, Constructor, Deleter, Tag, Release_value>::~raii_capsule() noexcept {
        destroy();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto Release_value>
    STORMKIT_FORCE_INLINE
    constexpr raii_capsule<T, Constructor, Deleter, Tag, Release_value>::raii_capsule(raii_capsule&& other) noexcept {
        m_handle = other.release();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto Release_value>
    STORMKIT_FORCE_INLINE
    constexpr auto raii_capsule<T, Constructor, Deleter, Tag, Release_value>::operator=(raii_capsule&& other) noexcept
      -> raii_capsule& {
        m_handle = other.release();
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto Release_value>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr raii_capsule<T, Constructor, Deleter, Tag, Release_value>::operator value_type() const noexcept {
        return m_handle;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto Release_value>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto raii_capsule<T, Constructor, Deleter, Tag, Release_value>::handle() noexcept -> reference_type {
        return m_handle;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto Release_value>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto raii_capsule<T, Constructor, Deleter, Tag, Release_value>::handle() const noexcept -> value_type {
        return m_handle;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto Release_value>
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    constexpr auto raii_capsule<T, Constructor, Deleter, Tag, Release_value>::release() noexcept -> value_type {
        auto tmp = std::exchange(m_handle, Release_value);
        return tmp;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto Release_value>
    STORMKIT_FORCE_INLINE
    constexpr auto raii_capsule<T, Constructor, Deleter, Tag, Release_value>::reset(value_type handle) noexcept -> void {
        destroy();
        m_handle = handle;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto Release_value>
    STORMKIT_FORCE_INLINE
    constexpr auto raii_capsule<T, Constructor, Deleter, Tag, Release_value>::destroy() noexcept -> void {
        if (m_handle != Release_value) { Deleter(release()); }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto Release_value>
    STORMKIT_FORCE_INLINE
    constexpr raii_capsule<T, Constructor, Deleter, Tag, Release_value>::raii_capsule(value_type handle) noexcept {
        m_handle = handle;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Constructor, auto Deleter, typename Tag, auto Release_value>
    STORMKIT_FORCE_INLINE
    constexpr raii_capsule<T, Constructor, Deleter, Tag, Release_value>::raii_capsule() noexcept = default;
}} // namespace stormkit::core
