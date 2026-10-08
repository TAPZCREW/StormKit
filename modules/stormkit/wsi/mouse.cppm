// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.wsi:mouse;

import std;

import stormkit.core;

using namespace stormkit::literals;

export namespace stormkit::wsi {
    inline constexpr auto global_mouse_id = 0_u8;

    enum class mouse_button : u8 {
        left = 0,
        right,
        middle,
        button_1,
        button_2,
        button_3,
        button_4,
        button_5,
        button_6,
        button_7,
        button_8,
        button_9,
        button_10,
        button_11,
        button_12,
    };

    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<string_view>, mouse_button button, const std::source_location&) noexcept -> string_view;
} // namespace stormkit::wsi

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::wsi {
    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto tag_invoke(as_fn<string_view>, mouse_button button, const std::source_location&) noexcept -> string_view {
        switch (button) {
            case mouse_button::left: return "mouse_button::left";
            case mouse_button::right: return "mouse_button::right";
            case mouse_button::middle: return "mouse_button::middle";
            case mouse_button::button_1: return "mouse_button::button_1";
            case mouse_button::button_2: return "mouse_button::button_2";
            case mouse_button::button_3: return "mouse_button::button_3";
            case mouse_button::button_4: return "mouse_button::button_4";
            case mouse_button::button_5: return "mouse_button::button_5";
            case mouse_button::button_6: return "mouse_button::button_6";
            case mouse_button::button_7: return "mouse_button::button_7";
            case mouse_button::button_8: return "mouse_button::button_8";
            case mouse_button::button_9: return "mouse_button::button_9";
            case mouse_button::button_10: return "mouse_button::button_10";
            case mouse_button::button_11: return "mouse_button::button_11";
            case mouse_button::button_12: return "mouse_button::button_12";
            default: break;
        }

        std::unreachable();
    }
} // namespace stormkit::wsi
