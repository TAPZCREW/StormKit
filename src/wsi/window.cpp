// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
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
    class window_impl: public impl::window {
      public:
        using impl::window::window;
    };

    /////////////////////////////////////
    /////////////////////////////////////
    window::window() noexcept : m_wm { wsi::wm() }, m_impl { m_wm } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    window::~window() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    window::window(window&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::operator=(window&&) noexcept -> window& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::open(string&& title, meta::in<math::uextent2> size, window_flag flags) noexcept -> window {
        auto window = wsi::window {};
        window.m_impl->open(std::move(title), size, flags);
        return window;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::allocate_and_open(string title, meta::in<math::uextent2> size, window_flag flags) noexcept -> heap_ptr<window> {
        auto window = allocate_unsafe<wsi::window>(wsi::window {});
        window->m_impl->open(std::move(title), size, flags);
        return window;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::close() noexcept -> void {
        m_impl->close();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::clear(meta::in<ucolor_rgb> color) noexcept -> void {
        m_impl->clear(color);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::fill_framebuffer(array_view<const ucolor_rgb> colors) noexcept -> void {
        m_impl->fill_framebuffer(colors);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::is_open() const noexcept -> bool {
        return m_impl->is_open();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::handle_events() noexcept -> void {
        m_impl->handle_events();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::visible() const noexcept -> bool {
        return m_impl->visible();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::current_monitor() const noexcept -> const monitor& {
        return m_impl->current_monitor();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::set_title(string&& title) noexcept -> void {
        m_impl->set_title(std::move(title));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::title() const noexcept -> const string& {
        return m_impl->title();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::set_extent(meta::in<math::uextent2> extent) noexcept -> void {
        m_impl->set_extent(extent);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::extent() const noexcept -> const math::uextent2& {
        return m_impl->extent();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::set_fullscreen(bool fullscreen) noexcept -> void {
        m_impl->set_fullscreen(fullscreen);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::fullscreen() const noexcept -> bool {
        return m_impl->fullscreen();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::confine_mouse(bool confine, u8 mouse_id) noexcept -> void {
        m_impl->confine_mouse(confine, mouse_id);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::is_mouse_confined(u8 mouse_id) const noexcept -> bool {
        return m_impl->is_mouse_confined(mouse_id);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::lock_mouse(bool locked, u8 mouse_id) noexcept -> void {
        m_impl->lock_mouse(locked, mouse_id);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::is_mouse_locked(u8 mouse_id) const noexcept -> bool {
        return m_impl->is_mouse_locked(mouse_id);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::hide_mouse(bool hidden, u8 mouse_id) noexcept -> void {
        m_impl->hide_mouse(hidden, mouse_id);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::is_mouse_hidden(u8 mouse_id) const noexcept -> bool {
        return m_impl->is_mouse_hidden(mouse_id);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::set_relative_mouse(bool enabled, u8 mouse_id) noexcept -> void {
        m_impl->set_relative_mouse(enabled, mouse_id);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::is_mouse_relative(u8 mouse_id) const noexcept -> bool {
        return m_impl->is_mouse_relative(mouse_id);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::set_key_repeat(bool enabled, u8 keyboard_id) noexcept -> void {
        return m_impl->set_key_repeat(enabled, keyboard_id);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::is_key_repeat_enabled(u8 keyboard_id) const noexcept -> bool {
        return m_impl->is_key_repeat_enabled(keyboard_id);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::show_virtual_keyboard(bool visible) noexcept -> void {
        m_impl->show_virtual_keyboard(visible);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::is_virtual_keyboard_visible() const noexcept -> bool {
        return m_impl->is_virtual_keyboard_visible();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::set_mouse_position(math::ivec2 position, u8 mouse_id) noexcept -> void {
        m_impl->set_mouse_position(position, mouse_id);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::native_handle() const noexcept -> native_handle_type {
        return m_impl->native_handle();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::on_closed(closed_event_cb_type&& callback) noexcept -> void {
        m_impl->set_closed_event(std::move(callback));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::on_monitor_changed(monitor_changed_event_cb_type&& callback) noexcept -> void {
        m_impl->set_monitor_changed_event(std::move(callback));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::on_resized(resized_event_cb_type&& callback) noexcept -> void {
        m_impl->set_resized_event(std::move(callback));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::on_restored(restored_event_cb_type&& callback) noexcept -> void {
        m_impl->set_restored_event(std::move(callback));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::on_minimized(minimized_event_cb_type&& callback) noexcept -> void {
        m_impl->set_minimized_event(std::move(callback));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::on_key_down(key_down_event_cb_type&& callback) noexcept -> void {
        m_impl->set_key_down_event(std::move(callback));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::on_key_up(key_up_event_cb_type&& callback) noexcept -> void {
        m_impl->set_key_up_event(std::move(callback));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::on_mouse_button_down(mouse_button_down_event_cb_type&& callback) noexcept -> void {
        m_impl->set_mouse_button_down_event(std::move(callback));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::on_mouse_button_up(mouse_button_up_event_cb_type&& callback) noexcept -> void {
        m_impl->set_mouse_button_up_event(std::move(callback));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::on_mouse_moved(mouse_moved_event_cb_type&& callback) noexcept -> void {
        m_impl->set_mouse_moved_event(std::move(callback));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::on_activate(activate_event_cb_type&& callback) noexcept -> void {
        m_impl->set_activate_event(std::move(callback));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::on_deactivate(deactivate_event_cb_type&& callback) noexcept -> void {
        m_impl->set_deactivate_event(std::move(callback));
    }
} // namespace stormkit::wsi
