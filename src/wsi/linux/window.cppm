// Copyright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
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
    class Window {
        using BackendWindow = std::variant<x11::Window, wayland::Window, std::monostate>;

        WM m_wm;

        BackendWindow m_impl = std::monostate {};

      public:
        explicit Window(WM wm) noexcept;
        ~Window() noexcept;

        Window(const Window&) noexcept                    = delete;
        auto operator=(const Window&) noexcept -> Window& = delete;

        Window(Window&&) noexcept;
        auto operator=(Window&&) noexcept -> Window&;

        auto open(string title, const math::uextent2& size, WindowFlag flags) noexcept -> void;
        auto close() noexcept -> void;

        [[nodiscard]]
        auto is_open() const noexcept -> bool;

        [[nodiscard]]
        auto visible() const noexcept -> bool;

        [[nodiscard]]
        auto current_monitor() const noexcept -> const Monitor&;

        auto handle_events() noexcept -> void;

        auto clear(const ucolor_rgb& color) noexcept -> void;
        auto fill_framebuffer(array_view<const ucolor_rgb> colors) noexcept -> void;

        auto set_title(string title) noexcept -> void;
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
        auto native_handle() const noexcept -> NativeHandle;

        auto set_closed_event(ClosedEventFunc&& func) noexcept -> void;
        auto set_monitor_changed_event(MonitorChangedEventFunc&& func) noexcept -> void;
        auto set_resized_event(ResizedEventFunc&& func) noexcept -> void;
        auto set_restored_event(RestoredEventFunc&& func) noexcept -> void;
        auto set_minimized_event(MinimizedEventFunc&& func) noexcept -> void;
        auto set_key_down_event(KeyDownEventFunc&& func) noexcept -> void;
        auto set_key_up_event(KeyUpEventFunc&& func) noexcept -> void;
        auto set_mouse_button_down_event(MouseButtonDownEventFunc&& func) noexcept -> void;
        auto set_mouse_button_up_event(MouseButtonUpEventFunc&& func) noexcept -> void;
        auto set_mouse_moved_event(MouseMovedEventFunc&& func) noexcept -> void;
        auto set_deactivate_event(DeactivateEventFunc&& func) noexcept -> void;
        auto set_activate_event(ActivateEventFunc&& func) noexcept -> void;
    };
} // namespace stormkit::wsi::linux

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::wsi::linux {
    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline Window::Window(WM wm) noexcept
        : m_wm { wm } {
        if (m_wm == WM::X11) m_impl = x11::Window {};
        else if (m_wm == WM::WAYLAND)
            m_impl = wayland::Window {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline Window::~Window() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline Window::Window(Window&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::operator=(Window&&) noexcept -> Window& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::open(string title, const math::uextent2& extent, WindowFlag flags) noexcept -> void {
        switch (m_wm) {
            case WM::X11: as<x11::Window>(m_impl).open(std::move(title), extent, flags); break;
            case WM::WAYLAND: as<wayland::Window>(m_impl).open(std::move(title), extent, flags); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::close() noexcept -> void {
        switch (m_wm) {
            case WM::X11: as<x11::Window>(m_impl).close(); break;
            case WM::WAYLAND: as<wayland::Window>(m_impl).close(); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto Window::is_open() const noexcept -> bool {
        switch (m_wm) {
            case WM::X11: return as<x11::Window>(m_impl).is_open();
            case WM::WAYLAND: return as<wayland::Window>(m_impl).is_open();

            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto Window::visible() const noexcept -> bool {
        switch (m_wm) {
            case WM::X11: return as<x11::Window>(m_impl).visible();
            case WM::WAYLAND: return as<wayland::Window>(m_impl).visible();

            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto Window::current_monitor() const noexcept -> const Monitor& {
        switch (m_wm) {
            case WM::X11: return as<x11::Window>(m_impl).current_monitor();
            case WM::WAYLAND: return as<wayland::Window>(m_impl).current_monitor();

            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::handle_events() noexcept -> void {
        switch (m_wm) {
            case WM::X11: as<x11::Window>(m_impl).handle_events(); break;
            case WM::WAYLAND: as<wayland::Window>(m_impl).handle_events(); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::clear(const ucolor_rgb& color) noexcept -> void {
        switch (m_wm) {
            case WM::X11: as<x11::Window>(m_impl).clear(color); break;
            case WM::WAYLAND: as<wayland::Window>(m_impl).clear(color); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::fill_framebuffer(array_view<const ucolor_rgb> colors) noexcept -> void {
        switch (m_wm) {
            case WM::X11: as<x11::Window>(m_impl).fill_framebuffer(colors); break;
            case WM::WAYLAND: as<wayland::Window>(m_impl).fill_framebuffer(colors); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::set_title(string title) noexcept -> void {
        switch (m_wm) {
            case WM::X11: as<x11::Window>(m_impl).set_title(std::move(title)); break;
            case WM::WAYLAND: as<wayland::Window>(m_impl).set_title(std::move(title)); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto Window::title() const noexcept -> const string& {
        switch (m_wm) {
            case WM::X11: return as<x11::Window>(m_impl).title();
            case WM::WAYLAND: return as<wayland::Window>(m_impl).title();

            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::set_extent(const math::uextent2& extent) noexcept -> void {
        switch (m_wm) {
            case WM::X11: as<x11::Window>(m_impl).set_extent(extent); break;
            case WM::WAYLAND: as<wayland::Window>(m_impl).set_extent(extent); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto Window::extent() const noexcept -> const math::uextent2& {
        switch (m_wm) {
            case WM::X11: return as<x11::Window>(m_impl).extent();
            case WM::WAYLAND: return as<wayland::Window>(m_impl).extent();

            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::set_fullscreen(bool enabled) noexcept -> void {
        switch (m_wm) {
            case WM::X11: as<x11::Window>(m_impl).set_fullscreen(enabled); break;
            case WM::WAYLAND: as<wayland::Window>(m_impl).set_fullscreen(enabled); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto Window::fullscreen() const noexcept -> bool {
        switch (m_wm) {
            case WM::X11: return as<x11::Window>(m_impl).fullscreen();
            case WM::WAYLAND: return as<wayland::Window>(m_impl).fullscreen();

            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::confine_mouse(bool confined, u8 mouse_id) noexcept -> void {
        switch (m_wm) {
            case WM::X11: as<x11::Window>(m_impl).confine_mouse(confined, mouse_id); break;
            case WM::WAYLAND: as<wayland::Window>(m_impl).confine_mouse(confined, mouse_id); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto Window::is_mouse_confined(u8 mouse_id) const noexcept -> bool {
        switch (m_wm) {
            case WM::X11: return as<x11::Window>(m_impl).is_mouse_confined(mouse_id);
            case WM::WAYLAND: return as<wayland::Window>(m_impl).is_mouse_confined(mouse_id);

            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::lock_mouse(bool locked, u8 mouse_id) noexcept -> void {
        switch (m_wm) {
            case WM::X11: as<x11::Window>(m_impl).lock_mouse(locked, mouse_id); break;
            case WM::WAYLAND: as<wayland::Window>(m_impl).lock_mouse(locked, mouse_id); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto Window::is_mouse_locked(u8 mouse_id) const noexcept -> bool {
        switch (m_wm) {
            case WM::X11: return as<x11::Window>(m_impl).is_mouse_locked(mouse_id);
            case WM::WAYLAND: return as<wayland::Window>(m_impl).is_mouse_locked(mouse_id);

            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::hide_mouse(bool hidden, u8 mouse_id) noexcept -> void {
        switch (m_wm) {
            case WM::X11: as<x11::Window>(m_impl).hide_mouse(hidden, mouse_id); break;
            case WM::WAYLAND: as<wayland::Window>(m_impl).hide_mouse(hidden, mouse_id); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto Window::is_mouse_hidden(u8 mouse_id) const noexcept -> bool {
        switch (m_wm) {
            case WM::X11: return as<x11::Window>(m_impl).is_mouse_hidden(mouse_id);
            case WM::WAYLAND: return as<wayland::Window>(m_impl).is_mouse_hidden(mouse_id);

            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::set_relative_mouse(bool enabled, u8 mouse_id) noexcept -> void {
        switch (m_wm) {
            case WM::X11: as<x11::Window>(m_impl).set_relative_mouse(enabled, mouse_id); break;
            case WM::WAYLAND: as<wayland::Window>(m_impl).set_relative_mouse(enabled, mouse_id); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto Window::is_mouse_relative(u8 mouse_id) const noexcept -> bool {
        switch (m_wm) {
            case WM::X11: return as<x11::Window>(m_impl).is_mouse_relative(mouse_id);
            case WM::WAYLAND: return as<wayland::Window>(m_impl).is_mouse_relative(mouse_id);

            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::set_key_repeat(bool enabled, u8 keyboard_id) noexcept -> void {
        switch (m_wm) {
            case WM::X11: return as<x11::Window>(m_impl).set_key_repeat(enabled, keyboard_id);
            case WM::WAYLAND: return as<wayland::Window>(m_impl).set_key_repeat(enabled, keyboard_id);

            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto Window::is_key_repeat_enabled(u8 keyboard_id) const noexcept -> bool {
        switch (m_wm) {
            case WM::X11: return as<x11::Window>(m_impl).is_key_repeat_enabled(keyboard_id);
            case WM::WAYLAND: return as<wayland::Window>(m_impl).is_key_repeat_enabled(keyboard_id);

            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::show_virtual_keyboard(bool visible) noexcept -> void {
        switch (m_wm) {
            case WM::X11: as<x11::Window>(m_impl).show_virtual_keyboard(visible); break;
            case WM::WAYLAND: as<wayland::Window>(m_impl).show_virtual_keyboard(visible); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto Window::is_virtual_keyboard_visible() const noexcept -> bool {
        switch (m_wm) {
            case WM::X11: return as<x11::Window>(m_impl).is_virtual_keyboard_visible();
            case WM::WAYLAND: return as<wayland::Window>(m_impl).is_virtual_keyboard_visible();

            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::set_mouse_position(const math::ivec2& position, u8 mouse_id) noexcept -> void {
        switch (m_wm) {
            case WM::X11: as<x11::Window>(m_impl).set_mouse_position(position, mouse_id); break;
            case WM::WAYLAND: as<wayland::Window>(m_impl).set_mouse_position(position, mouse_id); break;

            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto Window::native_handle() const noexcept -> NativeHandle {
        switch (m_wm) {
            case WM::X11: return as<x11::Window>(m_impl).native_handle();
            case WM::WAYLAND: return as<wayland::Window>(m_impl).native_handle();
            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::set_closed_event(ClosedEventFunc&& func) noexcept -> void {
        switch (m_wm) {
            case WM::X11: as<x11::Window>(m_impl).closed_event = std::move(func); break;
            case WM::WAYLAND: as<wayland::Window>(m_impl).closed_event = std::move(func); break;
            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::set_monitor_changed_event(MonitorChangedEventFunc&& func) noexcept -> void {
        switch (m_wm) {
            case WM::X11: as<x11::Window>(m_impl).monitor_changed_event = std::move(func); break;
            case WM::WAYLAND: as<wayland::Window>(m_impl).monitor_changed_event = std::move(func); break;
            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::set_resized_event(ResizedEventFunc&& func) noexcept -> void {
        switch (m_wm) {
            case WM::X11: as<x11::Window>(m_impl).resized_event = std::move(func); break;
            case WM::WAYLAND: as<wayland::Window>(m_impl).resized_event = std::move(func); break;
            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::set_restored_event(RestoredEventFunc&& func) noexcept -> void {
        switch (m_wm) {
            case WM::X11: as<x11::Window>(m_impl).restored_event = std::move(func); break;
            case WM::WAYLAND: as<wayland::Window>(m_impl).restored_event = std::move(func); break;
            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::set_minimized_event(MinimizedEventFunc&& func) noexcept -> void {
        switch (m_wm) {
            case WM::X11: as<x11::Window>(m_impl).minimized_event = std::move(func); break;
            case WM::WAYLAND: as<wayland::Window>(m_impl).minimized_event = std::move(func); break;
            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::set_key_down_event(KeyDownEventFunc&& func) noexcept -> void {
        switch (m_wm) {
            case WM::X11: as<x11::Window>(m_impl).key_down_event = std::move(func); break;
            case WM::WAYLAND: as<wayland::Window>(m_impl).key_down_event = std::move(func); break;
            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::set_key_up_event(KeyUpEventFunc&& func) noexcept -> void {
        switch (m_wm) {
            case WM::X11: as<x11::Window>(m_impl).key_up_event = std::move(func); break;
            case WM::WAYLAND: as<wayland::Window>(m_impl).key_up_event = std::move(func); break;
            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::set_mouse_button_down_event(MouseButtonDownEventFunc&& func) noexcept -> void {
        switch (m_wm) {
            case WM::X11: as<x11::Window>(m_impl).mouse_button_down_event = std::move(func); break;
            case WM::WAYLAND: as<wayland::Window>(m_impl).mouse_button_down_event = std::move(func); break;
            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::set_mouse_button_up_event(MouseButtonUpEventFunc&& func) noexcept -> void {
        switch (m_wm) {
            case WM::X11: as<x11::Window>(m_impl).mouse_button_up_event = std::move(func); break;
            case WM::WAYLAND: as<wayland::Window>(m_impl).mouse_button_up_event = std::move(func); break;
            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::set_mouse_moved_event(MouseMovedEventFunc&& func) noexcept -> void {
        switch (m_wm) {
            case WM::X11: as<x11::Window>(m_impl).mouse_moved_event = std::move(func); break;
            case WM::WAYLAND: as<wayland::Window>(m_impl).mouse_moved_event = std::move(func); break;
            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::set_deactivate_event(DeactivateEventFunc&& func) noexcept -> void {
        switch (m_wm) {
            case WM::X11: as<x11::Window>(m_impl).deactivate_event = std::move(func); break;
            case WM::WAYLAND: as<wayland::Window>(m_impl).deactivate_event = std::move(func); break;
            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::set_activate_event(ActivateEventFunc&& func) noexcept -> void {
        switch (m_wm) {
            case WM::X11: as<x11::Window>(m_impl).activate_event = std::move(func); break;
            case WM::WAYLAND: as<wayland::Window>(m_impl).activate_event = std::move(func); break;
            default: std::unreachable();
        }
    }
} // namespace stormkit::wsi::linux
