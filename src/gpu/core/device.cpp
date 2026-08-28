// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/memory_macro.hpp>
#include <stormkit/core/try_expected.hpp>

#include <stormkit/log/log_macro.hpp>

#include <stormkit/gpu/api.hpp>
#include <stormkit/gpu/vulkan.hpp>

module stormkit.gpu.core;

import std;

import stormkit.core;
import stormkit.log;

using namespace std::literals;

namespace stdr = std::ranges;
namespace stdv = std::views;

namespace cmonadic = stormkit::core::monadic;

namespace {
    constexpr auto RAYTRACING_EXTENSIONS = to_array<czstring>({
      VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
      VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
      VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
      VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
      VK_KHR_SPIRV_1_4_EXTENSION_NAME,
      VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
      VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
    });

    constexpr auto BASE_EXTENSIONS      = to_array<czstring>({
      VK_KHR_MAINTENANCE_3_EXTENSION_NAME,
      VK_KHR_MAINTENANCE_4_EXTENSION_NAME,
      VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
    });
    constexpr auto SWAPCHAIN_EXTENSIONS = to_array<czstring>({
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    });

    auto vma_import_functions_from_volk(const VmaAllocatorCreateInfo* pAllocatorCreateInfo,
                                        VolkDeviceTable*              device_table,
                                        VmaVulkanFunctions*           pDstVulkanFunctions) noexcept -> VkResult {
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

        COPY_GLOBAL_TO_VMA_FUNC(vkGetPhysicalDeviceMemoryProperties, vkGetPhysicalDeviceMemoryProperties)

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
            COPY_GLOBAL_TO_VMA_FUNC(vkGetPhysicalDeviceMemoryProperties2, vkGetPhysicalDeviceMemoryProperties2KHR)

            COPY_DEVICE_TO_VMA_FUNC(vkGetBufferMemoryRequirements2, vkGetBufferMemoryRequirements2KHR)

            COPY_DEVICE_TO_VMA_FUNC(vkGetImageMemoryRequirements2, vkGetImageMemoryRequirements2KHR)

            COPY_DEVICE_TO_VMA_FUNC(vkBindBufferMemory2, vkBindBufferMemory2KHR)

            COPY_DEVICE_TO_VMA_FUNC(vkBindImageMemory2, vkBindImageMemory2KHR)
        }

#endif

#if VMA_VULKAN_VERSION >= 1003000

        if (pAllocatorCreateInfo->vulkanApiVersion >= VK_MAKE_VERSION(1, 3, 0))

        {
            COPY_DEVICE_TO_VMA_FUNC(vkGetDeviceBufferMemoryRequirements, vkGetDeviceBufferMemoryRequirements)

            COPY_DEVICE_TO_VMA_FUNC(vkGetDeviceImageMemoryRequirements, vkGetDeviceImageMemoryRequirements)
        }

#endif

#if VMA_KHR_MAINTENANCE4

        if ((pAllocatorCreateInfo->flags & VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT) != 0)

        {
            COPY_DEVICE_TO_VMA_FUNC(vkGetDeviceBufferMemoryRequirementsKHR, vkGetDeviceBufferMemoryRequirements)

            COPY_DEVICE_TO_VMA_FUNC(vkGetDeviceImageMemoryRequirementsKHR, vkGetDeviceImageMemoryRequirements)
        }

#endif

#if VMA_DEDICATED_ALLOCATION

        if ((pAllocatorCreateInfo->flags & VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT) != 0)

        {
            COPY_DEVICE_TO_VMA_FUNC(vkGetBufferMemoryRequirements2KHR, vkGetBufferMemoryRequirements2KHR)

            COPY_DEVICE_TO_VMA_FUNC(vkGetImageMemoryRequirements2KHR, vkGetImageMemoryRequirements2KHR)
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
            COPY_GLOBAL_TO_VMA_FUNC(vkGetPhysicalDeviceMemoryProperties2KHR, vkGetPhysicalDeviceMemoryProperties2KHR)
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
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto DeviceInterface<Base>::wait_idle() const noexcept -> Expected<void> {
        const auto device_table = this->device_table();
        Try(vk::call_checked(device_table.vkDeviceWaitIdle, *this));
        Return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto DeviceInterface<Base>::wait_for_fences(array_view<const view::Fence>    fences,
                                                bool                             wait_all,
                                                const std::chrono::milliseconds& timeout) const noexcept -> Expected<Result> {
        const auto device_table = this->device_table();
        const auto _fences      = transform(fences, vk::monadic::to_vk());

        const auto result = Try((vk::call_checked<VkResult, VK_SUCCESS, VK_NOT_READY>(
          device_table.vkWaitForFences,
          *this,
          stdr::size(_fences),
          stdr::data(_fences),
          wait_all,
          std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count())));
        return vk::from_vk<Result>(result);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto DeviceInterface<Base>::reset_fences(array_view<const view::Fence> fences) const noexcept -> Expected<void> {
        const auto device_table = this->device_table();

        const auto _fences = transform(fences, vk::monadic::to_vk());
        Try(vk::call_checked(device_table.vkResetFences, *this, stdr::size(_fences), stdr::data(_fences)));
        Return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto DeviceInterface<Base>::set_object_name(u64 object, DebugObjectType type, string_view name) const noexcept
      -> Expected<void> {
        if (not vkSetDebugUtilsObjectNameEXT) return {};

        const auto info = VkDebugUtilsObjectNameInfoEXT {
            .sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext        = nullptr,
            .objectType   = vk::to_vk<VkObjectType>(type),
            .objectHandle = object,
            .pObjectName  = stdr::data(name),
        };

        Try(vk::call_checked(vkSetDebugUtilsObjectNameEXT, *this, &info));
        Return {};
    }

    template class DeviceInterface<DeviceImplementation>;
    template class DeviceInterface<view::DeviceImplementation>;

    /////////////////////////////////////
    /////////////////////////////////////
    auto DeviceImplementation::do_init(PrivateTag, const CreateInfo& info) noexcept -> Expected<void> {
        const auto  physical_device = owner();
        const auto& queue_families  = physical_device.queue_families();

        auto i          = 0_u32;
        auto priorities = dyn_array<dyn_array<f32>> {};
        priorities.reserve(stdr::size(queue_families));
        const auto queue_create_infos = transform(queue_families, [this, &i, &priorities](const auto& family) noexcept {
            auto& priority = priorities.emplace_back();
            priority.resize(family.count, 1.f);

            m_queue_entries.emplace_back(QueueEntry {
              .id    = i,
              .count = family.count,
              .flags = family.flags,
            });

            return VkDeviceQueueCreateInfo {
                .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext            = nullptr,
                .flags            = 0,
                .queueFamilyIndex = i++,
                .queueCount       = family.count,
                .pQueuePriorities = stdr::data(priority),
            };
        });

        // const auto& capabilities         = physical_device.capabilities();
        const auto enabled_1_0_features = init_by<VkPhysicalDeviceFeatures>([](auto& out) static noexcept {
            out.multiDrawIndirect = true;
            out.samplerAnisotropy = true;
        });
        const auto enabled_1_2_features = init_by<VkPhysicalDeviceVulkan12Features>([](auto& out) static noexcept {
            out.sType                                    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
            out.pNext                                    = nullptr;
            out.descriptorIndexing                       = true;
            out.descriptorBindingVariableDescriptorCount = true;
            out.runtimeDescriptorArray                   = true;
            out.bufferDeviceAddress                      = true;
        });
        const auto enabled_1_3_features = init_by<VkPhysicalDeviceVulkan13Features>([&enabled_1_2_features](auto& out) noexcept {
            out.sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
            out.pNext            = std::bit_cast<void*>(&enabled_1_2_features);
            out.synchronization2 = true;
            out.dynamicRendering = true;
        });

        const auto device_extensions = physical_device.extensions();

        const auto swapchain_available = [&] {
            for (const auto& ext : SWAPCHAIN_EXTENSIONS)
                if (stdr::none_of(device_extensions, cmonadic::is_equal(ext))) return false;

            return true;
        }();

        const auto raytracing_available = [&] {
            for (const auto& ext : RAYTRACING_EXTENSIONS)
                if (stdr::none_of(device_extensions, cmonadic::is_equal(ext))) return false;

            return true;
        }();

        const auto extensions = [&] {
            constexpr auto as_czstring = [](const auto& v) { return v; };

            auto e = transform(BASE_EXTENSIONS, as_czstring);
            if (swapchain_available and info.enable_swapchain) merge(e, transform(SWAPCHAIN_EXTENSIONS, as_czstring));
            if (raytracing_available and info.enable_raytracing) merge(e, transform(RAYTRACING_EXTENSIONS, as_czstring));

            return e;
        }();

        // const auto acceleration_feature = [] static noexcept {
        //     auto out  = zeroed<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
        //     out.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
        //     out.pNext = nullptr;
        //     return out;
        // }();
        // const auto rt_pipeline_feature = [&acceleration_feature] noexcept {
        //     auto out  = zeroed<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
        //     out.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
        //     out.pNext = std::bit_cast<void*>(&acceleration_feature);
        //     return out;
        // };

        // const auto next = [&]() -> void* {
        //     if (raytracing_available and info.enable_raytracing) return std::bit_cast<void*>(&rt_pipeline_feature);
        //     return nullptr;
        // }();

        const auto create_info = VkDeviceCreateInfo {
            .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext                   = std::bit_cast<void*>(&enabled_1_3_features),
            .flags                   = 0,
            .queueCreateInfoCount    = as<u32>(stdr::size(queue_create_infos)),
            .pQueueCreateInfos       = stdr::data(queue_create_infos),
            .enabledLayerCount       = 0,
            .ppEnabledLayerNames     = nullptr,
            .enabledExtensionCount   = as<u32>(stdr::size(extensions)),
            .ppEnabledExtensionNames = stdr::data(extensions),
            .pEnabledFeatures        = &enabled_1_0_features,
        };

        m_vk_handle = Try(vk::call_checked<VkDevice>(vkCreateDevice, physical_device.native_handle(), &create_info, nullptr));
        volkLoadDeviceTable(&m_vk_device_table, m_vk_handle);

        auto allocator_create_info = VmaAllocatorCreateInfo {
            .flags                          = 0,
            .physicalDevice                 = physical_device,
            .device                         = m_vk_handle,
            .preferredLargeHeapBlockSize    = 0,
            .pAllocationCallbacks           = nullptr,
            .pDeviceMemoryCallbacks         = nullptr,
            .pHeapSizeLimit                 = nullptr,
            .pVulkanFunctions               = nullptr,
            .instance                       = physical_device.instance(),
            .vulkanApiVersion               = vk::make_version<i32>(1, 4, 0),
            .pTypeExternalMemoryHandleTypes = nullptr,
        };
        m_vma_function_table = Try(vk::call_checked<VmaVulkanFunctions>(vma_import_functions_from_volk,
                                                                        &allocator_create_info,
                                                                        &m_vk_device_table));

        allocator_create_info.pVulkanFunctions = &m_vma_function_table;

        m_vma_allocator = Try(vk::call_checked<VmaAllocator>(vmaCreateAllocator, &allocator_create_info));

        const auto name = std::format("StormKit:device ({})", physical_device.info().device_name);
        if (not vkSetDebugUtilsObjectNameEXT) Return {};

        const auto vk_object  = native_handle();
        const auto debug_info = VkDebugUtilsObjectNameInfoEXT {
            .sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext        = nullptr,
            .objectType   = vk::to_vk<VkObjectType>(trait::GpuObject<DeviceTag>::DEBUG_TYPE),
            .objectHandle = as<u64>(std::bit_cast<uptr>(vk_object)),
            .pObjectName  = stdr::data(name),
        };

        Try(vk::call_checked(vkSetDebugUtilsObjectNameEXT, *this, &debug_info));

        Return {};
    }

    namespace vk {
        /////////////////////////////////////
        /////////////////////////////////////
        auto imgui_vk_loader(const char* _func_name, void* user_data) noexcept -> PFN_vkVoidFunction {
            const auto  func_name    = string_view { _func_name };
            const auto& device       = *std::bit_cast<const Device*>(user_data);
            const auto  device_table = device.device_table();

            if (func_name == "vkAllocateCommandBuffers")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkAllocateCommandBuffers);
            else if (func_name == "vkAllocateDescriptorSets")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkAllocateDescriptorSets);
            else if (func_name == "vkAllocateMemory")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkAllocateMemory);
            else if (func_name == "vkBeginCommandBuffer")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkBeginCommandBuffer);
            else if (func_name == "vkBindBufferMemory")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkBindBufferMemory);
            else if (func_name == "vkBindImageMemory")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkBindImageMemory);
            else if (func_name == "vkCmdBindDescriptorSets")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCmdBindDescriptorSets);
            else if (func_name == "vkCmdBindIndexBuffer")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCmdBindIndexBuffer);
            else if (func_name == "vkCmdBindPipeline")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCmdBindPipeline);
            else if (func_name == "vkCmdBindVertexBuffers")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCmdBindVertexBuffers);
            else if (func_name == "vkCmdCopyBufferToImage")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCmdCopyBufferToImage);
            else if (func_name == "vkCmdDrawIndexed")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCmdDrawIndexed);
            else if (func_name == "vkCmdPipelineBarrier")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCmdPipelineBarrier);
            else if (func_name == "vkCmdPushConstants")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCmdPushConstants);
            else if (func_name == "vkCmdSetScissor")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCmdSetScissor);
            else if (func_name == "vkCmdSetViewport")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCmdSetViewport);
            else if (func_name == "vkCreateBuffer")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCreateBuffer);
            else if (func_name == "vkCreateCommandPool")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCreateCommandPool);
            else if (func_name == "vkCreateDescriptorPool")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCreateDescriptorPool);
            else if (func_name == "vkCreateDescriptorSetLayout")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCreateDescriptorSetLayout);
            else if (func_name == "vkCreateFence")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCreateFence);
            else if (func_name == "vkCreateFramebuffer")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCreateFramebuffer);
            else if (func_name == "vkCreateGraphicsPipelines")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCreateGraphicsPipelines);
            else if (func_name == "vkCreateImage")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCreateImage);
            else if (func_name == "vkCreateImageView")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCreateImageView);
            else if (func_name == "vkCreatePipelineLayout")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCreatePipelineLayout);
            else if (func_name == "vkCreateRenderPass")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCreateRenderPass);
            else if (func_name == "vkCreateSampler")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCreateSampler);
            else if (func_name == "vkCreateSemaphore")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCreateSemaphore);
            else if (func_name == "vkCreateShaderModule")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCreateShaderModule);
            else if (func_name == "vkCreateSwapchainKHR")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCreateSwapchainKHR);
            else if (func_name == "vkDestroyBuffer")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkDestroyBuffer);
            else if (func_name == "vkDestroyCommandPool")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkDestroyCommandPool);
            else if (func_name == "vkDestroyDescriptorPool")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkDestroyDescriptorPool);
            else if (func_name == "vkDestroyDescriptorSetLayout")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkDestroyDescriptorSetLayout);
            else if (func_name == "vkDestroyFence")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkDestroyFence);
            else if (func_name == "vkDestroyFramebuffer")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkDestroyFramebuffer);
            else if (func_name == "vkDestroyImage")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkDestroyImage);
            else if (func_name == "vkDestroyImageView")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkDestroyImageView);
            else if (func_name == "vkDestroyPipeline")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkDestroyPipeline);
            else if (func_name == "vkDestroyPipelineLayout")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkDestroyPipelineLayout);
            else if (func_name == "vkDestroyRenderPass")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkDestroyRenderPass);
            else if (func_name == "vkDestroySampler")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkDestroySampler);
            else if (func_name == "vkDestroySemaphore")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkDestroySemaphore);
            else if (func_name == "vkDestroyShaderModule")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkDestroyShaderModule);
            else if (func_name == "vkDestroySurfaceKHR")
                return std::bit_cast<PFN_vkVoidFunction>(vkDestroySurfaceKHR);
            else if (func_name == "vkDestroySwapchainKHR")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkDestroySwapchainKHR);
            else if (func_name == "vkDeviceWaitIdle")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkDeviceWaitIdle);
            else if (func_name == "vkEnumeratePhysicalDevices")
                return std::bit_cast<PFN_vkVoidFunction>(vkEnumeratePhysicalDevices);
            else if (func_name == "vkEndCommandBuffer")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkEndCommandBuffer);
            else if (func_name == "vkFlushMappedMemoryRanges")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkFlushMappedMemoryRanges);
            else if (func_name == "vkFreeCommandBuffers")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkFreeCommandBuffers);
            else if (func_name == "vkFreeDescriptorSets")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkFreeDescriptorSets);
            else if (func_name == "vkFreeMemory")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkFreeMemory);
            else if (func_name == "vkGetBufferMemoryRequirements")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkGetBufferMemoryRequirements);
            else if (func_name == "vkGetDeviceQueue")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkGetDeviceQueue);
            else if (func_name == "vkGetImageMemoryRequirements")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkGetImageMemoryRequirements);
            else if (func_name == "vkGetPhysicalDeviceProperties")
                return std::bit_cast<PFN_vkVoidFunction>(vkGetPhysicalDeviceProperties);
            else if (func_name == "vkGetPhysicalDeviceMemoryProperties")
                return std::bit_cast<PFN_vkVoidFunction>(vkGetPhysicalDeviceMemoryProperties);
            else if (func_name == "vkGetPhysicalDeviceQueueFamilyProperties")
                return std::bit_cast<PFN_vkVoidFunction>(vkGetPhysicalDeviceQueueFamilyProperties);
            else if (func_name == "vkGetPhysicalDeviceSurfaceCapabilitiesKHR")
                return std::bit_cast<PFN_vkVoidFunction>(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
            else if (func_name == "vkGetPhysicalDeviceSurfaceFormatsKHR")
                return std::bit_cast<PFN_vkVoidFunction>(vkGetPhysicalDeviceSurfaceFormatsKHR);
            else if (func_name == "vkGetPhysicalDeviceSurfacePresentModesKHR")
                return std::bit_cast<PFN_vkVoidFunction>(vkGetPhysicalDeviceSurfacePresentModesKHR);
            else if (func_name == "vkGetSwapchainImagesKHR")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkGetSwapchainImagesKHR);
            else if (func_name == "vkMapMemory")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkMapMemory);
            else if (func_name == "vkQueueSubmit")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkQueueSubmit);
            else if (func_name == "vkQueueWaitIdle")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkQueueWaitIdle);
            else if (func_name == "vkResetCommandPool")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkResetCommandPool);
            else if (func_name == "vkResetFences")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkResetFences);
            else if (func_name == "vkUnmapMemory")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkUnmapMemory);
            else if (func_name == "vkUpdateDescriptorSets")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkUpdateDescriptorSets);
            else if (func_name == "vkWaitForFences")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkWaitForFences);
            else if (func_name == "vkCmdBeginRendering")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCmdBeginRendering);
            else if (func_name == "vkCmdEndRendering")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCmdEndRendering);
            else if (func_name == "vkCmdBeginRenderingKHR")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCmdBeginRenderingKHR);
            else if (func_name == "vkCmdEndRenderingKHR")
                return std::bit_cast<PFN_vkVoidFunction>(device_table.vkCmdEndRenderingKHR);

            ensures(false, std::format("Unhandled vk func {}", func_name));
            std::unreachable();
        }
    } // namespace vk
} // namespace stormkit::gpu
