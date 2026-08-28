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
    template class PipelineLayoutInterface<PipelineLayoutImplementation>;
    template class PipelineLayoutInterface<view::PipelineLayoutImplementation>;

    /////////////////////////////////////
    /////////////////////////////////////
    auto PipelineLayoutImplementation::do_init(PrivateTag, const RasterPipelineLayout& layout) noexcept -> Expected<void> {
        m_layout = core::allocate_unsafe<RasterPipelineLayout>(layout);

        const auto set_layouts = transform(m_layout->descriptor_set_layouts, vk::monadic::to_vk());

        const auto push_constant_ranges = transform(m_layout->push_constant_ranges, [](const auto& push_constant_range) noexcept {
            return VkPushConstantRange {
                .stageFlags = vk::to_vk<VkShaderStageFlags>(push_constant_range.stages),
                .offset     = push_constant_range.offset,
                .size       = as<u32>(push_constant_range.size),
            };
        });

        const auto create_info = VkPipelineLayoutCreateInfo {
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext                  = nullptr,
            .flags                  = 0,
            .setLayoutCount         = as<u32>(stdr::size(set_layouts)),
            .pSetLayouts            = stdr::data(set_layouts),
            .pushConstantRangeCount = as<u32>(stdr::size(push_constant_ranges)),
            .pPushConstantRanges    = stdr::data(push_constant_ranges),
        };

        const auto& device       = owner();
        const auto& device_table = device.device_table();

        m_vk_handle = Try(vk::call_checked<VkPipelineLayout>(device_table.vkCreatePipelineLayout, device, &create_info, nullptr));
        Return {};
    }
} // namespace stormkit::gpu
