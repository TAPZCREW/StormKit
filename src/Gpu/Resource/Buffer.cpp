// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module stormkit.Gpu;

import std;

import stormkit.Core;

import :Core;
import stormkit.Gpu.Vulkan;

import :Resource.Buffer;

namespace stormkit::gpu {
    /////////////////////////////////////
    /////////////////////////////////////
    Buffer::Buffer(const Device& device, const CreateInfo& info, bool persistently_mapped, Tag)
        : m_usages { info.usages }, m_size { info.size },
          m_is_persistently_mapped { persistently_mapped } {
        device.vkHandle()
            .createBuffer({ .size        = m_size,
                            .usage       = as<vk::BufferUsageFlagBits>(m_usages),
                            .sharingMode = vk::SharingMode::eExclusive })
            .transform(core::monadic::set(m_vk_buffer))
            .transform([this, &info, &device]() noexcept -> VulkanExpected<void> {
                const auto requirements = m_vk_buffer->getMemoryRequirements();

                auto allocate_info = vma::AllocationCreateInfo {}.setRequiredFlags(
                    as<vk::MemoryPropertyFlagBits>(info.property));

                auto&& allocator = device.vmaAllocator();

                auto&& [error, vma_allocation] =
                    allocator.allocateMemoryUnique(requirements, allocate_info);
                if (error != vk::Result::eSuccess)
                    return std::unexpected { as<vk::Result>(error) };

                m_vma_allocation = std::move(vma_allocation);

                error = allocator.bindBufferMemory(*m_vma_allocation, *m_vk_buffer.get());
                if (error != vk::Result::eSuccess)
                    return std::unexpected { as<vk::Result>(error) };

                if (m_is_persistently_mapped) [[maybe_unused]]
                    auto _ = map(device, 0u);

                return {};
            })
            .transform_error(core::monadic::map(core::monadic::as<Result>(), throwError()));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Buffer::findMemoryType(UInt                                           type_filter,
                                vk::MemoryPropertyFlags                        properties,
                                const vk::PhysicalDeviceMemoryProperties&      mem_properties,
                                [[maybe_unused]] const vk::MemoryRequirements& mem_requirements)
        -> UInt {
        for (auto i : range(mem_properties.memoryTypeCount)) {
            if ((type_filter & (1 << i)) and
                (checkFlag(mem_properties.memoryTypes[i].propertyFlags, properties)))
                return i;
        }

        return 0;
    }
} // namespace stormkit::gpu
