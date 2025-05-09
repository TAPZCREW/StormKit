// Copyright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#ifndef STORMKIT_APPLICATION_H
#define STORMKIT_APPLICATION_H

/////////// - AppKit - ///////////
#import <AppKit/NSApplication.h>

@interface StormKitApplication: NSApplication
+ (void)processEvent;
@end

@interface StormKitApplicationDelegate: NSObject<NSApplicationDelegate>
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication;
@end

#endif
