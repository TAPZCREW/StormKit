// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>

export module stormkit.core.containers.ringbuffer;

import std;

import stormkit.core.types;
import stormkit.core.typesafe;
import stormkit.core.meta;
import stormkit.core.contract;

export namespace stormkit { inline namespace core {
    template<typename T>
    class ring_buffer {
      public:
        using value_type = T;
        using size_type  = usize;

        ring_buffer(size_type capacity);

        ring_buffer(const ring_buffer& copy);
        auto operator=(const ring_buffer& copy) -> ring_buffer&;

        ring_buffer(ring_buffer&& moved) noexcept;
        auto operator=(ring_buffer&& moved) noexcept -> ring_buffer&;

        ~ring_buffer() noexcept;

        auto clear() noexcept -> void;

        [[nodiscard]]
        auto empty() const noexcept -> bool;
        [[nodiscard]]
        auto full() const noexcept -> bool;
        [[nodiscard]]
        auto size() const noexcept -> size_type;
        [[nodiscard]]
        auto capacity() const noexcept -> size_type;

        template<meta::plain::is<T> U>
        auto push(U&& value) noexcept(meta::noexcept_constructible_from<value_type, U>) -> void;

        template<typename... Ts>
        auto emplace(Ts&&... values) noexcept(meta::noexcept_constructible_from<value_type, Ts...>) -> void;

        auto next() noexcept -> void;

        auto pop() noexcept -> void;

        template<class Self>
        [[nodiscard]]
        auto get(this Self& self) noexcept -> decltype(auto);

        [[nodiscard]]
        auto data() const noexcept -> array_view<const value_type>;

      private:
        template<class Self>
        [[nodiscard]]
        auto get_ptr(this Self& self, size_type pos) noexcept -> decltype(auto);

        size_type m_capacity = 0;
        size_type m_count    = 0;

        dynarray<byte> m_buffer;

        size_type m_write = 0;
        size_type m_read  = 0;
    };
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    ring_buffer<T>::ring_buffer(size_type capacity) : m_capacity { capacity } {
        m_buffer.resize(m_capacity * sizeof(value_type));
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    ring_buffer<T>::ring_buffer(const ring_buffer& copy) {
        m_capacity = copy.m_capacity;
        m_count    = copy.m_count;
        m_write    = copy.m_write;
        m_read     = copy.m_read;

        m_buffer.resize(m_capacity * sizeof(value_type));
        if (not empty()) {
            for (auto i = m_read; i < m_write;) {
                new (&m_buffer[i * sizeof(value_type)]) T { *copy.get_ptr(i) };

                i += 1;
                if (i >= m_capacity) i -= m_capacity;
            }
        }
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    auto ring_buffer<T>::operator=(const ring_buffer& copy) -> ring_buffer& {
        if (&copy == this) return *this;

        m_capacity = copy.m_capacity;
        m_count    = copy.m_count;
        m_write    = copy.m_write;
        m_read     = copy.m_read;

        m_buffer.resize(m_capacity * sizeof(value_type));
        if (not empty())
            for (auto i = m_read; i < m_write;) {
                new (&m_buffer[i * sizeof(value_type)]) T { *copy.get_ptr(i) };

                i += 1;
                if (i >= m_capacity) i -= m_capacity;
            }

        return *this;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    ring_buffer<T>::ring_buffer(ring_buffer&& moved) noexcept {
        m_buffer = std::exchange(moved.m_buffer, dynarray<byte> {});

        m_capacity = std::exchange(moved.m_capacity, 0);
        m_count    = std::exchange(moved.m_count, 0);
        m_write    = std::exchange(moved.m_write, 0);
        m_read     = std::exchange(moved.m_read, 0);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    auto ring_buffer<T>::operator=(ring_buffer&& moved) noexcept -> ring_buffer& {
        if (&moved == this) return *this;

        m_buffer = std::exchange(moved.m_buffer, dynarray<byte> {});

        m_capacity = std::exchange(moved.m_capacity, 0);
        m_count    = std::exchange(moved.m_count, 0);
        m_write    = std::exchange(moved.m_write, 0);
        m_read     = std::exchange(moved.m_read, 0);

        return *this;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    ring_buffer<T>::~ring_buffer() noexcept {
        clear();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    auto ring_buffer<T>::clear() noexcept -> void {
        while (not empty()) pop();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    auto ring_buffer<T>::empty() const noexcept -> bool {
        return m_count == 0;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    auto ring_buffer<T>::full() const noexcept -> bool {
        return m_count == m_capacity;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    auto ring_buffer<T>::size() const noexcept -> size_type {
        return m_count;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    auto ring_buffer<T>::capacity() const noexcept -> size_type {
        return m_capacity;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    template<meta::plain::is<T> U>
    auto ring_buffer<T>::push(U&& value) noexcept(meta::noexcept_constructible_from<value_type, U>) -> void {
        emplace(std::forward(value));
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    template<typename... Ts>
    auto ring_buffer<T>::emplace(Ts&&... values) noexcept(meta::noexcept_constructible_from<value_type, Ts...>) -> void {
        if (m_count == m_capacity) pop();

        new (&m_buffer[m_write * sizeof(value_type)]) value_type { std::forward<Ts>(values)... };

        m_write += 1;
        if (m_write >= m_capacity) m_write -= m_capacity;

        m_count++;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    auto ring_buffer<T>::next() noexcept -> void {
        m_read += 1;
        if (m_read >= m_capacity) m_read -= m_capacity;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    auto ring_buffer<T>::pop() noexcept -> void {
        EXPECTS(not empty());

        get_ptr(m_write)->~value_type();

        --m_count;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    template<class Self>
    auto ring_buffer<T>::get(this Self& self) noexcept -> decltype(auto) {
        EXPECTS(not self.empty());

        return std::forward_like<Self>(self.get_ptr(self.m_read));
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    auto ring_buffer<T>::data() const noexcept -> array_view<const value_type> {
        return array_view<const value_type> { get_ptr(0), m_capacity };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    template<class Self>
    auto ring_buffer<T>::get_ptr(this Self& self, size_type pos) noexcept -> decltype(auto) {
        using OutPtr = meta::forward_const_to<Self, value_type*>;
        auto addr    = std::forward_like<Self>(&(self.m_buffer[pos * sizeof(value_type)]));

        return std::launder(std::bit_cast<OutPtr>(addr));
    }
}} // namespace stormkit::core
