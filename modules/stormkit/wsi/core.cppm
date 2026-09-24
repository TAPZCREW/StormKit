// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#include <stormkit/wsi/api.hpp>

export module stormkit.wsi:core;

import std;

import stormkit.core;

export namespace stormkit::wsi {
    enum class WM : u8 {
        WIN32 = 0,
        WAYLAND,
        X11,
        ANDROID,
        MACOS,
        IOS,
        TVOS,
        SWITCH,
    };

    constexpr auto as_string(WM wm) noexcept -> string_view;
    constexpr auto to_string(WM wm) noexcept -> string;

    STORMKIT_WSI_API
    auto parse_args(array_view<const string_view> args) noexcept -> void;

    [[nodiscard]]
    STORMKIT_WSI_API auto wm() noexcept -> WM;
} // namespace stormkit::wsi

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::wsi {
    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto as_string(WM wm) noexcept -> string_view {
        switch (wm) {
            case WM::WIN32: return "WM::WIN32";
            case WM::WAYLAND: return "WM::WAYLAND";
            case WM::X11: return "WM::X11";
            case WM::ANDROID: return "WM::ANDROID";
            case WM::MACOS: return "WM::MACOS";
            case WM::IOS: return "WM::IOS";
            case WM::TVOS: return "WM::TVOS";
            case WM::SWITCH: return "WM::SWITCH";
            default: break;
        }

        std::unreachable();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto to_string(WM wm) noexcept -> string {
        return string { as_string(wm) };
    }
} // namespace stormkit::wsi
