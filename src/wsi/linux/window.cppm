// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.wsi:linux.window;

import std;

import stormkit.core;
import stormkit.wsi;

import :common.window_base;
import :linux.x11.window;
import :linux.wayland.window;

export namespace stormkit::wsi::linux {
    class window {
        using backend_window = std::variant<x11::window, wayland::window, std::monostate>;

        window_manager m_wm;

        backend_window m_impl = std::monostate {};

      public:
        explicit window(window_manager wm) noexcept;
        ~window() noexcept;

        window(const window&) noexcept                    = delete;
        auto operator=(const window&) noexcept -> window& = delete;

        window(window&&) noexcept;
        auto operator=(window&&) noexcept -> window&;

        auto open(string title, const math::uextent2& size, window_flag flags) noexcept -> void;
        auto close() noexcept -> void;

        [[nodiscard]]
        auto is_open() const noexcept -> bool;

        [[nodiscard]]
        auto visible() const noexcept -> bool;

        [[nodiscard]]
        auto current_monitor() const noexcept -> const monitor&;

        auto handle_events() noexcept -> void;

        auto clear(const ucolor_rgb& color) noexcept -> void;
        auto fill_framebuffer(array_view<const ucolor_rgb> colors) noexcept -> void;

        auto set_title(string&& title) noexcept -> void;
        [[nodiscard]]
        auto title() const noexcept -> const string&;

        auto set_extent(const math::uextent2& extent) noexcept -> void;
        [[nodiscard]]
        auto extent() const noexcept -> const math::uextent2&;

        auto set_fullscreen(bool fullscreen) noexcept -> void;
        [[nodiscard]]
        auto fullscreen() const noexcept -> bool;

        auto confine_mouse(bool confined = true, u8 mouse_id = 0) noexcept -> void;
        [[nodiscard]]
        auto is_mouse_confined(u8 mouse_id) const noexcept -> bool;

        auto lock_mouse(bool locked = true, u8 mouse_id = 0) noexcept -> void;
        [[nodiscard]]
        auto is_mouse_locked(u8 mouse_id) const noexcept -> bool;

        auto hide_mouse(bool hidden = true, u8 mouse_id = 0) noexcept -> void;
        [[nodiscard]]
        auto is_mouse_hidden(u8 mouse_id) const noexcept -> bool;

        auto set_relative_mouse(bool enabled, u8 mouse_id = 0) noexcept -> void;
        [[nodiscard]]
        auto is_mouse_relative(u8 mouse_id = 0) const noexcept -> bool;

        auto set_key_repeat(bool enabled, u8 keyboard_id = 0) noexcept -> void;
        [[nodiscard]]
        auto is_key_repeat_enabled(u8 keyboard_id = 0) const noexcept -> bool;

        auto show_virtual_keyboard(bool visible = true) noexcept -> void;
        [[nodiscard]]
        auto is_virtual_keyboard_visible() const noexcept -> bool;

        auto set_mouse_position(const math::ivec2& position, u8 mouse_id = 0) noexcept -> void;

        [[nodiscard]]
        auto native_handle() const noexcept -> native_handle_type;

        auto set_closed_event(closed_event_cb_type&& func) noexcept -> void;
        auto set_monitor_changed_event(monitor_changed_event_cb_type&& func) noexcept -> void;
        auto set_resized_event(resized_event_cb_type&& func) noexcept -> void;
        auto set_restored_event(restored_event_cb_type&& func) noexcept -> void;
        auto set_minimized_event(minimized_event_cb_type&& func) noexcept -> void;
        auto set_key_down_event(key_down_event_cb_type&& func) noexcept -> void;
        auto set_key_up_event(key_up_event_cb_type&& func) noexcept -> void;
        auto set_mouse_button_down_event(mouse_button_down_event_cb_type&& func) noexcept -> void;
        auto set_mouse_button_up_event(mouse_button_up_event_cb_type&& func) noexcept -> void;
        auto set_mouse_moved_event(mouse_moved_event_cb_type&& func) noexcept -> void;
        auto set_deactivate_event(deactivate_event_cb_type&& func) noexcept -> void;
        auto set_activate_event(activate_event_cb_type&& func) noexcept -> void;
    };
} // namespace stormkit::wsi::linux

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::wsi::linux {
    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline window::window(window_manager wm) noexcept
        : m_wm { wm } {
        if (m_wm == window_manager::x11) m_impl = x11::window {};
        else if (m_wm == window_manager::wayland)
            m_impl = wayland::window {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline window::~window() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline window::window(window&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::operator=(window&&) noexcept -> window& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::open(string title, const math::uextent2& extent, window_flag flags) noexcept -> void {
        switch (m_wm) {
            case window_manager::x11: as<x11::window>(m_impl).open(std::move(title), extent, flags); break;
            case window_manager::wayland: as<wayland::window>(m_impl).open(std::move(title), extent, flags); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::close() noexcept -> void {
        switch (m_wm) {
            case window_manager::x11: as<x11::window>(m_impl).close(); break;
            case window_manager::wayland: as<wayland::window>(m_impl).close(); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto window::is_open() const noexcept -> bool {
        switch (m_wm) {
            case window_manager::x11: return as<x11::window>(m_impl).is_open();
            case window_manager::wayland: return as<wayland::window>(m_impl).is_open();

            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto window::visible() const noexcept -> bool {
        switch (m_wm) {
            case window_manager::x11: return as<x11::window>(m_impl).visible();
            case window_manager::wayland: return as<wayland::window>(m_impl).visible();

            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto window::current_monitor() const noexcept -> const monitor& {
        switch (m_wm) {
            case window_manager::x11: return as<x11::window>(m_impl).current_monitor();
            case window_manager::wayland: return as<wayland::window>(m_impl).current_monitor();

            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::handle_events() noexcept -> void {
        switch (m_wm) {
            case window_manager::x11: as<x11::window>(m_impl).handle_events(); break;
            case window_manager::wayland: as<wayland::window>(m_impl).handle_events(); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::clear(const ucolor_rgb& color) noexcept -> void {
        switch (m_wm) {
            case window_manager::x11: as<x11::window>(m_impl).clear(color); break;
            case window_manager::wayland: as<wayland::window>(m_impl).clear(color); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::fill_framebuffer(array_view<const ucolor_rgb> colors) noexcept -> void {
        switch (m_wm) {
            case window_manager::x11: as<x11::window>(m_impl).fill_framebuffer(colors); break;
            case window_manager::wayland: as<wayland::window>(m_impl).fill_framebuffer(colors); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::set_title(string&& title) noexcept -> void {
        switch (m_wm) {
            case window_manager::x11: as<x11::window>(m_impl).set_title(std::move(title)); break;
            case window_manager::wayland: as<wayland::window>(m_impl).set_title(std::move(title)); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto window::title() const noexcept -> const string& {
        switch (m_wm) {
            case window_manager::x11: return as<x11::window>(m_impl).title();
            case window_manager::wayland: return as<wayland::window>(m_impl).title();

            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::set_extent(const math::uextent2& extent) noexcept -> void {
        switch (m_wm) {
            case window_manager::x11: as<x11::window>(m_impl).set_extent(extent); break;
            case window_manager::wayland: as<wayland::window>(m_impl).set_extent(extent); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto window::extent() const noexcept -> const math::uextent2& {
        switch (m_wm) {
            case window_manager::x11: return as<x11::window>(m_impl).extent();
            case window_manager::wayland: return as<wayland::window>(m_impl).extent();

            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::set_fullscreen(bool enabled) noexcept -> void {
        switch (m_wm) {
            case window_manager::x11: as<x11::window>(m_impl).set_fullscreen(enabled); break;
            case window_manager::wayland: as<wayland::window>(m_impl).set_fullscreen(enabled); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto window::fullscreen() const noexcept -> bool {
        switch (m_wm) {
            case window_manager::x11: return as<x11::window>(m_impl).fullscreen();
            case window_manager::wayland: return as<wayland::window>(m_impl).fullscreen();

            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::confine_mouse(bool confined, u8 mouse_id) noexcept -> void {
        switch (m_wm) {
            case window_manager::x11: as<x11::window>(m_impl).confine_mouse(confined, mouse_id); break;
            case window_manager::wayland: as<wayland::window>(m_impl).confine_mouse(confined, mouse_id); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto window::is_mouse_confined(u8 mouse_id) const noexcept -> bool {
        switch (m_wm) {
            case window_manager::x11: return as<x11::window>(m_impl).is_mouse_confined(mouse_id);
            case window_manager::wayland: return as<wayland::window>(m_impl).is_mouse_confined(mouse_id);

            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::lock_mouse(bool locked, u8 mouse_id) noexcept -> void {
        switch (m_wm) {
            case window_manager::x11: as<x11::window>(m_impl).lock_mouse(locked, mouse_id); break;
            case window_manager::wayland: as<wayland::window>(m_impl).lock_mouse(locked, mouse_id); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto window::is_mouse_locked(u8 mouse_id) const noexcept -> bool {
        switch (m_wm) {
            case window_manager::x11: return as<x11::window>(m_impl).is_mouse_locked(mouse_id);
            case window_manager::wayland: return as<wayland::window>(m_impl).is_mouse_locked(mouse_id);

            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::hide_mouse(bool hidden, u8 mouse_id) noexcept -> void {
        switch (m_wm) {
            case window_manager::x11: as<x11::window>(m_impl).hide_mouse(hidden, mouse_id); break;
            case window_manager::wayland: as<wayland::window>(m_impl).hide_mouse(hidden, mouse_id); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto window::is_mouse_hidden(u8 mouse_id) const noexcept -> bool {
        switch (m_wm) {
            case window_manager::x11: return as<x11::window>(m_impl).is_mouse_hidden(mouse_id);
            case window_manager::wayland: return as<wayland::window>(m_impl).is_mouse_hidden(mouse_id);

            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::set_relative_mouse(bool enabled, u8 mouse_id) noexcept -> void {
        switch (m_wm) {
            case window_manager::x11: as<x11::window>(m_impl).set_relative_mouse(enabled, mouse_id); break;
            case window_manager::wayland: as<wayland::window>(m_impl).set_relative_mouse(enabled, mouse_id); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto window::is_mouse_relative(u8 mouse_id) const noexcept -> bool {
        switch (m_wm) {
            case window_manager::x11: return as<x11::window>(m_impl).is_mouse_relative(mouse_id);
            case window_manager::wayland: return as<wayland::window>(m_impl).is_mouse_relative(mouse_id);

            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::set_key_repeat(bool enabled, u8 keyboard_id) noexcept -> void {
        switch (m_wm) {
            case window_manager::x11: return as<x11::window>(m_impl).set_key_repeat(enabled, keyboard_id);
            case window_manager::wayland: return as<wayland::window>(m_impl).set_key_repeat(enabled, keyboard_id);

            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto window::is_key_repeat_enabled(u8 keyboard_id) const noexcept -> bool {
        switch (m_wm) {
            case window_manager::x11: return as<x11::window>(m_impl).is_key_repeat_enabled(keyboard_id);
            case window_manager::wayland: return as<wayland::window>(m_impl).is_key_repeat_enabled(keyboard_id);

            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::show_virtual_keyboard(bool visible) noexcept -> void {
        switch (m_wm) {
            case window_manager::x11: as<x11::window>(m_impl).show_virtual_keyboard(visible); break;
            case window_manager::wayland: as<wayland::window>(m_impl).show_virtual_keyboard(visible); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto window::is_virtual_keyboard_visible() const noexcept -> bool {
        switch (m_wm) {
            case window_manager::x11: return as<x11::window>(m_impl).is_virtual_keyboard_visible();
            case window_manager::wayland: return as<wayland::window>(m_impl).is_virtual_keyboard_visible();

            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::set_mouse_position(const math::ivec2& position, u8 mouse_id) noexcept -> void {
        switch (m_wm) {
            case window_manager::x11: as<x11::window>(m_impl).set_mouse_position(position, mouse_id); break;
            case window_manager::wayland: as<wayland::window>(m_impl).set_mouse_position(position, mouse_id); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto window::native_handle() const noexcept -> native_handle_type {
        switch (m_wm) {
            case window_manager::x11: return as<x11::window>(m_impl).native_handle();
            case window_manager::wayland: return as<wayland::window>(m_impl).native_handle();
            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::set_closed_event(closed_event_cb_type&& func) noexcept -> void {
        switch (m_wm) {
            case window_manager::x11: as<x11::window>(m_impl).closed_event = std::move(func); break;
            case window_manager::wayland: as<wayland::window>(m_impl).closed_event = std::move(func); break;
            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::set_monitor_changed_event(monitor_changed_event_cb_type&& func) noexcept -> void {
        switch (m_wm) {
            case window_manager::x11: as<x11::window>(m_impl).monitor_changed_event = std::move(func); break;
            case window_manager::wayland: as<wayland::window>(m_impl).monitor_changed_event = std::move(func); break;
            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::set_resized_event(resized_event_cb_type&& func) noexcept -> void {
        switch (m_wm) {
            case window_manager::x11: as<x11::window>(m_impl).resized_event = std::move(func); break;
            case window_manager::wayland: as<wayland::window>(m_impl).resized_event = std::move(func); break;
            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::set_restored_event(restored_event_cb_type&& func) noexcept -> void {
        switch (m_wm) {
            case window_manager::x11: as<x11::window>(m_impl).restored_event = std::move(func); break;
            case window_manager::wayland: as<wayland::window>(m_impl).restored_event = std::move(func); break;
            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::set_minimized_event(minimized_event_cb_type&& func) noexcept -> void {
        switch (m_wm) {
            case window_manager::x11: as<x11::window>(m_impl).minimized_event = std::move(func); break;
            case window_manager::wayland: as<wayland::window>(m_impl).minimized_event = std::move(func); break;
            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::set_key_down_event(key_down_event_cb_type&& func) noexcept -> void {
        switch (m_wm) {
            case window_manager::x11: as<x11::window>(m_impl).key_down_event = std::move(func); break;
            case window_manager::wayland: as<wayland::window>(m_impl).key_down_event = std::move(func); break;
            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::set_key_up_event(key_up_event_cb_type&& func) noexcept -> void {
        switch (m_wm) {
            case window_manager::x11: as<x11::window>(m_impl).key_up_event = std::move(func); break;
            case window_manager::wayland: as<wayland::window>(m_impl).key_up_event = std::move(func); break;
            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::set_mouse_button_down_event(mouse_button_down_event_cb_type&& func) noexcept -> void {
        switch (m_wm) {
            case window_manager::x11: as<x11::window>(m_impl).mouse_button_down_event = std::move(func); break;
            case window_manager::wayland: as<wayland::window>(m_impl).mouse_button_down_event = std::move(func); break;
            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::set_mouse_button_up_event(mouse_button_up_event_cb_type&& func) noexcept -> void {
        switch (m_wm) {
            case window_manager::x11: as<x11::window>(m_impl).mouse_button_up_event = std::move(func); break;
            case window_manager::wayland: as<wayland::window>(m_impl).mouse_button_up_event = std::move(func); break;
            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::set_mouse_moved_event(mouse_moved_event_cb_type&& func) noexcept -> void {
        switch (m_wm) {
            case window_manager::x11: as<x11::window>(m_impl).mouse_moved_event = std::move(func); break;
            case window_manager::wayland: as<wayland::window>(m_impl).mouse_moved_event = std::move(func); break;
            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::set_deactivate_event(deactivate_event_cb_type&& func) noexcept -> void {
        switch (m_wm) {
            case window_manager::x11: as<x11::window>(m_impl).deactivate_event = std::move(func); break;
            case window_manager::wayland: as<wayland::window>(m_impl).deactivate_event = std::move(func); break;
            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::set_activate_event(activate_event_cb_type&& func) noexcept -> void {
        switch (m_wm) {
            case window_manager::x11: as<x11::window>(m_impl).activate_event = std::move(func); break;
            case window_manager::wayland: as<wayland::window>(m_impl).activate_event = std::move(func); break;
            default: std::unreachable();
        }
    }
} // namespace stormkit::wsi::linux
