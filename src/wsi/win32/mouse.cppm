// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.wsi:win32.mouse;

import std;

import stormkit.core;
import stormkit.core.win32;
import stormkit.math.linear;
import stormkit.wsi;

export namespace stormkit::wsi::win32 {
    constexpr auto extract_mouse_button(::win32::UINT message, ::win32::WPARAM w_param, ::win32::LPARAM l_param) noexcept
      -> mouse_button;
    constexpr auto extract_mouse_position(::win32::HWND   handle,
                                          ::win32::WPARAM w_param,
                                          ::win32::LPARAM l_param,
                                          bool            to_client = true) noexcept -> math::ivec2;
} // namespace stormkit::wsi::win32

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::wsi::win32 {
    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_CONST
    constexpr auto extract_mouse_button(::win32::UINT message, ::win32::WPARAM w_param, ::win32::LPARAM) noexcept -> mouse_button {
        switch (message) {
            case ::win32::WM_LBUTTONDOWN: [[fallthrough]];
            case ::win32::WM_LBUTTONUP: return mouse_button::left;
            case ::win32::WM_RBUTTONDOWN: [[fallthrough]];
            case ::win32::WM_RBUTTONUP: return mouse_button::right;
            case ::win32::WM_MBUTTONDOWN: [[fallthrough]];
            case ::win32::WM_MBUTTONUP: return mouse_button::middle;
            case ::win32::WM_XBUTTONDOWN: [[fallthrough]];
            case ::win32::WM_XBUTTONUP: {
                const auto button = ::win32::GetXButtonWPARAM(w_param);
                if (button == ::win32::XBUTTON1) return mouse_button::button_1;
                else if (button == ::win32::XBUTTON2)
                    return mouse_button::button_2;
            } break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    constexpr auto extract_mouse_position(::win32::HWND handle, ::win32::WPARAM, ::win32::LPARAM l_param, bool to_client) noexcept
      -> math::ivec2 {
        auto position = ::win32::POINT { ::win32::GetXLPARAM(l_param), ::win32::GetYLPARAM(l_param) };
        if (to_client) ::win32::ScreenToClient(handle, &position);

        return { as<i32>(position.x), as<i32>(position.y) };
    }
} // namespace stormkit::wsi::win32
