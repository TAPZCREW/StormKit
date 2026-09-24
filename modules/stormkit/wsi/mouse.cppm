// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.wsi:mouse;

import std;

import stormkit.core;

export namespace stormkit::wsi {
    inline constexpr auto GLOBAL_MOUSE_ID = 0_u8;

    enum class MouseButton : u8 {
        LEFT = 0,
        RIGHT,
        MIDDLE,
        BUTTON_1,
        BUTTON_2,
        BUTTON_3,
        BUTTON_4,
        BUTTON_5,
        BUTTON_6,
        BUTTON_7,
        BUTTON_8,
        BUTTON_9,
        BUTTON_10,
        BUTTON_11,
        BUTTON_12,
    };

    constexpr auto as_string(MouseButton button) noexcept -> string_view;
    constexpr auto to_string(MouseButton button) noexcept -> string;
} // namespace stormkit::wsi

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::wsi {
    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto as_string(MouseButton button) noexcept -> string_view {
        switch (button) {
            case MouseButton::LEFT: return "MouseButton::LEFT";
            case MouseButton::RIGHT: return "MouseButton::RIGHT";
            case MouseButton::MIDDLE: return "MouseButton::MIDDLE";
            case MouseButton::BUTTON_1: return "MouseButton::BUTTON_1";
            case MouseButton::BUTTON_2: return "MouseButton::BUTTON_2";
            case MouseButton::BUTTON_3: return "MouseButton::BUTTON_3";
            case MouseButton::BUTTON_4: return "MouseButton::BUTTON_4";
            case MouseButton::BUTTON_5: return "MouseButton::BUTTON_5";
            case MouseButton::BUTTON_6: return "MouseButton::BUTTON_6";
            case MouseButton::BUTTON_7: return "MouseButton::BUTTON_7";
            case MouseButton::BUTTON_8: return "MouseButton::BUTTON_8";
            case MouseButton::BUTTON_9: return "MouseButton::BUTTON_9";
            case MouseButton::BUTTON_10: return "MouseButton::BUTTON_10";
            case MouseButton::BUTTON_11: return "MouseButton::BUTTON_11";
            case MouseButton::BUTTON_12: return "MouseButton::BUTTON_12";
            default: break;
        }

        std::unreachable();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto to_string(MouseButton button) noexcept -> string {
        return string { as_string(button) };
    }
} // namespace stormkit::wsi
