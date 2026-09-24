// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#include <stormkit/core/contract_macro.hpp>

module stormkit.wsi;

import std;

import stormkit.core;

import :window;

#if defined(STORMKIT_OS_WINDOWS)
import :win32.window;

namespace impl = stormkit::wsi::win32;
#elif defined(STORMKIT_OS_LINUX)
import :linux.window;

namespace impl = stormkit::wsi::linux;
#elif defined(STORMKIT_OS_MACOS)
import :macos.window;

namespace impl = stormkit::wsi::macos;
#elif defined(STORMKIT_OS_IOS)
import :ios.window;

namespace impl = stormkit::wsi::ios;
#else
    #error "OS not supported !"
#endif

using namespace std::literals;

namespace stormkit::wsi {
    class WindowImpl: public impl::Window {
      public:
        using impl::Window::Window;
    };

    /////////////////////////////////////
    /////////////////////////////////////
    Window::Window() noexcept : m_wm { wsi::wm() }, m_impl { m_wm } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    Window::~Window() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    Window::Window(Window&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::operator=(Window&&) noexcept -> Window& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::open(string title, const math::uextent2& size, WindowFlag flags) noexcept -> Window {
        auto window = Window {};
        window.m_impl->open(std::move(title), size, flags);
        return window;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::allocate_and_open(string title, const math::uextent2& size, WindowFlag flags) noexcept -> heap_ptr<Window> {
        auto window = allocate_unsafe<Window>(Window {});
        window->m_impl->open(std::move(title), size, flags);
        return window;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::close() noexcept -> void {
        m_impl->close();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::clear(const ucolor_rgb& color) noexcept -> void {
        m_impl->clear(color);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::fill_framebuffer(array_view<const ucolor_rgb> colors) noexcept -> void {
        m_impl->fill_framebuffer(colors);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::is_open() const noexcept -> bool {
        return m_impl->is_open();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::handle_events() noexcept -> void {
        m_impl->handle_events();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::visible() const noexcept -> bool {
        return m_impl->visible();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::current_monitor() const noexcept -> const Monitor& {
        return m_impl->current_monitor();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_title(string title) noexcept -> void {
        m_impl->set_title(std::move(title));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::title() const noexcept -> const string& {
        return m_impl->title();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_extent(const math::uextent2& extent) noexcept -> void {
        m_impl->set_extent(extent);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::extent() const noexcept -> const math::uextent2& {
        return m_impl->extent();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_fullscreen(bool fullscreen) noexcept -> void {
        m_impl->set_fullscreen(fullscreen);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::fullscreen() const noexcept -> bool {
        return m_impl->fullscreen();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::confine_mouse(bool confine, u8 mouse_id) noexcept -> void {
        m_impl->confine_mouse(confine, mouse_id);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::is_mouse_confined(u8 mouse_id) const noexcept -> bool {
        return m_impl->is_mouse_confined(mouse_id);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::lock_mouse(bool locked, u8 mouse_id) noexcept -> void {
        m_impl->lock_mouse(locked, mouse_id);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::is_mouse_locked(u8 mouse_id) const noexcept -> bool {
        return m_impl->is_mouse_locked(mouse_id);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::hide_mouse(bool hidden, u8 mouse_id) noexcept -> void {
        m_impl->hide_mouse(hidden, mouse_id);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::is_mouse_hidden(u8 mouse_id) const noexcept -> bool {
        return m_impl->is_mouse_hidden(mouse_id);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_relative_mouse(bool enabled, u8 mouse_id) noexcept -> void {
        m_impl->set_relative_mouse(enabled, mouse_id);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::is_mouse_relative(u8 mouse_id) const noexcept -> bool {
        return m_impl->is_mouse_relative(mouse_id);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_key_repeat(bool enabled, u8 keyboard_id) noexcept -> void {
        return m_impl->set_key_repeat(enabled, keyboard_id);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::is_key_repeat_enabled(u8 keyboard_id) const noexcept -> bool {
        return m_impl->is_key_repeat_enabled(keyboard_id);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::show_virtual_keyboard(bool visible) noexcept -> void {
        m_impl->show_virtual_keyboard(visible);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::is_virtual_keyboard_visible() const noexcept -> bool {
        return m_impl->is_virtual_keyboard_visible();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_mouse_position(const math::ivec2& position, u8 mouse_id) noexcept -> void {
        m_impl->set_mouse_position(position, mouse_id);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::native_handle() const noexcept -> NativeHandle {
        return m_impl->native_handle();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::on_closed(ClosedEventFunc&& callback) noexcept -> void {
        m_impl->set_closed_event(std::move(callback));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::on_monitor_changed(MonitorChangedEventFunc&& callback) noexcept -> void {
        m_impl->set_monitor_changed_event(std::move(callback));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::on_resized(ResizedEventFunc&& callback) noexcept -> void {
        m_impl->set_resized_event(std::move(callback));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::on_restored(RestoredEventFunc&& callback) noexcept -> void {
        m_impl->set_restored_event(std::move(callback));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::on_minimized(MinimizedEventFunc&& callback) noexcept -> void {
        m_impl->set_minimized_event(std::move(callback));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::on_key_down(KeyDownEventFunc&& callback) noexcept -> void {
        m_impl->set_key_down_event(std::move(callback));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::on_key_up(KeyUpEventFunc&& callback) noexcept -> void {
        m_impl->set_key_up_event(std::move(callback));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::on_mouse_button_down(MouseButtonDownEventFunc&& callback) noexcept -> void {
        m_impl->set_mouse_button_down_event(std::move(callback));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::on_mouse_button_up(MouseButtonUpEventFunc&& callback) noexcept -> void {
        m_impl->set_mouse_button_up_event(std::move(callback));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::on_mouse_moved(MouseMovedEventFunc&& callback) noexcept -> void {
        m_impl->set_mouse_moved_event(std::move(callback));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::on_activate(ActivateEventFunc&& callback) noexcept -> void {
        m_impl->set_activate_event(std::move(callback));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::on_deactivate(DeactivateEventFunc&& callback) noexcept -> void {
        m_impl->set_deactivate_event(std::move(callback));
    }
} // namespace stormkit::wsi
