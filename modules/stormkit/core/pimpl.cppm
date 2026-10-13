// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
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
    template<typename T, bool Defer = false>
    class STORMKIT_OWNER pimpl {
      public:
        using element_type = T;
        using pointer      = T*;

        constexpr pimpl() noexcept
            requires(Defer);
        template<typename... Ts>
        constexpr pimpl(Ts&&... args) noexcept(meta::noexcept_constructible_from<element_type, Ts...>)
            requires(not Defer);
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
        heap_ptr<element_type> impl_;
    };
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, bool Defer>
    STORMKIT_FORCE_INLINE
    constexpr pimpl<T, Defer>::pimpl() noexcept
        requires(Defer)
    = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, bool Defer>
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    constexpr pimpl<T, Defer>::pimpl(Ts&&... args) noexcept(meta::noexcept_constructible_from<element_type, Ts...>)
        requires(not Defer)
    {
        init(std::forward<Ts>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, bool Defer>
    STORMKIT_FORCE_INLINE
    constexpr pimpl<T, Defer>::~pimpl() noexcept(meta::noexcept_destructible<element_type>) = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, bool Defer>
    STORMKIT_FORCE_INLINE
    constexpr pimpl<T, Defer>::pimpl(pimpl&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, bool Defer>
    STORMKIT_FORCE_INLINE
    constexpr auto pimpl<T, Defer>::operator=(pimpl&&) noexcept -> pimpl& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, bool Defer>
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto pimpl<T, Defer>::init(Ts&&... args) noexcept(meta::noexcept_constructible_from<element_type, Ts...>) -> void {
        EXPECTS(impl_ == nullptr);
        impl_ = allocate_unsafe<T>(std::forward<Ts>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, bool Defer>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto pimpl<T, Defer>::operator->(this Self& self) noexcept -> meta::forward_const_to<Self, element_type>* {
        EXPECTS(self.impl_ != nullptr);
        return self.impl_.get();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, bool Defer>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto pimpl<T, Defer>::operator*(this Self& self) noexcept -> meta::forward_const_to<Self, element_type>& {
        EXPECTS(self.impl_ != nullptr);
        return *self.impl_;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, bool Defer>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto pimpl<T, Defer>::get(this Self& self) noexcept -> ref_ptr<meta::forward_const_to<Self, element_type>> {
        EXPECTS(self.impl_ != nullptr);
        return *self.impl_;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, bool Defer>
    STORMKIT_FORCE_INLINE
    constexpr auto pimpl<T, Defer>::operator==(std::nullptr_t) const noexcept -> bool {
        return impl_ != nullptr;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, bool Defer>
    STORMKIT_FORCE_INLINE
    constexpr pimpl<T, Defer>::operator bool() const noexcept {
        return impl_ != nullptr;
    }
}} // namespace stormkit::core
