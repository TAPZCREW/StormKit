// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

module stormkit.wsi;

import std;

import stormkit.core;

import :core;

namespace stormkit::wsi {
    namespace {
        constinit auto wm_hint = std::optional<WM> {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto parse_args(array_view<const string_view> args) noexcept -> void {
        auto hint = std::ranges::find_if(args, [](auto&& v) { return v == "--x11" or v == "--wayland"; });

        if (hint != std::ranges::cend(args)) {
            if (*hint == "--x11") wm_hint = WM::X11;
            else if (*hint == "--wayland")
                wm_hint = WM::WAYLAND;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    WM wm() noexcept {
#if defined(STORMKIT_OS_WINDOWS)
        return WM::WIN32;
#elif defined(STORMKIT_OS_MACOS)
        return WM::MACOS;
#elif defined(STORMKIT_OS_IOS)
        return WM::IOS;
#elif defined(STORMKIT_OS_ANDROID)
        return WM::ANDROID;
#elif defined(STORMKIT_OS_SWITCH)
        return WM::SWITCH;
#elif defined(STORMKIT_OS_LINUX)
        auto is_wayland = std::getenv("WAYLAND_DISPLAY") != nullptr;

        if (wm_hint) return wm_hint.value();
        else if (is_wayland)
            return WM::WAYLAND;
        else
            return WM::X11;
        std::unreachable();
#endif
    }
} // namespace stormkit::wsi
