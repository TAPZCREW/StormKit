// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.wsi:linux.wayland.log;

import std;

import stormkit.core;
import stormkit.log;

export namespace stormkit::wsi::linux::wayland {
    template<class... Ts>
    auto dlog(std::format_string<Ts...> format, Ts&&... args) noexcept -> void;

    template<class... Ts>
    auto ilog(std::format_string<Ts...> format, Ts&&... args) noexcept -> void;

    template<class... Ts>
    auto wlog(std::format_string<Ts...> format, Ts&&... args) noexcept -> void;

    template<class... Ts>
    auto elog(std::format_string<Ts...> format, Ts&&... args) noexcept -> void;

    template<class... Ts>
    auto flog(std::format_string<Ts...> format, Ts&&... args) noexcept -> void;
} // namespace stormkit::wsi::linux::wayland

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::wsi::linux::wayland {
    constexpr auto LOG_MODULE = log::module { "stormkit.wsi.linux.wayland" };

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>                                                   
    STORMKIT_FORCE_INLINE
    inline auto dlog(std::format_string<Ts...> format, Ts&&... args) noexcept -> void {
        LOG_MODULE.dlog(std::move(format), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>                                                   
    STORMKIT_FORCE_INLINE
    inline auto ilog(std::format_string<Ts...> format, Ts&&... args) noexcept -> void {
        LOG_MODULE.ilog(std::move(format), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>                                                   
    STORMKIT_FORCE_INLINE
    inline auto wlog(std::format_string<Ts...> format, Ts&&... args) noexcept -> void {
        LOG_MODULE.wlog(std::move(format), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>                                                   
    STORMKIT_FORCE_INLINE
    inline auto elog(std::format_string<Ts...> format, Ts&&... args) noexcept -> void {
        LOG_MODULE.elog(std::move(format), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>                                                   
    STORMKIT_FORCE_INLINE
    inline auto flog(std::format_string<Ts...> format, Ts&&... args) noexcept -> void {
        LOG_MODULE.flog(std::move(format), std::forward<Ts>(args)...);
    }
} // namespace stormkit::wsi::linux::wayland
