// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module stormkit.gpu;

import std;

import stormkit.core;
import stormkit.gpu.core;
import stormkit.gpu.vulkan;

import :resource.buffer;

namespace stormkit::gpu {
    /////////////////////////////////////
    /////////////////////////////////////
    auto Buffer::do_init(MemoryPropertyFlag memory_properties) noexcept -> Expected<void> {
        const auto create_info = VkBufferCreateInfo {
            .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext                 = nullptr,
            .flags                 = 0,
            .size                  = m_size,
            .usage                 = to_vkflags<VkBufferUsageFlagBits>(m_usages),
            .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices   = nullptr,
        };
        return vk_call<VkBuffer>(m_vk_device_table->vkCreateBuffer,
                                 m_vk_device,
                                 &create_info,
                                 nullptr)
          .transform(core::monadic::set(m_vk_handle))
          .and_then([this, &memory_properties] noexcept -> VulkanExpected<VmaAllocation> {
              const auto create_info = VmaAllocationCreateInfo {
                  .flags          = 0,
                  .usage          = VMA_MEMORY_USAGE_UNKNOWN,
                  .requiredFlags  = to_vkflags<VkMemoryPropertyFlagBits>(memory_properties),
                  .preferredFlags = 0,
                  .memoryTypeBits = 0,
                  .pool           = nullptr,
                  .pUserData      = nullptr,
                  .priority       = 0
              };

              auto allocation = VmaAllocation { nullptr };
              auto result     = vmaAllocateMemoryForBuffer(m_vma_allocator,
                                                       m_vk_handle,
                                                       &create_info,
                                                       &allocation,
                                                       nullptr);
              if (result != VK_SUCCESS) return std::unexpected { result };

              return VulkanExpected<VmaAllocation> { allocation };
          })
          .transform(core::monadic::set(m_vma_allocation))
          .and_then([this] noexcept {
              return vk_call(vmaBindBufferMemory, m_vma_allocator, m_vma_allocation, m_vk_handle);
          })
          .transform_error(monadic::from_vkflags<Result>())
          .and_then([this] noexcept -> Expected<Byte*> {
              if (m_is_persistently_mapped) return map(0u);

              return {};
          })
          .transform(core::monadic::discard());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Buffer::find_memory_type(UInt                                    type_filter,
                                  VkMemoryPropertyFlagBits                properties,
                                  const VkPhysicalDeviceMemoryProperties& mem_properties,
                                  const VkMemoryRequirements&) noexcept -> UInt {
        for (const auto i : range(mem_properties.memoryTypeCount)) {
            if ((type_filter & (1 << i))
                and (check_flag_bit(static_cast<VkMemoryPropertyFlagBits>(mem_properties
                                                                            .memoryTypes[i]
                                                                            .propertyFlags),
                                    properties)))
                return i;
        }

        return 0;
    }
} // namespace stormkit::gpu
