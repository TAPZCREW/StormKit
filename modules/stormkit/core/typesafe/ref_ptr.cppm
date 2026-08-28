// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#include <stormkit/core/contract_macro.hpp>

export module stormkit.core.typesafe.ref_ptr;

import std;

import stormkit.core.types;

import stormkit.core.meta.type_manipulation;
import stormkit.core.meta.concepts;
import stormkit.core.meta.algorithms;
import stormkit.core.meta.type_query;
import stormkit.core.contract;

namespace stdr = std::ranges;
namespace stdv = std::views;

export {
    namespace stormkit { inline namespace core {
        template<typename T>
        using owned_raw_ptr = T*;

        template<meta::negate<meta::raw_indirection> T>
        class STORMKIT_VIEW STORMKIT_TRIVIALLY_RELOCATABLE ref_ptr {
          public:
            using element_type  = T;
            using pointer       = T*;
            using const_pointer = const T*;

            constexpr ref_ptr(T& pointed STORMKIT_LIFETIMEBOUND) noexcept;

            template<meta::plain::pointer_to<T> U>
            constexpr ref_ptr(const U& pointed STORMKIT_LIFETIMEBOUND) noexcept;
            // constexpr ~ref_ptr() noexcept;

            template<meta::is<element_type> U>
            constexpr ref_ptr(const ref_ptr<U>&) noexcept;
            template<meta::is<element_type> U>
            constexpr ref_ptr(ref_ptr<U>&&) noexcept;

            template<meta::is<element_type> U>
            constexpr auto operator=(ref_ptr<U> other) noexcept -> ref_ptr&;

            template<typename Self>
            [[nodiscard]]
            constexpr auto operator->(this Self& self) noexcept -> meta::forward_const_to<Self, element_type>*;

            template<typename Self>
            [[nodiscard]]
            constexpr auto operator*(this Self& self) noexcept -> meta::forward_const_to<Self, element_type>&;

            template<typename Self>
            [[nodiscard]]
            constexpr auto get(this Self& self) noexcept -> meta::forward_const_to<Self, element_type>*;

            [[nodiscard]]
            constexpr operator pointer() noexcept;
            [[nodiscard]]
            constexpr operator const_pointer() const noexcept;
            [[nodiscard]]
            constexpr operator std::reference_wrapper<element_type>() const noexcept;
            [[nodiscard]]
            constexpr explicit operator bool() const noexcept;

            [[nodiscard]]
            constexpr auto operator==(std::nullptr_t) const noexcept -> bool;
            [[nodiscard]]
            constexpr auto operator<(std::nullptr_t) const noexcept -> bool;
            [[nodiscard]]
            constexpr auto operator<=(std::nullptr_t) const noexcept -> bool;
            [[nodiscard]]
            constexpr auto operator>(std::nullptr_t) const noexcept -> bool;
            [[nodiscard]]
            constexpr auto operator>=(std::nullptr_t) const noexcept -> bool;
            [[nodiscard]]
            constexpr auto operator<=>(std::nullptr_t) const noexcept -> std::compare_three_way_result_t<pointer, pointer>
                requires std::three_way_comparable<pointer, pointer>;

            template<meta::negate<meta::raw_indirection> U>
            [[nodiscard]]
            constexpr auto operator==(const ref_ptr<U>&) const noexcept -> bool;
            template<meta::negate<meta::raw_indirection> U>
            [[nodiscard]]
            constexpr auto operator<(const ref_ptr<U>&) const noexcept -> bool;
            template<meta::negate<meta::raw_indirection> U>
            [[nodiscard]]
            constexpr auto operator<=(const ref_ptr<U>&) const noexcept -> bool;
            template<meta::negate<meta::raw_indirection> U>
            [[nodiscard]]
            constexpr auto operator>(const ref_ptr<U>&) const noexcept -> bool;
            template<meta::negate<meta::raw_indirection> U>
            [[nodiscard]]
            constexpr auto operator>=(const ref_ptr<U>&) const noexcept -> bool;
            template<meta::negate<meta::raw_indirection> U>
            [[nodiscard]]
            constexpr auto operator<=>(const ref_ptr<U>& other) const noexcept
              -> std::compare_three_way_result_t<pointer, meta::pointer_type<ref_ptr<U>>>
                requires std::three_way_comparable<pointer, meta::pointer_type<ref_ptr<U>>>;

          private:
            pointer m_pointed;
        };
    }} // namespace stormkit::core

    template<typename T>
    struct std::pointer_traits<stormkit::core::ref_ptr<T>> {
        using pointer         = typename stormkit::core::ref_ptr<T>::pointer;
        using element_type    = typename stormkit::core::ref_ptr<T>::element_type;
        using difference_type = std::ptrdiff_t;
    };
}

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::negate<meta::raw_indirection> T>
    STORMKIT_FORCE_INLINE
    constexpr ref_ptr<T>::ref_ptr(T& pointed STORMKIT_LIFETIMEBOUND) noexcept
        : m_pointed { std::addressof(pointed) } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::negate<meta::raw_indirection> T>
    template<meta::plain::pointer_to<T> U>
    STORMKIT_FORCE_INLINE
    constexpr ref_ptr<T>::ref_ptr(const U& pointed STORMKIT_LIFETIMEBOUND) noexcept
        : m_pointed { std::addressof(*pointed) } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    // template<meta::negate<meta::raw_indirection> T>
    // STORMKIT_FORCE_INLINE
    // constexpr ref_ptr<T>::~ref_ptr() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::negate<meta::raw_indirection> T>
    template<meta::is<meta::element_type<ref_ptr<T>>> U>
    STORMKIT_FORCE_INLINE
    constexpr ref_ptr<T>::ref_ptr(const ref_ptr<U>& other) noexcept
        : m_pointed { static_cast<pointer>(other.m_pointed) } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::negate<meta::raw_indirection> T>
    template<meta::is<meta::element_type<ref_ptr<T>>> U>
    STORMKIT_FORCE_INLINE
    constexpr ref_ptr<T>::ref_ptr(ref_ptr<U>&& other) noexcept
        : m_pointed { static_cast<pointer>(other.m_pointed) } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::negate<meta::raw_indirection> T>
    template<meta::is<meta::element_type<ref_ptr<T>>> U>
    STORMKIT_FORCE_INLINE
    constexpr auto ref_ptr<T>::operator=(ref_ptr<U> other) noexcept -> ref_ptr& {
        if (&other == this) [[unlikely]]
            return *this;

        m_pointed = as<pointer>(other.pointed);

        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::negate<meta::raw_indirection> T>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto ref_ptr<T>::operator->(this Self& self) noexcept -> meta::forward_const_to<Self, element_type>* {
        return as<meta::forward_const_to<Self, element_type>*>(self.m_pointed);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::negate<meta::raw_indirection> T>
    STORMKIT_FORCE_INLINE
    template<typename Self>
    constexpr auto ref_ptr<T>::operator*(this Self& self) noexcept -> meta::forward_const_to<Self, element_type>& {
        return *self.m_pointed;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::negate<meta::raw_indirection> T>
    STORMKIT_FORCE_INLINE
    template<typename Self>
    constexpr auto ref_ptr<T>::get(this Self& self) noexcept -> meta::forward_const_to<Self, element_type>* {
        return static_cast<meta::forward_const_to<Self, element_type>*>(self.m_pointed);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::negate<meta::raw_indirection> T>
    STORMKIT_FORCE_INLINE
    constexpr ref_ptr<T>::operator pointer() noexcept {
        return m_pointed;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::negate<meta::raw_indirection> T>
    STORMKIT_FORCE_INLINE
    constexpr ref_ptr<T>::operator const_pointer() const noexcept {
        return static_cast<const_pointer>(m_pointed);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::negate<meta::raw_indirection> T>
    STORMKIT_FORCE_INLINE
    constexpr ref_ptr<T>::operator std::reference_wrapper<element_type>() const noexcept {
        if constexpr (meta::const_type<T>) return std::cref(*m_pointed);
        else
            return std::ref(*m_pointed);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::negate<meta::raw_indirection> T>
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr ref_ptr<T>::operator bool() const noexcept {
        return true;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::negate<meta::raw_indirection> T>
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto ref_ptr<T>::operator==(std::nullptr_t) const noexcept -> bool {
        return false;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::negate<meta::raw_indirection> T>
    STORMKIT_FORCE_INLINE
    constexpr auto ref_ptr<T>::operator<(std::nullptr_t) const noexcept -> bool {
        return std::less<pointer> {}(m_pointed, nullptr);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::negate<meta::raw_indirection> T>
    STORMKIT_FORCE_INLINE
    constexpr auto ref_ptr<T>::operator<=(std::nullptr_t) const noexcept -> bool {
        return !(nullptr < *this);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::negate<meta::raw_indirection> T>
    STORMKIT_FORCE_INLINE
    constexpr auto ref_ptr<T>::operator>(std::nullptr_t) const noexcept -> bool {
        return nullptr < *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::negate<meta::raw_indirection> T>
    STORMKIT_FORCE_INLINE
    constexpr auto ref_ptr<T>::operator>=(std::nullptr_t) const noexcept -> bool {
        return !(*this < nullptr);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::negate<meta::raw_indirection> T>
    STORMKIT_FORCE_INLINE
    constexpr auto ref_ptr<T>::operator<=>(std::nullptr_t) const noexcept -> std::compare_three_way_result_t<pointer, pointer>
        requires std::three_way_comparable<pointer, pointer>
    {
        return std::compare_three_way {}(m_pointed, static_cast<pointer>(nullptr));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::negate<meta::raw_indirection> T>
    template<meta::negate<meta::raw_indirection> U>
    STORMKIT_FORCE_INLINE
    constexpr auto ref_ptr<T>::operator==(const ref_ptr<U>& other) const noexcept -> bool {
        return m_pointed == other.m_pointed;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::negate<meta::raw_indirection> T>
    template<meta::negate<meta::raw_indirection> U>
    STORMKIT_FORCE_INLINE
    constexpr auto ref_ptr<T>::operator<(const ref_ptr<U>& other) const noexcept -> bool {
        return std::less<std::common_type_t<pointer, meta::pointer_type<ref_ptr<U>>>> {}(m_pointed, other.m_pointed);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::negate<meta::raw_indirection> T>
    template<meta::negate<meta::raw_indirection> U>
    STORMKIT_FORCE_INLINE
    constexpr auto ref_ptr<T>::operator<=(const ref_ptr<U>& other) const noexcept -> bool {
        return !(other < *this);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::negate<meta::raw_indirection> T>
    template<meta::negate<meta::raw_indirection> U>
    STORMKIT_FORCE_INLINE
    constexpr auto ref_ptr<T>::operator>(const ref_ptr<U>& other) const noexcept -> bool {
        return other < *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::negate<meta::raw_indirection> T>
    template<meta::negate<meta::raw_indirection> U>
    STORMKIT_FORCE_INLINE
    constexpr auto ref_ptr<T>::operator>=(const ref_ptr<U>& other) const noexcept -> bool {
        return !(*this < other);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::negate<meta::raw_indirection> T>
    template<meta::negate<meta::raw_indirection> U>
    STORMKIT_FORCE_INLINE
    constexpr auto ref_ptr<T>::operator<=>(const ref_ptr<U>& other) const noexcept
      -> std::compare_three_way_result_t<pointer, meta::pointer_type<ref_ptr<U>>>
        requires std::three_way_comparable<pointer, meta::pointer_type<ref_ptr<U>>>
    {
        return m_pointed <=> other.m_pointed;
    }
}} // namespace stormkit::core
