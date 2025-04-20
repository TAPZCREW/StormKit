// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module stormkit.Gpu;

import std;

import stormkit.Core;

import :Execution.RenderPass;

import stormkit.Gpu.Vulkan;
import :Core;
import :Execution.RenderPass;

namespace stormkit::gpu {
    namespace monadic {
        auto vkRef() noexcept -> decltype(auto) {
            return [](auto&& attachment_ref) noexcept -> vk::AttachmentReference {
                return vk::AttachmentReference {}
                    .setAttachment(attachment_ref.attachment_id)
                    .setLayout(narrow<vk::ImageLayout>(attachment_ref.layout));
            };
        }
    } // namespace monadic

    /////////////////////////////////////
    /////////////////////////////////////
    auto RenderPass::doInitRenderPass(const Device& device) noexcept -> VulkanExpected<void> {
        const auto attachments
            = m_description.attachments
              | std::views::transform([](auto&& attachment) {
                    return vk::AttachmentDescription {}
                        .setFormat(narrow<vk::Format>(attachment.format))
                        .setSamples(narrow<vk::SampleCountFlagBits>(attachment.samples))
                        .setLoadOp(narrow<vk::AttachmentLoadOp>(attachment.load_op))
                        .setStoreOp(narrow<vk::AttachmentStoreOp>(attachment.store_op))
                        .setStencilLoadOp(narrow<vk::AttachmentLoadOp>(attachment.stencil_load_op))
                        .setStencilStoreOp(
                            narrow<vk::AttachmentStoreOp>(attachment.stencil_store_op))
                        .setInitialLayout(narrow<vk::ImageLayout>(attachment.source_layout))
                        .setFinalLayout(narrow<vk::ImageLayout>(attachment.destination_layout));
                })
              | std::ranges::to<std::vector>();

        auto color_attachment_refs   = std::vector<std::vector<vk::AttachmentReference>> {};
        auto depth_attachment_ref    = std::optional<vk::AttachmentReference> {};
        auto resolve_attachment_refs = std::vector<std::vector<vk::AttachmentReference>> {};
        auto subpasses               = std::vector<vk::SubpassDescription> {};
        auto subpasses_deps          = std::vector<vk::SubpassDependency> {};

        color_attachment_refs.reserve(std::size(m_description.subpasses));
        resolve_attachment_refs.reserve(std::size(m_description.subpasses));
        subpasses.reserve(std::size(m_description.subpasses));
        subpasses_deps.reserve(std::size(m_description.subpasses));

        for (const auto& subpass : m_description.subpasses) {
            auto& color_attachment_ref
                = color_attachment_refs.emplace_back(subpass.color_attachment_refs
                                                     | std::views::transform(monadic::vkRef())
                                                     | std::ranges::to<std::vector>());
            auto& resolve_attachment_ref
                = resolve_attachment_refs.emplace_back(subpass.resolve_attachment_refs
                                                       | std::views::transform(monadic::vkRef())
                                                       | std::ranges::to<std::vector>());
            if (subpass.depth_attachment_ref)
                depth_attachment_ref = monadic::vkRef()(*subpass.depth_attachment_ref);

            subpasses.emplace_back(
                vk::SubpassDescription {}
                    .setPipelineBindPoint(narrow<vk::PipelineBindPoint>(subpass.bind_point))
                    .setColorAttachmentCount(as<UInt32>(std::size(color_attachment_ref)))
                    .setPColorAttachments(std::data(color_attachment_ref))
                    .setPResolveAttachments(std::data(resolve_attachment_ref))
                    .setPDepthStencilAttachment(depth_attachment_ref.has_value()
                                                    ? &depth_attachment_ref.value()
                                                    : nullptr));

            subpasses_deps.emplace_back(
                vk::SubpassDependency {}
                    .setSrcSubpass(vk::SubpassExternal)
                    .setDstSubpass(0)
                    .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                    .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                    .setSrcAccessMask(vk::AccessFlagBits {})
                    .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead
                                      | vk::AccessFlagBits::eColorAttachmentWrite));
        }

        const auto create_info = vk::RenderPassCreateInfo {}
                                     .setAttachments(attachments)
                                     .setSubpasses(subpasses)
                                     .setDependencies(subpasses_deps);

        return device.vkHandle()
            .createRenderPass(create_info)
            .transform(core::monadic::set(m_vk_render_pass));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    // TODO finish this
    auto
        RenderPassDescription::isCompatible(const RenderPassDescription& description) const noexcept
        -> bool {
        if (std::size(subpasses) == std::size(description.subpasses)) return false;

        for (auto i : range(std::size(subpasses))) {
            const auto& subpass_1 = subpasses[i];
            const auto& subpass_2 = description.subpasses[i];

            if (subpass_1.bind_point != subpass_2.bind_point) return false;

            const auto color_attachment_refs_count
                = std::min(std::size(subpass_1.color_attachment_refs),
                           std::size(subpass_2.color_attachment_refs));

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
