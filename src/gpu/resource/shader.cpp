// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/try_expected.hpp>

#include <stormkit/gpu/vulkan.hpp>

module stormkit.gpu.resource;

import std;

import stormkit.core;

import stormkit.gpu.core;
;

namespace stormkit::gpu {
    template class ShaderInterface<ShaderImplementation>;
    template class ShaderInterface<view::ShaderImplementation>;

    /////////////////////////////////////
    /////////////////////////////////////
    auto ShaderImplementation::do_init(PrivateTag, dyn_array<SpirvID>&& data, ShaderStageFlag type) -> Expected<void> {
        m_source = std::move(data);
        m_type   = type;

        const auto create_info = VkShaderModuleCreateInfo {
            .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext    = nullptr,
            .flags    = 0,
            .codeSize = stdr::size(m_source) * sizeof(SpirvID),
            .pCode    = stdr::data(m_source)
        };

        const auto& device = owner();
        m_vk_handle        = Try(vk::call_checked<
                                 VkShaderModule>(device.device_table().vkCreateShaderModule, device, &create_info, nullptr));
        Return {};
    }
} // namespace stormkit::gpu
