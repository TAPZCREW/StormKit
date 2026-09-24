// Copyright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#ifndef window_HPP
#define window_HPP

#include <string>

#include <storm/core/Platform.hpp>

#include <storm/window/AbstractWindow.hpp>

#ifdef __OBJC__
@class UIWindow;
using UIWindowPtr = UIWindow*;
@class ViewController;
using ViewControllerPtr = ViewController*;
@class View;
using ViewPtr = View*;
#else
using UIWindowPtr       = struct objc_class*;
using ViewControllerPtr = struct objc_class*;
using ViewPtr           = struct objc_class*;
#endif

namespace storm::window {
    class STORMKIT_PRIVATE Window: public storm::window::AbstractWindow {
      public:
        Window() noexcept;
        Window(const string&                  title,
                   const storm::window::VideoSettings& settings,
                   storm::window::WindowStyle          style) noexcept;
        ~Window() override;

        void create(const string&                  title,
                    const storm::window::VideoSettings& settings,
                    storm::window::WindowStyle          style) noexcept override;
        void close() noexcept override;

        bool poll_event(storm::window::Event& event, void* native_event) noexcept override;
        bool wait_event(storm::window::Event& event, void* native_event) noexcept override;

        void set_title(const string& title) noexcept override;
        void setVideoSettings(const storm::window::VideoSettings& settings) noexcept override;

        storm::core::extentu size() const noexcept override;

        bool is_open() const noexcept override;
        bool isVisible() const noexcept override;

        storm::window::NativeHandle native_handle() const noexcept override;

      private:
        UIWindowPtr       m_window;
        ViewControllerPtr m_view_controller;
        ViewPtr           m_view;
    };
} // namespace storm::window

#endif
