// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/try_expected.hpp>

#include <stormkit/gpu/vulkan.hpp>

module stormkit.gpu.resource;

import std;

import stormkit.core;

import stormkit.gpu.core;

namespace stormkit::gpu {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto BufferInterface<Base>::map(ioffset offset) noexcept -> Expected<byte*> {
        EXPECTS(allocation() and Base::native_handle());
        EXPECTS(offset < as<ioffset>(size()));

        const auto& device     = Base::owner();
        const auto& allocator  = device.allocator();
        const auto& allocation = this->allocation();

        auto ptr = Try(vk::call_checked<void*>(vmaMapMemory, allocator, allocation));

        Base::m_mapped_pointer = std::bit_cast<byte*>(ptr);
        Base::m_mapped_pointer += offset;
        return Base::m_mapped_pointer;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto BufferInterface<Base>::flush(ioffset offset, usize size) const noexcept -> Expected<void> {
        EXPECTS(allocation() and Base::native_handle());
        EXPECTS(offset <= as<ioffset>(this->size()));
        EXPECTS(size <= this->size());

        const auto& device     = Base::owner();
        const auto& allocator  = device.allocator();
        const auto& allocation = this->allocation();

        return vk::call_checked(vmaFlushAllocation, allocator, allocation, offset, size);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto BufferInterface<Base>::unmap() noexcept -> void {
        if (not mapped()) return;

        if constexpr (cmeta::SameAs<Base, view::BufferImplementation>)
            if (is_persistently_mapped()) return;

        EXPECTS(allocation() and Base::native_handle());

        const auto& device     = Base::owner();
        const auto& allocator  = device.allocator();
        const auto& allocation = this->allocation();

        vk::call(vmaUnmapMemory, allocator, allocation);

        Base::m_mapped_pointer = nullptr;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto BufferInterface<Base>::upload(byte_view<> data, ioffset offset) noexcept -> Expected<void> {
        EXPECTS(stdr::size(data) <= this->size());

        if (is_persistently_mapped()) {
            stdr::copy(data, Base::m_mapped_pointer);
            Return {};
        }

        auto gpu_data = Try(map(offset, stdr::size(data)));
        stdr::copy(data, stdr::begin(gpu_data));
        unmap();

        Return {};
    }

    template class BufferInterface<BufferImplementation>;
    template class BufferInterface<view::BufferImplementation>;

    /////////////////////////////////////
    /////////////////////////////////////
    auto BufferImplementation::do_init(PrivateTag, const CreateInfo& _create_info) noexcept -> Expected<void> {
        m_usages                 = _create_info.usages;
        m_size                   = _create_info.size;
        m_memory_properties      = _create_info.properties;
        m_is_persistently_mapped = _create_info.persistently_mapped;

        const auto& device       = owner();
        const auto& device_table = device.device_table();
        const auto  create_info  = VkBufferCreateInfo {
            .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext                 = nullptr,
            .flags                 = 0,
            .size                  = m_size,
            .usage                 = vk::to_vk<VkBufferUsageFlags>(m_usages),
            .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices   = nullptr,
        };
        m_vk_handle = Try(vk::call_checked<VkBuffer>(device_table.vkCreateBuffer, device, &create_info, nullptr));

        const auto vma_create_info = VmaAllocationCreateInfo {
            .flags          = 0,
            .usage          = VMA_MEMORY_USAGE_UNKNOWN,
            .requiredFlags  = vk::to_vk<VkMemoryPropertyFlags>(m_memory_properties),
            .preferredFlags = 0,
            .memoryTypeBits = 0,
            .pool           = nullptr,
            .pUserData      = nullptr,
            .priority       = 0
        };
        const auto allocator = device.allocator();
        auto       out       = VmaAllocation { VK_NULL_HANDLE };
        Try(vk::call_checked(vmaAllocateMemoryForBuffer, allocator, m_vk_handle, &vma_create_info, &out, nullptr));
        m_vma_allocation = { [allocator](VmaAllocation handle) noexcept {
            if (handle) { vmaFreeMemory(allocator, handle); }
        } };
        m_vma_allocation = std::move(out);
        Try(vk::call_checked(vmaBindBufferMemory, allocator, m_vma_allocation, m_vk_handle));

        if (m_is_persistently_mapped) {
            auto ptr         = Try(vk::call_checked<void*>(vmaMapMemory, allocator, m_vma_allocation));
            m_mapped_pointer = std::bit_cast<byte*>(ptr);
        }

        Return {};
    }

    // /////////////////////////////////////
    // /////////////////////////////////////
    // auto BufferImplementation::find_memory_type(u32                                     type_filter,
    //                                             VkMemoryPropertyFlagBits                properties,
    //                                             const VkPhysicalDeviceMemoryProperties& mem_properties,
    //                                             const VkMemoryRequirements&) noexcept -> u32 {
    //     for (const auto i : range(mem_properties.memoryTypeCount)) {
    //         if ((type_filter & (1 << i))
    //             and (check_flag_bit(static_cast<VkMemoryPropertyFlagBits>(mem_properties.memoryTypes[i].propertyFlags),
    //                                 properties)))
    //             return i;
    //     }

    //    return 0;
    // }
} // namespace stormkit::gpu
