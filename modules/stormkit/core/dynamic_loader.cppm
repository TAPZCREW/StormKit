// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/api.hpp>
#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>
#include <stormkit/core/try_expected.hpp>

export module stormkit.core.dynamic_loader;

import std;

import stormkit.core.contract;
import stormkit.core.types;

import stormkit.core.heap;
import stormkit.core.errors;
import stormkit.core.function_ref;
import stormkit.core.private_tag;

namespace stdfs = std::filesystem;

export namespace stormkit { inline namespace core {
    class STORMKIT_CORE_API dynamic_loader {
        using private_tag = private_tag<dynamic_loader>;

      public:
        explicit dynamic_loader(private_tag) noexcept;
        ~dynamic_loader();

        dynamic_loader(const dynamic_loader&)                    = delete;
        auto operator=(const dynamic_loader&) -> dynamic_loader& = delete;

        dynamic_loader(dynamic_loader&&) noexcept;
        auto operator=(dynamic_loader&&) noexcept -> dynamic_loader&;

        [[nodiscard]]
        static auto load(const stdfs::path& filepath) noexcept -> system_result<dynamic_loader>;

        [[nodiscard]]
        static auto allocate_and_load(const stdfs::path& filepath) noexcept -> system_result<heap_ptr<dynamic_loader>>;

        template<class Signature>
        [[nodiscard]]
        auto func(string_view name) const noexcept -> system_result<std23::function_ref<Signature>>;

        template<class Signature>
        [[nodiscard]]
        auto c_func(string_view name) const noexcept -> system_result<Signature*>;

        [[nodiscard]]
        auto filepath() const noexcept -> const stdfs::path&;

      private:
        auto do_load(const stdfs::path& filepath) -> system_result<void>;
        auto do_get_func(string_view name) const -> system_result<void*>;

        stdfs::path m_filepath;
        void*       m_library_handle = nullptr;
    };
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline dynamic_loader::dynamic_loader(private_tag) noexcept {};

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline dynamic_loader::dynamic_loader(dynamic_loader&& other) noexcept
        : m_filepath { std::move(other.m_filepath) }, m_library_handle { std::exchange(other.m_library_handle, nullptr) } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto dynamic_loader::operator=(dynamic_loader&& other) noexcept -> dynamic_loader& {
        if (&other == this) [[unlikely]]
            return *this;

        m_filepath       = std::move(other.m_filepath);
        m_library_handle = std::exchange(other.m_library_handle, nullptr);

        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto dynamic_loader::load(const stdfs::path& filepath) noexcept -> system_result<dynamic_loader> {
        auto loader = dynamic_loader { PRIVATE<dynamic_loader> };
        Try(loader.do_load(filepath));
        return { std::move(loader) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto dynamic_loader::allocate_and_load(const stdfs::path& filepath) noexcept
      -> system_result<heap_ptr<dynamic_loader>> {
        auto loader = allocate_unsafe<dynamic_loader>(PRIVATE<dynamic_loader>);
        Try(loader->do_load(filepath));
        return { std::move(loader) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<class Signature>
    inline auto dynamic_loader::func(string_view name) const noexcept -> system_result<std23::function_ref<Signature>> {
        TryTo(raw_func, c_func<Signature>(name));
        return { std23::function_ref<Signature> { raw_func } };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<class Signature>
    inline auto dynamic_loader::c_func(string_view name) const noexcept -> system_result<Signature*> {
        EXPECTS(not std::empty(name));

        TryTo(raw_func, do_get_func(name));
        return { std::bit_cast<Signature*>(raw_func) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto dynamic_loader::filepath() const noexcept -> const stdfs::path& {
        return m_filepath;
    }
}} // namespace stormkit::core
