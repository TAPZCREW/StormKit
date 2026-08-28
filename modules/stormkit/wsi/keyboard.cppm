// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.wsi:keyboard;

import std;

import stormkit.core;

export namespace stormkit::wsi {
    inline constexpr auto GLOBAL_KEYBOARD_ID = 0_u8;

    enum class Key : u8 {
        A = 0,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        NUM_0,
        NUM_1,
        NUM_2,
        NUM_3,
        NUM_4,
        NUM_5,
        NUM_6,
        NUM_7,
        NUM_8,
        NUM_9,

        LEFT,
        RIGHT,
        UP,
        DOWN,

        L_CONTROL,
        L_SHIFT,
        L_ALT,
        L_META,
        R_CONTROL,
        R_SHIFT,
        R_ALT,
        R_META,

        ESCAPE,
        TAB,
        MENU,

        QUOTE,
        BACK_SLASH,
        COMMA,
        EQUAL,

        GRAVE_ACCENT,
        L_BRACKET,
        MINUS,
        PERIOD,
        R_BRACKET,
        SEMI_COLON,
        SLASH,

        ISO,

        BACK_SPACE,
        CAPS_LOCK,
        ENTER,
        SPACE,

        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        F13,
        F14,
        F15,
        F16,
        F17,
        F18,
        F19,
        F20,

        PRINT_SCREEN,

        INSERT,
        DELETE,
        HOME,
        END,
        PAGE_UP,
        PAGE_DOWN,

        NUMPAD_LOCK,
        NUMPAD_ADD,
        NUMPAD_DECIMAL,
        NUMPAD_DIVIDE,
        NUMPAD_ENTER,
        NUMPAD_EQUAL,
        NUMPAD_MULTIPLY,
        NUMPAD_SUBTRACT,
        NUMPAD_0,
        NUMPAD_1,
        NUMPAD_2,
        NUMPAD_3,
        NUMPAD_4,
        NUMPAD_5,
        NUMPAD_6,
        NUMPAD_7,
        NUMPAD_8,
        NUMPAD_9,

        UNKNOWN = std::numeric_limits<u8>::max(),
    };
    constexpr auto as_string(Key key) noexcept -> string_view;
    constexpr auto to_string(Key key) noexcept -> string;
} // namespace stormkit::wsi

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::wsi {
    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto as_string(Key key) noexcept -> string_view {
        switch (key) {
            case Key::A: return "Key::A";
            case Key::B: return "Key::B";
            case Key::C: return "Key::C";
            case Key::D: return "Key::D";
            case Key::E: return "Key::E";
            case Key::F: return "Key::F";
            case Key::G: return "Key::G";
            case Key::H: return "Key::H";
            case Key::I: return "Key::I";
            case Key::J: return "Key::J";
            case Key::K: return "Key::K";
            case Key::L: return "Key::L";
            case Key::M: return "Key::M";
            case Key::N: return "Key::N";
            case Key::O: return "Key::O";
            case Key::P: return "Key::P";
            case Key::Q: return "Key::Q";
            case Key::R: return "Key::R";
            case Key::S: return "Key::S";
            case Key::T: return "Key::T";
            case Key::U: return "Key::U";
            case Key::V: return "Key::V";
            case Key::W: return "Key::W";
            case Key::X: return "Key::X";
            case Key::Y: return "Key::Y";
            case Key::Z: return "Key::Z";
            case Key::NUM_0: return "Key::NUM_0";
            case Key::NUM_1: return "Key::NUM_1";
            case Key::NUM_2: return "Key::NUM_2";
            case Key::NUM_3: return "Key::NUM_3";
            case Key::NUM_4: return "Key::NUM_4";
            case Key::NUM_5: return "Key::NUM_5";
            case Key::NUM_6: return "Key::NUM_6";
            case Key::NUM_7: return "Key::NUM_7";
            case Key::NUM_8: return "Key::NUM_8";
            case Key::NUM_9: return "Key::NUM_9";

            case Key::LEFT: return "Key::LEFT";
            case Key::RIGHT: return "Key::RIGHT";
            case Key::UP: return "Key::UP";
            case Key::DOWN: return "Key::DOWN";

            case Key::L_CONTROL: return "Key::L_CONTROL";
            case Key::L_SHIFT: return "Key::L_SHIFT";
            case Key::L_ALT: return "Key::L_ALT";
            case Key::L_META: return "Key::L_META";
            case Key::R_CONTROL: return "Key::R_CONTROL";
            case Key::R_SHIFT: return "Key::R_SHIFT";
            case Key::R_ALT: return "Key::R_ALT";
            case Key::R_META: return "Key::R_META";

            case Key::ESCAPE: return "Key::ESCAPE";
            case Key::TAB: return "Key::TAB";
            case Key::MENU: return "Key::MENU";

            case Key::QUOTE: return "Key::QUOTE";
            case Key::BACK_SLASH: return "Key::BACK_SLASH";
            case Key::COMMA: return "Key::COMMA";
            case Key::EQUAL: return "Key::EQUAL";

            case Key::GRAVE_ACCENT: return "Key::GRAVE_ACCENT";
            case Key::L_BRACKET: return "Key::L_BRACKET";
            case Key::MINUS: return "Key::MINUS";
            case Key::PERIOD: return "Key::PERIOD";
            case Key::R_BRACKET: return "Key::R_BRACKET";
            case Key::SEMI_COLON: return "Key::SEMI_COLON";
            case Key::SLASH: return "Key::SLASH";

            case Key::ISO: return "Key::ISO";

            case Key::BACK_SPACE: return "Key::BACK_SPACE";
            case Key::CAPS_LOCK: return "Key::CAPS_LOCK";
            case Key::ENTER: return "Key::ENTER";
            case Key::SPACE: return "Key::SPACE";

            case Key::F1: return "Key::F1";
            case Key::F2: return "Key::F2";
            case Key::F3: return "Key::F3";
            case Key::F4: return "Key::F4";
            case Key::F5: return "Key::F5";
            case Key::F6: return "Key::F6";
            case Key::F7: return "Key::F7";
            case Key::F8: return "Key::F8";
            case Key::F9: return "Key::F9";
            case Key::F10: return "Key::F10";
            case Key::F11: return "Key::F11";
            case Key::F12: return "Key::F12";
            case Key::F13: return "Key::F13";
            case Key::F14: return "Key::F14";
            case Key::F15: return "Key::F15";
            case Key::F16: return "Key::F16";
            case Key::F17: return "Key::F17";
            case Key::F18: return "Key::F18";
            case Key::F19: return "Key::F19";
            case Key::F20: return "Key::F20";

            case Key::PRINT_SCREEN: return "Key::PRINT_SCREEN";

            case Key::INSERT: return "Key::INSERT";
            case Key::DELETE: return "Key::DELETE";
            case Key::HOME: return "Key::HOME";
            case Key::END: return "Key::END";
            case Key::PAGE_UP: return "Key::PAGE_UP";
            case Key::PAGE_DOWN: return "Key::PAGE_DOWN";

            case Key::NUMPAD_LOCK: return "Key::NUMPAD_LOCK";
            case Key::NUMPAD_ADD: return "Key::NUMPAD_ADD";
            case Key::NUMPAD_DECIMAL: return "Key::NUMPAD_DECIMAL";
            case Key::NUMPAD_DIVIDE: return "Key::NUMPAD_DIVIDE";
            case Key::NUMPAD_ENTER: return "Key::NUMPAD_ENTER";
            case Key::NUMPAD_EQUAL: return "Key::NUMPAD_EQUAL";
            case Key::NUMPAD_MULTIPLY: return "Key::NUMPAD_MULTIPLY";
            case Key::NUMPAD_SUBTRACT: return "Key::NUMPAD_SUBTRACT";
            case Key::NUMPAD_0: return "Key::NUMPAD_0";
            case Key::NUMPAD_1: return "Key::NUMPAD_1";
            case Key::NUMPAD_2: return "Key::NUMPAD_2";
            case Key::NUMPAD_3: return "Key::NUMPAD_3";
            case Key::NUMPAD_4: return "Key::NUMPAD_4";
            case Key::NUMPAD_5: return "Key::NUMPAD_5";
            case Key::NUMPAD_6: return "Key::NUMPAD_6";
            case Key::NUMPAD_7: return "Key::NUMPAD_7";
            case Key::NUMPAD_8: return "Key::NUMPAD_8";
            case Key::NUMPAD_9: return "Key::NUMPAD_9";
            case Key::UNKNOWN: return "Key::UNKNOWN";
            default: break;
        }
        std::unreachable();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto to_string(Key key) noexcept -> string {
        return string { as_string(key) };
    }
} // namespace stormkit::wsi
