// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/Core/PlatformMacro.hpp> // can't use headerunit because macros won't be defined before the import

module stormkit.Wsi;

import std;

import stormkit.core;

#if defined(STORMKIT_OS_WINDOWS)
import :Win32.WindowImpl;

namespace stormkit::wsi {
    class WindowImpl: public win32::WindowImpl {
      public:
        using win32::WindowImpl::WindowImpl;
    };
} // namespace stormkit::wsi
#elif defined(STORMKIT_OS_LINUX)
import :Linux.WindowImpl;

namespace stormkit::wsi {
    class WindowImpl: public linux::WindowImpl {
      public:
        using linux::WindowImpl::WindowImpl;
    };
} // namespace stormkit::wsi
#elif defined(STORMKIT_OS_MACOS)
import :MacOS.WindowImpl;

namespace stormkit::wsi {
    class WindowImpl: public macos::WindowImpl {
      public:
        using macos::WindowImpl::WindowImpl;
    };
} // namespace stormkit::wsi
#elif defined(STORMKIT_OS_IOS)
import :IOS.WindowImpl;

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

    auto parseArgs(std::span<const std::string_view> args) noexcept -> void {
        auto hint
            = std::ranges::find_if(args, [](auto&& v) { return v == "--x11" or v == "--wayland"; });

        if (hint != std::ranges::cend(args)) {
            if (*hint == "--x11") wm_hint = WM::X11;
            else if (*hint == "--wayland")
                wm_hint = WM::Wayland;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    Window::Window() noexcept : m_impl { wm() } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    Window::Window(std::string title, const math::ExtentU& size, WindowStyle style) noexcept
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
    auto Window::create(std::string title, const math::ExtentU& size, WindowStyle style) noexcept
        -> void {
        m_impl->create(std::move(title), size, style);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::close() noexcept -> void {
        m_impl->close();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::pollEvent(Event& event) noexcept -> bool {
        return m_impl->pollEvent(event);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::waitEvent(Event& event) noexcept -> bool {
        return m_impl->waitEvent(event);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::setTitle(std::string title) noexcept -> void {
        m_impl->setTitle(std::move(title));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::setExtent(const math::ExtentU& extent) noexcept -> void {
        m_impl->setExtent(extent);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::setFullscreenEnabled(bool fullscreen) noexcept -> void {
        m_impl->setFullscreenEnabled(fullscreen);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::lockMouse() noexcept -> void {
        m_impl->lockMouse();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::unlockMouse() noexcept -> void {
        m_impl->unlockMouse();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::hideMouse() noexcept -> void {
        m_impl->hideMouse();
    }

    /////////////////////////////////////
    /////////////////////////////////////
#ifdef STORMKIT_OS_MACOS
    auto Window::extent() const noexcept -> math::ExtentU {
#else
    auto Window::extent() const noexcept -> const math::ExtentU& {
#endif
        return m_impl->extent();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::isOpen() const noexcept -> bool {
        return m_impl->isOpen();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::visible() const noexcept -> bool {
        return m_impl->visible();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::nativeHandle() const noexcept -> NativeHandle {
        return m_impl->nativeHandle();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    WM Window::wm() noexcept {
#if defined(STORMKIT_OS_WINDOWS)
        return WM::Win32;
#elif defined(STORMKIT_OS_MACOS)
        return WM::macOS;
#elif defined(STORMKIT_OS_IOS)
    return WM::iOS;
#elif defined(STORMKIT_OS_ANDROID)
    return WM::Android;
#elif defined(STORMKIT_OS_SWITCH)
    return WM::Switch;
#elif defined(STORMKIT_OS_LINUX)
    auto is_wayland = std::getenv("WAYLAND_DISPLAY") != nullptr;

    if (wm_hint) return wm_hint.value();
    else if (is_wayland)
        return WM::Wayland;
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
    auto Window::mouseLocked() const noexcept -> bool {
        return m_impl->mouseLocked();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::fullscreen() const noexcept -> bool {
        return m_impl->fullscreen();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::setKeyRepeatEnabled(bool enabled) noexcept -> void {
        return m_impl->setKeyRepeatEnabled(enabled);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::keyRepeatEnabled() const noexcept -> bool {
        return m_impl->keyRepeatEnabled();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::setVirtualKeyboardVisible(bool visible) noexcept -> void {
        m_impl->setVirtualKeyboardVisible(visible);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::setMousePosition(const math::Vector2I& position) noexcept -> void {
        m_impl->setMousePosition(position);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::setMousePositionOnDesktop(const math::Vector2U& position) noexcept -> void {
        WindowImpl::setMousePositionOnDesktop(wm(), position);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::getMonitorSettings() -> std::vector<Monitor> {
        return WindowImpl::getMonitorSettings(wm());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::getPrimaryMonitorSettings() -> Monitor {
        const auto settings = getMonitorSettings();

        const auto it = std::ranges::find_if(settings, [](const auto& monitor) {
            return checkFlag(monitor.flags, Monitor::Flags::Primary);
        });

        ensures(it != std::ranges::cend(settings));

        return *it;
    }

} // namespace stormkit::wsi
