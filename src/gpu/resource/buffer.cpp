// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <volk.h>

module stormkit.gpu.resource;

import std;

import stormkit.core;

import stormkit.gpu.core;

namespace stormkit::gpu {
    /////////////////////////////////////
    /////////////////////////////////////
    auto Buffer::do_init(MemoryPropertyFlag memory_properties) noexcept -> Expected<void> {
        const auto create_info = VkBufferCreateInfo {
            .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext                 = nullptr,
            .flags                 = 0,
            .size                  = m_size,
            .usage                 = to_vk<VkBufferUsageFlags>(m_usages),
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
                  .requiredFlags  = to_vk<VkMemoryPropertyFlags>(memory_properties),
                  .preferredFlags = 0,
                  .memoryTypeBits = 0,
                  .pool           = nullptr,
                  .pUserData      = nullptr,
                  .priority       = 0
              };

              auto out    = VulkanExpected<VmaAllocation> { std::in_place, nullptr };
              auto result = vmaAllocateMemoryForBuffer(m_vma_allocator,
                                                       m_vk_handle,
                                                       &create_info,
                                                       &*out,
                                                       nullptr);
              if (result != VK_SUCCESS) out = std::unexpected { result };
              else {
                  m_vma_allocation = {
                      [vma_allocator = m_vma_allocator](VmaAllocation handle) noexcept {
                          if (handle) { vmaFreeMemory(vma_allocator, handle); }
                      }
                  };
              }

              return out;
          })
          .transform(core::monadic::set(m_vma_allocation))
          .and_then([this] noexcept {
              return vk_call(vmaBindBufferMemory, m_vma_allocator, m_vma_allocation, m_vk_handle);
          })
          .transform_error(monadic::from_vk<Result>())
          .and_then([this] noexcept -> Expected<Byte*> {
              if (m_is_persistently_mapped) return map(0u);

              return {};
          })
          .transform(core::monadic::discard());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Buffer::find_memory_type(u32                                     type_filter,
                                  VkMemoryPropertyFlagBits                properties,
                                  const VkPhysicalDeviceMemoryProperties& mem_properties,
                                  const VkMemoryRequirements&) noexcept -> u32 {
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
