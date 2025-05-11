// Copyright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#import "macos_window.hpp"
#import "stormkit_application.h"
#import "stormkit_view.hpp"
#import "stormkit_window.hpp"

#import <Metal/Metal.h>

#import <QuartzCore/CAMetalLayer.h>

using namespace stormkit::wsi::macos;

namespace {
    auto from_std_string(const std::string &str) noexcept -> NSString * {
        return [NSString stringWithCString:str.c_str() encoding:[NSString defaultCStringEncoding]];
    }
} // namespace

@implementation StormKitWindowController {
    macOSWindow *requester;
    NSWindow *window;
    StormKitView *view;
}

/////////////////////////////////////
/////////////////////////////////////
- (id)initWithWidth:(std::uint32_t)width
         withHeight:(std::uint32_t)height
          withStyle:(int)style
          withTitle:(std::string)title
      withRequester:(macOSWindow *)_requester {
    self = [super init];

    if (self) {
        view   = nil;
        window = nil;

        auto window_style = NSUInteger();

        if ((style & 0b1) == 0b1) window_style |= NSWindowStyleMaskTitled;

        if ((style & 0b10) == 0b10) window_style |= NSWindowStyleMaskClosable;

        if ((style & 0b100) == 0b100) window_style |= NSWindowStyleMaskMiniaturizable;

        if ((style & 0b1000) == 0b1000) window_style |= NSWindowStyleMaskResizable;

        const auto rect = NSMakeRect(0, 0, width, height);
        window          = [[StormKitWindow alloc]
            initWithContentRect:rect
                      styleMask:window_style
                        backing:NSBackingStoreBuffered
                          defer:NO];

        auto frame = [window convertRectToBacking:[[window contentView] frame]];

        view =
            [[StormKitView alloc] initWithFrame:frame withRequester:_requester withWindow:window];
        [view setLayer:[CAMetalLayer layer]];
        [view setWantsLayer:YES];

        [window setReleasedWhenClosed:NO];
        [window makeFirstResponder:self];

        [window setContentView:view];
        [window setOpaque:YES];
        [window setTitle:from_std_string(title)];

        [window setDelegate:self];
        [window center];
        [window orderFrontRegardless];

        // if (fullscreen) [window toggleFullScreen:self];

        requester = _requester;
    }

    return self;
}

/////////////////////////////////////
/////////////////////////////////////
- (void)dealloc {
    [self close];

    window = nil;
    view   = nil;
}

/////////////////////////////////////
/////////////////////////////////////
- (void)setRequester:(macOSWindow *)_requester {
    requester = _requester;
    [view setRequester:requester];
}

/////////////////////////////////////
/////////////////////////////////////
- (void)close {
    [window close];
    [window setDelegate:nil];

    [self setRequester:nullptr];
}

/////////////////////////////////////
/////////////////////////////////////
- (BOOL)isVisible {
    return [window isVisible];
}

/////////////////////////////////////
/////////////////////////////////////
- (void)showWindow {
    [window orderOut:nil];
}

/////////////////////////////////////
/////////////////////////////////////
- (void)hideWindow {
    [window makeKeyAndOrderFront:nil];
}

/////////////////////////////////////
/////////////////////////////////////
- (void)processEvent {
    [StormKitApplication processEvent];

    // CGAssociateMouseAndMouseCursorPosition(TRUE);
}

/////////////////////////////////////
/////////////////////////////////////
- (StormKitView *)native_handle {
    return view;
}

/////////////////////////////////////
/////////////////////////////////////
- (NSSize)size {
    return [window contentRectForFrameRect:window.frame].size;
}

/////////////////////////////////////
/////////////////////////////////////
- (BOOL)windowShouldClose:(id)sender {
    return YES;
}

/////////////////////////////////////
/////////////////////////////////////
- (void)windowWillClose:(id)sender {
    requester->close_event();
}

/////////////////////////////////////
/////////////////////////////////////
- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize {
    requester->resize_event(frameSize.width, frameSize.height);

    return frameSize;
}

/////////////////////////////////////
/////////////////////////////////////
- (void)windowDidMiniaturize:(NSNotification *)notification {
    requester->minimize_event();
}

/////////////////////////////////////
/////////////////////////////////////
- (void)windowDidDeminiaturize:(NSNotification *)notification {
    requester->maximize_event();
}

/////////////////////////////////////
/////////////////////////////////////
- (void)set_mouse_position:(NSPoint)point {
    CGWarpMouseCursorPosition(point);
}

/////////////////////////////////////
/////////////////////////////////////
- (void)setWindowTitle:(std::string)title {
    [window setTitle:[NSString stringWithCString:title.c_str()
                                        encoding:[NSString defaultCStringEncoding]]];
}

/////////////////////////////////////
/////////////////////////////////////
- (NSPoint)convertPoint:(NSPoint)point {
    NSPoint pointInView = [view convertPoint:point fromView:nil];

    return pointInView;
}

@end

@implementation StormKitWindow

/////////////////////////////////////
/////////////////////////////////////
- (BOOL)acceptsFirstResponder {
    return YES;
}

/////////////////////////////////////
/////////////////////////////////////
- (BOOL)canBecomeKeyWindow {
    return YES;
}

/////////////////////////////////////
/////////////////////////////////////
- (void)performClose:(id)sender {
    [self close];
}

@end
