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

namespace stormkit::gpu {
    namespace monadic {
        auto vk_ref() noexcept -> decltype(auto) {
            return [](auto&& attachment_ref) noexcept -> VkAttachmentReference {
                return VkAttachmentReference {
                    .attachment = attachment_ref.attachment_id,
                    .layout     = vk::to_vk<VkImageLayout>(attachment_ref.layout),
                };
            };
        }
    } // namespace monadic

    template class RenderPassInterface<RenderPassImplementation>;
    template class RenderPassInterface<view::RenderPassImplementation>;

    /////////////////////////////////////
    /////////////////////////////////////
    auto RenderPassImplementation::do_init(PrivateTag, const RenderPassDescription& description) noexcept -> Expected<void> {
        m_description = core::allocate_unsafe<RenderPassDescription>(description);

        const auto attachments = transform(m_description->attachments, [](auto&& attachment) static noexcept {
            return VkAttachmentDescription {
                .flags          = 0,
                .format         = vk::to_vk<VkFormat>(attachment.format),
                .samples        = vk::to_vk<VkSampleCountFlagBits>(attachment.samples),
                .loadOp         = vk::to_vk<VkAttachmentLoadOp>(attachment.load_op),
                .storeOp        = vk::to_vk<VkAttachmentStoreOp>(attachment.store_op),
                .stencilLoadOp  = vk::to_vk<VkAttachmentLoadOp>(attachment.stencil_load_op),
                .stencilStoreOp = vk::to_vk<VkAttachmentStoreOp>(attachment.stencil_store_op),
                .initialLayout  = vk::to_vk<VkImageLayout>(attachment.source_layout),
                .finalLayout    = vk::to_vk<VkImageLayout>(attachment.destination_layout),
            };
        });

        auto color_attachment_refs   = dyn_array<dyn_array<VkAttachmentReference>> {};
        auto depth_attachment_ref    = std::optional<VkAttachmentReference> {};
        auto resolve_attachment_refs = dyn_array<dyn_array<VkAttachmentReference>> {};
        auto subpasses               = dyn_array<VkSubpassDescription> {};
        auto subpasses_deps          = dyn_array<VkSubpassDependency> {};

        color_attachment_refs.reserve(stdr::size(m_description->subpasses));
        resolve_attachment_refs.reserve(stdr::size(m_description->subpasses));
        subpasses.reserve(stdr::size(m_description->subpasses));
        subpasses_deps.reserve(stdr::size(m_description->subpasses));

        for (const auto& subpass : m_description->subpasses) {
            auto& color_attachment_ref   = color_attachment_refs.emplace_back(transform(subpass.color_attachment_refs,
                                                                                        monadic::vk_ref()));
            auto& resolve_attachment_ref = resolve_attachment_refs.emplace_back(transform(subpass.resolve_attachment_refs,
                                                                                          monadic::vk_ref()));
            if (subpass.depth_attachment_ref) depth_attachment_ref = monadic::vk_ref()(*subpass.depth_attachment_ref);

            subpasses.emplace_back(VkSubpassDescription {
              .flags                   = 0,
              .pipelineBindPoint       = vk::to_vk<VkPipelineBindPoint>(subpass.bind_point),
              .inputAttachmentCount    = 0,
              .pInputAttachments       = nullptr,
              .colorAttachmentCount    = as<u32>(stdr::size(color_attachment_ref)),
              .pColorAttachments       = stdr::data(color_attachment_ref),
              .pResolveAttachments     = stdr::data(resolve_attachment_ref),
              .pDepthStencilAttachment = depth_attachment_ref.has_value() ? &depth_attachment_ref.value() : nullptr,
              .preserveAttachmentCount = 0,
              .pPreserveAttachments    = nullptr,
            });

            subpasses_deps.emplace_back(VkSubpassDependency {
              .srcSubpass      = VK_SUBPASS_EXTERNAL,
              .dstSubpass      = 0,
              .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
              .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
              .srcAccessMask   = VkAccessFlagBits {},
              .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
              .dependencyFlags = 0,
            });
        }

        const auto create_info = VkRenderPassCreateInfo {
            .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext           = nullptr,
            .flags           = 0,
            .attachmentCount = as<u32>(stdr::size(attachments)),
            .pAttachments    = stdr::data(attachments),
            .subpassCount    = as<u32>(stdr::size(subpasses)),
            .pSubpasses      = stdr::data(subpasses),
            .dependencyCount = as<u32>(stdr::size(subpasses_deps)),
            .pDependencies   = stdr::data(subpasses_deps),
        };

        const auto& device       = owner();
        const auto& device_table = device.device_table();

        m_vk_handle = Try(vk::call_checked<VkRenderPass>(device_table.vkCreateRenderPass, device, &create_info, nullptr));
        Return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    // TODO finish this
    auto RenderPassDescription::is_compatible(const RenderPassDescription& description) const noexcept -> bool {
        if (stdr::size(subpasses) == stdr::size(description.subpasses)) return false;

        for (auto i : range(stdr::size(subpasses))) {
            const auto& subpass_1 = subpasses[i];
            const auto& subpass_2 = description.subpasses[i];

            if (subpass_1.bind_point != subpass_2.bind_point) return false;

            const auto color_attachment_refs_count = std::min(stdr::size(subpass_1.color_attachment_refs),
                                                              stdr::size(subpass_2.color_attachment_refs));

            for (auto j = 0u; j < color_attachment_refs_count; ++j) {
                const auto& attachment_ref_1 = subpass_1.color_attachment_refs[j];
                const auto& attachment_ref_2 = subpass_2.color_attachment_refs[j];

                if (attachment_ref_1.layout != attachment_ref_2.layout) return false;

                const auto& attachment_1 = description.attachments[attachment_ref_1.attachment_id];
                const auto& attachment_2 = description.attachments[attachment_ref_2.attachment_id];

                if (attachment_1.format != attachment_2.format) return false;
                if (attachment_1.samples != attachment_2.samples) return false;
            }
        }

        return true;
    }
} // namespace stormkit::gpu
