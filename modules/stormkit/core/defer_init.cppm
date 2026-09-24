// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.deferinit;

import std;

import stormkit.core.meta;
import stormkit.core.contract;
import stormkit.core.types;

export namespace stormkit { inline namespace core {
    template<typename T>
    using defer_init_default_storage = array<byte, sizeof(T)>;

    template<typename T, typename Storage = defer_init_default_storage<T>>
    class STORMKIT_OWNER defer_init {
      public:
        using value_type      = T;
        using reference       = value_type&;
        using const_reference = const value_type&;
        template<typename Self>
        using conditional_reference = meta::conditional<meta::const_type<Self>, const_reference, reference>;
        using pointer               = value_type*;
        using const_pointer         = const value_type*;
        template<typename Self>
        using conditional_pointer = meta::conditional<meta::const_type<Self>, const_pointer, pointer>;

        constexpr defer_init();
        constexpr ~defer_init();

        defer_init(const defer_init& other)                    = delete;
        auto operator=(const defer_init& other) -> defer_init& = delete;

        constexpr defer_init(defer_init&& other) noexcept(meta::noexcept_movable<value_type>);
        constexpr auto operator=(defer_init&& other) noexcept(meta::noexcept_move_assignable<value_type>) -> defer_init&;

        template<typename... Ts>
        constexpr auto construct(Ts&&... args) noexcept(meta::noexcept_constructible_from<value_type, Ts...>) -> void;

        template<typename... Ts>
        constexpr auto construct_with_narrowing(Ts&&... args) noexcept(meta::noexcept_constructible_from<value_type, Ts...>)
          -> void;

        constexpr auto operator=(value_type&& value) noexcept(meta::noexcept_movable<value_type>) -> void;

        template<typename Self>
        [[nodiscard]]
        constexpr auto value(STORMKIT_LIFETIMEBOUND this Self&& self) noexcept -> meta::forward_like<Self, value_type>;
        template<typename Self>
        [[nodiscard]]
        constexpr auto operator->(STORMKIT_LIFETIMEBOUND this Self& self) noexcept -> conditional_pointer<Self>;
        template<typename Self>
        [[nodiscard]]
        constexpr auto operator*(STORMKIT_LIFETIMEBOUND this Self&& self) noexcept -> meta::forward_like<Self, value_type>;

        [[nodiscard]]
        constexpr operator reference() noexcept STORMKIT_LIFETIMEBOUND;
        [[nodiscard]]
        constexpr operator const_reference() const noexcept STORMKIT_LIFETIMEBOUND;

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept;
        [[nodiscard]]
        constexpr auto has_value() const noexcept -> bool;

      private:
        constexpr auto reset() noexcept -> void;

        alignas(value_type) Storage m_data;
        pointer m_pointer = nullptr;
    };
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, typename Storage>
    STORMKIT_FORCE_INLINE
    constexpr defer_init<T, Storage>::defer_init() = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, typename Storage>
    STORMKIT_FORCE_INLINE
    constexpr defer_init<T, Storage>::~defer_init() {
        reset();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, typename Storage>
    STORMKIT_FORCE_INLINE
    constexpr defer_init<T, Storage>::defer_init(defer_init&& other) noexcept(meta::noexcept_movable<value_type>) {
        reset();

        if (other.has_value()) [[likely]] {
            m_pointer = new (std::data(m_data)) value_type { std::move(other.value()) };

            other.reset();
        }
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, typename Storage>
    STORMKIT_FORCE_INLINE
    constexpr auto defer_init<T, Storage>::operator=(defer_init&& other) noexcept(meta::noexcept_move_assignable<value_type>)
      -> defer_init& {
        if (&other == this) [[unlikely]]
            return *this;

        reset();

        if (other.has_value()) [[likely]] {
            m_pointer = new (std::data(m_data)) value_type { std::move(other.value()) };

            other.reset();
        }

        return *this;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, typename Storage>
    template<typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto defer_init<T, Storage>::construct(Ts&&... args) noexcept(meta::noexcept_constructible_from<value_type, Ts...>)
      -> void {
        reset();

        m_pointer = new (std::data(m_data)) value_type { std::forward<Ts>(args)... };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, typename Storage>
    template<typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto defer_init<T, Storage>::
      construct_with_narrowing(Ts&&... args) noexcept(meta::noexcept_constructible_from<value_type, Ts...>) -> void {
        reset();

        m_pointer = new (std::data(m_data)) value_type(std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, typename Storage>
    STORMKIT_FORCE_INLINE
    constexpr auto defer_init<T, Storage>::operator=(T&& value) noexcept(meta::noexcept_movable<value_type>) -> void {
        reset();

        m_pointer = new (std::data(m_data)) value_type { std::move(value) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, typename Storage>
    STORMKIT_FORCE_INLINE
    constexpr defer_init<T, Storage>::operator bool() const noexcept {
        return has_value();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, typename Storage>
    STORMKIT_FORCE_INLINE
    constexpr auto defer_init<T, Storage>::has_value() const noexcept -> bool {
        return m_pointer != nullptr;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, typename Storage>
    STORMKIT_FORCE_INLINE
    constexpr auto defer_init<T, Storage>::reset() noexcept -> void {
        if (m_pointer) [[likely]] {
            value().~value_type();
            m_pointer = nullptr;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, typename Storage>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto defer_init<T, Storage>::value(this Self&& self) noexcept -> meta::forward_like<Self, value_type> {
        expects(self.has_value(), "Underlying object is not has_value");

        return std::forward_like<Self>(*self.m_pointer);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, typename Storage>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto defer_init<T, Storage>::operator->(this Self& self) noexcept -> conditional_pointer<Self> {
        expects(self.has_value(), "Underlying object is not has_value");

        return self.m_pointer;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, typename Storage>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto defer_init<T, Storage>::operator*(this Self&& self) noexcept -> meta::forward_like<Self, value_type> {
        return std::forward_like<Self>(self.value());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, typename Storage>
    STORMKIT_FORCE_INLINE
    constexpr defer_init<T, Storage>::operator reference() noexcept {
        return value();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, typename Storage>
    STORMKIT_FORCE_INLINE
    constexpr defer_init<T, Storage>::operator const_reference() const noexcept {
        return value();
    }
}} // namespace stormkit::core
