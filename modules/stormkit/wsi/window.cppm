// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#include <stormkit/wsi/api.hpp>

export module stormkit.wsi:window;

import std;

import stormkit.core;
import stormkit.math.extent;
import stormkit.math.linear;

import :core;
import :monitor;
import :mouse;
import :keyboard;

namespace stormkit::wsi {
    class window_impl;
}

export {
    namespace stormkit { namespace wsi {
        enum class window_flag : u8 {
            standard         = 0b1,
            borderless       = 0b10,
            resizeable       = 0b100,
            external_context = 0b1000,
        };

        [[nodiscard]]
        constexpr auto tag_invoke(as_fn<string_view>, window_flag value, const std::source_location&) noexcept -> string_view;

        enum class event_type : u8 {
            none = 0,
            closed,
            monitor_changed,
            resized,
            restored,
            minimized,
            key_down,
            key_up,
            mouse_button_down,
            mouse_button_up,
            mouse_moved,
            activate,
            deactivate,
        };

        [[nodiscard]]
        constexpr auto tag_invoke(as_fn<string_view>,
                                  event_type value,
                                  source_location_arg = std::source_location::current()) noexcept -> string_view;

        using native_handle_type = void*;

        using closed_event_cb_type
          = strong_type<std23::move_only_function<bool()>, struct closed_event_cb_tag, "closed", capabilities::callable>;
        using monitor_changed_event_cb_type = strong_type<std23::move_only_function<void(const monitor&)>,
                                                          struct monitor_changed_event_cb_tag,
                                                          "monitor_changed",
                                                          capabilities::callable>;
        using resized_event_cb_type         = strong_type<std23::move_only_function<void(const math::uextent2&)>,
                                                          struct resized_event_cb_tag,
                                                          "resized",
                                                          capabilities::callable>;
        using restored_event_cb_type
          = strong_type<std23::move_only_function<void()>, struct restored_event_cb_tag, "restored", capabilities::callable>;
        using minimized_event_cb_type
          = strong_type<std23::move_only_function<void()>, struct minimized_event_cb_tag, "minimized", capabilities::callable>;
        using key_down_event_cb_type          = strong_type<std23::move_only_function<void(u8, key, char)>,
                                                            struct key_down_event_cb_tag,
                                                            "key_down_event",
                                                            capabilities::callable>;
        using key_up_event_cb_type            = strong_type<std23::move_only_function<void(u8, key, char)>,
                                                            struct key_up_event_cb_tag,
                                                            "key_up_event",
                                                            capabilities::callable>;
        using mouse_button_down_event_cb_type = strong_type<std23::move_only_function<void(u8, mouse_button, const math::ivec2&)>,
                                                            struct mouse_button_down_event_cb_tag,
                                                            "mouse_button_down_event",
                                                            capabilities::callable>;
        using mouse_button_up_event_cb_type   = strong_type<std23::move_only_function<void(u8, mouse_button, const math::ivec2&)>,
                                                            struct mouse_button_up_event_cb_tag,
                                                            "mouse_button_up_event",
                                                            capabilities::callable>;
        using mouse_moved_event_cb_type       = strong_type<std23::move_only_function<void(u8, const math::ivec2&)>,
                                                            struct mouse_moved_event_cb_tag,
                                                            "mouse_moved",
                                                            capabilities::callable>;
        using deactivate_event_cb_type
          = strong_type<std23::move_only_function<void()>, struct deactivate_event_cb_tag, "deactivate", capabilities::callable>;
        using activate_event_cb_type
          = strong_type<std23::move_only_function<void()>, struct activate_event_cb_tag, "activate", capabilities::callable>;

        template<typename T>
        concept event_cb_type
          = meta::is_any_of<T,
                            closed_event_cb_type,
                            monitor_changed_event_cb_type,
                            resized_event_cb_type,
                            restored_event_cb_type,
                            minimized_event_cb_type,
                            key_down_event_cb_type,
                            key_up_event_cb_type,
                            mouse_button_down_event_cb_type,
                            mouse_button_up_event_cb_type,
                            mouse_moved_event_cb_type,
                            deactivate_event_cb_type,
                            activate_event_cb_type>
            or meta::convertible_to_any_of<
              T,
              closed_event_cb_type,
              monitor_changed_event_cb_type,
              resized_event_cb_type,
              restored_event_cb_type,
              minimized_event_cb_type,
              key_down_event_cb_type,
              key_up_event_cb_type,
              mouse_button_down_event_cb_type,
              mouse_button_up_event_cb_type,
              mouse_moved_event_cb_type,
              deactivate_event_cb_type,
              activate_event_cb_type>;

        class STORMKIT_WSI_API window {
          public:
            ~window() noexcept;

            window(const window&)                    = delete;
            auto operator=(const window&) -> window& = delete;
            window(window&&) noexcept;
            auto operator=(window&&) noexcept -> window&;

            static auto open(const string& title, meta::in<math::uextent2> size, window_flag flags) noexcept -> window;
            static auto open(string&& title, meta::in<math::uextent2> size, window_flag flags) noexcept -> window;
            static auto allocate_and_open(string title, meta::in<math::uextent2> size, window_flag flags) noexcept
              -> heap_ptr<window>;

            auto close() noexcept -> void;
            [[nodiscard]]
            auto is_open() const noexcept -> bool;
            auto handle_events() noexcept -> void;

            auto clear(meta::in<ucolor_rgb> color = colors::BLACK<u8>) noexcept -> void;
            auto fill_framebuffer(array_view<const ucolor_rgb> colors) noexcept -> void;

            template<event_cb_type T>
            auto on(T&& callback) noexcept -> void;

            template<event_cb_type... Ts>
                requires(sizeof...(Ts) >= 2)
            auto on(Ts&&... callbacks) noexcept -> void;

            template<event_type Type, std::invocable T>
            auto on(T&& callback) noexcept -> void;

            [[nodiscard]]
            auto visible() const noexcept -> bool;

            [[nodiscard]]
            auto current_monitor() const noexcept -> const monitor&;

            [[nodiscard]]
            auto title() const noexcept -> const string&;
            auto set_title(const string& title) noexcept -> void;
            auto set_title(string&& title) noexcept -> void;

            auto set_extent(meta::in<math::uextent2> extent) noexcept -> void;

            [[nodiscard]]
            auto extent() const noexcept -> const math::uextent2&;

            auto set_fullscreen(bool fullscreen) noexcept -> void;
            auto toggle_fullscreen() noexcept -> void;
            [[nodiscard]]
            auto fullscreen() const noexcept -> bool;

            auto confine_mouse(bool confined = true, u8 mouse_id = global_mouse_id) noexcept -> void;
            auto unconfine_mouse(u8 mouse_id = global_mouse_id) noexcept -> void;
            auto toggle_confined_mouse(u8 mouse_id = global_mouse_id) noexcept -> void;
            [[nodiscard]]
            auto is_mouse_confined(u8 mouse_id = global_mouse_id) const noexcept -> bool;

            auto lock_mouse(bool locked = true, u8 mouse_id = global_mouse_id) noexcept -> void;
            auto unlock_mouse(u8 mouse_id = global_mouse_id) noexcept -> void;
            auto toggle_locked_mouse(u8 mouse_id = global_mouse_id) noexcept -> void;
            [[nodiscard]]
            auto is_mouse_locked(u8 mouse_id = global_mouse_id) const noexcept -> bool;

            auto hide_mouse(bool hidden = true, u8 mouse_id = global_mouse_id) noexcept -> void;
            auto unhide_mouse(u8 mouse_id = global_mouse_id) noexcept -> void;
            auto toggle_hidden_mouse(u8 mouse_id = global_mouse_id) noexcept -> void;
            [[nodiscard]]
            auto is_mouse_hidden(u8 mouse_id = global_mouse_id) const noexcept -> bool;

            auto set_relative_mouse(bool enabled = true, u8 mouse_id = global_mouse_id) noexcept -> void;
            auto toggle_relative_mouse(u8 mouse_id = global_mouse_id) noexcept -> void;
            [[nodiscard]]
            auto is_mouse_relative(u8 mouse_id = global_mouse_id) const noexcept -> bool;

            auto set_key_repeat(bool enabled = true, u8 keyboard_id = global_keyboard_id) noexcept -> void;
            auto disable_key_repeat(u8 keyboard_id = global_keyboard_id) noexcept -> void;
            auto toggle_key_repeat(u8 keyboard_id = global_keyboard_id) noexcept -> void;
            [[nodiscard]]
            auto is_key_repeat_enabled(u8 keyboard_id = global_keyboard_id) const noexcept -> bool;

            auto show_virtual_keyboard(bool visible = true) noexcept -> void;
            auto hide_virtual_keyboard() noexcept -> void;
            auto toggle_virtual_keyboard() noexcept -> void;
            [[nodiscard]]
            auto is_virtual_keyboard_visible() const noexcept -> bool;

            auto set_mouse_position(meta::in<math::ivec2> position, u8 mouse_id = global_mouse_id) noexcept -> void;

            [[nodiscard]]
            auto native_handle() const noexcept -> native_handle_type;

            [[nodiscard]]
            auto wm() const noexcept -> window_manager;

            auto event_loop() noexcept -> void;
            auto event_loop(std23::function_ref<void()> func) noexcept -> void;

          private:
            window() noexcept;

            auto on_closed(closed_event_cb_type&&) noexcept -> void;
            auto on_monitor_changed(monitor_changed_event_cb_type&&) noexcept -> void;
            auto on_resized(resized_event_cb_type&&) noexcept -> void;
            auto on_restored(restored_event_cb_type&&) noexcept -> void;
            auto on_minimized(minimized_event_cb_type&&) noexcept -> void;
            auto on_key_down(key_down_event_cb_type&&) noexcept -> void;
            auto on_key_up(key_up_event_cb_type&&) noexcept -> void;
            auto on_mouse_button_down(mouse_button_down_event_cb_type&&) noexcept -> void;
            auto on_mouse_button_up(mouse_button_up_event_cb_type&&) noexcept -> void;
            auto on_mouse_moved(mouse_moved_event_cb_type&&) noexcept -> void;
            auto on_activate(activate_event_cb_type&&) noexcept -> void;
            auto on_deactivate(deactivate_event_cb_type&&) noexcept -> void;

            window_manager m_wm;

            pimpl<window_impl> m_impl;
        };
    }} // namespace stormkit::wsi

    template<>
    inline constexpr auto stormkit::core::meta::FLAG_TRAIT<stormkit::wsi::window_flag> = true;
}

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stdr = std::ranges;

using namespace std::literals;

namespace stormkit::wsi {
    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto tag_invoke(as_fn<string_view>, window_flag flags, source_location_arg) noexcept -> string_view {
        using pair                    = std::pair<window_flag, string_view>;
        static constexpr auto mapping = core::generate_substitution_strings_for<window_flag, 4, window_flag::standard, 67>(
          "window_flag::",
          array {
            pair { window_flag::standard,         "standard"sv         },
            pair { window_flag::borderless,       "borderless"sv       },
            pair { window_flag::resizeable,       "resizeable"sv       },
            pair { window_flag::external_context, "external_context"sv },
        });

        const auto it = stdr::find_if(mapping, [flags](auto&& pair) { return pair.first == flags; });
        ensures(it != stdr::cend(mapping));

        return it->second;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto tag_invoke(as_fn<string_view>, event_type type, source_location_arg) noexcept -> string_view {
        switch (type) {
            case event_type::none: return "event_type::none";
            case event_type::closed: return "event_type::closed";
            case event_type::resized: return "event_type::resized";
            case event_type::restored: return "event_type::restored";
            case event_type::minimized: return "event_type::minimized";
            case event_type::key_down: return "event_type::key_down";
            case event_type::key_up: return "event_type::key_up";
            case event_type::mouse_button_down: return "event_type::mouse_button_down";
            case event_type::mouse_button_up: return "event_type::mouse_button_up";
            case event_type::mouse_moved: return "event_type::mouse_moved";
            case event_type::activate: return "event_type::activate";
            case event_type::deactivate: return "event_type::deactivate";
            default: break;
        }

        std::unreachable();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    inline auto window::open(const string& title, meta::in<math::uextent2> size, window_flag flags) noexcept -> window {
        return open(string { title }, size, flags);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<event_cb_type T>
    STORMKIT_FORCE_INLINE
    inline auto window::on(T&& callback) noexcept -> void {
        if constexpr (meta::plain::is<closed_event_cb_type, T>) on_closed(std::forward<T>(callback));
        else if constexpr (meta::plain::is<monitor_changed_event_cb_type, T>)
            on_monitor_changed(std::forward<T>(callback));
        else if constexpr (meta::plain::is<resized_event_cb_type, T>)
            on_resized(std::forward<T>(callback));
        else if constexpr (meta::plain::is<restored_event_cb_type, T>)
            on_restored(std::forward<T>(callback));
        else if constexpr (meta::plain::is<minimized_event_cb_type, T>)
            on_minimized(std::forward<T>(callback));
        else if constexpr (meta::plain::is<key_down_event_cb_type, T>)
            on_key_down(std::forward<T>(callback));
        else if constexpr (meta::plain::is<key_up_event_cb_type, T>)
            on_key_up(std::forward<T>(callback));
        else if constexpr (meta::plain::is<mouse_button_down_event_cb_type, T>)
            on_mouse_button_down(std::forward<T>(callback));
        else if constexpr (meta::plain::is<mouse_button_up_event_cb_type, T>)
            on_mouse_button_up(std::forward<T>(callback));
        else if constexpr (meta::plain::is<mouse_moved_event_cb_type, T>)
            on_mouse_moved(std::forward<T>(callback));
        else if constexpr (meta::plain::is<activate_event_cb_type, T>)
            on_activate(std::forward<T>(callback));
        else if constexpr (meta::plain::is<deactivate_event_cb_type, T>)
            on_deactivate(std::forward<T>(callback));
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<event_cb_type... Ts>
        requires(sizeof...(Ts) >= 2)
    STORMKIT_FORCE_INLINE
    inline auto window::on(Ts&&... callbacks) noexcept -> void {
        (on(std::forward<Ts>(callbacks)), ...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<event_type Type, std::invocable T>
    STORMKIT_FORCE_INLINE
    inline auto window::on(T&& callback) noexcept -> void {
        if constexpr (Type == event_type::closed) {
            static_assert(meta::constructible_from<closed_event_cb_type, T>);
            on_closed(closed_event_cb_type { std::forward<T>(callback) });
        } else if constexpr (Type == event_type::monitor_changed)
            on_monitor_changed(std::forward<T>(callback));
        else if constexpr (Type == event_type::resized)
            on_resized(std::forward<T>(callback));
        else if constexpr (Type == event_type::restored)
            on_restored(std::forward<T>(callback));
        else if constexpr (Type == event_type::minimized)
            on_minimized(std::forward<T>(callback));
        else if constexpr (Type == event_type::key_down)
            on_key_down(std::forward<T>(callback));
        else if constexpr (Type == event_type::key_up)
            on_key_up(std::forward<T>(callback));
        else if constexpr (Type == event_type::mouse_button_down)
            on_mouse_button_down(std::forward<T>(callback));
        else if constexpr (Type == event_type::mouse_button_up)
            on_mouse_button_up(std::forward<T>(callback));
        else if constexpr (Type == event_type::mouse_moved)
            on_mouse_moved(std::forward<T>(callback));
        else if constexpr (Type == event_type::activate)
            on_activate(std::forward<T>(callback));
        else if constexpr (Type == event_type::deactivate)
            on_deactivate(std::forward<T>(callback));
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    inline auto window::set_title(const string& title) noexcept -> void {
        set_title(string { title });
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::toggle_fullscreen() noexcept -> void {
        set_fullscreen(not fullscreen());
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::unconfine_mouse(u8 mouse_id) noexcept -> void {
        confine_mouse(false, mouse_id);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::toggle_confined_mouse(u8 mouse_id) noexcept -> void {
        confine_mouse(not is_mouse_confined(mouse_id), mouse_id);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::unlock_mouse(u8 mouse_id) noexcept -> void {
        lock_mouse(false, mouse_id);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::toggle_locked_mouse(u8 mouse_id) noexcept -> void {
        lock_mouse(not is_mouse_locked(mouse_id), mouse_id);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::unhide_mouse(u8 mouse_id) noexcept -> void {
        hide_mouse(false, mouse_id);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::toggle_hidden_mouse(u8 mouse_id) noexcept -> void {
        hide_mouse(not is_mouse_hidden(mouse_id), mouse_id);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::toggle_relative_mouse(u8 mouse_id) noexcept -> void {
        set_relative_mouse(not is_mouse_relative(mouse_id), mouse_id);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::toggle_key_repeat(u8 keyboard_id) noexcept -> void {
        set_key_repeat(not is_key_repeat_enabled(keyboard_id), keyboard_id);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::hide_virtual_keyboard() noexcept -> void {
        show_virtual_keyboard(false);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::toggle_virtual_keyboard() noexcept -> void {
        show_virtual_keyboard(not is_virtual_keyboard_visible());
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::wm() const noexcept -> window_manager {
        return m_wm;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::event_loop() noexcept -> void {
        event_loop(monadic::noop());
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::event_loop(std23::function_ref<void()> func) noexcept -> void {
        while (is_open()) {
            func();

            handle_events();
        }
    }
} // namespace stormkit::wsi
