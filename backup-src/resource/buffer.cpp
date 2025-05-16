// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module stormkit.gpu;

import std;

import stormkit.core;

import :core;
import stormkit.gpu.vulkan;

import :resource.buffer;

namespace stormkit::gpu {
    /////////////////////////////////////
    /////////////////////////////////////
    auto Buffer::do_init() noexcept -> Expected<void> {
        const auto create_info = VkBufferCreateInfo {
            .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size        = m_size,
            .usage       = to_vkflags(m_usages),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };
        return vk_call(vkCreateBuffer, m_vk_device, &create_info, nullptr)
          .transform(core::monadic::set(m_vk_handle))
          .transform([this, &info, &device]() noexcept -> VulkanExpected<void> {
              const auto requirements = vk_call<VkMemoryRequirements>(m_vk_device, m_vk_handle);

              const auto allocate_info = VmaAllocationCreateInfo {
                  .requiredFlags = to_vkflags(info.property)
              };

              auto&& allocator = device.vmaAllocator();

              auto&& [error, vma_allocation] = allocator
                                                 .allocateMemoryUnique(requirements, allocate_info);
              if (error != vk::Result::eSuccess)
                  return std::unexpected { narrow<vk::Result>(error) };

              m_vma_allocation = std::move(vma_allocation);

              error = allocator.bindBufferMemory(*m_vma_allocation, *m_vk_buffer.get());
              if (error != vk::Result::eSuccess)
                  return std::unexpected { narrow<vk::Result>(error) };

              if (m_is_persistently_mapped) auto _ = map(device, 0u);

              return {};
          })
          .transform_error(monadic::from_vkflags<Result>());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Buffer::find_memory_type(UInt                                      type_filter,
                                  vk::MemoryPropertyFlags                   properties,
                                  const vk::PhysicalDeviceMemoryProperties& mem_properties,
                                  const vk::MemoryRequirements&) -> UInt {
        for (auto i : range(mem_properties.memoryTypeCount)) {
            if ((type_filter & (1 << i))
                and (check_flag_bit(mem_properties.memoryTypes[i].propertyFlags, properties)))
                return i;
        }

        return 0;
    }
} // namespace stormkit::gpu
