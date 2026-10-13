// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
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

        size_type capacity_ = 0;
        size_type count_    = 0;

        dynarray<byte> buffer_;

        size_type write_ = 0;
        size_type read_  = 0;
    };
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    ring_buffer<T>::ring_buffer(size_type capacity) : capacity_ { capacity } {
        buffer_.resize(capacity_ * sizeof(value_type));
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    ring_buffer<T>::ring_buffer(const ring_buffer& copy) {
        capacity_ = copy.capacity_;
        count_    = copy.count_;
        write_    = copy.write_;
        read_     = copy.read_;

        buffer_.resize(capacity_ * sizeof(value_type));
        if (not empty()) {
            for (auto i = read_; i < write_;) {
                new (&buffer_[i * sizeof(value_type)]) T { *copy.get_ptr(i) };

                i += 1;
                if (i >= capacity_) i -= capacity_;
            }
        }
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    auto ring_buffer<T>::operator=(const ring_buffer& copy) -> ring_buffer& {
        if (&copy == this) return *this;

        capacity_ = copy.capacity_;
        count_    = copy.count_;
        write_    = copy.write_;
        read_     = copy.read_;

        buffer_.resize(capacity_ * sizeof(value_type));
        if (not empty())
            for (auto i = read_; i < write_;) {
                new (&buffer_[i * sizeof(value_type)]) T { *copy.get_ptr(i) };

                i += 1;
                if (i >= capacity_) i -= capacity_;
            }

        return *this;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    ring_buffer<T>::ring_buffer(ring_buffer&& moved) noexcept {
        buffer_ = std::exchange(moved.buffer_, dynarray<byte> {});

        capacity_ = std::exchange(moved.capacity_, 0);
        count_    = std::exchange(moved.count_, 0);
        write_    = std::exchange(moved.write_, 0);
        read_     = std::exchange(moved.read_, 0);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    auto ring_buffer<T>::operator=(ring_buffer&& moved) noexcept -> ring_buffer& {
        if (&moved == this) return *this;

        buffer_ = std::exchange(moved.buffer_, dynarray<byte> {});

        capacity_ = std::exchange(moved.capacity_, 0);
        count_    = std::exchange(moved.count_, 0);
        write_    = std::exchange(moved.write_, 0);
        read_     = std::exchange(moved.read_, 0);

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
        return count_ == 0;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    auto ring_buffer<T>::full() const noexcept -> bool {
        return count_ == capacity_;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    auto ring_buffer<T>::size() const noexcept -> size_type {
        return count_;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    auto ring_buffer<T>::capacity() const noexcept -> size_type {
        return capacity_;
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
        if (count_ == capacity_) pop();

        new (&buffer_[write_ * sizeof(value_type)]) value_type { std::forward<Ts>(values)... };

        write_ += 1;
        if (write_ >= capacity_) write_ -= capacity_;

        count_++;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    auto ring_buffer<T>::next() noexcept -> void {
        read_ += 1;
        if (read_ >= capacity_) read_ -= capacity_;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    auto ring_buffer<T>::pop() noexcept -> void {
        EXPECTS(not empty());

        get_ptr(write_)->~value_type();

        --count_;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    template<class Self>
    auto ring_buffer<T>::get(this Self& self) noexcept -> decltype(auto) {
        EXPECTS(not self.empty());

        return std::forward_like<Self>(self.get_ptr(self.read_));
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    auto ring_buffer<T>::data() const noexcept -> array_view<const value_type> {
        return array_view<const value_type> { get_ptr(0), capacity_ };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    template<class Self>
    auto ring_buffer<T>::get_ptr(this Self& self, size_type pos) noexcept -> decltype(auto) {
        using OutPtr = meta::forward_const_to<Self, value_type*>;
        auto addr    = std::forward_like<Self>(&(self.buffer_[pos * sizeof(value_type)]));

        return std::launder(std::bit_cast<OutPtr>(addr));
    }
}} // namespace stormkit::core
