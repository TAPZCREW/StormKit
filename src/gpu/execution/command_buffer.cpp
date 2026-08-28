// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>
#include <stormkit/core/try_expected.hpp>

#include <stormkit/gpu/vulkan.hpp>

module stormkit.gpu.execution;

import std;

import frozen;

import stormkit.core;

import stormkit.gpu.core;
import stormkit.gpu.resource;

using namespace std::literals;

namespace stdr = std::ranges;
namespace stdv = std::views;

namespace cmonadic = stormkit::core::monadic;

namespace stormkit::gpu {
    namespace {
        constexpr auto
          OLD_LAYOUT_ACCESS_MAP = frozen::make_unordered_map<VkImageLayout, std::pair<VkAccessFlags, VkPipelineStageFlags>>({
            { VK_IMAGE_LAYOUT_UNDEFINED,                        { VK_ACCESS_NONE, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT }            },
            { VK_IMAGE_LAYOUT_PREINITIALIZED,                   { VK_ACCESS_NONE, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT }            },
            { VK_IMAGE_LAYOUT_GENERAL,
             { VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }                                                                  },
            { VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
             { VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }                                                                  },
            { VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
             { VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT }                                                                      },
            { VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
             { VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT }                          },
            { VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
             { VK_ACCESS_INPUT_ATTACHMENT_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT }                                      },
            { VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             { VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT }  },
            { VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             { VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT } },
            { VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,                  { VK_ACCESS_MEMORY_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT }    },
        });

        constexpr auto NEW_LAYOUT_ACCESS_MAP = frozen::make_unordered_map<VkImageLayout,
                                                                          std::pair<VkAccessFlags, VkPipelineStageFlags>>({
          { VK_IMAGE_LAYOUT_UNDEFINED,                        { VK_ACCESS_NONE, {} }                                               },
          { VK_IMAGE_LAYOUT_PREINITIALIZED,                   { VK_ACCESS_NONE, {} }                                               },
          { VK_IMAGE_LAYOUT_GENERAL,
           { VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT }                         },
          { VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
           { VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
              VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }                                                                      },
          { VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
           { VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT }                                                                         },
          { VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  { VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT }    },
          { VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         { VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT } },
          { VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             { VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT }      },
          { VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             { VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT }     },
          { VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,                  { VK_ACCESS_MEMORY_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT }        },
        });
    } // namespace

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::reset() noexcept -> Expected<void> {
        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        Try(vk::call_checked(device_table.vkResetCommandBuffer, *this, 0));
        *Base::m_state = CommandBuffer::State::INITIAL;

        Return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::begin(bool one_time_submit, InheritanceInfo inheritance_info_variant) noexcept
      -> Expected<void> {
        auto& state = *Base::m_state;
        EXPECTS(state == CommandBuffer::State::INITIAL);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        auto rendering_color_attachments   = dyn_array<VkFormat> {};
        auto vk_rendering_inheritance_info = init_by<VkCommandBufferInheritanceRenderingInfo>([](auto& info) noexcept {
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO;
            info.pNext = nullptr;
        });

        const auto vk_inheritance_info =
          [&inheritance_info_variant, &rendering_color_attachments, &vk_rendering_inheritance_info] noexcept {
              auto info = init_by<VkCommandBufferInheritanceInfo>([](auto& info) noexcept {
                  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                  info.pNext = nullptr;
              });

              if (is<RenderPassInheritanceInfo>(inheritance_info_variant)) {
                  const auto& inheritance_info = as<RenderPassInheritanceInfo>(inheritance_info_variant);
                  info.renderPass              = vk::to_vk(*inheritance_info.render_pass);
                  info.subpass                 = inheritance_info.subpass;
                  info.framebuffer             = vk::to_vk(*inheritance_info.framebuffer);
              } else if (is<RenderingInheritanceInfo>(inheritance_info_variant)) {
                  info.pNext = &vk_rendering_inheritance_info;

                  const auto& inheritance_info = as<RenderingInheritanceInfo>(inheritance_info_variant);

                  rendering_color_attachments                           = inheritance_info.color_attachments
                                                                          | stdv::transform(gpu::vk::monadic::to_vk<VkFormat>())
                                                                          | stdr::to<dyn_array<VkFormat>>();
                  vk_rendering_inheritance_info.viewMask                = inheritance_info.view_mask;
                  vk_rendering_inheritance_info.colorAttachmentCount    = as<u32>(stdr::size(inheritance_info.color_attachments));
                  vk_rendering_inheritance_info.pColorAttachmentFormats = stdr::data(rendering_color_attachments);

                  if (inheritance_info.depth_attachment)
                      vk_rendering_inheritance_info.depthAttachmentFormat = gpu::vk::to_vk<
                        VkFormat>(*inheritance_info.depth_attachment);
                  if (inheritance_info.stencil_attachment)
                      vk_rendering_inheritance_info.stencilAttachmentFormat = gpu::vk::to_vk<
                        VkFormat>(*inheritance_info.stencil_attachment);

                  vk_rendering_inheritance_info.rasterizationSamples = gpu::vk::to_vk<
                    VkSampleCountFlagBits>(inheritance_info.rasterization_samples);
              }
              return info;
          }();

        const auto flags = [this, one_time_submit, &inheritance_info_variant] noexcept -> VkCommandBufferUsageFlags {
            auto flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            if (!one_time_submit) flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            if (level() == CommandBufferLevel::SECONDARY) {
                if (is<RenderPassInheritanceInfo>(inheritance_info_variant)
                    or is<RenderingInheritanceInfo>(inheritance_info_variant))
                    flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
            }

            return flags;
        }();

        const auto begin_info = VkCommandBufferBeginInfo {
            .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext            = nullptr,
            .flags            = flags,
            .pInheritanceInfo = &vk_inheritance_info,
        };

        Try(vk::call_checked(device_table.vkBeginCommandBuffer, *this, &begin_info));
        state = CommandBuffer::State::RECORDING;

        Return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::end() noexcept -> Expected<void> {
        auto& state = *Base::m_state;
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        Try(vk::call_checked(device_table.vkEndCommandBuffer, *this));
        state = CommandBuffer::State::EXECUTABLE;

        Return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::begin_debug_region(string_view name, const fcolor_rgb& color) const noexcept
      -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        if (not vkCmdBeginDebugUtilsLabelEXT) [[unlikely]]
            return *this;

        const auto info = VkDebugUtilsLabelEXT {
            .sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
            .pNext      = nullptr,
            .pLabelName = stdr::data(name),
            .color      = { color.r, color.g, color.b, 1.f }
        };

        vk::call(vkCmdBeginDebugUtilsLabelEXT, *this, &info);
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::insert_debug_label(string_view name, const fcolor_rgb& color) const noexcept
      -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        if (not vkCmdInsertDebugUtilsLabelEXT) [[unlikely]]
            return *this;

        const auto info = VkDebugUtilsLabelEXT {
            .sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
            .pNext      = nullptr,
            .pLabelName = stdr::data(name),
            .color      = { color.r, color.g, color.b, 1.f }
        };

        vk::call(vkCmdInsertDebugUtilsLabelEXT, *this, &info);
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::end_debug_region() const noexcept -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        if (not vkCmdEndDebugUtilsLabelEXT) [[unlikely]]
            return *this;

        vk::call(vkCmdEndDebugUtilsLabelEXT, *this);
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::begin_rendering(const RenderingInfo& info, bool secondary) const noexcept
      -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        auto to_vk_attachment = [](const auto& attachment) static noexcept {
            auto attachment_info = VkRenderingAttachmentInfo {
                .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .pNext              = nullptr,
                .imageView          = vk::to_vk(attachment.image_view),
                .imageLayout        = vk::to_vk<VkImageLayout>(attachment.layout),
                .resolveMode        = {},
                .resolveImageView   = nullptr,
                .resolveImageLayout = {},
                .loadOp             = vk::to_vk<VkAttachmentLoadOp>(attachment.load_op),
                .storeOp            = vk::to_vk<VkAttachmentStoreOp>(attachment.store_op),
                .clearValue         = {},
            };

            if (attachment.resolve) {
                auto& resolve = *attachment.resolve;

                attachment_info.resolveMode        = vk::to_vk<VkResolveModeFlagBits>(resolve.mode);
                attachment_info.resolveImageView   = vk::to_vk(resolve.image_view);
                attachment_info.resolveImageLayout = vk::to_vk<VkImageLayout>(resolve.layout);
            }
            if (attachment.clear_value) {
                attachment_info.clearValue = std::
                  visit(Overloaded { [](const ClearColor& clear_color) static noexcept -> decltype(auto) {
                                        return VkClearValue {
                                            .color = VkClearColorValue { .float32 = { clear_color.color.r,
                                                                                      clear_color.color.b,
                                                                                      clear_color.color.g,
                                                                                      clear_color.color.a } },
                                        };
                                    },
                                     [](const ClearDepthStencil& clear_depth_stencil) static noexcept -> decltype(auto) {
                                         return VkClearValue {
                                             .depthStencil = VkClearDepthStencilValue { .depth   = clear_depth_stencil.depth,
                                                                                       .stencil = clear_depth_stencil.stencil },
                                         };
                                     } },
                        *attachment.clear_value);
            }

            return attachment_info;
        };

        const auto color_attachments  = transform(info.color_attachments, to_vk_attachment);
        const auto depth_attachment   = info.depth_attachment ? to_vk_attachment(*info.depth_attachment)
                                                              : VkRenderingAttachmentInfo {};
        const auto stencil_attachment = info.stencil_attachment ? to_vk_attachment(*info.stencil_attachment)
                                                                : VkRenderingAttachmentInfo {};

        const auto rendering_info = VkRenderingInfo {
            .sType                = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .pNext                = nullptr,
            .flags                = as<VkRenderingFlags>((secondary) ? VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT : 0),
            .renderArea           = vk::to_vk(info.render_area),
            .layerCount           = info.layer_count,
            .viewMask             = info.view_mask,
            .colorAttachmentCount = as<u32>(stdr::size(color_attachments)),
            .pColorAttachments    = stdr::data(color_attachments),
            .pDepthAttachment     = info.depth_attachment ? &depth_attachment : nullptr,
            .pStencilAttachment   = info.stencil_attachment ? &stencil_attachment : nullptr,
        };

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        vk::call(device_table.vkCmdBeginRenderingKHR, *this, &rendering_info);
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::begin_render_pass(view::RenderPass             render_pass,
                                                         view::FrameBuffer            framebuffer,
                                                         array_view<const ClearValue> clear_values,
                                                         bool                         secondary_commandbuffers) const noexcept
      -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        const auto vk_clear_values = transform(
          clear_values,
          cmonadic::either(
            [](const ClearColor& clear_color) static noexcept -> decltype(auto) {
                return VkClearValue {
                    .color = VkClearColorValue { .float32 = { clear_color.color.r,
                                                              clear_color.color.b,
                                                              clear_color.color.g,
                                                              clear_color.color.a } },
                };
            },
            [](const ClearDepthStencil& clear_depth_stencil) static noexcept -> decltype(auto) {
                return VkClearValue {
                    .depthStencil = VkClearDepthStencilValue { .depth   = clear_depth_stencil.depth,
                                                              .stencil = clear_depth_stencil.stencil },
                };
            }));

        const auto begin_info = VkRenderPassBeginInfo {
            .sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext       = nullptr,
            .renderPass  = vk::to_vk(render_pass),
            .framebuffer = vk::to_vk(framebuffer),
            .renderArea  = VkRect2D { .offset = { 0, 0 }, .extent = { framebuffer.extent().width, framebuffer.extent().height } },
            .clearValueCount = as<u32>(stdr::size(vk_clear_values)),
            .pClearValues    = stdr::data(vk_clear_values),
        };

        const auto subpass_content = secondary_commandbuffers ? VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
                                                              : VK_SUBPASS_CONTENTS_INLINE;

        vk::call(device_table.vkCmdBeginRenderPass, *this, &begin_info, subpass_content);
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::next_subpass() const noexcept -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        vk::call(device_table.vkCmdNextSubpass, *this, VK_SUBPASS_CONTENTS_INLINE);
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::end_render_pass() const noexcept -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        vk::call(device_table.vkCmdEndRenderPass, *this);
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::end_rendering() const noexcept -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        vk::call(device_table.vkCmdEndRendering, *this);
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::bind_pipeline(view::Pipeline pipeline) const noexcept -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        const auto bind_point = (pipeline.type() == Pipeline::Type::RASTER)
                                  ? VK_PIPELINE_BIND_POINT_GRAPHICS
                                  : VK_PIPELINE_BIND_POINT_COMPUTE;

        vk::call(device_table.vkCmdBindPipeline, *this, bind_point, pipeline);
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::set_viewport(u32 first_viewport, array_view<const Viewport> viewports) const noexcept
      -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        const auto vk_viewports = transform(viewports, vk::monadic::to_vk());

        vk::call(device_table.vkCmdSetViewport, *this, first_viewport, stdr::size(vk_viewports), stdr::data(vk_viewports));
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::set_scissor(u32 first_scissor, array_view<const Scissor> scissors) const noexcept
      -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        const auto vk_scissors = transform(scissors, vk::monadic::to_vk());

        vk::call(device_table.vkCmdSetScissor, *this, first_scissor, stdr::size(vk_scissors), stdr::data(vk_scissors));
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::set_line_width(f32 width) const noexcept -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        vk::call(device_table.vkCmdSetLineWidth, *this, width);
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::set_depth_bias(f32 constant_factor, f32 clamp, f32 slope_factor) const noexcept
      -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        vk::call(device_table.vkCmdSetDepthBias, *this, constant_factor, clamp, slope_factor);
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::set_blend_constants(array_view<const f32> constants) const noexcept
      -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        f32 data[] = { constants[0], constants[1], constants[2], constants[3] };

        vk::call(device_table.vkCmdSetBlendConstants, *this, data);
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::set_depth_bounds(f32 min, f32 max) const noexcept -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        vk::call(device_table.vkCmdSetDepthBounds, *this, min, max);
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::set_stencil_compare_mask(StencilFaceFlag face, u32 mask) const noexcept
      -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        vk::call(device_table.vkCmdSetStencilCompareMask, *this, vk::to_vk<VkStencilFaceFlagBits>(face), mask);
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::set_stencil_write_mask(StencilFaceFlag face, u32 mask) const noexcept
      -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        vk::call(device_table.vkCmdSetStencilWriteMask, *this, vk::to_vk<VkStencilFaceFlagBits>(face), mask);
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::set_stencil_reference(StencilFaceFlag face, u32 reference) const noexcept
      -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        vk::call(device_table.vkCmdSetStencilReference, *this, vk::to_vk<VkStencilFaceFlagBits>(face), reference);
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::dispatch(u32 group_count_x, u32 group_count_y, u32 group_count_z) const noexcept
      -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        vk::call(device_table.vkCmdDispatch, *this, group_count_x, group_count_y, group_count_z);
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance)
      const noexcept -> const CommandBufferInterface& {
        EXPECTS(vertex_count > 0);

        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        vk::call(device_table.vkCmdDraw, *this, vertex_count, instance_count, first_vertex, first_instance);
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::
      draw_indexed(u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance) const noexcept
      -> const CommandBufferInterface& {
        EXPECTS(index_count > 0);

        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        vk::call(device_table.vkCmdDrawIndexed, *this, index_count, instance_count, first_index, vertex_offset, first_instance);
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::draw_indirect(view::Buffer buffer, usize offset, u32 draw_count, u32 stride) const noexcept
      -> const CommandBufferInterface& {
        EXPECTS(draw_count > 0);

        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        vk::call(device_table.vkCmdDrawIndirect, *this, buffer, offset, draw_count, stride);
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::draw_indexed_indirect(view::Buffer buffer, usize offset, u32 draw_count, u32 stride)
      const noexcept -> const CommandBufferInterface& {
        EXPECTS(draw_count > 0);

        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        vk::call(device_table.vkCmdDrawIndexedIndirect, *this, buffer, offset, draw_count, stride);
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::bind_vertex_buffers(array_view<const view::Buffer> buffers, array_view<const u64> offsets)
      const noexcept -> const CommandBufferInterface& {
        EXPECTS(not std::empty(buffers));
        EXPECTS(std::size(buffers) == std::size(offsets));

        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        const auto vk_buffers = transform(buffers, vk::monadic::to_vk());

        vk::call(device_table.vkCmdBindVertexBuffers,
                 *this,
                 0,
                 stdr::size(vk_buffers),
                 stdr::data(vk_buffers),
                 stdr::data(offsets));
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::bind_index_buffer(view::Buffer buffer, u64 offset, bool large_indices) const noexcept
      -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        vk::call(device_table.vkCmdBindIndexBuffer,
                 *this,
                 buffer,
                 offset,
                 (large_indices) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::bind_descriptor_sets(view::Pipeline                        pipeline,
                                                            view::PipelineLayout                  layout,
                                                            array_view<const view::DescriptorSet> descriptor_sets,
                                                            array_view<const u32>                 dynamic_offsets) const noexcept
      -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        const auto bind_point = (pipeline.type() == Pipeline::Type::RASTER)
                                  ? VK_PIPELINE_BIND_POINT_GRAPHICS
                                  : VK_PIPELINE_BIND_POINT_COMPUTE;

        const auto vk_descriptor_sets = transform(descriptor_sets, vk::monadic::to_vk());

        vk::call(device_table.vkCmdBindDescriptorSets,
                 *this,
                 bind_point,
                 layout,
                 0,
                 stdr::size(vk_descriptor_sets),
                 stdr::data(vk_descriptor_sets),
                 stdr::size(dynamic_offsets),
                 stdr::data(dynamic_offsets));
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::copy_buffer(view::Buffer src, view::Buffer dst, usize size, u64 src_offset, u64 dst_offset)
      const noexcept -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        const auto vk_copy_buffers = array {
            VkBufferCopy { .srcOffset = src_offset, .dstOffset = dst_offset, .size = size }
        };

        vk::call(device_table.vkCmdCopyBuffer, *this, src, dst, stdr::size(vk_copy_buffers), stdr::data(vk_copy_buffers));
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::copy_buffer_to_image(view::Buffer                      src,
                                                            view::Image                       dst,
                                                            array_view<const BufferImageCopy> buffer_image_copies) const noexcept
      -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        const auto DEFAULT_COPY = array {
            BufferImageCopy { 0, 0, 0, {}, { 0, 0, 0 }, dst.extent() }
        };

        if (stdr::empty(buffer_image_copies)) buffer_image_copies = DEFAULT_COPY;

        const auto vk_copy_regions = transform(buffer_image_copies, [](const auto& buffer_image_copy) static noexcept {
            const auto image_subresource = VkImageSubresourceLayers {
                .aspectMask     = vk::to_vk<VkImageAspectFlags>(buffer_image_copy.subresource_layers.aspect_mask),
                .mipLevel       = buffer_image_copy.subresource_layers.mip_level,
                .baseArrayLayer = buffer_image_copy.subresource_layers.base_array_layer,
                .layerCount     = buffer_image_copy.subresource_layers.layer_count,
            };

            return VkBufferImageCopy {
                .bufferOffset      = buffer_image_copy.buffer_offset,
                .bufferRowLength   = buffer_image_copy.buffer_row_length,
                .bufferImageHeight = buffer_image_copy.buffer_image_height,
                .imageSubresource  = image_subresource,
                .imageOffset       = vk::to_vk<VkOffset3D>(buffer_image_copy.offset),
                .imageExtent       = vk::to_vk(buffer_image_copy.extent)
            };
        });

        vk::call(device_table.vkCmdCopyBufferToImage,
                 *this,
                 src,
                 dst,
                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                 stdr::size(vk_copy_regions),
                 stdr::data(vk_copy_regions));
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::copy_image_to_buffer(view::Image                       src,
                                                            view::Buffer                      dst,
                                                            array_view<const BufferImageCopy> buffer_image_copies) const noexcept
      -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        const auto DEFAULT_COPY = into_array<BufferImageCopy>({
          BufferImageCopy { 0, 0, 0, {}, { 0, 0, 0 }, src.extent() }
        });

        if (stdr::empty(buffer_image_copies)) buffer_image_copies = DEFAULT_COPY;

        const auto vk_copy_regions = transform(buffer_image_copies, [](const auto& buffer_image_copy) static noexcept {
            const auto image_subresource = VkImageSubresourceLayers {
                .aspectMask     = vk::to_vk<VkImageAspectFlags>(buffer_image_copy.subresource_layers.aspect_mask),
                .mipLevel       = buffer_image_copy.subresource_layers.mip_level,
                .baseArrayLayer = buffer_image_copy.subresource_layers.base_array_layer,
                .layerCount     = buffer_image_copy.subresource_layers.layer_count,
            };

            return VkBufferImageCopy {
                .bufferOffset      = buffer_image_copy.buffer_offset,
                .bufferRowLength   = buffer_image_copy.buffer_row_length,
                .bufferImageHeight = buffer_image_copy.buffer_image_height,
                .imageSubresource  = image_subresource,
                .imageOffset       = vk::to_vk<VkOffset3D>(buffer_image_copy.offset),
                .imageExtent       = vk::to_vk(buffer_image_copy.extent)
            };
        });

        vk::call(device_table.vkCmdCopyImageToBuffer,
                 *this,
                 src,
                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                 dst,
                 stdr::size(vk_copy_regions),
                 stdr::data(vk_copy_regions));
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::copy_image(view::Image                   src,
                                                  view::Image                   dst,
                                                  ImageLayout                   src_layout,
                                                  ImageLayout                   dst_layout,
                                                  const ImageSubresourceLayers& src_subresource_layers,
                                                  const ImageSubresourceLayers& dst_subresource_layers,
                                                  const math::uextent3& extent) const noexcept -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        const auto vk_src_subresource_layers = VkImageSubresourceLayers {
            .aspectMask     = vk::to_vk<VkImageAspectFlags>(src_subresource_layers.aspect_mask),
            .mipLevel       = src_subresource_layers.mip_level,
            .baseArrayLayer = src_subresource_layers.base_array_layer,
            .layerCount     = src_subresource_layers.layer_count
        };

        const auto vk_dst_subresource_layers = VkImageSubresourceLayers {
            .aspectMask     = vk::to_vk<VkImageAspectFlags>(dst_subresource_layers.aspect_mask),
            .mipLevel       = dst_subresource_layers.mip_level,
            .baseArrayLayer = dst_subresource_layers.base_array_layer,
            .layerCount     = dst_subresource_layers.layer_count
        };

        const auto vk_regions = into_array<VkImageCopy>({
          VkImageCopy { .srcSubresource = vk_src_subresource_layers,
                       .srcOffset      = { 0, 0, 0 },
                       .dstSubresource = vk_dst_subresource_layers,
                       .dstOffset      = { 0, 0, 0 },
                       .extent         = vk::to_vk(extent) }
        });

        vk::call(device_table.vkCmdCopyImage,
                 *this,
                 src,
                 vk::to_vk<VkImageLayout>(src_layout),
                 dst,
                 vk::to_vk<VkImageLayout>(dst_layout),
                 stdr::size(vk_regions),
                 stdr::data(vk_regions));
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::resolve_image(view::Image                   src,
                                                     view::Image                   dst,
                                                     ImageLayout                   src_layout,
                                                     ImageLayout                   dst_layout,
                                                     const ImageSubresourceLayers& src_subresource_layers,
                                                     const ImageSubresourceLayers& dst_subresource_layers) const noexcept
      -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        const auto vk_extent = vk::to_vk(dst.extent());

        const auto vk_src_subresource_layers = VkImageSubresourceLayers {
            .aspectMask     = vk::to_vk<VkImageAspectFlags>(src_subresource_layers.aspect_mask),
            .mipLevel       = src_subresource_layers.mip_level,
            .baseArrayLayer = src_subresource_layers.base_array_layer,
            .layerCount     = src_subresource_layers.layer_count
        };

        const auto vk_dst_subresource_layers = VkImageSubresourceLayers {
            .aspectMask     = vk::to_vk<VkImageAspectFlags>(dst_subresource_layers.aspect_mask),
            .mipLevel       = dst_subresource_layers.mip_level,
            .baseArrayLayer = dst_subresource_layers.base_array_layer,
            .layerCount     = dst_subresource_layers.layer_count
        };

        const auto vk_regions = into_array<VkImageResolve>({
          VkImageResolve { .srcSubresource = vk_src_subresource_layers,
                          .srcOffset      = { 0, 0, 0 },
                          .dstSubresource = vk_dst_subresource_layers,
                          .dstOffset      = { 0, 0, 0 },
                          .extent         = vk_extent }
        });

        vk::call(device_table.vkCmdResolveImage,
                 *this,
                 src,
                 vk::to_vk<VkImageLayout>(src_layout),
                 dst,
                 vk::to_vk<VkImageLayout>(dst_layout),
                 stdr::size(vk_regions),
                 stdr::data(vk_regions));
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::blit_image(view::Image                  src,
                                                  view::Image                  dst,
                                                  ImageLayout                  src_layout,
                                                  ImageLayout                  dst_layout,
                                                  array_view<const BlitRegion> regions,
                                                  Filter filter) const noexcept -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        const auto vk_regions = transform(regions, [](const auto& region) static noexcept {
            const auto vk_src_subresource_layers = VkImageSubresourceLayers {
                .aspectMask     = vk::to_vk<VkImageAspectFlags>(region.src.aspect_mask),
                .mipLevel       = region.src.mip_level,
                .baseArrayLayer = region.src.base_array_layer,
                .layerCount     = region.src.layer_count
            };

            const auto vk_dst_subresource_layers = VkImageSubresourceLayers {
                .aspectMask     = vk::to_vk<VkImageAspectFlags>(region.dst.aspect_mask),
                .mipLevel       = region.dst.mip_level,
                .baseArrayLayer = region.dst.base_array_layer,
                .layerCount     = region.dst.layer_count
            };

            return VkImageBlit {
                .srcSubresource = vk_src_subresource_layers,
                .srcOffsets     = { vk::to_vk<VkOffset3D>(region.src_offset.position),
                                   vk::to_vk<VkOffset3D>(region.src_offset.extent) },
                .dstSubresource = vk_dst_subresource_layers,
                .dstOffsets     = { vk::to_vk<VkOffset3D>(region.dst_offset.position),
                                   vk::to_vk<VkOffset3D>(region.dst_offset.extent) },
            };
        });

        vk::call(device_table.vkCmdBlitImage,
                 *this,
                 vk::to_vk(src),
                 vk::to_vk<VkImageLayout>(src_layout),
                 vk::to_vk(dst),
                 vk::to_vk<VkImageLayout>(dst_layout),
                 stdr::size(vk_regions),
                 stdr::data(vk_regions),
                 vk::to_vk<VkFilter>(filter));
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::transition_image_layout(view::Image                  image,
                                                               ImageLayout                  src_layout,
                                                               ImageLayout                  dst_layout,
                                                               const ImageSubresourceRange& subresource_range) const noexcept
      -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        const auto vk_src_layout = vk::to_vk<VkImageLayout>(src_layout);
        const auto vk_dst_layout = vk::to_vk<VkImageLayout>(dst_layout);

        const auto& src_access = OLD_LAYOUT_ACCESS_MAP.find(vk_src_layout);
        const auto& dst_access = NEW_LAYOUT_ACCESS_MAP.find(vk_dst_layout);

        const auto src_stage = src_access->second.second;
        const auto dst_stage = dst_access->second.second;

        const auto vk_subresource_range = VkImageSubresourceRange {
            .aspectMask     = vk::to_vk<VkImageAspectFlags>(subresource_range.aspect_mask),
            .baseMipLevel   = subresource_range.base_mip_level,
            .levelCount     = subresource_range.level_count,
            .baseArrayLayer = subresource_range.base_array_layer,
            .layerCount     = subresource_range.layer_count,
        };

        const auto barriers = into_array<VkImageMemoryBarrier>({
          VkImageMemoryBarrier {
                                .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                .pNext               = nullptr,
                                .srcAccessMask       = src_access->second.first,
                                .dstAccessMask       = dst_access->second.first,
                                .oldLayout           = vk_src_layout,
                                .newLayout           = vk_dst_layout,
                                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                .image               = vk::to_vk(image),
                                .subresourceRange    = vk_subresource_range

          },
        });

        vk::call(device_table.vkCmdPipelineBarrier,
                 *this,
                 src_stage,
                 dst_stage,
                 0,
                 0,
                 nullptr,
                 0,
                 nullptr,
                 stdr::size(barriers),
                 stdr::data(barriers));
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::pipeline_barrier(PipelineStageFlag                     src_mask,
                                                        PipelineStageFlag                     dst_mask,
                                                        DependencyFlag                        dependency,
                                                        array_view<const MemoryBarrier>       memory_barriers,
                                                        array_view<const BufferMemoryBarrier> buffer_memory_barriers,
                                                        array_view<const ImageMemoryBarrier> image_memory_barriers) const noexcept
      -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        const auto vk_memory_barriers = transform(memory_barriers, [](const auto& barrier) static noexcept -> decltype(auto) {
            return VkMemoryBarrier {
                .sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
                .pNext         = nullptr,
                .srcAccessMask = vk::to_vk<VkAccessFlags>(barrier.src),
                .dstAccessMask = vk::to_vk<VkAccessFlags>(barrier.dst),
            };
        });
        const auto vk_buffer_memory_barriers = transform(buffer_memory_barriers,
                                                         [](const auto& barrier) static noexcept -> decltype(auto) {
                                                             return VkBufferMemoryBarrier {
                                                                 .sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                                                 .pNext               = nullptr,
                                                                 .srcAccessMask       = vk::to_vk<VkAccessFlags>(barrier.src),
                                                                 .dstAccessMask       = vk::to_vk<VkAccessFlags>(barrier.dst),
                                                                 .srcQueueFamilyIndex = barrier.src_queue_family_index,
                                                                 .dstQueueFamilyIndex = barrier.dst_queue_family_index,
                                                                 .buffer              = vk::to_vk(barrier.buffer),
                                                                 .offset              = barrier.offset,
                                                                 .size                = barrier.size
                                                             };
                                                         });
        const auto
          vk_image_memory_barriers = transform(image_memory_barriers, [](const auto& barrier) static noexcept -> decltype(auto) {
              const auto vk_subresource_range = VkImageSubresourceRange {
                  .aspectMask     = vk::to_vk<VkImageAspectFlags>(barrier.range.aspect_mask),
                  .baseMipLevel   = barrier.range.base_mip_level,
                  .levelCount     = barrier.range.level_count,
                  .baseArrayLayer = barrier.range.base_array_layer,
                  .layerCount     = barrier.range.layer_count
              };

              return VkImageMemoryBarrier {
                  .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                  .pNext               = nullptr,
                  .srcAccessMask       = vk::to_vk<VkAccessFlags>(barrier.src),
                  .dstAccessMask       = vk::to_vk<VkAccessFlags>(barrier.dst),
                  .oldLayout           = vk::to_vk<VkImageLayout>(barrier.old_layout),
                  .newLayout           = vk::to_vk<VkImageLayout>(barrier.new_layout),
                  .srcQueueFamilyIndex = barrier.src_queue_family_index,
                  .dstQueueFamilyIndex = barrier.dst_queue_family_index,
                  .image               = vk::to_vk(barrier.image),
                  .subresourceRange    = vk_subresource_range
              };
          });

        vk::call(device_table.vkCmdPipelineBarrier,
                 *this,
                 vk::to_vk<VkPipelineStageFlags>(src_mask),
                 vk::to_vk<VkPipelineStageFlags>(dst_mask),
                 vk::to_vk<VkDependencyFlags>(dependency),
                 stdr::size(vk_memory_barriers),
                 stdr::data(vk_memory_barriers),
                 stdr::size(vk_buffer_memory_barriers),
                 stdr::data(vk_buffer_memory_barriers),
                 stdr::size(vk_image_memory_barriers),
                 stdr::data(vk_image_memory_barriers));
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::push_constants(view::PipelineLayout pipeline_layout,
                                                      ShaderStageFlag      stage,
                                                      byte_view<>          data,
                                                      u32 offset) const noexcept -> const CommandBufferInterface& {
        EXPECTS(not std::empty(data));

        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        vk::call(device_table.vkCmdPushConstants,
                 *this,
                 vk::to_vk(pipeline_layout),
                 vk::to_vk<VkShaderStageFlags>(stage),
                 offset,
                 stdr::size(data),
                 stdr::data(data));
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto CommandBufferInterface<Base>::execute_sub_command_buffers(array_view<const view::CommandBuffer> commandbuffers)
      const noexcept -> const CommandBufferInterface& {
        const auto state = this->state();
        EXPECTS(state == CommandBuffer::State::RECORDING);

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        const auto expects_secondary = [](auto&& cmb) static noexcept -> decltype(auto) {
            EXPECTS(cmb.level() == CommandBufferLevel::SECONDARY);
            return cmb;
        };

        const auto vk_command_buffers = transform(commandbuffers, cmonadic::map(expects_secondary, vk::monadic::to_vk()));
        vk::call(device_table.vkCmdExecuteCommands, *this, stdr::size(vk_command_buffers), stdr::data(vk_command_buffers));
        return *this;
    }

    template class CommandBufferInterface<CommandBufferImplementation>;
    template class CommandBufferInterface<view::CommandBufferImplementation>;

    /////////////////////////////////////
    /////////////////////////////////////
    auto CommandBufferImplementation::do_init(PrivateTag,
                                              CommandBufferLevel     level,
                                              VkCommandBuffer&&      handle,
                                              CommandBufferDeleter&& deleter) noexcept -> void {
        m_state = core::allocate_unsafe<State>(State::INITIAL);

        m_level = level;

        m_vk_handle = std::move(handle);
        m_deleter   = std::move(deleter);
    }
} // namespace stormkit::gpu
