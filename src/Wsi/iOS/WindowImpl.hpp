// Copyright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#ifndef WINDOW_IMPL_HPP
#define WINDOW_IMPL_HPP

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
    class STORMKIT_PRIVATE WindowImpl: public storm::window::AbstractWindow {
      public:
        WindowImpl() noexcept;
        WindowImpl(const std::string&                  title,
                   const storm::window::VideoSettings& settings,
                   storm::window::WindowStyle          style) noexcept;
        ~WindowImpl() override;

        void create(const std::string&                  title,
                    const storm::window::VideoSettings& settings,
                    storm::window::WindowStyle          style) noexcept override;
        void close() noexcept override;

        bool pollEvent(storm::window::Event& event, void* native_event) noexcept override;
        bool waitEvent(storm::window::Event& event, void* native_event) noexcept override;

        void setTitle(const std::string& title) noexcept override;
        void setVideoSettings(const storm::window::VideoSettings& settings) noexcept override;

        storm::core::Extentu size() const noexcept override;

        bool isOpen() const noexcept override;
        bool isVisible() const noexcept override;

        storm::window::NativeHandle nativeHandle() const noexcept override;

      private:
        UIWindowPtr       m_window;
        ViewControllerPtr m_view_controller;
        ViewPtr           m_view;
    };
} // namespace storm::window

#endif
