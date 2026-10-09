// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>

#if defined(STORMKIT_OS_WINDOWS) and not defined(STORMKIT_COMPILER_LIBCPP)
    #include <beman/optional/optional.hpp>
#endif

export module stormkit.wsi:common.window_base;

import std;

import stormkit.core;
import stormkit.math.extent;
import stormkit.math.linear;
import stormkit.wsi;

import :common.input_base;

export namespace stormkit::wsi::common {
    class window_base {
      public:
        STORMKIT_FORCE_INLINE
        inline window_base() noexcept {
            mouse_states_.push_back({ .id = global_mouse_id });
            keyboard_states_.push_back({ .id = global_keyboard_id });
        }

        window_base(const window_base&)                               = delete;
        auto operator=(const window_base&) -> window_base&            = delete;

        STORMKIT_FORCE_INLINE
        inline window_base(window_base&&) noexcept                    = default;

        STORMKIT_FORCE_INLINE
        inline auto operator=(window_base&&) noexcept -> window_base& = default;

        STORMKIT_FORCE_INLINE
        inline ~window_base() noexcept                                = default;

        auto set_open(bool open) noexcept -> void;
        [[nodiscard]]
        auto is_open() const noexcept -> bool;

        [[nodiscard]]
        auto visible() const noexcept -> bool;

        [[nodiscard]]
        auto current_monitor() const noexcept -> const monitor&;
        auto set_current_monitor(const monitor& extent) noexcept -> void;
        auto set_current_monitor(monitor&& extent) noexcept -> void;

        auto set_title(string&& title) noexcept -> bool;
        [[nodiscard]]
        auto title() const noexcept -> const string&;

        auto set_extent(const math::uextent2& extent) noexcept -> bool;
        [[nodiscard]]
        auto extent() const noexcept -> const math::uextent2&;

        auto set_fullscreen(bool fullscreen) noexcept -> bool;
        [[nodiscard]]
        auto fullscreen() const noexcept -> bool;

        closed_event_cb_type            closed_event { std::in_place, [] static noexcept { return true; } };
        monitor_changed_event_cb_type   monitor_changed_event { std::in_place, monadic::noop() };
        resized_event_cb_type           resized_event { std::in_place, monadic::noop() };
        restored_event_cb_type          restored_event { std::in_place, monadic::noop() };
        minimized_event_cb_type         minimized_event { std::in_place, monadic::noop() };
        key_down_event_cb_type          key_down_event { std::in_place, monadic::noop() };
        key_up_event_cb_type            key_up_event { std::in_place, monadic::noop() };
        mouse_button_down_event_cb_type mouse_button_down_event { std::in_place, monadic::noop() };
        mouse_button_up_event_cb_type   mouse_button_up_event { std::in_place, monadic::noop() };
        mouse_moved_event_cb_type       mouse_moved_event { std::in_place, monadic::noop() };
        deactivate_event_cb_type        deactivate_event { std::in_place, monadic::noop() };
        activate_event_cb_type          activate_event { std::in_place, monadic::noop() };

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

        template<typename T>
        auto mouse_state(this T& self, u8 id) noexcept -> core::meta::forward_const_to<T, mouse_state>&;

        template<typename T>
        auto keyboard_state(this T& self, u8 id) noexcept -> core::meta::forward_const_to<T, keyboard_state>&;

      protected:
        struct {
            bool           open       = false;
            bool           minimized  = false;
            bool           active     = false;
            bool           fullscreen = false;
            bool           visible    = false;
            math::uextent2 extent;
#if defined(STORMKIT_OS_WINDOWS) and not defined(STORMKIT_COMPILER_LIBCPP)
            beman::optional::optional<const monitor&> current_monitor;
#else
            std::optional<const monitor&> current_monitor;
#endif
            string title;

            f32 dpi = 1.f;

            math::ivec2 position = { 0, 0 };
        } state_;

        dynarray<common::mouse_state>    mouse_states_;
        dynarray<common::keyboard_state> keyboard_states_;
    };
} // namespace stormkit::wsi::common

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::wsi::common {
    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window_base::set_open(bool open) noexcept -> void {
        state_.open = open;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window_base::is_open() const noexcept -> bool {
        return state_.open;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window_base::visible() const noexcept -> bool {
        return state_.visible;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window_base::current_monitor() const noexcept -> const monitor& {
#if defined(STORMKIT_OS_WINDOWS) and not defined(STORMKIT_COMPILER_LIBCPP)
        EXPECTS(state_.current_monitor != beman::optional::nullopt);
#else
        EXPECTS(state_.current_monitor != std::nullopt);
#endif
        return state_.current_monitor.value();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window_base::set_current_monitor(const monitor& monitor) noexcept -> void {
        state_.current_monitor = monitor;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window_base::set_current_monitor(monitor&& monitor) noexcept -> void {
        state_.current_monitor = monitor;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window_base::set_title(string&& title) noexcept -> bool {
        if (not state_.open) return false;

        state_.title = std::move(title);
        return true;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window_base::title() const noexcept -> const string& {
        return state_.title;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window_base::set_extent(const math::uextent2& extent) noexcept -> bool {
        if (not state_.open) return false;

        state_.extent = extent;
        return true;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window_base::extent() const noexcept -> const math::uextent2& {
        return state_.extent;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window_base::set_fullscreen(bool fullscreen) noexcept -> bool {
        if (not state_.open) return false;

        state_.fullscreen = fullscreen;
        return true;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window_base::fullscreen() const noexcept -> bool {
        return state_.fullscreen;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window_base::set_closed_event(closed_event_cb_type&& func) noexcept -> void {
        closed_event = std::move(func);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window_base::set_monitor_changed_event(monitor_changed_event_cb_type&& func) noexcept -> void {
        monitor_changed_event = std::move(func);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window_base::set_resized_event(resized_event_cb_type&& func) noexcept -> void {
        resized_event = std::move(func);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window_base::set_restored_event(restored_event_cb_type&& func) noexcept -> void {
        restored_event = std::move(func);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window_base::set_minimized_event(minimized_event_cb_type&& func) noexcept -> void {
        minimized_event = std::move(func);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window_base::set_key_down_event(key_down_event_cb_type&& func) noexcept -> void {
        key_down_event = std::move(func);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window_base::set_key_up_event(key_up_event_cb_type&& func) noexcept -> void {
        key_up_event = std::move(func);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window_base::set_mouse_button_down_event(mouse_button_down_event_cb_type&& func) noexcept -> void {
        mouse_button_down_event = std::move(func);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window_base::set_mouse_button_up_event(mouse_button_up_event_cb_type&& func) noexcept -> void {
        mouse_button_up_event = std::move(func);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window_base::set_mouse_moved_event(mouse_moved_event_cb_type&& func) noexcept -> void {
        mouse_moved_event = std::move(func);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window_base::set_deactivate_event(deactivate_event_cb_type&& func) noexcept -> void {
        deactivate_event = std::move(func);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window_base::set_activate_event(activate_event_cb_type&& func) noexcept -> void {
        activate_event = std::move(func);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline auto window_base::mouse_state(this T& self, u8 id) noexcept -> core::meta::forward_const_to<T, common::mouse_state>& {
        expects(id < stdr::size(self.mouse_states_));
        return self.mouse_states_[id];
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline auto window_base::keyboard_state(this T& self, u8 id) noexcept
      -> core::meta::forward_const_to<T, common::keyboard_state>& {
        expects(id < stdr::size(self.keyboard_states_));
        return self.keyboard_states_[id];
    }
} // namespace stormkit::wsi::common
