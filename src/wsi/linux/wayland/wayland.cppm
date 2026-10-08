// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <viewporter.h>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <xdg-shell.h>

#include <content-type-v1.h>
#include <cursor-shape-v1.h>
#include <pointer-constraints-unstable-v1.h>
#include <pointer-warp-v1.h>
#include <relative-pointer-unstable-v1.h>
#include <single-pixel-buffer-v1.h>
#include <xdg-decoration-unstable-v1.h>

export module stormkit.wsi:linux.wayland;

import std;

import stormkit.core;

import stormkit.wsi;

export namespace stormkit::wsi::linux::wayland::wl {
    using display = stormkit::raii_capsule<wl_display*, wl_display_connect, wl_display_disconnect, struct display_tag, nullptr>;

    using registry = stormkit::
      raii_capsule<wl_registry*, wl_display_get_registry, wl_registry_destroy, struct registry_tag, nullptr>;

    using compositor = stormkit::
      raii_capsule<wl_compositor*, monadic::noop(), wl_compositor_destroy, struct compositor_tag, nullptr>;

    using output = stormkit::raii_capsule<wl_output*, monadic::noop(), wl_output_release, struct output_tag, nullptr>;

    using xdg_wm_base = stormkit::
      raii_capsule<xdg_wm_base*, monadic::noop(), xdg_wm_base_destroy, struct xdg_wm_base_tag, nullptr>;

    using xdg_decoration_manager = stormkit::raii_capsule<
      zxdg_decoration_manager_v1*,
      monadic::noop(),
      zxdg_decoration_manager_v1_destroy,
      struct xdg_decoration_manager_tag,
      nullptr>;

    using Buffer = stormkit::raii_capsule<wl_buffer*, monadic::noop(), wl_buffer_destroy, struct Buffer_tag, nullptr>;

    using keyboard = stormkit::
      raii_capsule<wl_keyboard*, wl_seat_get_keyboard, wl_keyboard_release, struct keyboard_tag, nullptr>;

    using pointer = stormkit::raii_capsule<wl_pointer*, wl_seat_get_pointer, wl_pointer_release, struct pointer_tag, nullptr>;

    using touch = stormkit::raii_capsule<wl_touch*, wl_seat_get_touch, wl_touch_release, struct touch_tag, nullptr>;

    using shm = stormkit::raii_capsule<wl_shm*, monadic::noop(), wl_shm_release, struct shm_tag, nullptr>;

    using seat = stormkit::raii_capsule<wl_seat*, monadic::noop(), wl_seat_release, struct seat_tag, nullptr>;

    using single_pixel_buffer_manager = stormkit::raii_capsule<
      wp_single_pixel_buffer_manager_v1*,
      monadic::noop(),
      wp_single_pixel_buffer_manager_v1_destroy,
      struct single_pixel_buffer_manager_tag,
      nullptr>;

    using viewporter = stormkit::
      raii_capsule<wp_viewporter*, monadic::noop(), wp_viewporter_destroy, struct viewporter_tag, nullptr>;

    using content_type_manager = stormkit::raii_capsule<
      wp_content_type_manager_v1*,
      monadic::noop(),
      wp_content_type_manager_v1_destroy,
      struct content_type_manager_tag,
      nullptr>;

    using shm_pool = stormkit::raii_capsule<wl_shm_pool*, wl_shm_create_pool, wl_shm_pool_destroy, struct shm_pool_tag, nullptr>;

    using cursor_theme = stormkit::
      raii_capsule<wl_cursor_theme*, wl_cursor_theme_load, wl_cursor_theme_destroy, struct cursor_theme_tag, nullptr>;

    using cursor_shape_manager = stormkit::raii_capsule<
      wp_cursor_shape_manager_v1*,
      monadic::noop(),
      wp_cursor_shape_manager_v1_destroy,
      struct cursor_shape_manager_tag,
      nullptr>;

    using cursor_shape_device = stormkit::raii_capsule<
      wp_cursor_shape_device_v1*,
      wp_cursor_shape_manager_v1_get_pointer,
      wp_cursor_shape_device_v1_destroy,
      struct cursor_shape_device_tag,
      nullptr>;

    using pointer_constraints = stormkit::raii_capsule<
      zwp_pointer_constraints_v1*,
      monadic::noop(),
      zwp_pointer_constraints_v1_destroy,
      struct pointer_constraints_tag,
      nullptr>;

    using pointer_warp = stormkit::
      raii_capsule<wp_pointer_warp_v1*, monadic::noop(), wp_pointer_warp_v1_destroy, struct pointer_warp_tag, nullptr>;

    using relative_pointer_manager = stormkit::raii_capsule<
      zwp_relative_pointer_manager_v1*,
      monadic::noop(),
      zwp_relative_pointer_manager_v1_destroy,
      struct relative_pointer_manager_tag,
      nullptr>;

    using surface = stormkit::
      raii_capsule<wl_surface*, wl_compositor_create_surface, wl_surface_destroy, struct surface_tag, nullptr>;

    using xdg_surface = stormkit::
      raii_capsule<xdg_surface*, xdg_wm_base_get_xdg_surface, xdg_surface_destroy, struct xdg_surface_tag, nullptr>;

    using xdg_top_level = stormkit::
      raii_capsule<xdg_toplevel*, xdg_surface_get_toplevel, xdg_toplevel_destroy, struct xdg_top_level_tag, nullptr>;

    using xdg_top_level_decoration = stormkit::raii_capsule<
      zxdg_toplevel_decoration_v1*,
      zxdg_decoration_manager_v1_get_toplevel_decoration,
      zxdg_toplevel_decoration_v1_destroy,
      struct xdg_top_level_decoration_tag,
      nullptr>;

    using locked_pointer = stormkit::raii_capsule<zwp_locked_pointer_v1*,
                                                  zwp_pointer_constraints_v1_lock_pointer,
                                                  zwp_locked_pointer_v1_destroy,
                                                  struct locked_pointer_tag,
                                                  nullptr>;

    using confined_pointer = stormkit::raii_capsule<zwp_confined_pointer_v1*,
                                                    zwp_pointer_constraints_v1_confine_pointer,
                                                    zwp_confined_pointer_v1_destroy,
                                                    struct confined_pointer_tag,
                                                    nullptr>;

    using relative_pointer = stormkit::raii_capsule<zwp_relative_pointer_v1*,
                                                    zwp_relative_pointer_manager_v1_get_relative_pointer,
                                                    zwp_relative_pointer_v1_destroy,
                                                    struct relative_pointer_tag,
                                                    nullptr>;

    using viewport = stormkit::
      raii_capsule<wp_viewport*, wp_viewporter_get_viewport, wp_viewport_destroy, struct viewport_tag, nullptr>;

    using content_type = stormkit::raii_capsule<wp_content_type_v1*,
                                                wp_content_type_manager_v1_get_surface_content_type,
                                                wp_content_type_v1_destroy,
                                                struct content_type_tag,
                                                nullptr>;
} // namespace stormkit::wsi::linux::wayland::wl
