// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/api.hpp>
#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>
#include <stormkit/core/try_expected.hpp>

export module stormkit.core.containers.shmbuffer;

import std;

import stormkit.core.typesafe;
import stormkit.core.functional;
import stormkit.core.contract;
import stormkit.core.types;
import stormkit.core.heap;
import stormkit.core.errors;
import stormkit.core.meta.concepts;
import stormkit.core.meta.type_manipulation;
import stormkit.core.filesystem;
import stormkit.core.private_tag;

namespace stdr = std::ranges;

export namespace stormkit { inline namespace core {
    class STORMKIT_CORE_API shm_buffer final {
        using private_tag = private_tag<shm_buffer>;

      public:
        using value_type = byte;

        static auto create(usize size, string name, io::access access = io::access::read | io::access::write) noexcept
          -> system_result<shm_buffer>;
        static auto allocate(usize size, string name, io::access access = io::access::read | io::access::write) noexcept
          -> system_result<heap_ptr<shm_buffer>>;

        explicit shm_buffer(private_tag) noexcept;
        ~shm_buffer();

        shm_buffer(const shm_buffer&)                    = delete;
        auto operator=(const shm_buffer&) -> shm_buffer& = delete;

        shm_buffer(shm_buffer&&) noexcept;
        auto operator=(shm_buffer&&) noexcept -> shm_buffer&;

        template<typename Self>
        auto begin(this Self&) noexcept -> decltype(auto);
        auto cbegin() const noexcept -> decltype(auto);

        template<typename Self>
        auto end(this Self&) noexcept -> decltype(auto);
        auto cend() const noexcept -> decltype(auto);

        template<typename Self>
        auto operator[](this Self&, usize index) noexcept -> meta::forward_const_to<Self, value_type>&;
        template<typename Self>
        auto at(this Self&, usize index) noexcept -> meta::forward_const_to<Self, value_type>&;

        auto size() const noexcept -> usize;
        template<typename Self>
        auto data(this Self&) noexcept -> array_view<meta::forward_const_to<Self, value_type>>;
        template<typename Self>
        auto native_handle(this Self&) noexcept -> meta::forward_const_to<Self, void>*;
        auto name() const noexcept -> string_view;
        auto access() const noexcept -> io::access;

      private:
        auto do_init(usize, string&&, io::access) noexcept -> system_result<void>;

        io::access             access_;
        void*                  handle_ = nullptr;
        usize                  size_;
        string                 name_;
        array_view<value_type> data_;
    };
}} // namespace stormkit::core

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto shm_buffer::create(usize size, string name, io::access access) noexcept -> system_result<shm_buffer> {
        auto out = shm_buffer { private_token<shm_buffer> };
        Try(out.do_init(size, std::move(name), access));
        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto shm_buffer::allocate(usize size, string name, io::access access) noexcept -> system_result<heap_ptr<shm_buffer>> {
        auto out = allocate_unsafe<shm_buffer>(private_token<shm_buffer>);
        Try(out->do_init(size, std::move(name), access));
        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline shm_buffer::shm_buffer(private_tag) noexcept {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline shm_buffer::shm_buffer(shm_buffer&& other) noexcept
        : access_ { other.access() },
          handle_ { std::exchange(other.handle_, nullptr) },
          size_ { std::exchange(other.size_, 0u) },
          name_ { std::exchange(other.name_, {}) },
          data_ { std::exchange(other.data_, {}) } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto shm_buffer::operator=(shm_buffer&& other) noexcept -> shm_buffer& {
        if (&other == this) [[unlikely]]
            return *this;

        access_ = other.access_;
        handle_ = std::exchange(other.handle_, nullptr);
        data_   = std::exchange(other.data_, {});
        size_   = std::exchange(other.size_, 0u);
        name_   = std::exchange(other.name_, {});

        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Self>
    STORMKIT_FORCE_INLINE
    inline auto shm_buffer::begin(this Self& self) noexcept -> decltype(auto) {
        EXPECTS(self.handle_);
        return stdr::begin(std::forward<Self&>(self).data_);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto shm_buffer::cbegin() const noexcept -> decltype(auto) {
        EXPECTS(handle_);
        return stdr::cbegin(data_);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Self>
    STORMKIT_FORCE_INLINE
    inline auto shm_buffer::end(this Self& self) noexcept -> decltype(auto) {
        EXPECTS(self.handle_);
        return stdr::end(std::forward<Self&>(self).data_);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto shm_buffer::cend() const noexcept -> decltype(auto) {
        EXPECTS(handle_);
        return stdr::cend(data_);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Self>
    STORMKIT_FORCE_INLINE
    inline auto shm_buffer::operator[](this Self& self, usize index) noexcept -> meta::forward_const_to<Self, value_type>& {
        EXPECTS(self.handle_);
        EXPECTS(index < self.size_);
        return std::forward<Self&>(self).data_[index];
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Self>
    STORMKIT_FORCE_INLINE
    inline auto shm_buffer::at(this Self& self, usize index) noexcept -> meta::forward_const_to<Self, value_type>& {
        EXPECTS(self.handle_);
        EXPECTS(index < self.size_);
        return std::forward<Self&>(self).data_.at(index);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto shm_buffer::size() const noexcept -> usize {
        return size_;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Self>
    STORMKIT_FORCE_INLINE
    inline auto shm_buffer::data(this Self& self) noexcept -> array_view<meta::forward_const_to<Self, value_type>> {
        EXPECTS(self.handle_);
        return std::forward<Self&>(self).data_;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Self>
    STORMKIT_FORCE_INLINE
    inline auto shm_buffer::native_handle(this Self& self) noexcept -> meta::forward_const_to<Self, void>* {
        return std::forward<Self&>(self).handle_;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto shm_buffer::name() const noexcept -> string_view {
        return name_;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto shm_buffer::access() const noexcept -> io::access {
        return access_;
    }
}} // namespace stormkit::core
