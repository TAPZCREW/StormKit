// Copyright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#ifndef STORMKIT_WINDOW_CONTROLLER_HPP
#define STORMKIT_WINDOW_CONTROLLER_HPP

#include <cstdint>
#include <string>

#import <AppKit/NSWindow.h>

namespace stormkit::wsi::macos {
    class macOSWindow;
}

@class StormKitView;
@interface StormKitWindowController: NSResponder<NSWindowDelegate>
- (id)initWithWidth:(std::uint32_t)width
         withHeight:(std::uint32_t)height
          withStyle:(int)style
          withTitle:(std::string)title
      withRequester:(stormkit::wsi::macos::macOSWindow*)_requester;
- (void)dealloc;

- (void)setRequester:(stormkit::wsi::macos::macOSWindow*)requester;

- (void)close;
- (BOOL)isVisible;

- (void)showWindow;
- (void)hideWindow;

- (void)processEvent;
- (StormKitView*)native_handle;

- (NSSize)size;

- (BOOL)windowShouldClose:(id)sender;

- (void)windowWillClose:(id)sender;
- (NSSize)windowWillResize:(NSWindow*)sender toSize:(NSSize)frameSize;

- (void)windowDidMiniaturize:(NSNotification*)notification;
- (void)windowDidDeminiaturize:(NSNotification*)notification;

- (void)set_mouse_position:(NSPoint)point;
- (void)setWindowTitle:(std::string)title;

- (NSPoint)convertPoint:(NSPoint)point;
@end

@interface StormKitWindow: NSWindow
- (BOOL)acceptsFirstResponder;
- (BOOL)canBecomeKeyWindow;
- (void)performClose:(id)sender;
@end

#endif
