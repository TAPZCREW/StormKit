// Copyright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
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
    using Display  = stormkit::raii_capsule<wl_display*, wl_display_connect, wl_display_disconnect, struct DisplayTag, nullptr>;
    using Registry = stormkit::
      raii_capsule<wl_registry*, wl_display_get_registry, wl_registry_destroy, struct RegistryTag, nullptr>;
    using Compositor = stormkit::
      raii_capsule<wl_compositor*, monadic::noop(), wl_compositor_destroy, struct CompositorTag, nullptr>;
    using Output    = stormkit::raii_capsule<wl_output*, monadic::noop(), wl_output_release, struct OutputTag, nullptr>;
    using XDGWmBase = stormkit::raii_capsule<xdg_wm_base*, monadic::noop(), xdg_wm_base_destroy, struct XDGWmBaseTag, nullptr>;
    using XDGDecorationManager = stormkit::raii_capsule<
      zxdg_decoration_manager_v1*,
      monadic::noop(),
      zxdg_decoration_manager_v1_destroy,
      struct XDGDescorationManagerTag,
      nullptr>;
    using Buffer   = stormkit::raii_capsule<wl_buffer*, monadic::noop(), wl_buffer_destroy, struct BufferTag, nullptr>;
    using Keyboard = stormkit::raii_capsule<wl_keyboard*, wl_seat_get_keyboard, wl_keyboard_release, struct KeyboardTag, nullptr>;
    using Pointer  = stormkit::raii_capsule<wl_pointer*, wl_seat_get_pointer, wl_pointer_release, struct PointerTag, nullptr>;
    using Touch    = stormkit::raii_capsule<wl_touch*, wl_seat_get_touch, wl_touch_release, struct TouchTag, nullptr>;
    using Shm      = stormkit::raii_capsule<wl_shm*, monadic::noop(), wl_shm_release, struct ShmTag, nullptr>;
    using Seat     = stormkit::raii_capsule<wl_seat*, monadic::noop(), wl_seat_release, struct SeatTag, nullptr>;
    using SinglePixelBufferManager = stormkit::raii_capsule<
      wp_single_pixel_buffer_manager_v1*,
      monadic::noop(),
      wp_single_pixel_buffer_manager_v1_destroy,
      struct SinglePixelBufferManagerTag,
      nullptr>;
    using Viewporter = stormkit::
      raii_capsule<wp_viewporter*, monadic::noop(), wp_viewporter_destroy, struct ViewporterTag, nullptr>;
    using ContentTypeManager = stormkit::raii_capsule<wp_content_type_manager_v1*,
                                                     monadic::noop(),
                                                     wp_content_type_manager_v1_destroy,
                                                     struct ContentTypeManagerTag,
                                                     nullptr>;

    using ShmPool     = stormkit::raii_capsule<wl_shm_pool*, wl_shm_create_pool, wl_shm_pool_destroy, struct ShmPoolTag, nullptr>;
    using CursorTheme = stormkit::
      raii_capsule<wl_cursor_theme*, wl_cursor_theme_load, wl_cursor_theme_destroy, struct CursorThemeTag, nullptr>;
    using CursorShapeManager = stormkit::raii_capsule<wp_cursor_shape_manager_v1*,
                                                     monadic::noop(),
                                                     wp_cursor_shape_manager_v1_destroy,
                                                     struct CursorShapeManagerTag,
                                                     nullptr>;
    using CursorShapeDevice  = stormkit::raii_capsule<wp_cursor_shape_device_v1*,
                                                     wp_cursor_shape_manager_v1_get_pointer,
                                                     wp_cursor_shape_device_v1_destroy,
                                                     struct CursorShapeDeviceTag,
                                                     nullptr>;
    using PointerConstraints = stormkit::raii_capsule<zwp_pointer_constraints_v1*,
                                                     monadic::noop(),
                                                     zwp_pointer_constraints_v1_destroy,
                                                     struct PointerConstraintsTag,
                                                     nullptr>;
    using PointerWarp        = stormkit::
      raii_capsule<wp_pointer_warp_v1*, monadic::noop(), wp_pointer_warp_v1_destroy, struct PointerWarpTag, nullptr>;
    using RelativePointerManager = stormkit::raii_capsule<
      zwp_relative_pointer_manager_v1*,
      monadic::noop(),
      zwp_relative_pointer_manager_v1_destroy,
      struct RelativePointerManagerTag,
      nullptr>;

    using Surface = stormkit::
      raii_capsule<wl_surface*, wl_compositor_create_surface, wl_surface_destroy, struct SurfaceTag, nullptr>;
    using XDGSurface = stormkit::
      raii_capsule<xdg_surface*, xdg_wm_base_get_xdg_surface, xdg_surface_destroy, struct XDGSurfaceTag, nullptr>;
    using XDGTopLevel = stormkit::
      raii_capsule<xdg_toplevel*, xdg_surface_get_toplevel, xdg_toplevel_destroy, struct XDGTopLevelTag, nullptr>;
    using XDGTopLevelDecoration = stormkit::raii_capsule<
      zxdg_toplevel_decoration_v1*,
      zxdg_decoration_manager_v1_get_toplevel_decoration,
      zxdg_toplevel_decoration_v1_destroy,
      struct XDGTopLevelDecorationTag,
      nullptr>;
    using LockedPointer   = stormkit::raii_capsule<zwp_locked_pointer_v1*,
                                                  zwp_pointer_constraints_v1_lock_pointer,
                                                  zwp_locked_pointer_v1_destroy,
                                                  struct LockedPointerTag,
                                                  nullptr>;
    using ConfinedPointer = stormkit::raii_capsule<zwp_confined_pointer_v1*,
                                                  zwp_pointer_constraints_v1_confine_pointer,
                                                  zwp_confined_pointer_v1_destroy,
                                                  struct ConfinedPointerTag,
                                                  nullptr>;
    using RelativePointer = stormkit::raii_capsule<zwp_relative_pointer_v1*,
                                                  zwp_relative_pointer_manager_v1_get_relative_pointer,
                                                  zwp_relative_pointer_v1_destroy,
                                                  struct RelativePointerTag,
                                                  nullptr>;
    using Viewport        = stormkit::
      raii_capsule<wp_viewport*, wp_viewporter_get_viewport, wp_viewport_destroy, struct ViewportTag, nullptr>;
    using ContentType = stormkit::raii_capsule<wp_content_type_v1*,
                                              wp_content_type_manager_v1_get_surface_content_type,
                                              wp_content_type_v1_destroy,
                                              struct ContentTypeTag,
                                              nullptr>;
} // namespace stormkit::wsi::linux::wayland::wl
