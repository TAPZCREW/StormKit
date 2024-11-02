// Copyright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#import "macOSWindow.hpp"
#import "StormKitApplication.h"
#import "StormKitWindow.hpp"

#import <AppKit/NSScreen.h>
#import <IOKit/graphics/IOGraphicsLib.h>

#include <bit>
#include <cstdint>
#include <string>
#include <vector>

static io_service_t IOServicePortFromCGDisplayID(CGDirectDisplayID displayID) {
    io_iterator_t iter;
    io_service_t serv, servicePort = 0;

    CFMutableDictionaryRef matching = IOServiceMatching("IODisplayConnect");

    // releases matching for us
    kern_return_t err = IOServiceGetMatchingServices(kIOMainPortDefault, matching, &iter);
    if (err) return 0;

    while ((serv = IOIteratorNext(iter)) != 0) {
        CFDictionaryRef info;
        CFIndex vendorID, productID, serialNumber;
        CFNumberRef vendorIDRef, productIDRef, serialNumberRef;
        Boolean success;

        info = IODisplayCreateInfoDictionary(serv, kIODisplayOnlyPreferredName);

        vendorIDRef
            = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(info, CFSTR(kDisplayVendorID)));
        productIDRef
            = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(info, CFSTR(kDisplayProductID)));
        serialNumberRef = reinterpret_cast<CFNumberRef>(
            CFDictionaryGetValue(info, CFSTR(kDisplaySerialNumber)));

        success = CFNumberGetValue(vendorIDRef, kCFNumberCFIndexType, &vendorID);
        success &= CFNumberGetValue(productIDRef, kCFNumberCFIndexType, &productID);
        success &= CFNumberGetValue(serialNumberRef, kCFNumberCFIndexType, &serialNumber);

        if (!success) {
            CFRelease(info);
            continue;
        }

        // If the vendor and product id along with the serial don't match
        // then we are not looking at the correct monitor.
        // NOTE: The serial number is important in cases where two monitors
        //       are the exact same.
        if (CGDisplayVendorNumber(displayID) != vendorID
            || CGDisplayModelNumber(displayID) != productID
            || CGDisplaySerialNumber(displayID) != serialNumber) {
            CFRelease(info);
            continue;
        }

        // The VendorID, Product ID, and the Serial Number all Match Up!
        // Therefore we have found the appropriate display io_service
        servicePort = serv;
        CFRelease(info);
        break;
    }

    IOObjectRelease(iter);
    return servicePort;
}

namespace stormkit::wsi::macos {
    namespace {
        static auto is_process_set_as_application = false;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    macOSWindow::macOSWindow(std::string title,
                             std::uint32_t width,
                             std::uint32_t height,
                             int style) noexcept
        : m_controller { nil }, m_handles { .view = nil }, m_title { std::move(title) },
          m_width { width }, m_height { height } {
        initCocoaProcess();

        m_controller = [[StormKitWindowController alloc]
            initWithWidth:width
               withHeight:height
                withStyle:static_cast<int>(style)
                withTitle:m_title
            withRequester:this];

        m_handles.view = [m_controller nativeHandle];
    }

    /////////////////////////////////////
    /////////////////////////////////////
    macOSWindow::~macOSWindow() {
        [m_controller close];
    }

    /////////////////////////////////////
    /////////////////////////////////////
    macOSWindow::macOSWindow(macOSWindow &&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto macOSWindow::operator=(macOSWindow &&) noexcept -> macOSWindow & = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto macOSWindow::pollEvent() noexcept -> void {
        [m_controller processEvent];
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto macOSWindow::waitEvent() noexcept -> void {
        [m_controller processEvent];
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto macOSWindow::setTitle(std::string title) noexcept -> void {
        m_title = std::move(title);

        [m_controller setWindowTitle:m_title];
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto macOSWindow::setExtent(std::uint32_t width, std::uint32_t height) noexcept -> void {
        m_width  = width;
        m_height = height;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto macOSWindow::setFullscreenEnabled(bool) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto macOSWindow::lockMouse() noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto macOSWindow::unlockMouse() noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto macOSWindow::hideMouse() noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto macOSWindow::unhideMouse() noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto macOSWindow::width() const noexcept -> std::uint32_t {
        auto size = [m_controller size];

        return size.width;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto macOSWindow::height() const noexcept -> std::uint32_t {
        auto size = [m_controller size];

        return size.height;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto macOSWindow::macOSHandles() const noexcept -> const Handles & {
        return m_handles;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto macOSWindow::setKeyRepeatEnabled([[maybe_unused]] bool enabled) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto macOSWindow::setVirtualKeyboardVisible([[maybe_unused]] bool visible) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto macOSWindow::setMousePosition([[maybe_unused]] std::int32_t x,
                                       [[maybe_unused]] std::int32_t y) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto macOSWindow::setMousePositionOnDesktop([[maybe_unused]] std::uint32_t x,
                                                [[maybe_unused]] std::uint32_t y) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto macOSWindow::getMonitorSettings() -> std::vector<Monitor> {
        // TODO handle multiple monitors
        auto display_id  = CGMainDisplayID();
        auto modes       = (__bridge NSArray *)CGDisplayCopyAllDisplayModes(display_id, nullptr);
        auto device_info = (__bridge NSDictionary *)
            IODisplayCreateInfoDictionary(IOServicePortFromCGDisplayID(display_id),
                                          kIODisplayOnlyPreferredName);

        const auto mode_count = [modes count];

        auto monitors = std::vector<Monitor> {};
        auto &monitor = monitors.emplace_back();
        monitor.extents.reserve(mode_count);
        monitor.flags  = Monitor::Flags::Primary;
        monitor.handle = std::bit_cast<void *>(static_cast<std::intptr_t>(display_id));

        auto screen_name = @"";
        id localized_names =
            [device_info objectForKey:[NSString stringWithUTF8String:kDisplayProductName]];
        if ([localized_names count] > 0) [[likely]]
            screen_name =
                [localized_names objectForKey:[[localized_names allKeys] objectAtIndex:0]];

        monitor.name = [screen_name UTF8String];

        for (auto i = CFIndex { 0 }; i < static_cast<CFIndex>(mode_count); ++i) {
            auto mode = (__bridge CGDisplayModeRef)([modes objectAtIndex:i]);

            monitor.extents.emplace_back(CGDisplayModeGetWidth(mode), CGDisplayModeGetHeight(mode));
        }

        CFRelease(reinterpret_cast<CFTypeRef>(device_info));
        CFRelease(reinterpret_cast<CFTypeRef>(modes));
        return monitors;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto initCocoaProcess() -> void {
        if (!is_process_set_as_application) {
            [StormKitApplication sharedApplication];

            [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
            [NSApp activateIgnoringOtherApps:YES];

            if (![[StormKitApplication sharedApplication] delegate])
                [[StormKitApplication sharedApplication]
                    setDelegate:[[StormKitApplicationDelegate alloc] init]];

            [[StormKitApplication sharedApplication] finishLaunching];

            is_process_set_as_application = true;
        }
    }
} // namespace stormkit::wsi::macos
