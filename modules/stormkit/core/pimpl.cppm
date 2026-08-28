// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#include <stormkit/core/contract_macro.hpp>

export module stormkit.core.pimpl;

import std;

import stormkit.core.heap;
import stormkit.core.meta.concepts;
import stormkit.core.meta.type_manipulation;
import stormkit.core.contract;
import stormkit.core.typesafe.ref_ptr;

export namespace stormkit { inline namespace core {
    template<typename T, bool DEFER = false>
    class STORMKIT_OWNER pimpl {
      public:
        using element_type = T;
        using pointer      = T*;

        constexpr pimpl() noexcept
            requires(DEFER);
        template<typename... Ts>
        constexpr pimpl(Ts&&... args) noexcept(meta::noexcept_constructible_from<element_type, Ts...>)
            requires(not DEFER);
        constexpr ~pimpl() noexcept(meta::noexcept_destructible<element_type>);

        pimpl(const pimpl&) noexcept                    = delete;
        auto operator=(const pimpl&) noexcept -> pimpl& = delete;

        constexpr pimpl(pimpl&&) noexcept;
        constexpr auto operator=(pimpl&&) noexcept -> pimpl&;

        template<typename... Ts>
        constexpr auto init(Ts&&... args) noexcept(meta::noexcept_constructible_from<element_type, Ts...>) -> void;

        template<typename Self>
        [[nodiscard]]
        constexpr auto operator->(STORMKIT_LIFETIMEBOUND this Self& self) noexcept -> meta::forward_const_to<Self, element_type>*;

        template<typename Self>
        [[nodiscard]]
        constexpr auto operator*(STORMKIT_LIFETIMEBOUND this Self& self) noexcept -> meta::forward_const_to<Self, element_type>&;

        template<typename Self>
        [[nodiscard]]
        constexpr auto get(STORMKIT_LIFETIMEBOUND this Self& self) noexcept
          -> ref_ptr<meta::forward_const_to<Self, element_type>>;

        [[nodiscard]]
        constexpr auto operator==(std::nullptr_t) const noexcept -> bool;
        [[nodiscard]]
        constexpr explicit operator bool() const noexcept;

      private:
        heap_ptr<element_type> m_implementation;
    };
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, bool DEFER>
    STORMKIT_FORCE_INLINE
    constexpr pimpl<T, DEFER>::pimpl() noexcept
        requires(DEFER)
    = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, bool DEFER>
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    constexpr pimpl<T, DEFER>::pimpl(Ts&&... args) noexcept(meta::noexcept_constructible_from<element_type, Ts...>)
        requires(not DEFER)
    {
        init(std::forward<Ts>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, bool DEFER>
    STORMKIT_FORCE_INLINE
    constexpr pimpl<T, DEFER>::~pimpl() noexcept(meta::noexcept_destructible<element_type>) = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, bool DEFER>
    STORMKIT_FORCE_INLINE
    constexpr pimpl<T, DEFER>::pimpl(pimpl&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, bool DEFER>
    STORMKIT_FORCE_INLINE
    constexpr auto pimpl<T, DEFER>::operator=(pimpl&&) noexcept -> pimpl& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, bool DEFER>
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto pimpl<T, DEFER>::init(Ts&&... args) noexcept(meta::noexcept_constructible_from<element_type, Ts...>)
      -> void {
        EXPECTS(m_implementation == nullptr);
        m_implementation = allocate_unsafe<T>(std::forward<Ts>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, bool DEFER>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto pimpl<T, DEFER>::operator->(this Self& self) noexcept -> meta::forward_const_to<Self, element_type>* {
        EXPECTS(self.m_implementation != nullptr);
        return self.m_implementation.get();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, bool DEFER>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto pimpl<T, DEFER>::operator*(this Self& self) noexcept -> meta::forward_const_to<Self, element_type>& {
        EXPECTS(self.m_implementation != nullptr);
        return *self.m_implementation;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, bool DEFER>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto pimpl<T, DEFER>::get(this Self& self) noexcept -> ref_ptr<meta::forward_const_to<Self, element_type>> {
        EXPECTS(self.m_implementation != nullptr);
        return *self.m_implementation;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, bool DEFER>
    STORMKIT_FORCE_INLINE
    constexpr auto pimpl<T, DEFER>::operator==(std::nullptr_t) const noexcept -> bool {
        return m_implementation != nullptr;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, bool DEFER>
    STORMKIT_FORCE_INLINE
    constexpr pimpl<T, DEFER>::operator bool() const noexcept {
        return m_implementation != nullptr;
    }
}} // namespace stormkit::core
