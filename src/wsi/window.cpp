// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#include <stormkit/core/contract_macro.hpp>

module stormkit.wsi;

import std;

import stormkit.core;

#if defined(STORMKIT_OS_WINDOWS)
import :win32.window_impl;

namespace stormkit::wsi {
    class WindowImpl: public win32::WindowImpl {
      public:
        using win32::WindowImpl::WindowImpl;
    };
} // namespace stormkit::wsi
#elif defined(STORMKIT_OS_LINUX)
import :linux.window_impl;

namespace stormkit::wsi {
    class WindowImpl: public linux::WindowImpl {
      public:
        using linux::WindowImpl::WindowImpl;
    };
} // namespace stormkit::wsi
#elif defined(STORMKIT_OS_MACOS)
import :macos.window_impl;

namespace stormkit::wsi {
    class WindowImpl: public macos::WindowImpl {
      public:
        using macos::WindowImpl::WindowImpl;
    };
} // namespace stormkit::wsi
#elif defined(STORMKIT_OS_IOS)
import :ios.window_impl;

namespace stormkit::wsi {
    class WindowImpl: public ios::WindowImpl {
      public:
        using ios::WindowImpl::WindowImpl;
    };
} // namespace stormkit::wsi
#else
    #error "OS not supported !"
#endif

using namespace std::literals;

namespace stormkit::wsi {
    namespace {
        auto wm_hint = std::optional<WM> {};
    }

    auto parse_args(std::span<const std::string_view> args) noexcept -> void {
        auto hint = std::ranges::find_if(args, [](auto&& v) {
            return v == "--x11" or v == "--wayland";
        });

        if (hint != std::ranges::cend(args)) {
            if (*hint == "--x11") wm_hint = WM::X11;
            else if (*hint == "--wayland")
                wm_hint = WM::WAYLAND;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    Window::Window() noexcept : m_impl { wm() } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    Window::Window(std::string title, const math::Extent2<u32>& size, WindowStyle style) noexcept
        : m_impl { wm() } {
        create(std::move(title), size, style);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    Window::~Window() = default;

    /////////////////////////////////////
    /////////////////////////////////////
    Window::Window(Window&&) = default;

    /////////////////////////////////////
    /////////////////////////////////////
    Window& Window::operator=(Window&&) = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::create(std::string               title,
                        const math::Extent2<u32>& size,
                        WindowStyle               style) noexcept -> void {
        m_impl->create(std::move(title), size, style);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::close() noexcept -> void {
        m_impl->close();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::poll_event(Event& event) noexcept -> bool {
        return m_impl->poll_event(event);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::wait_event(Event& event) noexcept -> bool {
        return m_impl->wait_event(event);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_title(std::string title) noexcept -> void {
        m_impl->set_title(std::move(title));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_extent(const math::Extent2<u32>& extent) noexcept -> void {
        m_impl->set_extent(extent);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::toggle_fullscreen(bool fullscreen) noexcept -> void {
        m_impl->toggle_fullscreen(fullscreen);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::lock_mouse() noexcept -> void {
        m_impl->lock_mouse();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::unlock_mouse() noexcept -> void {
        m_impl->unlock_mouse();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::hide_mouse() noexcept -> void {
        m_impl->hide_mouse();
    }

    /////////////////////////////////////
    /////////////////////////////////////
#ifdef STORMKIT_OS_MACOS
    auto Window::extent() const noexcept -> math::Extent2<u32> {
#else
    auto Window::extent() const noexcept -> const math::Extent2<u32>& {
#endif
        return m_impl->extent();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::is_open() const noexcept -> bool {
        return m_impl->is_open();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::visible() const noexcept -> bool {
        return m_impl->visible();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::native_handle() const noexcept -> NativeHandle {
        return m_impl->native_handle();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    WM Window::wm() noexcept {
#if defined(STORMKIT_OS_WINDOWS)
        return WM::WIN32;
#elif defined(STORMKIT_OS_MACOS)
        return WM::MACOS;
#elif defined(STORMKIT_OS_IOS)
    return WM::IOS;
#elif defined(STORMKIT_OS_ANDROID)
    return WM::ANDROID;
#elif defined(STORMKIT_OS_SWITCH)
    return WM::SWITCH;
#elif defined(STORMKIT_OS_LINUX)
    auto is_wayland = std::getenv("WAYLAND_DISPLAY") != nullptr;

    if (wm_hint) return wm_hint.value();
    else if (is_wayland)
        return WM::WAYLAND;
    else
        return WM::X11;
#endif
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::title() const noexcept -> const std::string& {
        return m_impl->title();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::is_mouse_locked() const noexcept -> bool {
        return m_impl->is_mouse_locked();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::fullscreen() const noexcept -> bool {
        return m_impl->fullscreen();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::toggle_key_repeat(bool enabled) noexcept -> void {
        return m_impl->toggle_key_repeat(enabled);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::is_key_repeat_enabled() const noexcept -> bool {
        return m_impl->is_key_repeat_enabled();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::toggle_virtual_keyboard_visibility(bool visible) noexcept -> void {
        m_impl->toggle_virtual_keyboard_visibility(visible);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_mouse_position(const math::Vector2I& position) noexcept -> void {
        m_impl->set_mouse_position(position);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_mouse_position_on_desktop(const math::Vector2U& position) noexcept -> void {
        WindowImpl::set_mouse_position_on_desktop(wm(), position);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::get_monitor_settings() -> std::vector<Monitor> {
        return WindowImpl::get_monitor_settings(wm());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::get_primary_monitor_settings() -> Monitor {
        const auto settings = get_monitor_settings();

        const auto it = std::ranges::find_if(settings, [](const auto& monitor) {
            return check_flag_bit(monitor.flags, Monitor::Flags::PRIMARY);
        });

        ENSURES(it != std::ranges::cend(settings));

        return *it;
    }

} // namespace stormkit::wsi
