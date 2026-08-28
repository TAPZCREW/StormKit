// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#include <stormkit/wsi/api.hpp>

export module stormkit.wsi:window;

import std;

import stormkit.core;

import :core;
import :monitor;
import :mouse;
import :keyboard;

namespace stormkit::wsi {
    class WindowImpl;
}

export {
    namespace stormkit { namespace wsi {
        enum class WindowFlag : u8 {
            // DEFAULT          = 0b0,
            // BORDERLESS       = 0b1,
            // RESIZEABLE       = 0b10,
            // EXTERNAL_CONTEXT = 0b100,
            DEFAULT          = 0b1,
            BORDERLESS       = 0b10,
            RESIZEABLE       = 0b100,
            EXTERNAL_CONTEXT = 0b1000,
        };

        [[nodiscard]]
        constexpr auto tag_invoke(as_fn<string_view>, WindowFlag value, const std::source_location&) noexcept -> string_view;

        enum class EventType : u8 {
            NONE = 0,
            CLOSED,
            MONITOR_CHANGED,
            RESIZED,
            RESTORED,
            MINIMIZED,
            KEY_DOWN,
            KEY_UP,
            MOUSE_BUTTON_DOWN,
            MOUSE_BUTTON_UP,
            MOUSE_MOVED,
            ACTIVATE,
            DEACTIVATE,
        };
        constexpr auto as_string(EventType type) noexcept -> string_view;
        constexpr auto to_string(EventType type) noexcept -> string;

        using NativeHandle = void*;

        struct ClosedEventFunc: std::function<bool()> {
            using std::function<bool()>::function;
        };

        struct MonitorChangedEventFunc: std::function<void(const Monitor&)> {
            using std::function<void(const Monitor&)>::function;
        };

        struct ResizedEventFunc: std::function<void(const math::uextent2&)> {
            using std::function<void(const math::uextent2&)>::function;
        };

        struct RestoredEventFunc: std::function<void()> {
            using std::function<void()>::function;
        };

        struct MinimizedEventFunc: std::function<void()> {
            using std::function<void()>::function;
        };

        struct KeyDownEventFunc: std::function<void(u8, Key, char)> {
            using std::function<void(u8, Key, char)>::function;
        };

        struct KeyUpEventFunc: std::function<void(u8, Key, char)> {
            using std::function<void(u8, Key, char)>::function;
        };

        struct MouseButtonDownEventFunc: std::function<void(u8, MouseButton, const math::ivec2&)> {
            using std::function<void(u8, MouseButton, const math::ivec2&)>::function;
        };

        struct MouseButtonUpEventFunc: std::function<void(u8, MouseButton, const math::ivec2&)> {
            using std::function<void(u8, MouseButton, const math::ivec2&)>::function;
        };

        struct MouseMovedEventFunc: std::function<void(u8, const math::ivec2&)> {
            using std::function<void(u8, const math::ivec2&)>::function;
        };

        struct DeactivateEventFunc: std::function<void()> {
            using std::function<void()>::function;
        };

        struct ActivateEventFunc: std::function<void()> {
            using std::function<void()>::function;
        };

        template<typename T>
        concept EventCallbackFunc = meta::IsConvertibleToOneOf<
          T,
          ClosedEventFunc,
          MonitorChangedEventFunc,
          ResizedEventFunc,
          RestoredEventFunc,
          MinimizedEventFunc,
          KeyDownEventFunc,
          KeyUpEventFunc,
          MouseButtonDownEventFunc,
          MouseButtonUpEventFunc,
          MouseMovedEventFunc,
          DeactivateEventFunc,
          ActivateEventFunc>;

        class STORMKIT_WSI_API Window {
          public:
            ~Window() noexcept;

            Window(Window&&) noexcept;
            auto operator=(Window&&) noexcept -> Window&;

            static auto open(string title, const math::uextent2& size, WindowFlag flags) noexcept -> Window;
            static auto allocate_and_open(string title, const math::uextent2& size, WindowFlag flags) noexcept
              -> heap_ptr<Window>;

            auto close() noexcept -> void;
            [[nodiscard]]
            auto is_open() const noexcept -> bool;
            auto handle_events() noexcept -> void;

            auto clear(const ucolor_rgb& color = colors::BLACK<u8>) noexcept -> void;
            auto fill_framebuffer(array_view<const ucolor_rgb> colors) noexcept -> void;

            template<EventCallbackFunc T>
            auto on(T&& callback) noexcept -> void;

            template<EventCallbackFunc... Ts>
                requires(sizeof...(Ts) >= 2)
            auto on(Ts&&... callbacks) noexcept -> void;

            template<EventType TYPE, EventCallbackFunc T>
            auto on(T&& callback) noexcept -> void;

            [[nodiscard]]
            auto visible() const noexcept -> bool;

            [[nodiscard]]
            auto current_monitor() const noexcept -> const Monitor&;

            [[nodiscard]]
            auto title() const noexcept -> const string&;
            auto set_title(string title) noexcept -> void;

            auto set_extent(const math::uextent2& extent) noexcept -> void;

            [[nodiscard]]
            auto extent() const noexcept -> const math::uextent2&;

            auto set_fullscreen(bool fullscreen) noexcept -> void;
            auto toggle_fullscreen() noexcept -> void;
            [[nodiscard]]
            auto fullscreen() const noexcept -> bool;

            auto confine_mouse(bool confined = true, u8 mouse_id = GLOBAL_MOUSE_ID) noexcept -> void;
            auto unconfine_mouse(u8 mouse_id = GLOBAL_MOUSE_ID) noexcept -> void;
            auto toggle_confined_mouse(u8 mouse_id = GLOBAL_MOUSE_ID) noexcept -> void;
            [[nodiscard]]
            auto is_mouse_confined(u8 mouse_id = GLOBAL_MOUSE_ID) const noexcept -> bool;

            auto lock_mouse(bool locked = true, u8 mouse_id = GLOBAL_MOUSE_ID) noexcept -> void;
            auto unlock_mouse(u8 mouse_id = GLOBAL_MOUSE_ID) noexcept -> void;
            auto toggle_locked_mouse(u8 mouse_id = GLOBAL_MOUSE_ID) noexcept -> void;
            [[nodiscard]]
            auto is_mouse_locked(u8 mouse_id = GLOBAL_MOUSE_ID) const noexcept -> bool;

            auto hide_mouse(bool hidden = true, u8 mouse_id = GLOBAL_MOUSE_ID) noexcept -> void;
            auto unhide_mouse(u8 mouse_id = GLOBAL_MOUSE_ID) noexcept -> void;
            auto toggle_hidden_mouse(u8 mouse_id = GLOBAL_MOUSE_ID) noexcept -> void;
            [[nodiscard]]
            auto is_mouse_hidden(u8 mouse_id = GLOBAL_MOUSE_ID) const noexcept -> bool;

            auto set_relative_mouse(bool enabled = true, u8 mouse_id = GLOBAL_MOUSE_ID) noexcept -> void;
            auto toggle_relative_mouse(u8 mouse_id = GLOBAL_MOUSE_ID) noexcept -> void;
            [[nodiscard]]
            auto is_mouse_relative(u8 mouse_id = GLOBAL_MOUSE_ID) const noexcept -> bool;

            auto set_key_repeat(bool enabled = true, u8 keyboard_id = GLOBAL_KEYBOARD_ID) noexcept -> void;
            auto disable_key_repeat(u8 keyboard_id = GLOBAL_KEYBOARD_ID) noexcept -> void;
            auto toggle_key_repeat(u8 keyboard_id = GLOBAL_KEYBOARD_ID) noexcept -> void;
            [[nodiscard]]
            auto is_key_repeat_enabled(u8 keyboard_id = GLOBAL_KEYBOARD_ID) const noexcept -> bool;

            auto show_virtual_keyboard(bool visible = true) noexcept -> void;
            auto hide_virtual_keyboard() noexcept -> void;
            auto toggle_virtual_keyboard() noexcept -> void;
            [[nodiscard]]
            auto is_virtual_keyboard_visible() const noexcept -> bool;

            auto set_mouse_position(const math::ivec2& position, u8 mouse_id = GLOBAL_MOUSE_ID) noexcept -> void;

            [[nodiscard]]
            auto native_handle() const noexcept -> NativeHandle;

            [[nodiscard]]
            auto wm() const noexcept -> WM;

            auto event_loop() noexcept -> void;
            auto event_loop(std::invocable<> auto&& func) noexcept -> void;

          private:
            Window() noexcept;

            auto on_closed(ClosedEventFunc&&) noexcept -> void;
            auto on_monitor_changed(MonitorChangedEventFunc&&) noexcept -> void;
            auto on_resized(ResizedEventFunc&&) noexcept -> void;
            auto on_restored(RestoredEventFunc&&) noexcept -> void;
            auto on_minimized(MinimizedEventFunc&&) noexcept -> void;
            auto on_key_down(KeyDownEventFunc&&) noexcept -> void;
            auto on_key_up(KeyUpEventFunc&&) noexcept -> void;
            auto on_mouse_button_down(MouseButtonDownEventFunc&&) noexcept -> void;
            auto on_mouse_button_up(MouseButtonUpEventFunc&&) noexcept -> void;
            auto on_mouse_moved(MouseMovedEventFunc&&) noexcept -> void;
            auto on_activate(ActivateEventFunc&&) noexcept -> void;
            auto on_deactivate(DeactivateEventFunc&&) noexcept -> void;

            WM m_wm;

            pimpl<WindowImpl> m_impl;
        };
    }} // namespace stormkit::wsi

    template<>
    inline constexpr auto stormkit::core::meta::FLAG_TRAIT<stormkit::wsi::WindowFlag> = true;
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
    constexpr auto as_string(WindowFlag flag) noexcept -> string_view {
        using Pair                    = std::pair<WindowFlag, string_view>;
        static constexpr auto MAPPING = core::generate_substitutions_as_string_for<WindowFlag, 4, WindowFlag::DEFAULT, 67>(
          "WindowFlag::",
          {
            Pair { WindowFlag::DEFAULT,          "DEFAULT"sv          },
            Pair { WindowFlag::BORDERLESS,       "BORDERLESS"sv       },
            Pair { WindowFlag::RESIZEABLE,       "RESIZEABLE"sv       },
            Pair { WindowFlag::EXTERNAL_CONTEXT, "EXTERNAL_CONTEXT"sv },
        });

        const auto it = stdr::find_if(MAPPING, [&flag](auto&& pair) { return pair.first == flag; });
        ensures(it != stdr::cend(MAPPING));
        return it->second;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto to_string(WindowFlag flag) noexcept -> string {
        return string { as_string(flag) };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto as_string(EventType type) noexcept -> string_view {
        switch (type) {
            case EventType::NONE: return "EventType::NONE";
            case EventType::CLOSED: return "EventType::CLOSED";
            case EventType::RESIZED: return "EventType::RESIZED";
            case EventType::RESTORED: return "EventType::RESTORED";
            case EventType::MINIMIZED: return "EventType::MINIMIZED";
            case EventType::KEY_DOWN: return "EventType::KEY_DOWN";
            case EventType::KEY_UP: return "EventType::KEY_UP";
            case EventType::MOUSE_BUTTON_DOWN: return "EventType::MOUSE_BUTTON_DOWN";
            case EventType::MOUSE_BUTTON_UP: return "EventType::MOUSE_BUTTON_UP";
            case EventType::MOUSE_MOVED: return "EventType::MOUSE_MOVED";
            case EventType::ACTIVATE: return "EventType::ACTIVATE";
            case EventType::DEACTIVATE: return "EventType::DEACTIVATE";
            default: break;
        }

        std::unreachable();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto to_string(EventType type) noexcept -> string {
        return string { as_string(type) };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<EventCallbackFunc T>
    STORMKIT_FORCE_INLINE
    inline auto Window::on(T&& callback) noexcept -> void {
        if constexpr (meta::IsCanonical<ClosedEventFunc, T>) on_closed(std::forward<T>(callback));
        else if constexpr (meta::IsCanonical<MonitorChangedEventFunc, T>)
            on_monitor_changed(std::forward<T>(callback));
        else if constexpr (meta::IsCanonical<ResizedEventFunc, T>)
            on_resized(std::forward<T>(callback));
        else if constexpr (meta::IsCanonical<RestoredEventFunc, T>)
            on_restored(std::forward<T>(callback));
        else if constexpr (meta::IsCanonical<MinimizedEventFunc, T>)
            on_minimized(std::forward<T>(callback));
        else if constexpr (meta::IsCanonical<KeyDownEventFunc, T>)
            on_key_down(std::forward<T>(callback));
        else if constexpr (meta::IsCanonical<KeyUpEventFunc, T>)
            on_key_up(std::forward<T>(callback));
        else if constexpr (meta::IsCanonical<MouseButtonDownEventFunc, T>)
            on_mouse_button_down(std::forward<T>(callback));
        else if constexpr (meta::IsCanonical<MouseButtonUpEventFunc, T>)
            on_mouse_button_up(std::forward<T>(callback));
        else if constexpr (meta::IsCanonical<MouseMovedEventFunc, T>)
            on_mouse_moved(std::forward<T>(callback));
        else if constexpr (meta::IsCanonical<ActivateEventFunc, T>)
            on_activate(std::forward<T>(callback));
        else if constexpr (meta::IsCanonical<DeactivateEventFunc, T>)
            on_deactivate(std::forward<T>(callback));
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<EventCallbackFunc... Ts>
        requires(sizeof...(Ts) >= 2)
    STORMKIT_FORCE_INLINE
    inline auto Window::on(Ts&&... callbacks) noexcept -> void {
        (on(std::forward<Ts>(callbacks)), ...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<EventType TYPE, EventCallbackFunc T>
    STORMKIT_FORCE_INLINE
    inline auto Window::on(T&& callback) noexcept -> void {
        if constexpr (TYPE == EventType::CLOSED) on_closed(std::forward<T>(callback));
        else if constexpr (TYPE == EventType::MONITOR_CHANGED)
            on_monitor_changed(std::forward<T>(callback));
        else if constexpr (TYPE == EventType::RESIZED)
            on_resized(std::forward<T>(callback));
        else if constexpr (TYPE == EventType::RESTORED)
            on_restored(std::forward<T>(callback));
        else if constexpr (TYPE == EventType::MINIMIZED)
            on_minimized(std::forward<T>(callback));
        else if constexpr (TYPE == EventType::KEY_DOWN)
            on_key_down(std::forward<T>(callback));
        else if constexpr (TYPE == EventType::KEY_UP)
            on_key_up(std::forward<T>(callback));
        else if constexpr (TYPE == EventType::MOUSE_BUTTON_DOWN)
            on_mouse_button_down(std::forward<T>(callback));
        else if constexpr (TYPE == EventType::MOUSE_BUTTON_UP)
            on_mouse_button_up(std::forward<T>(callback));
        else if constexpr (TYPE == EventType::MOUSE_MOVED)
            on_mouse_moved(std::forward<T>(callback));
        else if constexpr (TYPE == EventType::ACTIVATE)
            on_activate(std::forward<T>(callback));
        else if constexpr (TYPE == EventType::DEACTIVATE)
            on_deactivate(std::forward<T>(callback));
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::toggle_fullscreen() noexcept -> void {
        set_fullscreen(not fullscreen());
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::unconfine_mouse(u8 mouse_id) noexcept -> void {
        confine_mouse(false, mouse_id);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::toggle_confined_mouse(u8 mouse_id) noexcept -> void {
        confine_mouse(not is_mouse_confined(mouse_id), mouse_id);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::unlock_mouse(u8 mouse_id) noexcept -> void {
        lock_mouse(false, mouse_id);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::toggle_locked_mouse(u8 mouse_id) noexcept -> void {
        lock_mouse(not is_mouse_locked(mouse_id), mouse_id);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::unhide_mouse(u8 mouse_id) noexcept -> void {
        hide_mouse(false, mouse_id);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::toggle_hidden_mouse(u8 mouse_id) noexcept -> void {
        hide_mouse(not is_mouse_hidden(mouse_id), mouse_id);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::toggle_relative_mouse(u8 mouse_id) noexcept -> void {
        set_relative_mouse(not is_mouse_relative(mouse_id), mouse_id);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::toggle_key_repeat(u8 keyboard_id) noexcept -> void {
        set_key_repeat(not is_key_repeat_enabled(keyboard_id), keyboard_id);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::hide_virtual_keyboard() noexcept -> void {
        show_virtual_keyboard(false);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::toggle_virtual_keyboard() noexcept -> void {
        show_virtual_keyboard(not is_virtual_keyboard_visible());
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::wm() const noexcept -> WM {
        return m_wm;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::event_loop() noexcept -> void {
        event_loop(monadic::noop());
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::event_loop(std::invocable<> auto&& func) noexcept -> void {
        while (is_open()) {
            func();

            handle_events();
        }
    }
} // namespace stormkit::wsi
