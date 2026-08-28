
// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/try_expected.hpp>

#include <stormkit/gpu/vulkan.hpp>

module stormkit.gpu.execution;

import std;

import stormkit.core;

import stormkit.gpu.core;

namespace stdr = std::ranges;
namespace stdv = std::views;

using namespace std::literals;

namespace stormkit::gpu {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto DescriptorPoolInterface<Base>::create_vk_descriptor_sets(usize count, view::DescriptorSetLayout&& layout) const noexcept
      -> Expected<dyn_array<VkDescriptorSet>> {
        const auto vk_layout     = vk::to_vk(layout);
        const auto allocate_info = VkDescriptorSetAllocateInfo {
            .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext              = nullptr,
            .descriptorPool     = *this,
            .descriptorSetCount = as<u32>(count),
            .pSetLayouts        = &vk_layout,
        };

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        return vk::allocate_checked<VkDescriptorSet>(count, device_table.vkAllocateDescriptorSets, device, &allocate_info);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto DescriptorPoolInterface<Base>::delete_vk_descriptor_set(view::Device         device,
                                                                 view::DescriptorPool pool,
                                                                 VkDescriptorSet      set) noexcept -> void {
        const auto& device_table = device.device_table();
        TryAssert(vk::call_checked(device_table.vkFreeDescriptorSets, device, pool, 1, &set), "Failed to free a descriptor set");
    }

    template class DescriptorPoolInterface<DescriptorPoolImplementation>;
    template class DescriptorPoolInterface<view::DescriptorPoolImplementation>;

    /////////////////////////////////////
    /////////////////////////////////////
    auto DescriptorPoolImplementation::do_init(PrivateTag, array_view<const Size>&& sizes, u32 max_sets) noexcept
      -> Expected<void> {
        const auto pool_sizes = transform(sizes, [](const Size& size) static noexcept {
            return VkDescriptorPoolSize {
                .type            = vk::to_vk<VkDescriptorType>(size.type),
                .descriptorCount = size.descriptor_count,
            };
        });

        const auto create_info = VkDescriptorPoolCreateInfo {
            .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext         = nullptr,
            .flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets       = max_sets,
            .poolSizeCount = as<u32>(stdr::size(sizes)),
            .pPoolSizes    = stdr::data(pool_sizes),
        };

        const auto& device       = owner();
        const auto& device_table = device.device_table();

        m_vk_handle = Try(vk::call_checked<VkDescriptorPool>(device_table.vkCreateDescriptorPool, device, &create_info, nullptr));
        Return {};
    }
} // namespace stormkit::gpu
