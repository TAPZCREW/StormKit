// Copyright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <sys/mman.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include <xkbcommon/xkbcommon.h>

#include <wayland-client.h>

#include <linux/input-event-codes.h>
#include <pointer-constraints-unstable-v1.h>
#include <relative-pointer-unstable-v1.h>

module stormkit.wsi;

import std;

import stormkit.core;

import :linux.common.xkb;
import :linux.wayland;
import :linux.wayland.context;
import :linux.wayland.input;
import :linux.wayland.window_impl;
import :linux.wayland.log;

namespace stormkit::wsi::linux::wayland::wl {
    auto keyboard_enter_handler(void*, wl_keyboard*, u32, wl_surface*, wl_array*) noexcept -> void;
    auto keyboard_leave_handler(void*, wl_keyboard*, u32, wl_surface*) noexcept -> void;
    auto keyboard_keymap_handler(void*, wl_keyboard*, u32, i32, u32) noexcept -> void;
    auto keyboard_key_handler(void*, wl_keyboard*, u32, u32, u32, u32) noexcept -> void;
    auto keyboard_modifiers_handler(void*, wl_keyboard*, u32, u32, u32, u32, u32) noexcept -> void;
    auto keyboard_repeat_info_handler(void*, wl_keyboard*, i32, i32) noexcept -> void;
    auto update_keymap(KeyboardState&, std::string_view) noexcept -> void;

    auto pointer_enter_handler(void*,
                               wl_pointer*,
                               u32,
                               wl_surface*,
                               wl_fixed_t,
                               wl_fixed_t) noexcept -> void;
    auto pointer_leave_handler(void*, wl_pointer*, u32, wl_surface*) noexcept -> void;
    auto pointer_motion_handler(void*, wl_pointer*, u32, wl_fixed_t, wl_fixed_t) noexcept -> void;
    auto pointer_button_handler(void*, wl_pointer*, u32, u32, u32, u32) noexcept -> void;
    auto pointer_axis_handler(void*, wl_pointer*, u32, u32, wl_fixed_t) noexcept -> void;
    auto pointer_frame_handler(void*, wl_pointer*) noexcept -> void;
    auto pointer_axis_source_handler(void*, wl_pointer*, u32) noexcept -> void;
    auto pointer_axis_stop_handler(void*, wl_pointer*, u32, u32) noexcept -> void;
    auto pointer_axis_discrete_handler(void*, wl_pointer*, u32, i32) noexcept -> void;

    namespace {
        constexpr auto g_keyboard_listener = wl_keyboard_listener {
            .keymap      = keyboard_keymap_handler,
            .enter       = keyboard_enter_handler,
            .leave       = keyboard_leave_handler,
            .key         = keyboard_key_handler,
            .modifiers   = keyboard_modifiers_handler,
            .repeat_info = keyboard_repeat_info_handler,
        };

        constexpr auto g_pointer_listener = wl_pointer_listener {
            .enter                   = pointer_enter_handler,
            .leave                   = pointer_leave_handler,
            .motion                  = pointer_motion_handler,
            .button                  = pointer_button_handler,
            .axis                    = pointer_axis_handler,
            .frame                   = pointer_frame_handler,
            .axis_source             = pointer_axis_source_handler,
            .axis_stop               = pointer_axis_stop_handler,
            .axis_discrete           = pointer_axis_discrete_handler,
            .axis_value120           = nullptr,
            .axis_relative_direction = nullptr,
        };
    } // namespace

    /////////////////////////////////////
    /////////////////////////////////////
    auto seat_capabilities_handler(void* data, wl_seat* seat, u32 capabilities) noexcept -> void {
        auto& globals       = *std::bit_cast<Globals*>(data);
        auto  _capabilities = narrow<wl_seat_capability>(capabilities);
        if (check_flag_bit(_capabilities, WL_SEAT_CAPABILITY_KEYBOARD)) {
            auto& [keyboard,
                   state] = globals.keyboards
                              .emplace_back(wl::Keyboard { std::in_place, seat }, KeyboardState {});
            wl_keyboard_add_listener(keyboard, &g_keyboard_listener, &state);

            state.repeat.timer_fd = common::FD {
                timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK)
            };
        }
        if (check_flag_bit(_capabilities, WL_SEAT_CAPABILITY_POINTER)) {
            auto& [pointer,
                   state] = globals.pointers
                              .emplace_back(wl::Pointer { std::in_place, seat }, PointerState {});
            wl_pointer_add_listener(pointer, &g_pointer_listener, &state);
            state.cursor.surface = wl::Surface { std::in_place, globals.compositor };
            if (globals.cursor_shape_manager)
                state.cursor.shape_device = wl::CursorShapeDevice { std::in_place,
                                                                    globals.cursor_shape_manager,
                                                                    pointer };
        }
        if (check_flag_bit(_capabilities, WL_SEAT_CAPABILITY_TOUCH)) {
            auto& _ = globals.touchs.emplace_back(wl::Touch { std::in_place, seat }, TouchState {});
            // wl_touch_add_listener(touch, &g_touch_listener, &globals);
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto seat_name_handler(void*, wl_seat*, const char* name) noexcept -> void {
        dlog("Seat {}", name);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboard_enter_handler(void* data,
                                wl_keyboard*,
                                u32,
                                wl_surface* surface,
                                wl_array*) noexcept -> void {
        if (data == nullptr) return;
        auto& globals = get_globals();

        auto& state = *std::bit_cast<KeyboardState*>(data);
        for (auto&& [_surface, window] : globals.windows) {
            if (_surface == surface) {
                state.focused_window = window;
                break;
            }
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboard_leave_handler(void* data, wl_keyboard*, u32, wl_surface*) noexcept -> void {
        if (data == nullptr) return;
        auto& state          = *std::bit_cast<KeyboardState*>(data);
        state.focused_window = nullptr;

        const auto timer = zeroed<itimerspec>();
        timerfd_settime(state.repeat.timer_fd, 0, &timer, nullptr);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboard_keymap_handler(void* data, wl_keyboard*, u32 format, i32 fd, u32 size) noexcept
      -> void {
        if (data == nullptr) return;
        auto& globals = get_globals();
        if (not globals.xkb_context)
            globals.xkb_context = common::xkb::Context { std::in_place, XKB_CONTEXT_NO_FLAGS };

        auto& state = *std::bit_cast<KeyboardState*>(data);
        if (format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
            auto map_shm = std::bit_cast<char*>(mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0));

            update_keymap(state, std::string_view { map_shm, size });

            munmap(map_shm, size);
            ::close(fd);
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboard_key_handler(void* data, wl_keyboard*, u32, u32, u32 key, u32 kstate) noexcept
      -> void {
        if (data == nullptr) return;
        auto& state = *std::bit_cast<KeyboardState*>(data);
        if (not state.focused_window or not state.xkb_state) return;

        auto character = char {};

        const auto keycode = key + 8;

        const auto symbol = xkb_state_key_get_one_sym(state.xkb_state, keycode);
        xkb_state_key_get_utf8(state.xkb_state, keycode, &character, sizeof(char));

        const auto skey = common::xkb_key_to_stormkit(symbol);

        const auto down = kstate == WL_KEYBOARD_KEY_STATE_PRESSED;

        auto timer = zeroed<itimerspec>();
        if (state.repeat.enabled and down) {
            if (xkb_keymap_key_repeats(state.xkb_keymap, key) and state.repeat.rate > 0) {
                state.repeat.c   = character;
                state.repeat.key = skey;

                if (state.repeat.rate > 1)
                    timer.it_interval.tv_nsec = 1'000'000'000 / state.repeat.rate;
                else
                    timer.it_interval.tv_sec = 1;

                timer.it_value.tv_sec  = state.repeat.delay / 1000;
                timer.it_value.tv_nsec = (state.repeat.delay % 1000) * 1000000;
            }
        }

        timerfd_settime(state.repeat.timer_fd, 0, &timer, nullptr);

        state.focused_window->handle_keyboard_key(skey, character, down);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboard_modifiers_handler(void* data,
                                    wl_keyboard*,
                                    u32,
                                    u32 mods_depressed,
                                    u32 mods_latched,
                                    u32 mods_locked,
                                    u32 group) noexcept -> void {
        if (data == nullptr) return;

        auto& state = *std::bit_cast<KeyboardState*>(data);
        if (not state.xkb_state) return;

        xkb_state_update_mask(state.xkb_state,
                              mods_depressed,
                              mods_latched,
                              mods_locked,
                              0,
                              0,
                              group);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboard_repeat_info_handler(void* data, wl_keyboard*, i32 rate, i32 delay) noexcept
      -> void {
        if (data == nullptr) return;

        auto& state = *std::bit_cast<KeyboardState*>(data);

        state.repeat.delay = delay;
        state.repeat.rate  = rate;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto update_keymap(KeyboardState& state, std::string_view keymap) noexcept -> void {
        auto& globals    = get_globals();
        state.xkb_keymap = common::xkb::Keymap {
            std::in_place,
            globals.xkb_context,
            std::data(keymap),
            XKB_KEYMAP_FORMAT_TEXT_V1,
            XKB_KEYMAP_COMPILE_NO_FLAGS
        };

        if (not state.xkb_keymap) {
            elog("Failed to compile a keymap");
            return;
        }

        state.xkb_state = common::xkb::State { std::in_place, state.xkb_keymap };

        if (not state.xkb_state) {
            elog("Failed to create XKB state");
            return;
        }

        state.xkb_mods = common::xkb::Mods {
            .shift   = xkb_keymap_mod_get_index(state.xkb_keymap, XKB_MOD_NAME_SHIFT),
            .lock    = xkb_keymap_mod_get_index(state.xkb_keymap, XKB_MOD_NAME_CAPS),
            .control = xkb_keymap_mod_get_index(state.xkb_keymap, XKB_MOD_NAME_CTRL),
            .mod1    = xkb_keymap_mod_get_index(state.xkb_keymap, "Mod1"),
            .mod2    = xkb_keymap_mod_get_index(state.xkb_keymap, "Mod2"),
            .mod3    = xkb_keymap_mod_get_index(state.xkb_keymap, "Mod3"),
            .mod4    = xkb_keymap_mod_get_index(state.xkb_keymap, "Mod4"),
            .mod5    = xkb_keymap_mod_get_index(state.xkb_keymap, "Mod5")
        };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_enter_handler(void*       data,
                               wl_pointer* pointer,
                               u32         serial,
                               wl_surface* surface,
                               wl_fixed_t  surface_x,
                               wl_fixed_t  surface_y) noexcept -> void {
        if (data == nullptr) return;
        auto& globals = get_globals();

        auto& state = *std::bit_cast<PointerState*>(data);
        for (auto&& [_surface, window] : globals.windows) {
            if (_surface == surface) {
                state.focused_window = window;
                break;
            }
        }

        if (not state.focused_window) return;

        state.serial = serial;
        state.focused_window->handle_pointer_enter(pointer, state);
        state.focused_window->handle_pointer_motion(surface_x, surface_y);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_leave_handler(void* data, wl_pointer*, u32, wl_surface*) noexcept -> void {
        if (data == nullptr) return;

        auto& state = *std::bit_cast<PointerState*>(data);
        if (not state.focused_window) return;

        state.serial = std::nullopt;
        state.focused_window->handle_pointer_leave();
        state.focused_window = nullptr;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_motion_handler(void* data,
                                wl_pointer*,
                                u32,
                                wl_fixed_t surface_x,
                                wl_fixed_t surface_y) noexcept -> void {
        if (data == nullptr) return;

        auto& state = *std::bit_cast<PointerState*>(data);
        if (not state.focused_window
            or (state.relative_pointer
                and check_flag_bit(state.flags, PointerState::Flag::RELATIVE)))
            return;

        state.x = surface_x;
        state.y = surface_y;

        state.focused_window->handle_pointer_motion(surface_x, surface_y);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_button_handler(void* data, wl_pointer*, u32, u32, u32 button, u32 sstate) noexcept
      -> void {
        if (data == nullptr) return;

        auto& state = *std::bit_cast<PointerState*>(data);
        if (not state.focused_window) return;

        state.focused_window->handle_pointer_button(button, sstate, state.x, state.y);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_axis_handler(void*, wl_pointer*, u32, u32, wl_fixed_t) noexcept -> void {
        // nothing
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_frame_handler(void*, wl_pointer*) noexcept -> void {
        // nothing
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_axis_source_handler(void*, wl_pointer*, u32) noexcept -> void {
        // nothing
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_axis_stop_handler(void*, wl_pointer*, u32, u32) noexcept -> void {
        // nothing
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_axis_discrete_handler(void*, wl_pointer*, u32, i32) noexcept -> void {
        // nothing
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_contraints_locked_handler(void*, zwp_locked_pointer_v1*) -> void {
        // nothing
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_contraints_unlocked_handler(void*, zwp_locked_pointer_v1*) -> void {
        // nothing
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_contraints_confined_handler(void*, zwp_confined_pointer_v1*) -> void {
        // nothing
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_contraints_unconfined_handler(void*, zwp_confined_pointer_v1*) -> void {
        // nothing
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto relative_pointer_relative_motion_handler(void* data,
                                                  zwp_relative_pointer_v1*,
                                                  u32,
                                                  u32,
                                                  wl_fixed_t surface_x,
                                                  wl_fixed_t surface_y,
                                                  wl_fixed_t,
                                                  wl_fixed_t) noexcept -> void {
        if (data == nullptr) return;

        auto& state = *std::bit_cast<PointerState*>(data);
        if (not state.focused_window) return;

        state.x = surface_x;
        state.y = surface_y;

        state.focused_window->handle_pointer_motion(surface_x, surface_y);
    }
} // namespace stormkit::wsi::linux::wayland::wl
