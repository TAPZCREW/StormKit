// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.wsi:common.input_base;

import std;

import stormkit.core;
import stormkit.wsi;

import :mouse;
import :keyboard;

export namespace stormkit::wsi::common {
    constexpr auto as_index(MouseButton button) noexcept -> usize;
    constexpr auto as_index(Key key) noexcept -> usize;

    enum class KeyState : u8 {
        UP = 0,
        DOWN,
    };

    struct KeyboardState {
        u8   id;
        bool key_repeat = false;

        array<KeyState, 102> keys = filled_with<102>(KeyState::UP);
    };

    enum class ButtonState : u8 {
        UP = 0,
        DOWN,
    };

    struct MouseState {
        u8   id;
        bool hidden   = false;
        bool locked   = false;
        bool relative = false;
        bool confined = false;

        math::uvec2 locked_at     = {};
        math::uvec2 last_position = {};

        array<ButtonState, 15> buttons = filled_with<15>(ButtonState::UP);
    };
} // namespace stormkit::wsi::common

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::wsi::common {
    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto as_index(MouseButton button) noexcept -> usize {
        switch (button) {
            case MouseButton::LEFT: return 0;
            case MouseButton::RIGHT: return 1;
            case MouseButton::MIDDLE: return 2;
            case MouseButton::BUTTON_1: return 3;
            case MouseButton::BUTTON_2: return 4;
            case MouseButton::BUTTON_3: return 5;
            case MouseButton::BUTTON_4: return 6;
            case MouseButton::BUTTON_5: return 7;
            case MouseButton::BUTTON_6: return 8;
            case MouseButton::BUTTON_7: return 9;
            case MouseButton::BUTTON_8: return 10;
            case MouseButton::BUTTON_9: return 11;
            case MouseButton::BUTTON_10: return 12;
            case MouseButton::BUTTON_11: return 13;
            case MouseButton::BUTTON_12: return 14;
            default: break;
        }

        std::unreachable();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto as_index(Key key) noexcept -> usize {
        EXPECTS(key != Key::UNKNOWN);

        return unchecked_narrow<usize>(key);
    }
} // namespace stormkit::wsi::common
