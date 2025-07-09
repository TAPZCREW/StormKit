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

#include <volk.h>

module stormkit.gpu.core;

import std;

import stormkit.core;
import stormkit.wsi;

;

namespace stormkit::gpu {
    /////////////////////////////////////
    /////////////////////////////////////
    auto Surface::do_init_offscreen(const Instance& instance) noexcept -> Expected<void> {
        m_vk_instance = instance.native_handle();
        assert(false, "not implemented yet");
        return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Surface::do_init_from_window(const Instance& instance, const wsi::Window& window) noexcept
      -> Expected<void> {
        m_vk_instance = instance.native_handle();
#if defined(STORMKIT_OS_WINDOWS)
        const auto create_surface = [&window, &instance] {
            const auto create_info = VkWin32SurfaceCreateInfoKHR {
                .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
                .pNext     = nullptr,
                .flags     = 0,
                .hinstance = GetModuleHandleW(nullptr),
                .hwnd      = reinterpret_cast<HWND>(window.native_handle())
            };
            return vk_call<VkSurfaceKHR>(vkCreateWin32SurfaceKHR,
                                         instance.native_handle(),
                                         &create_info,
                                         nullptr);
        };
#elif defined(STORMKIT_OS_MACOS)
        const auto create_surface = [&window, &instance] {
            const auto create_info = VkMacOSSurfaceCreateInfoMVK {
                .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
                .pNext = nullptr,
                .flags = 0,
                .pView = window.native_handle()
            };
            return vk_call<VkSurfaceKHR>(vkCreateMacOSSurfaceCreateMVK,
                                         instance.native_handle(),
                                         &create_info,
                                         nullptr);
        };
#elif defined(STORMKIT_OS_LINUX)
        const auto make_wayland_surface = [&window, &instance] {
            struct Handles {
                wl_display* display;
                wl_surface* surface;
            }* handles = std::bit_cast<Handles*>(window.native_handle());

            const auto create_info = VkWaylandSurfaceCreateInfoKHR {
                .sType   = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
                .pNext   = nullptr,
                .flags   = 0,
                .display = handles->display,
                .surface = handles->surface
            };
            return vk_call<VkSurfaceKHR>(vkCreateWaylandSurfaceKHR,
                                         instance.native_handle(),
                                         &create_info,
                                         nullptr);
        };
        const auto make_xcb_surface = [&window, &instance] {
            struct Handles {
                xcb_connection_t* connection;
                xcb_window_t      window;
            }* handles = reinterpret_cast<Handles*>(window.native_handle());

            const auto create_info = VkXcbSurfaceCreateInfoKHR {
                .sType      = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
                .pNext      = nullptr,
                .flags      = 0,
                .connection = handles->connection,
                .window     = handles->window
            };
            return vk_call<VkSurfaceKHR>(vkCreateXcbSurfaceKHR,
                                         instance.native_handle(),
                                         &create_info,
                                         nullptr);
        };

        const auto create_surface = [&window, &make_wayland_surface, &make_xcb_surface] noexcept
          -> FunctionRef<VulkanExpected<VkSurfaceKHR>()> {
            const auto is_wayland = window.wm() == wsi::WM::WAYLAND;

            if (is_wayland) return make_wayland_surface;

            return make_xcb_surface;
        }();

#elif defined(STORMKIT_OS_IOS)
        const auto create_surface = [this, &window, &instance] noexcept {
            const auto create_info = VkIOSSurfaceCreateInfoMVK {
                .sType = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK,
                .pNext = nullptr,
                .flags = 0,
                .pView = window->native_handle()
            };
            CHECK_VK_ERROR(vkCreateIOSSurfaceMVK(instance, &create_info, &m_surface));
        };
#else
        const auto create_surface = [] static noexcept {};
        assertWithMessage(true, "This platform WSI is not supported !");
#endif
        m_vk_handle = { [vk_instance = m_vk_instance](auto handle) noexcept {
            vkDestroySurfaceKHR(vk_instance, handle, nullptr);
        } };
        return create_surface()
          .transform(core::monadic::set(m_vk_handle))
          .transform_error(core::monadic::narrow<Result>());
    }
} // namespace stormkit::gpu
