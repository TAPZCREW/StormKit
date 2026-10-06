// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#ifndef APP_DELEGATE_HPP
#define APP_DELEGATE_HPP

#import <UIKit/UIApplication.h>
#import <UIKit/UIResponder.h>

#import "window.hpp"

@interface AppDelegate: UIResponder<UIApplicationDelegate>

@property (nonatomic) storm::window::Window* window;

@end

#endif
