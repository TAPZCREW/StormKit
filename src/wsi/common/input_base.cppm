// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.wsi:common.input_base;

import std;

import stormkit.core;
import stormkit.wsi;
import stormkit.math.linear;

import :mouse;
import :keyboard;

export namespace stormkit::wsi::common {
    constexpr auto as_index(mouse_button button) noexcept -> usize;
    constexpr auto as_index(key key) noexcept -> usize;

    enum class key_state : u8 {
        up = 0,
        down,
    };

    struct keyboard_state {
        u8   id;
        bool key_repeat = false;

        array<key_state, 102> keys = filled_with<102>(key_state::up);
    };

    enum class button_state : u8 {
        up = 0,
        down,
    };

    struct mouse_state {
        u8   id;
        bool hidden   = false;
        bool locked   = false;
        bool relative = false;
        bool confined = false;

        math::uvec2 locked_at     = {};
        math::uvec2 last_position = {};

        array<button_state, 15> buttons = filled_with<15>(button_state::up);
    };
} // namespace stormkit::wsi::common

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::wsi::common {
    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto as_index(mouse_button button) noexcept -> usize {
        switch (button) {
            case mouse_button::left: return 0;
            case mouse_button::right: return 1;
            case mouse_button::middle: return 2;
            case mouse_button::button_1: return 3;
            case mouse_button::button_2: return 4;
            case mouse_button::button_3: return 5;
            case mouse_button::button_4: return 6;
            case mouse_button::button_5: return 7;
            case mouse_button::button_6: return 8;
            case mouse_button::button_7: return 9;
            case mouse_button::button_8: return 10;
            case mouse_button::button_9: return 11;
            case mouse_button::button_10: return 12;
            case mouse_button::button_11: return 13;
            case mouse_button::button_12: return 14;
            default: break;
        }

        std::unreachable();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto as_index(key k) noexcept -> usize {
        EXPECTS(k != key::unknown);

        return as<usize>(k);
    }
} // namespace stormkit::wsi::common
