// Copyright (C) 2023 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.string.static_string;

import std;

import stormkit.core.types;
import stormkit.core.typesafe.safecasts;
import stormkit.core.typesafe.ref_ptr;
import stormkit.core.meta.algorithms;
import stormkit.core.meta.concepts;
import stormkit.core.meta.type_manipulation;

namespace stdr = std::ranges;

export namespace stormkit { inline namespace core {
    template<usize N, typename CharT = char>
    struct STORMKIT_OWNER static_string {
        using value_type       = CharT;
        using storage_type     = array<CharT, N>;
        using string_view_type = basic_string_view<value_type>;
        using size_type        = typename storage_type::size_type;
        using difference_type  = typename storage_type::difference_type;
        using reference        = value_type&;
        using const_reference  = const value_type&;
        template<typename Self>
        using conditional_reference = meta::conditional<meta::const_type<Self>, const_reference, reference>;
        using pointer               = value_type*;
        using const_pointer         = const value_type*;
        template<typename Self>
        using conditional_pointer = meta::conditional<meta::const_type<Self>, const_pointer, pointer>;
        using iterator            = typename storage_type::iterator;
        using const_iterator      = typename storage_type::const_iterator;
        template<typename Self>
        using conditional_iterator   = meta::conditional<meta::const_type<Self>, const_iterator, iterator>;
        using reverse_iterator       = std::reverse_iterator<typename storage_type::iterator>;
        using const_reverse_iterator = std::reverse_iterator<typename storage_type::const_iterator>;
        template<typename Self>
        using conditional_reverse_iterator = meta::conditional<meta::const_type<Self>, const_reverse_iterator, reverse_iterator>;

        static constexpr auto SIZE = as<size_type>(N - 1u);
        static constexpr auto npos = SIZE + 1;

        consteval static_string() noexcept;
        consteval static_string(const value_type (&str)[N]) noexcept;
        constexpr ~static_string();

        constexpr static_string(const static_string&) noexcept;
        constexpr static_string(static_string&&) noexcept;
        constexpr auto operator=(const static_string&) noexcept -> static_string&;
        constexpr auto operator=(static_string&&) noexcept -> static_string&;

        template<typename Self>
        [[nodiscard]]
        constexpr auto begin(STORMKIT_LIFETIMEBOUND this Self& self) noexcept -> conditional_iterator<Self>;
        [[nodiscard]]
        constexpr auto cbegin() const noexcept STORMKIT_LIFETIMEBOUND -> const_iterator;
        template<typename Self>
        [[nodiscard]]
        constexpr auto end(STORMKIT_LIFETIMEBOUND this Self& self) noexcept -> conditional_iterator<Self>;
        [[nodiscard]]
        constexpr auto cend() const noexcept STORMKIT_LIFETIMEBOUND -> const_iterator;
        template<typename Self>
        [[nodiscard]]
        constexpr auto rbegin(STORMKIT_LIFETIMEBOUND this Self& self) noexcept -> conditional_reverse_iterator<Self>;
        [[nodiscard]]
        constexpr auto crbegin() const noexcept STORMKIT_LIFETIMEBOUND -> const_iterator;
        template<typename Self>
        [[nodiscard]]
        constexpr auto rend(STORMKIT_LIFETIMEBOUND this Self& self) noexcept -> conditional_reverse_iterator<Self>;
        [[nodiscard]]
        constexpr auto crend() const noexcept STORMKIT_LIFETIMEBOUND -> const_iterator;

        template<typename Self>
        [[nodiscard]]
        constexpr auto operator[](STORMKIT_LIFETIMEBOUND this Self& self, size_type index) noexcept
          -> meta::forward_const_to<Self, reference>;
        template<typename Self>
        [[nodiscard]]
        constexpr auto at(STORMKIT_LIFETIMEBOUND this Self& self, size_type index) noexcept
          -> meta::forward_const_to<Self, reference>;
        template<typename Self>
        [[nodiscard]]
        constexpr auto front(STORMKIT_LIFETIMEBOUND this Self& self) noexcept -> meta::forward_const_to<Self, reference>;
        template<typename Self>
        [[nodiscard]]
        constexpr auto back(STORMKIT_LIFETIMEBOUND this Self& self) noexcept -> meta::forward_const_to<Self, reference>;
        template<typename Self>
        [[nodiscard]]
        constexpr auto data(STORMKIT_LIFETIMEBOUND this Self& self) noexcept -> meta::forward_const_to<Self, value_type>*;

        [[nodiscard]]
        constexpr auto size() const noexcept -> size_type;
        [[nodiscard]]
        constexpr auto length() const noexcept -> size_type;
        [[nodiscard]]
        constexpr auto max_size() const noexcept -> size_type;
        [[nodiscard]]
        constexpr auto empty() const noexcept -> bool;

        [[nodiscard]]
        constexpr auto copy(ref_ptr<value_type> dest, size_type count, size_type pos) const noexcept -> size_type;
        [[nodiscard]]
        constexpr auto substr(size_type pos, size_type count = npos) const & noexcept -> string_view_type;
        [[nodiscard]]
        constexpr auto subview(size_type pos, size_type count = npos) const & noexcept -> string_view_type;
        [[nodiscard]]
        constexpr auto compare(string_view_type other) const noexcept -> int;
        [[nodiscard]]
        constexpr auto compare(size_type pos, size_type count, string_view_type other) const noexcept -> int;
        [[nodiscard]]
        constexpr auto compare(size_type        pos,
                               size_type        count,
                               string_view_type other,
                               size_type        other_post,
                               size_type        other_count) const noexcept -> int;
        [[nodiscard]]
        constexpr auto compare(ref_ptr<const value_type> other) const noexcept -> int;
        [[nodiscard]]
        constexpr auto compare(size_type pos, size_type count, ref_ptr<const value_type> other) const noexcept -> int;
        [[nodiscard]]
        constexpr auto compare(size_type                 pos,
                               size_type                 count,
                               ref_ptr<const value_type> other,
                               size_type                 other_post,
                               size_type                 other_count) const noexcept -> int;
        [[nodiscard]]
        constexpr auto starts_with(string_view_type prefix) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto starts_with(value_type prefix) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto starts_with(ref_ptr<const value_type> prefix) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto ends_with(string_view_type prefix) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto ends_with(value_type prefix) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto ends_with(ref_ptr<const value_type> prefix) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto contains(string_view_type prefix) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto contains(value_type prefix) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto contains(ref_ptr<const value_type> prefix) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto find(string_view_type prefix, size_type pos = 0) const noexcept -> size_type;
        [[nodiscard]]
        constexpr auto find(value_type prefix, size_type pos = 0) const noexcept -> size_type;
        [[nodiscard]]
        constexpr auto find(ref_ptr<const value_type> prefix, size_type pos, size_type count) const noexcept -> size_type;
        [[nodiscard]]
        constexpr auto find(ref_ptr<const value_type> prefix, size_type pos = 0) const noexcept -> size_type;
        [[nodiscard]]
        constexpr auto rfind(string_view_type prefix, size_type pos = 0) const noexcept -> size_type;
        [[nodiscard]]
        constexpr auto rfind(value_type prefix, size_type pos = 0) const noexcept -> size_type;
        [[nodiscard]]
        constexpr auto rfind(ref_ptr<const value_type> prefix, size_type pos, size_type count) const noexcept -> size_type;
        [[nodiscard]]
        constexpr auto rfind(ref_ptr<const value_type> prefix, size_type pos = 0) const noexcept -> size_type;
        [[nodiscard]]
        constexpr auto find_first_of(string_view_type prefix, size_type pos = 0) const noexcept -> size_type;
        [[nodiscard]]
        constexpr auto find_first_of(value_type prefix, size_type pos = 0) const noexcept -> size_type;
        [[nodiscard]]
        constexpr auto find_first_of(ref_ptr<const value_type> prefix, size_type pos, size_type count) const noexcept
          -> size_type;
        [[nodiscard]]
        constexpr auto find_first_of(ref_ptr<const value_type> prefix, size_type pos = 0) const noexcept -> size_type;
        [[nodiscard]]
        constexpr auto find_last_of(string_view_type prefix, size_type pos = 0) const noexcept -> size_type;
        [[nodiscard]]
        constexpr auto find_last_of(value_type prefix, size_type pos = 0) const noexcept -> size_type;
        [[nodiscard]]
        constexpr auto find_last_of(ref_ptr<const value_type> prefix, size_type pos, size_type count) const noexcept -> size_type;
        [[nodiscard]]
        constexpr auto find_last_of(ref_ptr<const value_type> prefix, size_type pos = 0) const noexcept -> size_type;
        [[nodiscard]]
        constexpr auto find_first_not_of(string_view_type prefix, size_type pos = 0) const noexcept -> size_type;
        [[nodiscard]]
        constexpr auto find_first_not_of(value_type prefix, size_type pos = 0) const noexcept -> size_type;
        [[nodiscard]]
        constexpr auto find_first_not_of(ref_ptr<const value_type> prefix, size_type pos, size_type count) const noexcept
          -> size_type;
        [[nodiscard]]
        constexpr auto find_first_not_of(ref_ptr<const value_type> prefix, size_type pos = 0) const noexcept -> size_type;
        [[nodiscard]]
        constexpr auto find_last_not_of(string_view_type prefix, size_type pos = 0) const noexcept -> size_type;
        [[nodiscard]]
        constexpr auto find_last_not_of(value_type prefix, size_type pos = 0) const noexcept -> size_type;
        [[nodiscard]]
        constexpr auto find_last_not_of(ref_ptr<const value_type> prefix, size_type pos, size_type count) const noexcept
          -> size_type;
        [[nodiscard]]
        constexpr auto find_last_not_of(ref_ptr<const value_type> prefix, size_type pos = 0) const noexcept -> size_type;

        [[nodiscard]]
        constexpr operator string_view_type() const noexcept STORMKIT_LIFETIMEBOUND;

        storage_type m_data = {};
    };

    static_assert(meta::structural_type<static_string<1>>);
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    consteval static_string<N, CharT>::static_string() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    consteval static_string<N, CharT>::static_string(const value_type (&str)[N]) noexcept {
        std::copy_n(str, SIZE, std::data(m_data));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr static_string<N, CharT>::~static_string() = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr static_string<N, CharT>::static_string(const static_string&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr static_string<N, CharT>::static_string(static_string&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::operator=(const static_string&) noexcept -> static_string& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::operator=(static_string&&) noexcept -> static_string& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::begin(this Self& self) noexcept -> conditional_iterator<Self> {
        return stdr::begin(std::forward_like<Self&>(self.m_data));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::cbegin() const noexcept -> const_iterator {
        return stdr::cbegin(m_data);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::end(this Self& self) noexcept -> conditional_iterator<Self> {
        return stdr::end(std::forward_like<Self&>(self.m_data));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::cend() const noexcept -> const_iterator {
        return stdr::cend(m_data);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::rbegin(this Self& self) noexcept -> conditional_reverse_iterator<Self> {
        return stdr::rbegin(std::forward_like<Self&>(self.m_data));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::crbegin() const noexcept -> const_iterator {
        return stdr::crbegin(m_data);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::rend(this Self& self) noexcept -> conditional_reverse_iterator<Self> {
        return stdr::rend(std::forward_like<Self&>(self.m_data));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::crend() const noexcept -> const_iterator {
        return stdr::crend(m_data);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::operator[](this Self& self, size_type index) noexcept
      -> meta::forward_const_to<Self, reference> {
        return std::forward_like<Self&>(self.m_data[index]);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::at(this Self& self, size_type index) noexcept
      -> meta::forward_const_to<Self, reference> {
        EXPECTS(index < SIZE);
        return std::forward_like<Self&>(self.operator[](index));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::front(this Self& self) noexcept -> meta::forward_const_to<Self, reference> {
        return std::forward_like<Self&>(self.operator[](0));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::back(this Self& self) noexcept -> meta::forward_const_to<Self, reference> {
        return std::forward_like<Self&>(self.operator[](SIZE - 1));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::data(this Self& self) noexcept -> meta::forward_const_to<Self, value_type>* {
        return stdr::data(std::forward_like<Self&>(self.m_data));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto static_string<N, CharT>::size() const noexcept -> size_type {
        return SIZE;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto static_string<N, CharT>::length() const noexcept -> size_type {
        return SIZE;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto static_string<N, CharT>::max_size() const noexcept -> size_type {
        return SIZE;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto static_string<N, CharT>::empty() const noexcept -> bool {
        return SIZE != 0;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::copy(ref_ptr<value_type> dest, size_type count, size_type pos) const noexcept
      -> size_type {
        EXPECTS(count <= (SIZE - pos));
        stdr::copy_n(dest, count, begin() + pos);
        return SIZE;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::substr(size_type pos, size_type count) const & noexcept -> string_view_type {
        return subview(pos, count);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::subview(size_type pos, size_type count) const & noexcept -> string_view_type {
        EXPECTS(count <= (SIZE - pos));
        return operator string_view_type().subview(pos, (count == npos) ? string_view_type::npos : count);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::compare(string_view_type other) const noexcept -> int {
        return operator string_view_type().compare(other);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::compare(size_type pos, size_type count, string_view_type other) const noexcept
      -> int {
        return operator string_view_type().compare(pos, count, other);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::
      compare(size_type pos, size_type count, string_view_type other, size_type other_post, size_type other_count) const noexcept
      -> int {
        return operator string_view_type().compare(pos, count, other, other_post, other_count);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::compare(ref_ptr<const value_type> other) const noexcept -> int {
        return operator string_view_type().compare(other);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::compare(size_type pos, size_type count, ref_ptr<const value_type> other)
      const noexcept -> int {
        return operator string_view_type().compare(pos, count, other);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::compare(size_type                 pos,
                                                    size_type                 count,
                                                    ref_ptr<const value_type> other,
                                                    size_type                 other_post,
                                                    size_type                 other_count) const noexcept -> int {
        return operator string_view_type().compare(pos, count, other, other_post, other_count);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::starts_with(string_view_type prefix) const noexcept -> bool {
        return operator string_view_type().starts_with(prefix);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::starts_with(value_type prefix) const noexcept -> bool {
        return operator string_view_type().starts_with(prefix);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::starts_with(ref_ptr<const value_type> prefix) const noexcept -> bool {
        return operator string_view_type().starts_with(prefix);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::ends_with(string_view_type prefix) const noexcept -> bool {
        return operator string_view_type().ends_with(prefix);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::ends_with(value_type prefix) const noexcept -> bool {
        return operator string_view_type().ends_with(prefix);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::ends_with(ref_ptr<const value_type> prefix) const noexcept -> bool {
        return operator string_view_type().ends_with(prefix);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::contains(string_view_type prefix) const noexcept -> bool {
        return operator string_view_type().contains(prefix);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::contains(value_type prefix) const noexcept -> bool {
        return operator string_view_type().contains(prefix);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::contains(ref_ptr<const value_type> prefix) const noexcept -> bool {
        return operator string_view_type().contains(prefix);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::find(string_view_type prefix, size_type pos) const noexcept -> size_type {
        return operator string_view_type().find(prefix, pos);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::find(value_type prefix, size_type pos) const noexcept -> size_type {
        return operator string_view_type().find(prefix, pos);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::find(ref_ptr<const value_type> prefix, size_type pos, size_type count) const noexcept
      -> size_type {
        return operator string_view_type().find(prefix, pos, count);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::find(ref_ptr<const value_type> prefix, size_type pos) const noexcept -> size_type {
        return operator string_view_type().find(prefix, pos);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::rfind(string_view_type prefix, size_type pos) const noexcept -> size_type {
        return operator string_view_type().rfind(prefix, pos);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::rfind(value_type prefix, size_type pos) const noexcept -> size_type {
        return operator string_view_type().rfind(prefix, pos);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::rfind(ref_ptr<const value_type> prefix, size_type pos, size_type count) const noexcept
      -> size_type {
        return operator string_view_type().rfind(prefix, pos, count);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::rfind(ref_ptr<const value_type> prefix, size_type pos) const noexcept -> size_type {
        return operator string_view_type().rfind(prefix, pos);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::find_first_of(string_view_type prefix, size_type pos) const noexcept -> size_type {
        return operator string_view_type().find_first_of(prefix, pos);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::find_first_of(value_type prefix, size_type pos) const noexcept -> size_type {
        return operator string_view_type().find_first_of(prefix, pos);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::find_first_of(ref_ptr<const value_type> prefix, size_type pos, size_type count)
      const noexcept -> size_type {
        return operator string_view_type().find_first_of(prefix, pos, count);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::find_first_of(ref_ptr<const value_type> prefix, size_type pos) const noexcept
      -> size_type {
        return operator string_view_type().find_first_of(prefix, pos);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::find_last_of(string_view_type prefix, size_type pos) const noexcept -> size_type {
        return operator string_view_type().find_last_of(prefix, pos);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::find_last_of(value_type prefix, size_type pos) const noexcept -> size_type {
        return operator string_view_type().find_last_of(prefix, pos);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::find_last_of(ref_ptr<const value_type> prefix, size_type pos, size_type count)
      const noexcept -> size_type {
        return operator string_view_type().find_last_of(prefix, pos, count);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::find_last_of(ref_ptr<const value_type> prefix, size_type pos) const noexcept
      -> size_type {
        return operator string_view_type().find_last_of(prefix, pos);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::find_first_not_of(string_view_type prefix, size_type pos) const noexcept
      -> size_type {
        return operator string_view_type().find_first_not_of(prefix, pos);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::find_first_not_of(value_type prefix, size_type pos) const noexcept -> size_type {
        return operator string_view_type().find_first_not_of(prefix, pos);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::find_first_not_of(ref_ptr<const value_type> prefix, size_type pos, size_type count)
      const noexcept -> size_type {
        return operator string_view_type().find_first_not_of(prefix, pos, count);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::find_first_not_of(ref_ptr<const value_type> prefix, size_type pos) const noexcept
      -> size_type {
        return operator string_view_type().find_first_not_of(prefix, pos);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::find_last_not_of(string_view_type prefix, size_type pos) const noexcept -> size_type {
        return operator string_view_type().find_last_not_of(prefix, pos);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::find_last_not_of(value_type prefix, size_type pos) const noexcept -> size_type {
        return operator string_view_type().find_last_not_of(prefix, pos);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::find_last_not_of(ref_ptr<const value_type> prefix, size_type pos, size_type count)
      const noexcept -> size_type {
        return operator string_view_type().find_last_not_of(prefix, pos, count);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto static_string<N, CharT>::find_last_not_of(ref_ptr<const value_type> prefix, size_type pos) const noexcept
      -> size_type {
        return operator string_view_type().find_last_not_of(prefix, pos);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<usize N, typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr static_string<N, CharT>::operator string_view_type() const noexcept {
        return string_view_type { stdr::data(m_data), stdr::size(m_data) };
    }
}} // namespace stormkit::core
