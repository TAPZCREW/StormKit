// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.at_scope_exit;

import std;

export namespace stormkit { inline namespace core {
    template<auto Do>
    constexpr auto at_scope_exit_do() noexcept -> decltype(auto);

    template<std::regular_invocable<> T_do>
    constexpr auto at_scope_exit_do(T_do&& do_at_exit) noexcept -> decltype(auto);
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    namespace impl {
        template<auto Do>
        struct at_scope_exit_nttp {
            constexpr ~at_scope_exit_nttp() noexcept { Do(); }
        };

        template<typename T_do>
        struct at_scope_exit {
            constexpr at_scope_exit(T_do&& do_at_exit_) noexcept
                : do_at_exit { std::forward<T_do>(do_at_exit_) }

            {}

            constexpr ~at_scope_exit() noexcept(noexcept(std::declval<T_do>()())) { this->do_at_exit(); }

            T_do do_at_exit;
        };
    } // namespace impl

    template<auto Do>
    constexpr auto at_scope_exit_do() noexcept -> decltype(auto) {
        return impl::at_scope_exit_nttp<Do> {};
    }

    template<std::regular_invocable<> T_do>
    constexpr auto at_scope_exit_do(T_do&& do_at_exit) noexcept -> decltype(auto) {
        return impl::at_scope_exit<T_do> { std::forward<T_do>(do_at_exit) };
    }
}} // namespace stormkit::core
