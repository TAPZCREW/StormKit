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
                PRIMARY = 1,
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

        auto poll_event() noexcept -> void;
        auto wait_event() noexcept -> void;

        auto set_title(std::string title) noexcept -> void;

        auto set_extent(std::uint32_t width, std::uint32_t height) noexcept -> void;

        auto toggle_fullscreen(bool enabled) noexcept -> void;

        auto lock_mouse() noexcept -> void;
        auto unlock_mouse() noexcept -> void;

        auto hide_mouse() noexcept -> void;
        auto unhide_mouse() noexcept -> void;

        [[nodiscard]]
        auto width() const noexcept -> std::uint32_t;
        [[nodiscard]]
        auto height() const noexcept -> std::uint32_t;
        [[nodiscard]]
        auto title() const noexcept -> const std::string&;

        [[nodiscard]]
        auto visible() const noexcept -> bool;

        [[nodiscard]]
        auto macos_handles() const noexcept -> const Handles&;

        [[nodiscard]]
        auto is_mouse_locked() const noexcept -> bool;
        [[nodiscard]]
        auto is_mouse_hided() const noexcept -> bool;
        [[nodiscard]]
        auto fullscreen() const noexcept -> bool;

        auto toggle_key_repeat(bool enabled) noexcept -> void;
        [[nodiscard]]
        auto is_key_repeat_enabled() const noexcept -> bool;

        auto toggle_virtual_keyboard_visibility(bool visible) noexcept -> void;

        auto        set_mouse_position(std::int32_t width, std::int32_t height) noexcept -> void;
        static auto set_mouse_position_on_desktop(std::uint32_t width,
                                                  std::uint32_t height) noexcept -> void;

        [[nodiscard]]
        static auto get_monitor_settings() -> std::vector<Monitor>;

        std::function<void(int, std::int32_t, std::int32_t)> mouse_down_event;
        std::function<void(int, std::int32_t, std::int32_t)> mouse_up_event;

        std::function<void(std::int32_t, std::int32_t)> mouse_move_event;

        std::function<void()> mouse_entered_event;
        std::function<void()> mouse_exited_event;

        std::function<void(int, char)> key_down_event;
        std::function<void(int, char)> key_up_event;

        std::function<void()> close_event;

        std::function<void(std::uint32_t, std::uint32_t)> resize_event;
        std::function<void()>                             minimize_event;
        std::function<void()>                             maximize_event;

        std::function<void()> lost_focus_event;
        std::function<void()> gained_focus_event;

      private:
        StormKitWindowControllerPtr m_controller;

        Handles m_handles;

        std::string m_title;

        std::uint32_t m_width  = 0;
        std::uint32_t m_height = 0;

        bool m_visible            = false;
        bool m_is_mouse_locked    = false;
        bool m_mouse_hided        = false;
        bool m_fullscreen         = false;
        bool m_key_repeat_enabled = false;
    };

    auto init_cocoa_process() -> void;
} // namespace stormkit::wsi::macos

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::wsi::macos {
    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto macOSWindow::title() const noexcept -> const std::string& {
        return m_title;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto macOSWindow::visible() const noexcept -> bool {
        return m_visible;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto macOSWindow::is_mouse_locked() const noexcept -> bool {
        return m_is_mouse_locked;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto macOSWindow::is_mouse_hided() const noexcept -> bool {
        return m_mouse_hided;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto macOSWindow::fullscreen() const noexcept -> bool {
        return m_fullscreen;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto macOSWindow::is_key_repeat_enabled() const noexcept -> bool {
        return m_key_repeat_enabled;
    }
} // namespace stormkit::wsi::macos

#endif
