// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.wsi:common.window_base;

import std;

import stormkit.core;
import stormkit.wsi;

import :common.input_base;

export namespace stormkit::wsi::common {
    class WindowBase {
      public:
        STORMKIT_FORCE_INLINE
        inline WindowBase() noexcept {
            m_mouse_states.push_back({ .id = GLOBAL_MOUSE_ID });
            m_keyboard_states.push_back({ .id = GLOBAL_KEYBOARD_ID });
        }

        WindowBase(const WindowBase&)                               = delete;
        auto operator=(const WindowBase&) -> WindowBase&            = delete;

        STORMKIT_FORCE_INLINE
        inline WindowBase(WindowBase&&) noexcept                    = default;

        STORMKIT_FORCE_INLINE
        inline auto operator=(WindowBase&&) noexcept -> WindowBase& = default;

        STORMKIT_FORCE_INLINE
        inline ~WindowBase() noexcept                               = default;

        auto set_open(bool open) noexcept -> void;
        [[nodiscard]]
        auto is_open() const noexcept -> bool;

        [[nodiscard]]
        auto visible() const noexcept -> bool;

        [[nodiscard]]
        auto current_monitor() const noexcept -> const Monitor&;
        auto set_current_monitor(const Monitor& extent) noexcept -> void;

        auto set_title(string title) noexcept -> bool;
        [[nodiscard]]
        auto title() const noexcept -> const string&;

        auto set_extent(const math::uextent2& extent) noexcept -> bool;
        [[nodiscard]]
        auto extent() const noexcept -> const math::uextent2&;

        auto set_fullscreen(bool fullscreen) noexcept -> bool;
        [[nodiscard]]
        auto fullscreen() const noexcept -> bool;

        ClosedEventFunc          closed_event            = [] static noexcept { return true; };
        MonitorChangedEventFunc  monitor_changed_event   = monadic::noop();
        ResizedEventFunc         resized_event           = monadic::noop();
        RestoredEventFunc        restored_event          = monadic::noop();
        MinimizedEventFunc       minimized_event         = monadic::noop();
        KeyDownEventFunc         key_down_event          = monadic::noop();
        KeyUpEventFunc           key_up_event            = monadic::noop();
        MouseButtonDownEventFunc mouse_button_down_event = monadic::noop();
        MouseButtonUpEventFunc   mouse_button_up_event   = monadic::noop();
        MouseMovedEventFunc      mouse_moved_event       = monadic::noop();
        DeactivateEventFunc      deactivate_event        = monadic::noop();
        ActivateEventFunc        activate_event          = monadic::noop();

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

        template<typename T>
        auto mouse_state(this T& self, u8 id) noexcept -> core::meta::forward_const_to<T, MouseState>&;

        template<typename T>
        auto keyboard_state(this T& self, u8 id) noexcept -> core::meta::forward_const_to<T, KeyboardState>&;

      protected:
        struct {
            bool                  open       = false;
            bool                  minimized  = false;
            bool                  active     = false;
            bool                  fullscreen = false;
            bool                  visible    = false;
            math::uextent2        extent;
            optref<const Monitor> current_monitor;
            string                title;

            f32 dpi = 1.f;

            math::ivec2 position = { 0, 0 };
        } m_state;

        dynarray<MouseState>    m_mouse_states;
        dynarray<KeyboardState> m_keyboard_states;
    };
} // namespace stormkit::wsi::common

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::wsi::common {
    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto WindowBase::set_open(bool open) noexcept -> void {
        m_state.open = open;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto WindowBase::is_open() const noexcept -> bool {
        return m_state.open;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto WindowBase::visible() const noexcept -> bool {
        return m_state.visible;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto WindowBase::current_monitor() const noexcept -> const Monitor& {
        return m_state.current_monitor;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto WindowBase::set_current_monitor(const Monitor& monitor) noexcept -> void {
        m_state.current_monitor = as_optref(monitor);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto WindowBase::set_title(string title) noexcept -> bool {
        if (not m_state.open) return false;

        m_state.title = std::move(title);
        return true;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto WindowBase::title() const noexcept -> const string& {
        return m_state.title;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto WindowBase::set_extent(const math::uextent2& extent) noexcept -> bool {
        if (not m_state.open) return false;

        m_state.extent = extent;
        return true;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto WindowBase::extent() const noexcept -> const math::uextent2& {
        return m_state.extent;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto WindowBase::set_fullscreen(bool fullscreen) noexcept -> bool {
        if (not m_state.open) return false;

        m_state.fullscreen = fullscreen;
        return true;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto WindowBase::fullscreen() const noexcept -> bool {
        return m_state.fullscreen;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto WindowBase::set_closed_event(ClosedEventFunc&& func) noexcept -> void {
        closed_event = std::move(func);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto WindowBase::set_monitor_changed_event(MonitorChangedEventFunc&& func) noexcept -> void {
        monitor_changed_event = std::move(func);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto WindowBase::set_resized_event(ResizedEventFunc&& func) noexcept -> void {
        resized_event = std::move(func);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto WindowBase::set_restored_event(RestoredEventFunc&& func) noexcept -> void {
        restored_event = std::move(func);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto WindowBase::set_minimized_event(MinimizedEventFunc&& func) noexcept -> void {
        minimized_event = std::move(func);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto WindowBase::set_key_down_event(KeyDownEventFunc&& func) noexcept -> void {
        key_down_event = std::move(func);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto WindowBase::set_key_up_event(KeyUpEventFunc&& func) noexcept -> void {
        key_up_event = std::move(func);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto WindowBase::set_mouse_button_down_event(MouseButtonDownEventFunc&& func) noexcept -> void {
        mouse_button_down_event = std::move(func);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto WindowBase::set_mouse_button_up_event(MouseButtonUpEventFunc&& func) noexcept -> void {
        mouse_button_up_event = std::move(func);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto WindowBase::set_mouse_moved_event(MouseMovedEventFunc&& func) noexcept -> void {
        mouse_moved_event = std::move(func);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto WindowBase::set_deactivate_event(DeactivateEventFunc&& func) noexcept -> void {
        deactivate_event = std::move(func);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto WindowBase::set_activate_event(ActivateEventFunc&& func) noexcept -> void {
        activate_event = std::move(func);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline auto WindowBase::mouse_state(this T& self, u8 id) noexcept -> core::meta::forward_const_to<T, MouseState>& {
        expects(id < stdr::size(self.m_mouse_states));
        return self.m_mouse_states[id];
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline auto WindowBase::keyboard_state(this T& self, u8 id) noexcept -> core::meta::forward_const_to<T, KeyboardState>& {
        expects(id < stdr::size(self.m_keyboard_states));
        return self.m_keyboard_states[id];
    }
} // namespace stormkit::wsi::common
