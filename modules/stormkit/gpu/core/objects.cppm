// Copyright (C) 2023 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/gpu/vulkan.hpp>

export module stormkit.gpu.core:objects;

import std;

import stormkit.core;

import :vulkan;
import :base;

namespace stormkit::gpu {
    class InstanceImplementation;
    class DebugCallbackImplementation;
    class SurfaceImplementation;
    class PhysicalDeviceImplementation;
    class DeviceImplementation;
    class FenceImplementation;
    class SemaphoreImplementation;

    namespace view {
        class InstanceImplementation;
        class DebugCallbackImplementation;
        class SurfaceImplementation;
        class PhysicalDeviceImplementation;
        class DeviceImplementation;
        class FenceImplementation;
        class SemaphoreImplementation;
    } // namespace view

    export {
        class InstanceTag;
        template<typename>
        class InstanceInterface;

        class DebugCallbackTag;
        template<typename>
        class DebugCallbackInterface;

        class SurfaceTag;
        template<typename>
        class SurfaceInterface;

        class PhysicalDeviceTag;
        template<typename>
        class PhysicalDeviceInterface;

        class DeviceTag;
        template<typename>
        class DeviceInterface;

        class FenceTag;
        template<typename>
        class FenceInterface;

        class SemaphoreTag;
        template<typename>
        class SemaphoreInterface;

        using Instance = InstanceInterface<InstanceImplementation>;

        namespace view {
            using Instance = InstanceInterface<InstanceImplementation>;
        }

        template<typename Interface>
        class InstanceObject: public Interface {
          public:
            using Interface::Interface;
            using Interface::operator=;
            using TagType = Interface::TagType;

            auto instance() const noexcept -> view::Instance;
        };

        using DebugCallback  = DebugCallbackInterface<DebugCallbackImplementation>;
        using Surface        = SurfaceInterface<SurfaceImplementation>;
        using PhysicalDevice = PhysicalDeviceInterface<PhysicalDeviceImplementation>;

        namespace view {
            using PhysicalDevice = PhysicalDeviceInterface<PhysicalDeviceImplementation>;
            using Surface        = SurfaceInterface<SurfaceImplementation>;
            using DebugCallback  = DebugCallbackInterface<DebugCallbackImplementation>;
        } // namespace view

        template<typename Interface>
        class PhysicalDeviceObject: public Interface {
          public:
            using Interface::Interface;
            using Interface::operator=;
            using TagType = Interface::TagType;

            auto instance() const noexcept -> view::Instance;
            auto physical_device() const noexcept -> view::PhysicalDevice;
        };

        using Device = DeviceInterface<DeviceImplementation>;

        namespace view {
            using Device = DeviceInterface<DeviceImplementation>;
        }

        template<typename Interface>
        class DeviceObject: public Interface {
          public:
            using Interface::Interface;
            using Interface::operator=;
            using TagType = Interface::TagType;

            auto instance() const noexcept -> view::Instance;
            auto physical_device() const noexcept -> view::PhysicalDevice;
            auto device() const noexcept -> view::Device;
        };

        using Fence     = FenceInterface<FenceImplementation>;
        using Semaphore = SemaphoreInterface<SemaphoreImplementation>;

        namespace view {
            using Fence     = FenceInterface<FenceImplementation>;
            using Semaphore = SemaphoreInterface<SemaphoreImplementation>;
        } // namespace view

        namespace trait {
            template<>
            struct GpuObject<InstanceTag> {
                using value_type   = VkInstance;
                using DeleterType = PFN_vkDestroyInstance;
                using ObjectType  = Instance;
                using ViewType    = view::Instance;

                static constexpr auto DEBUG_TYPE = DebugObjectType::INSTANCE;
            };

            template<>
            struct GpuObject<DebugCallbackTag> {
                using value_type   = VkDebugUtilsMessengerEXT;
                using DeleterType = PFN_vkDestroyDebugUtilsMessengerEXT;
                using ObjectType  = DebugCallback;
                using ViewType    = view::DebugCallback;
                using OwnerType   = Instance;

                static constexpr auto DEBUG_TYPE = DebugObjectType::DEBUG_UTILS_MESSENGER;
            };

            template<>
            struct GpuObject<SurfaceTag> {
                using value_type   = VkSurfaceKHR;
                using DeleterType = PFN_vkDestroySurfaceKHR;
                using ObjectType  = Surface;
                using ViewType    = view::Surface;
                using OwnerType   = Instance;

                static constexpr auto DEBUG_TYPE = DebugObjectType::SURFACE;
            };

            template<>
            struct GpuObject<PhysicalDeviceTag> {
                using value_type        = VkPhysicalDevice;
                using DeleterType      = decltype(monadic::noop());
                using DoInitreturn_type = void;
                using ObjectType       = PhysicalDevice;
                using ViewType         = view::PhysicalDevice;
                using OwnerType        = Instance;

                static constexpr auto DEBUG_TYPE = DebugObjectType::PHYSICAL_DEVICE;
            };

            template<>
            struct GpuObject<DeviceTag> {
                using value_type   = VkDevice;
                using DeleterType = PFN_vkDestroyDevice VolkDeviceTable::*;
                using ObjectType  = Device;
                using ViewType    = view::Device;
                using OwnerType   = PhysicalDevice;

                static constexpr auto DEBUG_TYPE = DebugObjectType::DEVICE;
            };

            template<>
            struct GpuObject<FenceTag> {
                using value_type   = VkFence;
                using DeleterType = PFN_vkDestroyFence VolkDeviceTable::*;
                using ObjectType  = Fence;
                using ViewType    = view::Fence;
                using OwnerType   = Device;

                static constexpr auto DEBUG_TYPE = DebugObjectType::FENCE;
            };

            template<>
            struct GpuObject<SemaphoreTag> {
                using value_type   = VkSemaphore;
                using DeleterType = PFN_vkDestroySemaphore VolkDeviceTable::*;
                using ObjectType  = Semaphore;
                using ViewType    = view::Semaphore;
                using OwnerType   = Device;

                static constexpr auto DEBUG_TYPE = DebugObjectType::SEMAPHORE;
            };
        } // namespace trait
    }
} // namespace stormkit::gpu
