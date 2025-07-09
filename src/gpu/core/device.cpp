// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/memory_macro.hpp>

#include <stormkit/log/log_macro.hpp>

#include <stormkit/core/contract_macro.hpp>

#include <volk.h>

module stormkit.gpu.core;

import std;

import stormkit.core;
import stormkit.log;

using namespace std::literals;

namespace stdr = std::ranges;
namespace stdv = std::views;

namespace {
    constexpr auto RAYTRACING_EXTENSIONS = std::array {
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
        VK_KHR_SPIRV_1_4_EXTENSION_NAME,
        VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
    };

    constexpr auto BASE_EXTENSIONS = std::array {
        VK_KHR_MAINTENANCE_3_EXTENSION_NAME,
        // "VK_KHR_maintenance4"sv, "VK_KHR_maintenance5"sv, "VK_KHR_maintenance6"sv
    };
    constexpr auto SWAPCHAIN_EXTENSIONS = std::array {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    auto vma_import_functions_from_volk(const VmaAllocatorCreateInfo* pAllocatorCreateInfo,
                                        VolkDeviceTable*              device_table,
                                        VmaVulkanFunctions*           pDstVulkanFunctions) noexcept
      -> VkResult {
        using std::memset;
        EXPECTS(pAllocatorCreateInfo != nullptr);
        EXPECTS(pAllocatorCreateInfo->instance != nullptr);
        EXPECTS(pAllocatorCreateInfo->device != nullptr);

        memset(pDstVulkanFunctions, 0, sizeof(*pDstVulkanFunctions));

        auto& src = *device_table;

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
            std::optional<u32>  id    = std::nullopt;
            u32                 count = 0u;
            std::array<Byte, 3> _     = {}; // padding
            QueueFlag           flags = QueueFlag {};
        };

        const auto raster_queue = [&queue_families]() -> Queue_ {
            const auto it = stdr::find_if(queue_families, find_queue<QueueFlag::GRAPHICS>());
            if (it == stdr::cend(queue_families)) return {};

            return {
                .id    = as<u32>(std::distance(stdr::cbegin(queue_families), it)),
                .count = it->count,
                .flags = it->flags,
            };
        }();

        const auto compute_queue = [&queue_families]() -> Queue_ {
            const auto it = stdr::find_if(queue_families,
                                          find_queue<QueueFlag::TRANSFER, QueueFlag::GRAPHICS>());
            if (it == stdr::cend(queue_families)) return {};

            return { .id    = as<u32>(std::distance(stdr::cbegin(queue_families), it)),
                     .count = it->count,
                     .flags = it->flags };
        }();

        const auto transfert_queue = [&queue_families]() -> Queue_ {
            const auto it = stdr::
              find_if(queue_families,
                      find_queue<QueueFlag::COMPUTE, QueueFlag::GRAPHICS, QueueFlag::TRANSFER>());
            if (it == stdr::cend(queue_families)) return {};

            return { .id    = as<u32>(std::distance(stdr::cbegin(queue_families), it)),
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
        priorities.reserve(stdr::size(queues));

        const auto queue_create_infos = transform(queues, [&priorities](auto queue) {
            auto& priority = priorities.emplace_back();

            priority.resize(queue->count, 1.f);

            return VkDeviceQueueCreateInfo {
                .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext            = nullptr,
                .flags            = 0,
                .queueFamilyIndex = queue->id.value(),
                .queueCount       = 1,
                .pQueuePriorities = stdr::data(priority),
            };
        });

        const auto& capabilities     = m_physical_device->capabilities();
        const auto  enabled_features = [&capabilities] noexcept {
            auto out              = zeroed<VkPhysicalDeviceFeatures>();
            out.sampleRateShading = capabilities.features.sampler_rate_shading;
            out.multiDrawIndirect = capabilities.features.multi_draw_indirect;
            out.fillModeNonSolid  = capabilities.features.fill_Mode_non_solid;
            out.samplerAnisotropy = capabilities.features.sampler_anisotropy;
            return out;
        }();

        const auto device_extensions = m_physical_device->extensions();

        device_logger.dlog("Device extensions: {}", device_extensions);

        const auto swapchain_available = [&] {
            for (const auto& ext : SWAPCHAIN_EXTENSIONS)
                if (stdr::none_of(device_extensions, core::monadic::is(ext))) return false;

            return true;
        }();

        if (not swapchain_available) device_logger.wlog("Swapchain extension are not available");

        const auto raytracing_available = [&] {
            for (const auto& ext : RAYTRACING_EXTENSIONS)
                if (stdr::none_of(device_extensions, core::monadic::is(ext))) return false;

            return true;
        }();

        const auto extensions = [&] {
            constexpr auto as_czstring = [](const auto& v) { return v; };

            auto e = transform(BASE_EXTENSIONS, as_czstring);
            if (swapchain_available and info.enable_swapchain)
                merge(e, transform(SWAPCHAIN_EXTENSIONS, as_czstring));
            if (raytracing_available and info.enable_raytracing)
                merge(e, transform(RAYTRACING_EXTENSIONS, as_czstring));

            return e;
        }();
        device_logger.ilog("Enabled device extensions: {}", extensions);

        const auto acceleration_feature = [] static noexcept {
            auto out  = zeroed<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
            out.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
            out.pNext = nullptr;
            return out;
        }();
        const auto rt_pipeline_feature = [&acceleration_feature] noexcept {
            auto out  = zeroed<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
            out.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
            out.pNext = std::bit_cast<void*>(&acceleration_feature);
            return out;
        };

        const auto next = [&]() -> void* {
            if (raytracing_available and info.enable_raytracing)
                return std::bit_cast<void*>(&rt_pipeline_feature);
            return nullptr;
        }();

        const auto create_info = VkDeviceCreateInfo {
            .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext                   = next,
            .flags                   = 0,
            .queueCreateInfoCount    = as<u32>(stdr::size(queue_create_infos)),
            .pQueueCreateInfos       = stdr::data(queue_create_infos),
            .enabledLayerCount       = 0,
            .ppEnabledLayerNames     = nullptr,
            .enabledExtensionCount   = as<u32>(stdr::size(extensions)),
            .ppEnabledExtensionNames = stdr::data(extensions),
            .pEnabledFeatures        = &enabled_features,
        };

        auto allocator_create_info = VmaAllocatorCreateInfo {
            .flags                          = 0,
            .physicalDevice                 = m_physical_device->native_handle(),
            .device                         = nullptr,
            .preferredLargeHeapBlockSize    = 0,
            .pAllocationCallbacks           = nullptr,
            .pDeviceMemoryCallbacks         = nullptr,
            .pHeapSizeLimit                 = nullptr,
            .pVulkanFunctions               = nullptr,
            .instance                       = instance.native_handle(),
            .vulkanApiVersion               = vk_make_version<i32>(1, 4, 0),
            .pTypeExternalMemoryHandleTypes = nullptr,
        };

        return vk_call<VkDevice>(vkCreateDevice,
                                 m_physical_device->native_handle(),
                                 &create_info,
                                 nullptr)
          .transform(core::monadic::set(m_vk_handle))
          .and_then([this, &allocator_create_info] mutable noexcept {
              allocator_create_info.device = m_vk_handle;
              volkLoadDeviceTable(&m_vk_device_table, m_vk_handle);
              auto raw    = m_vk_handle.value();
              m_vk_handle = { [vk_device_table = m_vk_device_table](auto handle) noexcept {
                  vk_device_table.vkDestroyDevice(handle, nullptr);
              } };
              m_vk_handle = std::move(raw);
              return vk_call<VmaVulkanFunctions>(vma_import_functions_from_volk,
                                                 &allocator_create_info,
                                                 &m_vk_device_table);
          })
          .transform(core::monadic::set(m_vma_function_table))
          .and_then([this, &allocator_create_info] mutable noexcept {
              allocator_create_info.pVulkanFunctions = &m_vma_function_table;

              return vk_call<VmaAllocator>(vmaCreateAllocator, &allocator_create_info);
          })
          .transform(core::monadic::set(m_vma_allocator))
          .transform_error(monadic::from_vk<Result>())
          .and_then([this, &raster_queue] noexcept {
              if (raster_queue.id)
                  m_raster_queue = QueueEntry { .id    = *raster_queue.id,
                                                .count = raster_queue.count,
                                                .flags = raster_queue.flags };

              return set_object_name(*this,
                                     std::format("StormKit:Device ({})",
                                                 m_physical_device->info().device_name));
          });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Device::wait_for_fences(std::span<const Ref<const Fence>> fences,
                                 bool                              wait_all,
                                 const std::chrono::milliseconds&  timeout) const noexcept
      -> Expected<Result> {
        const auto vk_fences = fences | stdv::transform(monadic::to_vk()) | stdr::to<std::vector>();

        return vk_call<VkResult>(m_vk_device_table.vkWaitForFences,
                                 { VK_SUCCESS, VK_NOT_READY },
                                 m_vk_handle,
                                 stdr::size(vk_fences),
                                 stdr::data(vk_fences),
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
        const auto vk_fences = fences | stdv::transform(monadic::to_vk()) | stdr::to<std::vector>();

        return vk_call(m_vk_device_table.vkResetFences,
                       m_vk_handle,
                       std ::ranges::size(vk_fences),
                       stdr::data(vk_fences))
          .transform_error(core::monadic::narrow<Result>());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Device::set_object_name(u64 object, DebugObjectType type, std::string_view name) const
      -> Expected<void> {
        if (not vkSetDebugUtilsObjectNameEXT) return {};

        const auto info = VkDebugUtilsObjectNameInfoEXT {
            .sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext        = nullptr,
            .objectType   = to_vk<VkObjectType>(type),
            .objectHandle = object,
            .pObjectName  = std::data(name),
        };

        return vk_call(vkSetDebugUtilsObjectNameEXT, m_vk_handle, &info)
          .transform_error(monadic::from_vk<Result>());
    }
} // namespace stormkit::gpu
