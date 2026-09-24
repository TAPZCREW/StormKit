// Copyright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <wayland-client-protocol.h>
#include <xdg-shell.h>

#include <xdg-decoration-unstable-v1.h>

#include <stormkit/core/memory_macro.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.wsi:linux.wayland.window;

import std;

import stormkit.core;
import stormkit.wsi;

import :common.window_base;

import :linux.wayland;
import :linux.wayland.context;

export {
    namespace stormkit::wsi::linux::wayland {
        class Window: public stormkit::wsi::common::WindowBase {
          public:
            struct Handles {
                wl_display* display;
                wl_surface* surface;
            };

            Window() noexcept;
            ~Window() noexcept;

            Window(const Window&) noexcept                    = delete;
            auto operator=(const Window&) noexcept -> Window& = delete;

            Window(Window&&) noexcept;
            auto operator=(Window&&) noexcept -> Window&;

            auto open(string title, const math::uextent2& size, WindowFlag flags) noexcept -> void;
            auto close() noexcept -> void;

            auto handle_events() noexcept -> void;

            auto clear(const ucolor_rgb& color) noexcept -> void;
            auto fill_framebuffer(array_view<const ucolor_rgb> colors) noexcept -> void;

            auto set_title(string title) noexcept -> void;
            auto set_extent(const math::uextent2& extent) noexcept -> void;
            auto set_fullscreen(bool fullscreen) noexcept -> void;

            auto confine_mouse(bool confined, u8 id) noexcept -> void;
            [[nodiscard]]
            auto is_mouse_confined(u8 id) const noexcept -> bool;

            auto lock_mouse(bool locked, u8 id) noexcept -> void;
            [[nodiscard]]
            auto is_mouse_locked(u8 id) const noexcept -> bool;

            auto hide_mouse(bool hidden, u8 id) noexcept -> void;
            [[nodiscard]]
            auto is_mouse_hidden(u8 id) const noexcept -> bool;

            auto set_relative_mouse(bool enabled, u8 id) noexcept -> void;
            [[nodiscard]]
            auto is_mouse_relative(u8 id) const noexcept -> bool;

            auto set_key_repeat(bool enabled, u8 id) noexcept -> void;
            [[nodiscard]]
            auto is_key_repeat_enabled(u8 id) const noexcept -> bool;

            auto show_virtual_keyboard(bool visible) noexcept -> void;
            [[nodiscard]]
            auto is_virtual_keyboard_visible() const noexcept -> bool;

            auto set_mouse_position(const math::ivec2& position, u8 id) noexcept -> void;

            [[nodiscard]]
            auto native_handle() const noexcept -> NativeHandle;

            auto handle_xdg_surface_configure(u32) noexcept -> void;
            auto handle_xdg_surface_close() noexcept -> void;

            auto handle_xdg_top_level_configure(u32, u32, array_view<const xdg_toplevel_state>) noexcept -> void;
            auto handle_surface_enter(wl_surface*, wl_output*) noexcept -> void;

            auto handle_keyboard_key(Key, char, bool) noexcept -> void;

            auto handle_pointer_enter(wl_pointer*, wl::PointerState&) noexcept -> void;
            auto handle_pointer_leave() noexcept -> void;
            auto handle_pointer_motion(wl_fixed_t, wl_fixed_t) noexcept -> void;
            auto handle_pointer_button(u32, u32, wl_fixed_t, wl_fixed_t) noexcept -> void;

          private:
            auto reallocate_pixel_buffer() noexcept -> void;

            auto hide_mouse(bool hidden, wl_pointer*, wl::PointerState&) noexcept -> void;
            auto set_cursor(string_view, wl_pointer*, wl::PointerState&) noexcept -> void;

            auto handle_key_repeat() noexcept -> void;

            u8 m_scale = 2u;

            bool       m_configured    = false;
            bool       m_scale_content = false;
            WindowFlag m_flags;
            wl_output* m_current_output = nullptr;

            string m_title;

            Handles m_handles;

            wl::Surface               m_surface                  = wl::Surface::empty();
            wl::XDGSurface            m_xdg_surface              = wl::XDGSurface::empty();
            wl::XDGTopLevel           m_xdg_top_level            = wl::XDGTopLevel::empty();
            wl::XDGTopLevelDecoration m_xdg_top_level_decoration = wl::XDGTopLevelDecoration::empty();
            wl::ContentType           m_content_type             = wl::ContentType::empty();
            wl::Viewport              m_viewport                 = wl::Viewport::empty();

            defer_init<shm_buffer> m_shm_buffer;
            wl::ShmPool            m_shm_pool     = wl::ShmPool::empty();
            wl::Buffer             m_pixel_buffer = wl::Buffer::empty();

            struct {
                bool restored   = false;
                bool activated  = false;
                bool suspended  = false;
                bool fullscreen = false;

                std::optional<math::uextent2> resizing;
            } m_pending_state;
        };
    } // namespace stormkit::wsi::linux::wayland
}

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::wsi::linux::wayland {
    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    inline auto Window::is_virtual_keyboard_visible() const noexcept -> bool {
        return false;
    }
} // namespace stormkit::wsi::linux::wayland
