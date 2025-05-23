// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <volk.h>

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
                    .layout     = to_vk<VkImageLayout>(attachment_ref.layout),
                };
            };
        }
    } // namespace monadic

    /////////////////////////////////////
    /////////////////////////////////////
    auto RenderPass::do_init() noexcept -> Expected<void> {
        const auto attachments
          = m_description.attachments
            | stdv::transform([](auto&& attachment) static noexcept {
                  return VkAttachmentDescription {
                      .flags          = 0,
                      .format         = to_vk<VkFormat>(attachment.format),
                      .samples        = to_vk<VkSampleCountFlagBits>(attachment.samples),
                      .loadOp         = to_vk<VkAttachmentLoadOp>(attachment.load_op),
                      .storeOp        = to_vk<VkAttachmentStoreOp>(attachment.store_op),
                      .stencilLoadOp  = to_vk<VkAttachmentLoadOp>(attachment.stencil_load_op),
                      .stencilStoreOp = to_vk<VkAttachmentStoreOp>(attachment.stencil_store_op),
                      .initialLayout  = to_vk<VkImageLayout>(attachment.source_layout),
                      .finalLayout    = to_vk<VkImageLayout>(attachment.destination_layout),
                  };
              })
            | stdr::to<std::vector>();

        auto color_attachment_refs   = std::vector<std::vector<VkAttachmentReference>> {};
        auto depth_attachment_ref    = std::optional<VkAttachmentReference> {};
        auto resolve_attachment_refs = std::vector<std::vector<VkAttachmentReference>> {};
        auto subpasses               = std::vector<VkSubpassDescription> {};
        auto subpasses_deps          = std::vector<VkSubpassDependency> {};

        color_attachment_refs.reserve(stdr::size(m_description.subpasses));
        resolve_attachment_refs.reserve(stdr::size(m_description.subpasses));
        subpasses.reserve(stdr::size(m_description.subpasses));
        subpasses_deps.reserve(stdr::size(m_description.subpasses));

        for (const auto& subpass : m_description.subpasses) {
            auto& color_attachment_ref = color_attachment_refs
                                           .emplace_back(subpass.color_attachment_refs
                                                         | stdv::transform(monadic::vk_ref())
                                                         | stdr::to<std::vector>());
            auto& resolve_attachment_ref = resolve_attachment_refs
                                             .emplace_back(subpass.resolve_attachment_refs
                                                           | stdv::transform(monadic::vk_ref())
                                                           | stdr::to<std::vector>());
            if (subpass.depth_attachment_ref)
                depth_attachment_ref = monadic::vk_ref()(*subpass.depth_attachment_ref);

            subpasses.emplace_back(VkSubpassDescription {
              .flags                   = 0,
              .pipelineBindPoint       = to_vk<VkPipelineBindPoint>(subpass.bind_point),
              .inputAttachmentCount    = 0,
              .pInputAttachments       = nullptr,
              .colorAttachmentCount    = as<u32>(stdr::size(color_attachment_ref)),
              .pColorAttachments       = stdr::data(color_attachment_ref),
              .pResolveAttachments     = stdr::data(resolve_attachment_ref),
              .pDepthStencilAttachment = depth_attachment_ref.has_value()
                                           ? &depth_attachment_ref.value()
                                           : nullptr,
              .preserveAttachmentCount = 0,
              .pPreserveAttachments    = nullptr,
            });

            subpasses_deps.emplace_back(VkSubpassDependency {
              .srcSubpass    = VK_SUBPASS_EXTERNAL,
              .dstSubpass    = 0,
              .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
              .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
              .srcAccessMask = VkAccessFlagBits {},
              .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
                               | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
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

        return vk_call<VkRenderPass>(m_vk_device_table->vkCreateRenderPass,
                                     m_vk_device,
                                     &create_info,
                                     nullptr)
          .transform(core::monadic::set(m_vk_handle))
          .transform_error(monadic::from_vk<Result>());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    // TODO finish this
    auto RenderPassDescription::is_compatible(const RenderPassDescription& description)
      const noexcept -> bool {
        if (stdr::size(subpasses) == stdr::size(description.subpasses)) return false;

        for (auto i : range(stdr::size(subpasses))) {
            const auto& subpass_1 = subpasses[i];
            const auto& subpass_2 = description.subpasses[i];

            if (subpass_1.bind_point != subpass_2.bind_point) return false;

            const auto color_attachment_refs_count = std::min(stdr::size(subpass_1
                                                                           .color_attachment_refs),
                                                              stdr::size(subpass_2
                                                                           .color_attachment_refs));

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
