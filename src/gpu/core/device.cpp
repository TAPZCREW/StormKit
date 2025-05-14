// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <volk.h>

#include <vk_mem_alloc.h>

#include <stormkit/gpu/core/vulkan_macro.hpp>
#include <stormkit/log/log_macro.hpp>

module stormkit.gpu;

import std;

import stormkit.core;
import stormkit.log;

import :core.device;
import :core.sync;
import stormkit.gpu.vulkan;

using namespace std::literals;

namespace {
    constexpr auto RAYTRACING_EXTENSIONS = std::array {
        "VK_KHR_ray_tracing_pipeline"sv,  "VK_KHR_acceleration_structure"sv,
        "VK_KHR_buffer_device_address"sv, "VK_KHR_deferred_host_operations"sv,
        "VK_EXT_descriptor_indexing"sv,   "VK_KHR_spirv_1_4"sv,
        "VK_KHR_shader_float_controls"sv
    };

    constexpr auto BASE_EXTENSIONS      = std::array { "VK_KHR_maintenance3"sv };
    constexpr auto SWAPCHAIN_EXTENSIONS = std::array { "VK_KHR_swapchain"sv };

    VkResult vmaImportVulkanFunctionsFromVolk(const VmaAllocatorCreateInfo* pAllocatorCreateInfo,
                                              VmaVulkanFunctions* pDstVulkanFunctions) noexcept {
        using std::memset;
        stormkit::expects(pAllocatorCreateInfo != nullptr);
        stormkit::expects(pAllocatorCreateInfo->instance != VK_NULL_HANDLE);
        stormkit::expects(pAllocatorCreateInfo->device != VK_NULL_HANDLE);

        memset(pDstVulkanFunctions, 0, sizeof(*pDstVulkanFunctions));

        VolkDeviceTable src = {};

        volkLoadDeviceTable(&src, pAllocatorCreateInfo->device);

#define COPY_GLOBAL_TO_VMA_FUNC(volkName, vmaName) \
    if (!pDstVulkanFunctions->vmaName) pDstVulkanFunctions->vmaName = volkName;

#define COPY_DEVICE_TO_VMA_FUNC(volkName, vmaName) \
    if (!pDstVulkanFunctions->vmaName) pDstVulkanFunctions->vmaName = src.volkName;

        COPY_GLOBAL_TO_VMA_FUNC(vkGetInstanceProcAddr, vkGetInstanceProcAddr)

        COPY_GLOBAL_TO_VMA_FUNC(vkGetDeviceProcAddr, vkGetDeviceProcAddr)

        COPY_GLOBAL_TO_VMA_FUNC(vkGetPhysicalDeviceProperties, vkGetPhysicalDeviceProperties)

        COPY_GLOBAL_TO_VMA_FUNC(vkGetPhysicalDeviceMemoryProperties,
                                vkGetPhysicalDeviceMemoryProperties)

        COPY_DEVICE_TO_VMA_FUNC(vkAllocateMemory, vkAllocateMemory)

        COPY_DEVICE_TO_VMA_FUNC(vkFreeMemory, vkFreeMemory)

        COPY_DEVICE_TO_VMA_FUNC(vkMapMemory, vkMapMemory)

        COPY_DEVICE_TO_VMA_FUNC(vkUnmapMemory, vkUnmapMemory)

        COPY_DEVICE_TO_VMA_FUNC(vkFlushMappedMemoryRanges, vkFlushMappedMemoryRanges)

        COPY_DEVICE_TO_VMA_FUNC(vkInvalidateMappedMemoryRanges, vkInvalidateMappedMemoryRanges)

        COPY_DEVICE_TO_VMA_FUNC(vkBindBufferMemory, vkBindBufferMemory)

        COPY_DEVICE_TO_VMA_FUNC(vkBindImageMemory, vkBindImageMemory)

        COPY_DEVICE_TO_VMA_FUNC(vkGetBufferMemoryRequirements, vkGetBufferMemoryRequirements)

        COPY_DEVICE_TO_VMA_FUNC(vkGetImageMemoryRequirements, vkGetImageMemoryRequirements)

        COPY_DEVICE_TO_VMA_FUNC(vkCreateBuffer, vkCreateBuffer)

        COPY_DEVICE_TO_VMA_FUNC(vkDestroyBuffer, vkDestroyBuffer)

        COPY_DEVICE_TO_VMA_FUNC(vkCreateImage, vkCreateImage)

        COPY_DEVICE_TO_VMA_FUNC(vkDestroyImage, vkDestroyImage)

        COPY_DEVICE_TO_VMA_FUNC(vkCmdCopyBuffer, vkCmdCopyBuffer)

#if VMA_VULKAN_VERSION >= 1001000

        if (pAllocatorCreateInfo->vulkanApiVersion >= VK_MAKE_VERSION(1, 1, 0))

        {
            COPY_GLOBAL_TO_VMA_FUNC(vkGetPhysicalDeviceMemoryProperties2,
                                    vkGetPhysicalDeviceMemoryProperties2KHR)

            COPY_DEVICE_TO_VMA_FUNC(vkGetBufferMemoryRequirements2,
                                    vkGetBufferMemoryRequirements2KHR)

            COPY_DEVICE_TO_VMA_FUNC(vkGetImageMemoryRequirements2, vkGetImageMemoryRequirements2KHR)

            COPY_DEVICE_TO_VMA_FUNC(vkBindBufferMemory2, vkBindBufferMemory2KHR)

            COPY_DEVICE_TO_VMA_FUNC(vkBindImageMemory2, vkBindImageMemory2KHR)
        }

#endif

#if VMA_VULKAN_VERSION >= 1003000

        if (pAllocatorCreateInfo->vulkanApiVersion >= VK_MAKE_VERSION(1, 3, 0))

        {
            COPY_DEVICE_TO_VMA_FUNC(vkGetDeviceBufferMemoryRequirements,
                                    vkGetDeviceBufferMemoryRequirements)

            COPY_DEVICE_TO_VMA_FUNC(vkGetDeviceImageMemoryRequirements,
                                    vkGetDeviceImageMemoryRequirements)
        }

#endif

#if VMA_KHR_MAINTENANCE4

        if ((pAllocatorCreateInfo->flags & VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT) != 0)

        {
            COPY_DEVICE_TO_VMA_FUNC(vkGetDeviceBufferMemoryRequirementsKHR,
                                    vkGetDeviceBufferMemoryRequirements)

            COPY_DEVICE_TO_VMA_FUNC(vkGetDeviceImageMemoryRequirementsKHR,
                                    vkGetDeviceImageMemoryRequirements)
        }

#endif

#if VMA_DEDICATED_ALLOCATION

        if ((pAllocatorCreateInfo->flags & VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT) != 0)

        {
            COPY_DEVICE_TO_VMA_FUNC(vkGetBufferMemoryRequirements2KHR,
                                    vkGetBufferMemoryRequirements2KHR)

            COPY_DEVICE_TO_VMA_FUNC(vkGetImageMemoryRequirements2KHR,
                                    vkGetImageMemoryRequirements2KHR)
        }

#endif

#if VMA_BIND_MEMORY2

        if ((pAllocatorCreateInfo->flags & VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT) != 0)

        {
            COPY_DEVICE_TO_VMA_FUNC(vkBindBufferMemory2KHR, vkBindBufferMemory2KHR)

            COPY_DEVICE_TO_VMA_FUNC(vkBindImageMemory2KHR, vkBindImageMemory2KHR)
        }

#endif

#if VMA_MEMORY_BUDGET

        if ((pAllocatorCreateInfo->flags & VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT) != 0)

        {
            COPY_GLOBAL_TO_VMA_FUNC(vkGetPhysicalDeviceMemoryProperties2KHR,
                                    vkGetPhysicalDeviceMemoryProperties2KHR)
        }

#endif

#if VMA_EXTERNAL_MEMORY_WIN32

        if ((pAllocatorCreateInfo->flags & VMA_ALLOCATOR_CREATE_KHR_EXTERNAL_MEMORY_WIN32_BIT) != 0)

        {
            COPY_DEVICE_TO_VMA_FUNC(vkGetMemoryWin32HandleKHR, vkGetMemoryWin32HandleKHR)
        }

#endif

#undef COPY_DEVICE_TO_VMA_FUNC

#undef COPY_GLOBAL_TO_VMA_FUNC

        return VK_SUCCESS;
    }
} // namespace

namespace stormkit::gpu {
    NAMED_LOGGER(device_logger, "stormkit.gpu:core.Device")

    template<QueueFlag flag, QueueFlag... no_flag>
    constexpr auto find_queue() {
        return [](const auto& family) static noexcept {
            return core::check_flag_bit(family.flags, flag)
                   and (not core::check_flag_bit(family.flags, no_flag) and ...);
        };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Device::do_init(const Instance& instance, const Info& info) noexcept -> Expected<void> {
        const auto& queue_families = m_physical_device->queue_families();

        struct Queue_ {
            std::optional<UInt32> id    = std::nullopt;
            UInt32                count = 0u;
            Byte                  _[3];
            QueueFlag             flags = QueueFlag {};
        };

        const auto raster_queue = [&queue_families]() -> Queue_ {
            const auto it = std::ranges::find_if(queue_families, find_queue<QueueFlag::GRAPHICS>());
            if (it == std::ranges::cend(queue_families)) return {};

            return { .id    = as<UInt32>(std::distance(std::ranges::cbegin(queue_families), it)),
                     .count = it->count,
                     .flags = it->flags };
        }();

        const auto compute_queue = [&queue_families]() -> Queue_ {
            const auto it = std::ranges::find_if(queue_families,
                                                 find_queue<QueueFlag::TRANSFER,
                                                            QueueFlag::GRAPHICS>());
            if (it == std::ranges::cend(queue_families)) return {};

            return { .id    = as<UInt32>(std::distance(std::ranges::cbegin(queue_families), it)),
                     .count = it->count,
                     .flags = it->flags };
        }();

        const auto transfert_queue = [&queue_families]() -> Queue_ {
            const auto it = std::ranges::
              find_if(queue_families,
                      find_queue<QueueFlag::COMPUTE, QueueFlag::GRAPHICS, QueueFlag::TRANSFER>());
            if (it == std::ranges::cend(queue_families)) return {};

            return { .id    = as<UInt32>(std::distance(std::ranges::cbegin(queue_families), it)),
                     .count = it->count,
                     .flags = it->flags };
        }();

        const auto queues = [&] {
            auto q = std::vector<const Queue_*> {};
            q.reserve(3);

            if (raster_queue.id) q.push_back(&raster_queue);
            if (compute_queue.id) q.push_back(&compute_queue);
            if (transfert_queue.id) q.push_back(&transfert_queue);

            return q;
        }();

        auto priorities = std::vector<std::vector<float>> {};
        priorities.reserve(std::size(queues));

        const auto queue_create_infos = transform(queues, [&priorities](auto queue) {
            auto& priority = priorities.emplace_back();

            priority.resize(queue->count, 1.f);

            return VkDeviceQueueCreateInfo { .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                             .queueFamilyIndex = queue->id.value(),
                                             .queueCount       = 1,
                                             .pQueuePriorities = std::data(priority) };
        });

        const auto& capabilities     = m_physical_device->capabilities();
        const auto  enabled_features = VkPhysicalDeviceFeatures {
             .sampleRateShading = capabilities.features.sampler_rate_shading,
             .multiDrawIndirect = capabilities.features.multi_draw_indirect,
             .fillModeNonSolid  = capabilities.features.fill_Mode_non_solid,
             .samplerAnisotropy = capabilities.features.sampler_anisotropy
        };

        const auto device_extensions = m_physical_device->extensions();

        device_logger.dlog("Device extensions: {}", device_extensions);

        const auto swapchain_available = [&] {
            for (const auto& ext : SWAPCHAIN_EXTENSIONS)
                if (std::ranges::none_of(device_extensions, core::monadic::is(ext))) return false;

            return true;
        }();

        const auto raytracing_available = [&] {
            for (const auto& ext : RAYTRACING_EXTENSIONS)
                if (std::ranges::none_of(device_extensions, core::monadic::is(ext))) return false;

            return true;
        }();

        const auto extensions = [&] {
            constexpr auto as_czstring = [](const auto& v) { return std::data(v); };

            auto e = transform(BASE_EXTENSIONS, as_czstring);
            if (swapchain_available and info.enable_swapchain)
                merge(e, transform(SWAPCHAIN_EXTENSIONS, as_czstring));
            if (raytracing_available and info.enable_raytracing)
                merge(e, transform(RAYTRACING_EXTENSIONS, as_czstring));

            return e;
        }();

        const auto acceleration_feature = VkPhysicalDeviceAccelerationStructureFeaturesKHR {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR
        };
        const auto rt_pipeline_feature = VkPhysicalDeviceRayTracingPipelineFeaturesKHR {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
            .pNext = std::bit_cast<void*>(&acceleration_feature)
        };

        const auto next = [&]() -> void* {
            if (raytracing_available and info.enable_raytracing)
                return std::bit_cast<void*>(&rt_pipeline_feature);
            return nullptr;
        }();

        const auto create_info = VkDeviceCreateInfo {
            .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext                   = next,
            .queueCreateInfoCount    = as<UInt32>(std::ranges::size(queue_create_infos)),
            .pQueueCreateInfos       = std::ranges::data(queue_create_infos),
            .ppEnabledExtensionNames = std::ranges::data(extensions),
            .pEnabledFeatures        = &enabled_features
        };

        auto allocator_create_info = VmaAllocatorCreateInfo {
            .physicalDevice = m_physical_device->native_handle(),
            .instance       = instance.native_handle(),
        };

        return vk_call<VkDevice>(vkCreateDevice,
                                 m_physical_device->native_handle(),
                                 &create_info,
                                 nullptr)
          .transform(core::monadic::set(m_vk_handle))
          .and_then([this, &allocator_create_info] mutable noexcept {
              allocator_create_info.device = m_vk_handle;
              volkLoadDeviceTable(&m_vk_device_table, m_vk_handle);
              return vk_call<VmaVulkanFunctions>(vmaImportVulkanFunctionsFromVolk,
                                                 &allocator_create_info);
          })
          .transform(core::monadic::set(m_vma_function_table))
          .and_then([this, &allocator_create_info]() mutable noexcept {
              allocator_create_info.pVulkanFunctions = &m_vma_function_table;

              return vk_call<VmaAllocator>(vmaCreateAllocator, &allocator_create_info);
          })
          .transform(core::monadic::set(m_vma_allocator))
          .transform([this, &raster_queue] noexcept {
              if (raster_queue.id)
                  m_raster_queue = QueueEntry { .id    = *raster_queue.id,
                                                .count = raster_queue.count,
                                                .flags = raster_queue.flags };

              set_object_name(*this,
                              std::format("StormKit:Device ({})",
                                          m_physical_device->info().device_name));
          })
          .transform_error(core::monadic::narrow<Result>());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Device::wait_for_fences(std::span<const Ref<const Fence>> fences,
                                 bool                              wait_all,
                                 const std::chrono::milliseconds&  timeout) const noexcept
      -> Expected<Result> {
        const auto vk_fences = fences
                               | std::views::transform(to_vkhandle<Fence>)
                               | std::ranges::to<std::vector>();

        return vk_call<VkResult>(m_vk_device_table.vkWaitForFences,
                                 { VK_SUCCESS, VK_NOT_READY },
                                 m_vk_handle,
                                 std::ranges::size(vk_fences),
                                 std::ranges::data(vk_fences),
                                 wait_all,
                                 std::chrono::duration_cast<std::chrono::nanoseconds>(timeout)
                                   .count())
          .transform(core::monadic::narrow<Result>())
          .transform_error(core::monadic::narrow<Result>());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Device::reset_fences(std::span<const Ref<const Fence>> fences) const noexcept
      -> Expected<void> {
        const auto vk_fences = fences
                               | std::views::transform(to_vkhandle<Fence>)
                               | std::ranges::to<std::vector>();

        return vk_call(m_vk_device_table.vkResetFences,
                       m_vk_handle,
                       std ::ranges::size(vk_fences),
                       std::ranges::data(vk_fences))
          .transform_error(core::monadic::narrow<Result>());
    }
} // namespace stormkit::gpu
