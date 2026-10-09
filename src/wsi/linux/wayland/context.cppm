
// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <wayland-client.h>
#include <xdg-shell.h>

#include <pointer-constraints-unstable-v1.h>
#include <relative-pointer-unstable-v1.h>
#include <xdg-decoration-unstable-v1.h>

export module stormkit.wsi:linux.wayland.context;

import std;

import stormkit.core;

import :linux.common.fd;
import :linux.common.xkb;
import :linux.wayland;
import :linux.wayland.input;

export namespace stormkit::wsi::linux::wayland {
    class window;

    namespace wl {
        struct wayland_monitor {
            uptr         id;
            wsi::monitor monitor;
        };

        struct wl_globals {
            bool                            initialized = false;
            wl::display                     display     = wl::display::empty();
            wl::registry                    registry    = wl::registry::empty();
            wl::compositor                  compositor  = wl::compositor::empty();
            dynarray<wl::output>            outputs;
            wl::xdg_wm_base                 xdg_wm_base                 = wl::xdg_wm_base::empty();
            wl::shm                         shm                         = wl::shm::empty();
            wl::xdg_decoration_manager      decoration_manager          = wl::xdg_decoration_manager::empty();
            wl::seat                        seat                        = wl::seat::empty();
            wl::single_pixel_buffer_manager single_pixel_buffer_manager = wl::single_pixel_buffer_manager::empty();
            wl::viewporter                  viewporter                  = wl::viewporter::empty();
            wl::cursor_shape_manager        cursor_shape_manager        = wl::cursor_shape_manager::empty();
            wl::cursor_shape_device         cursor_shape_device         = wl::cursor_shape_device::empty();
            wl::pointer_warp                pointer_warp                = wl::pointer_warp::empty();
            wl::pointer_constraints         pointer_constraints         = wl::pointer_constraints::empty();
            wl::content_type_manager        content_type_manager        = wl::content_type_manager::empty();

            wl::cursor_theme cursor_theme          = wl::cursor_theme::empty();
            wl::cursor_theme cursor_theme_high_dpi = wl::cursor_theme::empty();

            dynarray<std::pair<keyboard, keyboard_state>> keyboards;
            dynarray<std::pair<pointer, pointer_state>>   pointers;
            dynarray<std::pair<touch, touch_state>>       touchs;

            wl::relative_pointer_manager relative_pointer_manager = wl::relative_pointer_manager::empty();

            dynarray<wayland_monitor> monitors;

            dynarray<std::pair<wl_surface*, window*>> windows;

            common::xkb::context xkb_context = common::xkb::context::empty();
        };

        auto init() noexcept -> bool;
        auto get_globals() noexcept -> wl_globals&;
        auto get_monitor(wl_globals& globals, void* output) noexcept -> monitor&;
    } // namespace wl
} // namespace stormkit::wsi::linux::wayland

template<>
inline constexpr auto stormkit::core::meta::FLAG_TRAIT<stormkit::wsi::linux::wayland::wl::pointer_state::flag> = true;
