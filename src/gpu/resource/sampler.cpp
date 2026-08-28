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

namespace stormkit::gpu {
    template class SamplerInterface<SamplerImplementation>;
    template class SamplerInterface<view::SamplerImplementation>;

    /////////////////////////////////////
    /////////////////////////////////////
    auto SamplerImplementation::do_init(PrivateTag, const Settings& settings) noexcept -> Expected<void> {
        m_settings             = settings;
        const auto create_info = VkSamplerCreateInfo {
            .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext                   = nullptr,
            .flags                   = 0,
            .magFilter               = vk::to_vk<VkFilter>(m_settings.mag_filter),
            .minFilter               = vk::to_vk<VkFilter>(m_settings.min_filter),
            .mipmapMode              = vk::to_vk<VkSamplerMipmapMode>(m_settings.mipmap_mode),
            .addressModeU            = vk::to_vk<VkSamplerAddressMode>(m_settings.address_mode_u),
            .addressModeV            = vk::to_vk<VkSamplerAddressMode>(m_settings.address_mode_v),
            .addressModeW            = vk::to_vk<VkSamplerAddressMode>(m_settings.address_mode_w),
            .mipLodBias              = m_settings.mip_lod_bias,
            .anisotropyEnable        = m_settings.enable_anisotropy,
            .maxAnisotropy           = m_settings.max_anisotropy,
            .compareEnable           = m_settings.compare_enable,
            .compareOp               = vk::to_vk<VkCompareOp>(m_settings.compare_operation),
            .minLod                  = m_settings.min_lod,
            .maxLod                  = m_settings.max_lod,
            .borderColor             = vk::to_vk<VkBorderColor>(m_settings.border_color),
            .unnormalizedCoordinates = m_settings.unnormalized_coordinates
        };
        const auto& device = owner();

        m_vk_handle = Try(vk::call_checked<VkSampler>(device.device_table().vkCreateSampler, device, &create_info, nullptr));
        Return {};
    }

} // namespace stormkit::gpu
