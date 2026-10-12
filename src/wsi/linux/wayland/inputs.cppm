// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <xkbcommon/xkbcommon.h>

#include <wayland-client.h>

#include <pointer-constraints-unstable-v1.h>
#include <relative-pointer-unstable-v1.h>

export module stormkit.wsi:linux.wayland.input;

import std;

import stormkit.core;

import :linux.wayland;
import :linux.common.fd;
import :linux.common.xkb;

export namespace stormkit::wsi::linux::wayland {
    class window;

    namespace wl {
        struct pointer_state {
            struct pointer_button_state {
                u32  button;
                bool down;
            };

            enum class flag : u8 {
                none     = 0,
                hidden   = 1,
                locked   = 2,
                confined = 4,
                relative = 4,
            } flags;

            std::optional<u32> serial = std::nullopt;

            array<pointer_button_state, 5> button_state;

            wl::confined_pointer confined_pointer = wl::confined_pointer::empty();
            wl::locked_pointer   locked_pointer   = wl::locked_pointer::empty();
            wl::relative_pointer relative_pointer = wl::relative_pointer::empty();

            struct {
                string name;

                wl::surface             surface      = wl::surface::empty();
                wl::cursor_shape_device shape_device = wl::cursor_shape_device::empty();
            } cursor;

            window* focused_window = nullptr;

            wl_fixed_t x;
            wl_fixed_t y;
        };

        struct keyboard_state {
            struct key_state {
                xkb_keysym_t key;
                bool         down;
            };

            std::optional<u32> serial = std::nullopt;

            struct {
                i32        rate;
                i32        delay;
                common::fd timer_fd = common::fd::empty();

                char c;
                wsi::key  key;

                bool enabled = false;
            } repeat;

            common::xkb::keymap xkb_keymap = common::xkb::keymap::empty();
            common::xkb::state  xkb_state  = common::xkb::state::empty();
            common::xkb::mods   xkb_mods;

            window* focused_window = nullptr;

            array<key_state, 102> keyboard_state = {
                key_state { XKB_KEY_a,            false },
                 key_state { XKB_KEY_b,            false },
                key_state { XKB_KEY_c,            false },
                 key_state { XKB_KEY_d,            false },
                key_state { XKB_KEY_e,            false },
                 key_state { XKB_KEY_f,            false },
                key_state { XKB_KEY_g,            false },
                 key_state { XKB_KEY_h,            false },
                key_state { XKB_KEY_i,            false },
                 key_state { XKB_KEY_j,            false },
                key_state { XKB_KEY_k,            false },
                 key_state { XKB_KEY_l,            false },
                key_state { XKB_KEY_m,            false },
                 key_state { XKB_KEY_n,            false },
                key_state { XKB_KEY_o,            false },
                 key_state { XKB_KEY_p,            false },
                key_state { XKB_KEY_q,            false },
                 key_state { XKB_KEY_r,            false },
                key_state { XKB_KEY_s,            false },
                 key_state { XKB_KEY_t,            false },
                key_state { XKB_KEY_u,            false },
                 key_state { XKB_KEY_v,            false },
                key_state { XKB_KEY_w,            false },
                 key_state { XKB_KEY_x,            false },
                key_state { XKB_KEY_y,            false },
                 key_state { XKB_KEY_z,            false },
                key_state { XKB_KEY_0,            false },
                 key_state { XKB_KEY_1,            false },
                key_state { XKB_KEY_2,            false },
                 key_state { XKB_KEY_3,            false },
                key_state { XKB_KEY_4,            false },
                 key_state { XKB_KEY_5,            false },
                key_state { XKB_KEY_6,            false },
                 key_state { XKB_KEY_7,            false },
                key_state { XKB_KEY_8,            false },
                 key_state { XKB_KEY_9,            false },
                key_state { XKB_KEY_Escape,       false },
                 key_state { XKB_KEY_Control_L,    false },
                key_state { XKB_KEY_Shift_L,      false },
                 key_state { XKB_KEY_Alt_L,        false },
                key_state { XKB_KEY_Super_L,      false },
                 key_state { XKB_KEY_Control_R,    false },
                key_state { XKB_KEY_Shift_R,      false },
                 key_state { XKB_KEY_Alt_R,        false },
                key_state { XKB_KEY_Super_R,      false },
                 key_state { XKB_KEY_Menu,         false },
                key_state { XKB_KEY_bracketleft,  false },
                 key_state { XKB_KEY_bracketright, false },
                key_state { XKB_KEY_semicolon,    false },
                 key_state { XKB_KEY_comma,        false },
                key_state { XKB_KEY_period,       false },
                 key_state { XKB_KEY_quoteleft,    false },
                key_state { XKB_KEY_slash,        false },
                 key_state { XKB_KEY_backslash,    false },
                key_state { XKB_KEY_dead_grave,   false },
                 key_state { XKB_KEY_equal,        false },
                key_state { XKB_KEY_hyphen,       false },
                 key_state { XKB_KEY_space,        false },
                key_state { XKB_KEY_Return,       false },
                 key_state { XKB_KEY_BackSpace,    false },
                key_state { XKB_KEY_Tab,          false },
                 key_state { XKB_KEY_Page_Up,      false },
                key_state { XKB_KEY_Page_Down,    false },
                 key_state { XKB_KEY_Begin,        false },
                key_state { XKB_KEY_End,          false },
                 key_state { XKB_KEY_Home,         false },
                key_state { XKB_KEY_Insert,       false },
                 key_state { XKB_KEY_Delete,       false },
                key_state { XKB_KEY_KP_Add,       false },
                 key_state { XKB_KEY_KP_Subtract,  false },
                key_state { XKB_KEY_KP_Multiply,  false },
                 key_state { XKB_KEY_KP_Divide,    false },
                key_state { XKB_KEY_Left,         false },
                 key_state { XKB_KEY_Right,        false },
                key_state { XKB_KEY_Up,           false },
                 key_state { XKB_KEY_Down,         false },
                key_state { XKB_KEY_KP_0,         false },
                 key_state { XKB_KEY_KP_1,         false },
                key_state { XKB_KEY_KP_2,         false },
                 key_state { XKB_KEY_KP_3,         false },
                key_state { XKB_KEY_KP_4,         false },
                 key_state { XKB_KEY_KP_5,         false },
                key_state { XKB_KEY_KP_6,         false },
                 key_state { XKB_KEY_KP_7,         false },
                key_state { XKB_KEY_KP_8,         false },
                 key_state { XKB_KEY_KP_9,         false },
                key_state { XKB_KEY_F1,           false },
                 key_state { XKB_KEY_F2,           false },
                key_state { XKB_KEY_F3,           false },
                 key_state { XKB_KEY_F4,           false },
                key_state { XKB_KEY_F5,           false },
                 key_state { XKB_KEY_F6,           false },
                key_state { XKB_KEY_F7,           false },
                 key_state { XKB_KEY_F8,           false },
                key_state { XKB_KEY_F9,           false },
                 key_state { XKB_KEY_F10,          false },
                key_state { XKB_KEY_F11,          false },
                 key_state { XKB_KEY_F12,          false },
                key_state { XKB_KEY_F13,          false },
                 key_state { XKB_KEY_F14,          false },
                key_state { XKB_KEY_F15,          false },
                 key_state { XKB_KEY_Pause,        false },
            };
        };

        struct touch_state {};

        auto seat_capabilities_handler(void*, wl_seat*, u32) noexcept -> void;
        auto seat_name_handler(void*, wl_seat*, const char*) noexcept -> void;

        auto pointer_contraints_locked_handler(void* data, zwp_locked_pointer_v1*) -> void;
        auto pointer_contraints_unlocked_handler(void* data, zwp_locked_pointer_v1*) -> void;

        auto pointer_contraints_confined_handler(void* data, zwp_confined_pointer_v1*) -> void;
        auto pointer_contraints_unconfined_handler(void* data, zwp_confined_pointer_v1*) -> void;

        auto relative_pointer_relative_motion_handler(void*,
                                                      zwp_relative_pointer_v1*,
                                                      u32,
                                                      u32,
                                                      wl_fixed_t,
                                                      wl_fixed_t,
                                                      wl_fixed_t,
                                                      wl_fixed_t) noexcept -> void;
    } // namespace wl
} // namespace stormkit::wsi::linux::wayland
