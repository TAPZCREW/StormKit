// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#include <stormkit/core/platform_macro.hpp>

#ifdef STORMKIT_DEFINE_VK_PLATFORM
    #ifdef STORMKIT_OS_LINUX
        #define VK_USE_PLATFORM_XCB_KHR
        #define VK_USE_PLATFORM_WAYLAND_KHR
    #elifdef STORMKIT_OS_WINDOWS
        #define VK_USE_PLATFORM_WIN32_KHR
        #include <stormkit/core/platform/windows.hpp>
        #undef OPAQUE
    #elifdef STORMKIT_OS_MACOS
        #define VK_USE_PLATFORM_MACOS_MVK
    #elifdef STORMKIT_OS_IOS
        #define VK_USE_PLATFORM_IOS_MVK
    #endif
#endif

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include <volk.h>

#ifdef STORMKIT_BUILD_DEBUG
    #define STORMKIT_RENDER_VERBOSE_VULKAN
#endif

#ifdef STORMKIT_RENDER_VERBOSE_VULKAN
    #define CHECK_VK_ERROR(line)                             \
        do {                                                 \
            const auto _result = line;                       \
            checkVkError(_result, STORMKIT_STRINGIFY(line)); \
        } while (false);
#else
    #define CHECK_VK_ERROR(line)                            \
        {                                                   \
            const auto _result = line;                      \
            core::ensures(_result == vk::Result::eSuccess); \
        }
#endif
