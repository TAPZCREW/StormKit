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
    template class DescriptorSetLayoutInterface<DescriptorSetLayoutImplementation>;
    template class DescriptorSetLayoutInterface<view::DescriptorSetLayoutImplementation>;

    /////////////////////////////////////
    /////////////////////////////////////
    auto DescriptorSetLayoutImplementation::do_init(PrivateTag, dyn_array<DescriptorSetLayoutBinding>&& bindings) noexcept
      -> Expected<void> {
        m_bindings             = std::move(bindings);
        const auto vk_bindings = transform(m_bindings, [](const DescriptorSetLayoutBinding& binding) static noexcept {
            return VkDescriptorSetLayoutBinding {
                .binding            = binding.binding,
                .descriptorType     = vk::to_vk<VkDescriptorType>(binding.type),
                .descriptorCount    = as<u32>(binding.descriptor_count),
                .stageFlags         = vk::to_vk<VkShaderStageFlags>(binding.stages),
                .pImmutableSamplers = nullptr,
            };
        });

        const auto create_info = VkDescriptorSetLayoutCreateInfo {
            .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext        = nullptr,
            .flags        = 0,
            .bindingCount = as<u32>(stdr::size(vk_bindings)),
            .pBindings    = stdr::data(vk_bindings)
        };

        const auto& device       = owner();
        const auto& device_table = device.device_table();
        m_vk_handle = Try(vk::call_checked<
                          VkDescriptorSetLayout>(device_table.vkCreateDescriptorSetLayout, device, &create_info, nullptr));

        Return {};
    }
} // namespace stormkit::gpu
