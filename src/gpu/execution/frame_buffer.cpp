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
    template class FrameBufferInterface<FrameBufferImplementation>;
    template class FrameBufferInterface<view::FrameBufferImplementation>;

    /////////////////////////////////////
    /////////////////////////////////////
    auto FrameBufferImplementation::do_init(PrivateTag,
                                            view::RenderPass&&           render_pass,
                                            const math::uextent2&        extent,
                                            dyn_array<view::ImageView>&& attachments) noexcept -> Expected<void> {
        m_extent                  = extent;
        m_attachments             = std::move(attachments);
        const auto vk_attachments = transform(m_attachments, vk::monadic::to_vk());

        const auto create_info = VkFramebufferCreateInfo {
            .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext           = nullptr,
            .flags           = 0,
            .renderPass      = render_pass,
            .attachmentCount = as<u32>(std::ranges::size(vk_attachments)),
            .pAttachments    = std::ranges::data(vk_attachments),
            .width           = m_extent.width,
            .height          = m_extent.height,
            .layers          = 1,
        };

        const auto& device       = owner();
        const auto& device_table = device.device_table();

        m_vk_handle = Try(vk::call_checked<VkFramebuffer>(device_table.vkCreateFramebuffer, device, &create_info, nullptr));
        Return {};
    }
} // namespace stormkit::gpu
