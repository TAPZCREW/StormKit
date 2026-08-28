// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.at_scope_exit;

import std;

export namespace stormkit { inline namespace core {
    template<auto DO>
    constexpr auto at_scope_exit_do() noexcept -> decltype(auto);

    template<std::regular_invocable<> TDO>
    constexpr auto at_scope_exit_do(TDO&& do_at_exit) noexcept -> decltype(auto);
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    namespace impl {
        template<auto DO>
        struct at_scope_exit_nttp {
            constexpr ~at_scope_exit_nttp() noexcept { DO(); }
        };

        template<typename TDO>
        struct at_scope_exit {
            constexpr at_scope_exit(TDO&& do_at_exit_) noexcept
                : do_at_exit { std::forward<TDO>(do_at_exit_) }

            {}

            constexpr ~at_scope_exit() noexcept(noexcept(std::declval<TDO>()())) { this->do_at_exit(); }

            TDO do_at_exit;
        };
    } // namespace impl

    template<auto DO>
    constexpr auto at_scope_exit_do() noexcept -> decltype(auto) {
        return impl::at_scope_exit_nttp<DO> {};
    }

    template<std::regular_invocable<> TDO>
    constexpr auto at_scope_exit_do(TDO&& do_at_exit) noexcept -> decltype(auto) {
        return impl::at_scope_exit<TDO> { std::forward<TDO>(do_at_exit) };
    }
}} // namespace stormkit::core
