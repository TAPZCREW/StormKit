// Copyright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#import "StormKitView.hpp"
#import "macOSWindow.hpp"

#include "Utils.hpp"

extern "C" {
#import <AppKit/NSScreen.h>
#import <AppKit/NSTrackingArea.h>
#import <AppKit/NSWindow.h>
}

using namespace stormkit::wsi::macos;

@implementation StormKitView {
    NSTrackingArea *tracking_area;
    macOSWindow *requester;
    NSWindow *window_handle;
    BOOL is_mouse_inside;
    void *native_event;
}

/////////////////////////////////////
/////////////////////////////////////
- (NSWindow *)myWindow {
    return window_handle;
}
/////////////////////////////////////
/////////////////////////////////////
- (BOOL)acceptsFirstResponder {
    return YES;
}

/////////////////////////////////////
/////////////////////////////////////
- (id)initWithFrame:(NSRect)frame
      withRequester:(macOSWindow *)_requester
         withWindow:(NSWindow *)window {
    self = [super initWithFrame:frame];

    window_handle   = window;
    is_mouse_inside = NO;
    native_event    = nullptr;
    requester       = _requester;

    return self;
}

/////////////////////////////////////
/////////////////////////////////////
- (void)setRequester:(macOSWindow *)_requester {
    requester = _requester;
}

/////////////////////////////////////
/////////////////////////////////////
- (void)mouseDown:(NSEvent *)event {
    /*if(native_event)
     *reinterpret_cast<NSEvent*>(native_event) = *event;*/

    [self handleMouseDown:event];

    [[self nextResponder] mouseDown:event];
}

/////////////////////////////////////
/////////////////////////////////////
- (void)rightMouseDown:(NSEvent *)event {
    [self handleMouseDown:event];

    [[self nextResponder] mouseDown:event];
}

/////////////////////////////////////
/////////////////////////////////////
- (void)otherMouseDown:(NSEvent *)event {
    [self handleMouseDown:event];

    [[self nextResponder] mouseDown:event];
}

/////////////////////////////////////
/////////////////////////////////////
- (void)handleMouseDown:(NSEvent *)event {
    requester->mouseDownEvent(mouseButton([event buttonNumber]),
                              event.locationInWindow.x,
                              event.locationInWindow.y);
}

/////////////////////////////////////
/////////////////////////////////////
- (void)mouseUp:(NSEvent *)event {
    [self handleMouseUp:event];

    [[self nextResponder] mouseUp:event];
}

/////////////////////////////////////
/////////////////////////////////////
- (void)rightMouseUp:(NSEvent *)event {
    [self handleMouseUp:event];

    [[self nextResponder] mouseUp:event];
}

/////////////////////////////////////
/////////////////////////////////////
- (void)otherMouseUp:(NSEvent *)event {
    [self handleMouseUp:event];

    [[self nextResponder] mouseUp:event];
}

/////////////////////////////////////
/////////////////////////////////////
- (void)handleMouseUp:(NSEvent *)event {
    requester->mouseUpEvent(mouseButton([event buttonNumber]),
                            event.locationInWindow.x,
                            event.locationInWindow.y);
}

/////////////////////////////////////
/////////////////////////////////////
- (void)mouseMoved:(NSEvent *)event {
    [self handleMouseMove:event];

    [[self nextResponder] mouseMoved:event];
}

/////////////////////////////////////
/////////////////////////////////////
- (void)mouseDragged:(NSEvent *)event {
    [self handleMouseMove:event];

    [[self nextResponder] mouseDragged:event];
}

/////////////////////////////////////
/////////////////////////////////////
- (void)rightMouseDragged:(NSEvent *)event {
    [self handleMouseMove:event];

    [[self nextResponder] rightMouseDragged:event];
}

/////////////////////////////////////
/////////////////////////////////////
- (void)handleMouseMove:(NSEvent *)event {
    if (is_mouse_inside) {
        auto point = event.locationInWindow;

        point.y = requester->height() - point.y;

        requester->mouseMoveEvent(point.x, point.y);
    }
}

/////////////////////////////////////
/////////////////////////////////////
- (void)mouseEntered:(NSEvent *)event {
    is_mouse_inside = YES;

    requester->mouseEnteredEvent();
}

/////////////////////////////////////
/////////////////////////////////////
- (void)mouseExited:(NSEvent *)event {
    is_mouse_inside = NO;

    requester->mouseExitedEvent();
}

/////////////////////////////////////
/////////////////////////////////////
- (void)keyDown:(NSEvent *)event {
    auto string = [event charactersIgnoringModifiers];
    auto key    = std::numeric_limits<std::uint8_t>::max();

    if ([string length] > 0) key = localizedKey([string characterAtIndex:0]);

    if (key == std::numeric_limits<std::uint8_t>::max()) key = nonLocalizedKey(event.keyCode);

    requester->keyDownEvent(key, [string characterAtIndex:0]);
}

/////////////////////////////////////
/////////////////////////////////////
- (void)keyUp:(NSEvent *)event {
    auto string = [event charactersIgnoringModifiers];
    auto key    = std::numeric_limits<std::uint8_t>::max();

    if ([string length] > 0) key = localizedKey([string characterAtIndex:0]);

    if (key == std::numeric_limits<std::uint8_t>::max()) key = nonLocalizedKey(event.keyCode);

    requester->keyUpEvent(key, [string characterAtIndex:0]);
}

/////////////////////////////////////
/////////////////////////////////////
- (void)updateTrackingAreas {
    [super updateTrackingAreas];
    if (tracking_area != nil) {
        [self removeTrackingArea:tracking_area];
        tracking_area = nil;
    }

    NSUInteger opts = (NSTrackingMouseEnteredAndExited | NSTrackingActiveAlways
                       | NSTrackingMouseMoved | NSTrackingEnabledDuringMouseDrag);
    tracking_area =
        [[NSTrackingArea alloc] initWithRect:[self bounds] options:opts owner:self userInfo:nil];
    [self addTrackingArea:tracking_area];
}

/////////////////////////////////////
/////////////////////////////////////
- (NSPoint)convertPointToScreen:(NSPoint)point {
    NSRect rect = NSZeroRect;
    rect.origin = point;
    rect        = [[self window] convertRectToScreen:rect];
    return rect.origin;
}

/////////////////////////////////////
/////////////////////////////////////
- (NSPoint)relativeToGlobal:(NSPoint)point {
    point.y = [self frame].size.height - point.y;

    point = [self convertPoint:point toView:self];
    point = [self convertPoint:point toView:nil];

    point = [self convertPointToScreen:point];

    const auto screenHeight = [[[self window] screen] frame].size.height;
    point.y                 = screenHeight - point.y;

    return point;
}

/////////////////////////////////////
/////////////////////////////////////
- (CGDirectDisplayID)displayId {
    auto screen   = [[self window] screen];
    id display_id = [[screen deviceDescription] objectForKey:@"NSScreenNumber"];

    return [display_id intValue];
}

/////////////////////////////////////
/////////////////////////////////////
- (void)setNativeEventRetriever:(void *)native_event {
    //native_event = native_event;
}

@end
