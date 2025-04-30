#import "WindowImpl.hpp"
#import "View.h"
#import "ViewController.h"

#import <UIKit/UIApplication.h>
#import <UIKit/UIScreen.h>
#import <UIKit/UIWindow.h>

#include <storm/window/WindowStyle.hpp>

using namespace storm;
using namespace storm::window;

/////////////////////////////////////
/////////////////////////////////////
WindowImpl::WindowImpl() noexcept
    : AbstractWindow {}, m_window { nil }, m_view_controller { nil }, m_view { nil } {
}

/////////////////////////////////////
/////////////////////////////////////
WindowImpl::WindowImpl(const std::string &title,
                       const storm::window::VideoSettings &settings,
                       const storm::window::WindowStyle style) noexcept
    : WindowImpl {} {
    create(title, settings, style);
}

/////////////////////////////////////
/////////////////////////////////////
WindowImpl::~WindowImpl() {
}

/////////////////////////////////////
/////////////////////////////////////
void WindowImpl::create(const std::string &title,
                        const VideoSettings &settings,
                        WindowStyle style) noexcept {
    m_title          = title;
    m_video_settings = settings;
    m_style          = style;

    const auto scale_factor = [[UIScreen mainScreen] nativeScale];
    const auto screen_rect  = [UIScreen mainScreen].bounds;

    m_video_settings = VideoSettings::getDesktopModes()[0];

    m_window = [[UIWindow alloc] initWithFrame:screen_rect];

    m_view = [[View alloc] initWithFrame:screen_rect withScaleFactor:scale_factor];
    [m_view resignFirstResponder];

    m_view.backgroundColor = UIColor.redColor;

    m_view_controller = [ViewController alloc];
    m_view_controller.hideStatusBar
        = ((m_style & WindowStyle::Fullscreen) == WindowStyle::Fullscreen) ? YES : NO;
    m_view_controller.view = m_view;
    // m_view_controller.title = title;

    m_window.rootViewController = m_view_controller;
    [m_window makeKeyAndVisible];
}

/////////////////////////////////////
/////////////////////////////////////
void WindowImpl::close() noexcept {
}

/////////////////////////////////////
/////////////////////////////////////
bool WindowImpl::pollEvent(storm::window::Event &event, void *) noexcept {
    while (CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.0001, true) == kCFRunLoopRunHandledSource);

    return AbstractWindow::pollEvent(event);
}

/////////////////////////////////////
/////////////////////////////////////
bool WindowImpl::waitEvent(storm::window::Event &event, void *native_event) noexcept {
    ;
    while (!AbstractWindow::waitEvent(event, native_event));

    return true;
}

/////////////////////////////////////
/////////////////////////////////////
void WindowImpl::setTitle(const std::string &title) noexcept {
}

/////////////////////////////////////
/////////////////////////////////////
void WindowImpl::setVideoSettings(const VideoSettings &settings) noexcept {
}

/////////////////////////////////////
/////////////////////////////////////
storm::core::Extentu WindowImpl::size() const noexcept {
    return m_video_settings.size;
}

/////////////////////////////////////
/////////////////////////////////////
bool WindowImpl::isOpen() const noexcept {
    return true;
}

/////////////////////////////////////
/////////////////////////////////////
bool WindowImpl::isVisible() const noexcept {
    return true;
}

/////////////////////////////////////
/////////////////////////////////////
storm::window::NativeHandle WindowImpl::nativeHandle() const noexcept {
    return m_view;
}
