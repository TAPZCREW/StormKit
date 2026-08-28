// Copyright (C) 2023 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>
#include <stormkit/core/try_expected.hpp>

#include <stormkit/gpu/api.hpp>
#include <stormkit/gpu/vulkan.hpp>

export module stormkit.gpu.core:device;

import std;

import stormkit.core;

import :vulkan;
import :base;

import :structs;
import :objects;

import :instance;

namespace cmeta = stormkit::core::meta;

using namespace stormkit;

namespace stormkit::gpu {
    export {
        struct QueueEntry {
            u32       id;
            u32       count;
            QueueFlag flags = QueueFlag {};
        };

        struct DeviceInterfaceBase {
            struct CreateInfo {
                bool enable_swapchain  = true;
                bool enable_raytracing = false;
            };
        };

        template<typename Base>
        class STORMKIT_GPU_API DeviceInterface final: public PhysicalDeviceObject<Base>, public DeviceInterfaceBase {
          public:
            using PhysicalDeviceObject<Base>::PhysicalDeviceObject;
            using PhysicalDeviceObject<Base>::operator=;
            using TagType = DeviceTag;

            auto wait_idle() const noexcept -> Expected<void>;

            auto wait_for_fences(array_view<const view::Fence>    fences,
                                 bool                             wait_all = true,
                                 const std::chrono::milliseconds& timeout  = std::chrono::milliseconds::max()) const noexcept
              -> Expected<Result>;
            auto wait_for_fence(view::Fence                      fence,
                                const std::chrono::milliseconds& timeout = std::chrono::milliseconds::max()) const noexcept
              -> Expected<Result>;

            auto reset_fences(array_view<const view::Fence> fences) const noexcept -> Expected<void>;
            auto reset_fence(view::Fence fence) const noexcept -> Expected<void>;

            template<meta::IsGpuObjectOrView T>
            auto set_object_name(const T& object, string_view name) const noexcept -> Expected<void>;

            auto set_object_name(u64 object, DebugObjectType type, string_view name) const noexcept -> Expected<void>;

            [[nodiscard]]
            auto queue_entries() const noexcept -> array_view<const QueueEntry>;

            [[nodiscard]]
            auto device_table() const noexcept -> const VolkDeviceTable&;

            [[nodiscard]]
            auto allocator() const noexcept -> vk::Observer<VmaAllocator>;
        };

        namespace vk {
            STORMKIT_GPU_API auto imgui_vk_loader(const char* func_name, void*) noexcept -> PFN_vkVoidFunction;
        }

        namespace monadic {
            template<QueueFlag flag, QueueFlag... no_flag>
            constexpr auto find_queue() noexcept -> decltype(auto);
        } // namespace monadic
    }

    class STORMKIT_GPU_API
      DeviceImplementation: public GpuObjectImplementation<DeviceTag, const DeviceInterfaceBase::CreateInfo&> {
      public:
        using CreateInfo = DeviceInterfaceBase::CreateInfo;

        DeviceImplementation(PrivateTag, view::PhysicalDevice&&) noexcept;
        ~DeviceImplementation() noexcept;

        DeviceImplementation(const DeviceImplementation&) noexcept                    = delete;
        auto operator=(const DeviceImplementation&) noexcept -> DeviceImplementation& = delete;

        DeviceImplementation(DeviceImplementation&&) noexcept;
        auto operator=(DeviceImplementation&&) noexcept -> DeviceImplementation&;

        auto do_init(PrivateTag, const CreateInfo&) noexcept -> Expected<void>;

      protected:
        dyn_array<QueueEntry> m_queue_entries;

        VolkDeviceTable         m_vk_device_table    = {};
        VmaVulkanFunctions      m_vma_function_table = {};
        vk::Owned<VmaAllocator> m_vma_allocator      = { vmaDestroyAllocator };
    };

    namespace view {
        class DeviceImplementation: public GpuObjectViewImplementation<DeviceTag> {
          public:
            DeviceImplementation(const gpu::Device& of) noexcept;
            template<cmeta::IsContainerOrPointerOf<gpu::Device> TContainerOrPointer>
            DeviceImplementation(const TContainerOrPointer&) noexcept;
            ~DeviceImplementation() noexcept;

            DeviceImplementation(const DeviceImplementation&) noexcept;
            auto operator=(const DeviceImplementation&) noexcept -> DeviceImplementation&;

            DeviceImplementation(DeviceImplementation&&) noexcept;
            auto operator=(DeviceImplementation&&) noexcept -> DeviceImplementation&;

          protected:
            array_view<const QueueEntry> m_queue_entries;

            VolkDeviceTable            m_vk_device_table;
            vk::Observer<VmaAllocator> m_vma_allocator;
        };
    } // namespace view
} // namespace stormkit::gpu

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::gpu {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    template<meta::IsGpuObjectOrView T>
    STORMKIT_FORCE_INLINE
    inline auto DeviceInterface<Base>::set_object_name(const T& object, string_view name) const noexcept -> Expected<void> {
        if (not vkSetDebugUtilsObjectNameEXT) return {};

        const auto vk_object = vk::to_vk(object);
        return set_object_name(as<u64>(std::bit_cast<uptr>(vk_object)),
                               trait::GpuObject<typename T::TagType>::DEBUG_TYPE,
                               std::move(name));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto DeviceInterface<Base>::queue_entries() const noexcept -> array_view<const QueueEntry> {
        return Base::m_queue_entries;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto DeviceInterface<Base>::device_table() const noexcept -> const VolkDeviceTable& {
        return Base::m_vk_device_table;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto DeviceInterface<Base>::allocator() const noexcept -> vk::Observer<VmaAllocator> {
        return Base::m_vma_allocator;
    }

    namespace monadic {
        /////////////////////////////////////
        /////////////////////////////////////
        template<QueueFlag flag, QueueFlag... no_flag>
        constexpr auto find_queue() noexcept -> decltype(auto) {
            return [](const auto& family) static noexcept {
                return core::check_flag_bit(family.flags, flag) and (not core::check_flag_bit(family.flags, no_flag) and ...);
            };
        }
    } // namespace monadic

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline DeviceImplementation::DeviceImplementation(PrivateTag, view::PhysicalDevice&& physical_device) noexcept
        : GpuObjectImplementation { std::move(physical_device), &VolkDeviceTable::vkDestroyDevice } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline DeviceImplementation::~DeviceImplementation() noexcept {
        if (m_deleter_ptr != nullptr and m_vk_handle != VK_NULL_HANDLE)
            vk::call(m_vk_device_table.*m_deleter_ptr, m_vk_handle, nullptr);
        m_vk_handle = VK_NULL_HANDLE;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline DeviceImplementation::DeviceImplementation(DeviceImplementation&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto DeviceImplementation::operator=(DeviceImplementation&&) noexcept -> DeviceImplementation& = default;

    namespace view {
        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline DeviceImplementation::DeviceImplementation(const gpu::Device& of) noexcept
            : GpuObjectViewImplementation { of },
              m_queue_entries { of.queue_entries() },
              m_vk_device_table { of.device_table() },
              m_vma_allocator { of.allocator() } {
        }

        /////////////////////////////////////
        /////////////////////////////////////
        template<cmeta::IsContainerOrPointerOf<gpu::Device> TContainerOrPointer>
        STORMKIT_FORCE_INLINE
        inline DeviceImplementation::DeviceImplementation(const TContainerOrPointer& of) noexcept
            : DeviceImplementation { *of } {
        }

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline DeviceImplementation::~DeviceImplementation() noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline DeviceImplementation::DeviceImplementation(const DeviceImplementation&) noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto DeviceImplementation::operator=(const DeviceImplementation&) noexcept -> DeviceImplementation& = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline DeviceImplementation::DeviceImplementation(DeviceImplementation&&) noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto DeviceImplementation::operator=(DeviceImplementation&&) noexcept -> DeviceImplementation& = default;
    } // namespace view

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Interface>
    STORMKIT_FORCE_INLINE
    inline auto DeviceObject<Interface>::instance() const noexcept -> view::Instance {
        return Interface::owner().instance();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Interface>
    STORMKIT_FORCE_INLINE
    inline auto DeviceObject<Interface>::physical_device() const noexcept -> view::PhysicalDevice {
        return Interface::owner().physical_device();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Interface>
    STORMKIT_FORCE_INLINE
    inline auto DeviceObject<Interface>::device() const noexcept -> view::Device {
        return Interface::owner();
    }
} // namespace stormkit::gpu
