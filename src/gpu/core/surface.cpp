// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>
#include <stormkit/core/try_expected.hpp>

#if defined(STORMKIT_OS_LINUX)
    #include <wayland-client.h>
    #include <xcb/xcb.h>
#elif defined(STORMKIT_OS_WINDOWS)
    #include <stormkit/core/platform/windows.hpp>
#endif

#include <stormkit/gpu/api.hpp>
#define STORMKIT_DEFINE_VK_PLATFORM
#include <stormkit/gpu/vulkan.hpp>

module stormkit.gpu.core;

import std;

import stormkit.core;
import stormkit.wsi;

namespace stormkit::gpu {
    template class SurfaceInterface<SurfaceImplementation>;
    template class SurfaceInterface<view::SurfaceImplementation>;

    /////////////////////////////////////
    /////////////////////////////////////
    auto SurfaceImplementation::do_init(PrivateTag, const OffscreenCreateInfo&) noexcept -> Expected<void> {
        assert(false, "not implemented yet");
        return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto SurfaceImplementation::do_init(PrivateTag, const CreateInfo& create_info) noexcept -> Expected<void> {
        const auto& window = *create_info.window;

        EXPECTS(window.is_open());
        const auto instance = owner();

#if defined(STORMKIT_OS_WINDOWS)
        const auto create_surface = [&window, &instance] {
            const auto create_info = VkWin32SurfaceCreateInfoKHR {
                .sType       = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
                .pNext       = nullptr,
                .flags       = 0,
                .m_hinstance = GetModuleHandleW(nullptr),
                .hwnd        = std::bit_cast<HWND>(window.native_handle())
            };
            return vk::call_checked<VkSurfaceKHR>(vkCreateWin32SurfaceKHR, instance, &create_info, nullptr);
        };
#elif defined(STORMKIT_OS_MACOS)
        const auto create_surface = [&window, &instance] {
            const auto create_info = VkMacOSSurfaceCreateInfoMVK {
                .sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK,
                .pNext = nullptr,
                .flags = 0,
                .pView = window.native_handle()
            };
            return vk::call_checked<VkSurfaceKHR>(vkCreateMacOSSurfaceMVK, instance, &create_info, nullptr);
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
            return vk::call_checked<VkSurfaceKHR>(vkCreateWaylandSurfaceKHR, instance, &create_info, nullptr);
        };
        const auto make_xcb_surface = [&window, &instance] {
            struct Handles {
                xcb_connection_t* connection;
                xcb_window_t      window;
                void*             key_symbols;
                void*             state;
            }* handles = reinterpret_cast<Handles*>(window.native_handle());

            const auto create_info = VkXcbSurfaceCreateInfoKHR {
                .sType      = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
                .pNext      = nullptr,
                .flags      = 0,
                .connection = handles->connection,
                .window     = handles->window
            };
            return vk::call_checked<VkSurfaceKHR>(vkCreateXcbSurfaceKHR, instance, &create_info, nullptr);
        };

        const auto create_surface =
          [&window, &make_wayland_surface, &make_xcb_surface] noexcept -> std23::function_ref<Expected<VkSurfaceKHR>()> {
            const auto is_wayland = window.wm() == wsi::WM::WAYLAND;

            if (is_wayland) return make_wayland_surface;

            return make_xcb_surface;
        }();

#elif defined(STORMKIT_OS_IOS)
        const auto create_surface = [this, &window, &m_instance] noexcept {
            const auto create_info = VkIOSSurfaceCreateInfoMVK {
                .sType = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK,
                .pNext = nullptr,
                .flags = 0,
                .pView = window->native_handle()
            };
            CHECK_VK_ERROR(vkCreateIOSSurfaceMVK(m_instance, &create_info, &m_surface));
        };
#else
        const auto create_surface = [] static noexcept {};
        assertWithMessage(true, "This platform WSI is not supported !");
#endif

        m_vk_handle = Try(create_surface());

        Return {};
    }
} // namespace stormkit::gpu
