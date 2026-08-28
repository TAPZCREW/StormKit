// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform/windows.hpp>
#include <stormkit/core/platform_macro.hpp>

#include <windowsx.h>

export module stormkit.wsi:win32.mouse;

import stormkit.core;
import stormkit.wsi;

export namespace stormkit::wsi::win32 {
    constexpr auto extract_mouse_button(UINT message, WPARAM w_param, LPARAM l_param) noexcept
      -> MouseButton;
    constexpr auto extract_mouse_position(HWND   handle,
                                          WPARAM w_param,
                                          LPARAM l_param,
                                          bool   to_client = true) noexcept -> math::ivec2;
} // namespace stormkit::wsi::win32

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::wsi::win32 {
    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_CONST
    constexpr auto extract_mouse_button(UINT message, WPARAM w_param, LPARAM) noexcept
      -> MouseButton {
        switch (message) {
            case WM_LBUTTONDOWN: [[fallthrough]];
            case WM_LBUTTONUP: return MouseButton::LEFT;
            case WM_RBUTTONDOWN: [[fallthrough]];
            case WM_RBUTTONUP: return MouseButton::RIGHT;
            case WM_MBUTTONDOWN: [[fallthrough]];
            case WM_MBUTTONUP: return MouseButton::MIDDLE;
            case WM_XBUTTONDOWN: [[fallthrough]];
            case WM_XBUTTONUP: {
                const auto button = GET_XBUTTON_WPARAM(w_param);
                if (button == XBUTTON1) return MouseButton::BUTTON_1;
                else if (button == XBUTTON2)
                    return MouseButton::BUTTON_2;
            } break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    constexpr auto extract_mouse_position(HWND handle,
                                          WPARAM,
                                          LPARAM l_param,
                                          bool   to_client) noexcept -> math::ivec2 {
        auto position = POINT { GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param) };
        if (to_client) ScreenToClient(handle, &position);

        return { as<i32>(position.x), as<i32>(position.y) };
    }
} // namespace stormkit::wsi::win32
