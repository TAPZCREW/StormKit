// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#include <stormkit/wsi/api.hpp>

export module stormkit.wsi:core;

import std;

import stormkit.core;

export namespace stormkit::wsi {
    enum class window_manager : u8 {
        win32 = 0,
        wayland,
        x11,
        android,
        macos,
        ios,
        tvos,
        nintendo_switch,
    };

    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<string_view>, window_manager value, const std::source_location&) noexcept -> string_view;

    STORMKIT_WSI_API
    auto parse_args(array_view<const string_view> args) noexcept -> void;

    [[nodiscard]]
    STORMKIT_WSI_API auto wm() noexcept -> window_manager;
} // namespace stormkit::wsi

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::wsi {
    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto tag_invoke(as_fn<string_view>, window_manager wm, const std::source_location&) noexcept -> string_view {
        switch (wm) {
            case window_manager::win32: return "window_manager::win32";
            case window_manager::wayland: return "window_manager::wayland";
            case window_manager::x11: return "window_manager::x11";
            case window_manager::android: return "window_manager::android";
            case window_manager::macos: return "window_manager::macos";
            case window_manager::ios: return "window_manager::ios";
            case window_manager::tvos: return "window_manager::tvos";
            case window_manager::nintendo_switch: return "window_manager::nintendo_switch";
            default: break;
        }

        std::unreachable();
    }
} // namespace stormkit::wsi
