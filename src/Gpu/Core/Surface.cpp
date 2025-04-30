// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#if defined(STORMKIT_OS_LINUX)
    #include <wayland-client.h>
    #include <xcb/xcb.h>
#elif defined(STORMKIT_OS_WINDOWS)
    #include <windows.h>
#endif

module stormkit.Gpu;

import std;

import stormkit.core;
import stormkit.Wsi;

import stormkit.Gpu.Vulkan;

namespace stormkit::gpu {
    /////////////////////////////////////
    /////////////////////////////////////
    Surface::Surface(const Instance& instance, const wsi::Window& window, Tag) {
#if defined(STORMKIT_OS_WINDOWS)
        const auto create_surface = [&window, &instance] {
            const auto create_info
                = vk::Win32SurfaceCreateInfoKHR { .flags     = {},
                                                  .hinstance = GetModuleHandleW(nullptr),
                                                  .hwnd
                                                  = reinterpret_cast<HWND>(window.nativeHandle()) };
            return instance.vkHandle().createWin32SurfaceKHR(create_info, nullptr);
        };
#elif defined(STORMKIT_OS_MACOS)
        const auto create_surface = [&window, &instance] {
            const auto create_info
                = vk::MacOSSurfaceCreateInfoMVK { .pView = window.nativeHandle() };
            return instance.vkHandle().createMacOSSurfaceMVK(create_info, nullptr);
        };
#elif defined(STORMKIT_OS_LINUX)
        const auto make_wayland_surface = [&window, &instance] {
            struct Handles {
                wl_display* display;
                wl_surface* surface;
            }* handles = std::bit_cast<Handles*>(window.nativeHandle());

            const auto create_info
                = vk::WaylandSurfaceCreateInfoKHR { .display = handles->display,
                                                    .surface = handles->surface };

            return instance.vkHandle().createWaylandSurfaceKHR(create_info, nullptr);
        };
        const auto make_xcb_surface = [&window, &instance] {
            struct Handles {
                xcb_connection_t* connection;
                xcb_window_t      window;
            }* handles = reinterpret_cast<Handles*>(window.nativeHandle());

            const auto create_info
                = vk::XcbSurfaceCreateInfoKHR { .connection = handles->connection,
                                                .window     = handles->window };

            return instance.vkHandle().createXcbSurfaceKHR(create_info, nullptr);
        };

        const auto create_surface
            = [&window,
               &make_wayland_surface,
               &make_xcb_surface] noexcept -> FunctionRef<VulkanExpected<vk::raii::SurfaceKHR>()> {
            const auto is_wayland = window.wm() == wsi::WM::Wayland;

            if (is_wayland) return make_wayland_surface;

            return make_xcb_surface;
        }();

#elif defined(STORMKIT_OS_IOS)
        const auto create_surface = [this, &window, &instance] noexcept {
            const auto create_info
                = VkIOSSurfaceCreateInfoMVK { .sType
                                              = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK,
                                              .pView = m_window->nativeHandle() };
            CHECK_VK_ERROR(vkCreateIOSSurfaceMVK(instance, &create_info, &m_surface));
        };
#else
        const auto create_surface = [] static noexcept {};
        assertWithMessage(true, "This platform WSI is not supported !");
#endif

        create_surface()
            .transform(core :.monadic::set(m_vk_surface))
            .transform_error(core :.monadic::map(core :.monadic::narrow<Result>(),
                                                 core :.monadic::throw_as_exception()));
    }
} // namespace stormkit::gpu
