// Copyright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#ifndef MACOS_WINDOW_HPP
#define MACOS_WINDOW_HPP

#include <stormkit/core/platform_macro.hpp>

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#ifdef __OBJC__
@class StormKitWindowController;
using StormKitWindowControllerPtr = StormKitWindowController*;
@class StormKitView;
using ViewPtr = StormKitView*;
#else
using StormKitWindowControllerPtr = void*;
using ViewPtr                     = void*;
#endif

namespace stormkit::wsi::macos {
    class macOSWindow {
      public:
        struct Handles {
            ViewPtr view;
        };

        struct Monitor {
            enum class Flags {
                NONE    = 0,
                Primary = 1
            };

            Flags       flags = Flags::NONE;
            std::string name;

            struct Extent {
                std::uint32_t width;
                std::uint32_t height;
            };

            std::vector<Extent> extents;

            void* handle = nullptr;
        };

        macOSWindow(std::string   title,
                    std::uint32_t width,
                    std::uint32_t height,
                    int           style) noexcept;
        ~macOSWindow();

        macOSWindow(const macOSWindow&) noexcept                    = delete;
        auto operator=(const macOSWindow&) noexcept -> macOSWindow& = delete;

        macOSWindow(macOSWindow&&) noexcept;
        auto operator=(macOSWindow&&) noexcept -> macOSWindow&;

        auto pollEvent() noexcept -> void;
        auto waitEvent() noexcept -> void;

        auto setTitle(std::string title) noexcept -> void;

        auto setExtent(std::uint32_t width, std::uint32_t height) noexcept -> void;

        auto setFullscreenEnabled(bool enabled) noexcept -> void;

        auto lockMouse() noexcept -> void;
        auto unlockMouse() noexcept -> void;

        auto hideMouse() noexcept -> void;
        auto unhideMouse() noexcept -> void;

        [[nodiscard]] auto width() const noexcept -> std::uint32_t;
        [[nodiscard]] auto height() const noexcept -> std::uint32_t;
        [[nodiscard]] auto title() const noexcept -> const std::string&;

        [[nodiscard]] auto visible() const noexcept -> bool;

        [[nodiscard]] auto macOSHandles() const noexcept -> const Handles&;

        [[nodiscard]] auto mouseLocked() const noexcept -> bool;
        [[nodiscard]] auto mouseHided() const noexcept -> bool;
        [[nodiscard]] auto fullscreen() const noexcept -> bool;

        auto               setKeyRepeatEnabled(bool enabled) noexcept -> void;
        [[nodiscard]] auto keyRepeatEnabled() const noexcept -> bool;

        auto setVirtualKeyboardVisible(bool visible) noexcept -> void;

        auto        setMousePosition(std::int32_t width, std::int32_t height) noexcept -> void;
        static auto setMousePositionOnDesktop(std::uint32_t width, std::uint32_t height) noexcept
            -> void;

        [[nodiscard]] static auto getMonitorSettings() -> std::vector<Monitor>;

        std::function<void(int, std::int32_t, std::int32_t)> mouseDownEvent;
        std::function<void(int, std::int32_t, std::int32_t)> mouseUpEvent;

        std::function<void(std::int32_t, std::int32_t)> mouseMoveEvent;

        std::function<void()> mouseEnteredEvent;
        std::function<void()> mouseExitedEvent;

        std::function<void(int, char)> keyDownEvent;
        std::function<void(int, char)> keyUpEvent;

        std::function<void()> closeEvent;

        std::function<void(std::uint32_t, std::uint32_t)> resizeEvent;
        std::function<void()>                             minimizeEvent;
        std::function<void()>                             maximizeEvent;

        std::function<void()> lostFocusEvent;
        std::function<void()> gainedFocusEvent;

      private:
        StormKitWindowControllerPtr m_controller;

        Handles m_handles;

        std::string m_title;

        std::uint32_t m_width  = 0;
        std::uint32_t m_height = 0;

        bool m_visible            = false;
        bool m_mouse_locked       = false;
        bool m_mouse_hided        = false;
        bool m_fullscreen         = false;
        bool m_key_repeat_enabled = false;
    };

    auto initCocoaProcess() -> void;
} // namespace stormkit::wsi::macos

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::wsi::macos {
    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE auto macOSWindow::title() const noexcept -> const std::string& {
        return m_title;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE auto macOSWindow::visible() const noexcept -> bool {
        return m_visible;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE auto macOSWindow::mouseLocked() const noexcept -> bool {
        return m_mouse_locked;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE auto macOSWindow::mouseHided() const noexcept -> bool {
        return m_mouse_hided;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE auto macOSWindow::fullscreen() const noexcept -> bool {
        return m_fullscreen;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE auto macOSWindow::keyRepeatEnabled() const noexcept -> bool {
        return m_key_repeat_enabled;
    }
} // namespace stormkit::wsi::macos

#endif
