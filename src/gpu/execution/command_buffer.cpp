// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <volk.h>

#include <stormkit/core/platform_macro.hpp>

#include <stormkit/core/contract_macro.hpp>

module stormkit.gpu.execution;

import std;

import frozen;

import stormkit.core;

import stormkit.gpu.core;
import stormkit.gpu.resource;

using namespace std::literals;

namespace stdr = std::ranges;
namespace stdv = std::views;

namespace stormkit::gpu {
    namespace {
        constexpr auto OLD_LAYOUT_ACCESS_MAP = frozen::make_unordered_map<
          VkImageLayout,
          std::pair<VkAccessFlags, VkPipelineStageFlags>>({
          { VK_IMAGE_LAYOUT_UNDEFINED,                        { VK_ACCESS_NONE, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT } },
          { VK_IMAGE_LAYOUT_PREINITIALIZED,                   { VK_ACCESS_NONE, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT } },
          { VK_IMAGE_LAYOUT_GENERAL,
           { VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
              VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }                                                       },
          { VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
           { VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
              VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }                                                       },
          { VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
           { VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
                | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
              VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT }                                                           },
          { VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
           { VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
              VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT }                                                           },
          { VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
           { VK_ACCESS_INPUT_ATTACHMENT_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT }                           },
          { VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
           { VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT }                                          },
          { VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
           { VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT }                                         },
          { VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
           { VK_ACCESS_MEMORY_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT }                                            },
        });

        constexpr auto NEW_LAYOUT_ACCESS_MAP = frozen::
          make_unordered_map<VkImageLayout, std::pair<VkAccessFlags, VkPipelineStageFlags>>({
            { VK_IMAGE_LAYOUT_UNDEFINED,                        { VK_ACCESS_NONE, {} } },
            { VK_IMAGE_LAYOUT_PREINITIALIZED,                   { VK_ACCESS_NONE, {} } },
            { VK_IMAGE_LAYOUT_GENERAL,
             { VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT,
                VK_PIPELINE_STAGE_VERTEX_SHADER_BIT }                                  },
            { VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
             { VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }                        },
            { VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
             { VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
                  | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT }                           },
            { VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
             { VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT }         },
            { VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
             { VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT }      },
            { VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
             { VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT }           },
            { VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
             { VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT }          },
            { VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
             { VK_ACCESS_MEMORY_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT }             },
        });
    } // namespace

    /////////////////////////////////////
    /////////////////////////////////////
    auto CommandBuffer::begin(bool                           one_time_submit,
                              std::optional<InheritanceInfo> inheritance_info) noexcept
      -> Expected<void> {
        EXPECTS(m_state == State::INITIAL);

        const auto vk_inheritance_info = [&inheritance_info] {
            auto info  = zeroed<VkCommandBufferInheritanceInfo>();
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            if (inheritance_info) {
                info.renderPass  = to_vk(*inheritance_info->render_pass);
                info.subpass     = inheritance_info->subpass;
                info.framebuffer = to_vk(*inheritance_info->framebuffer);
            }
            return info;
        }();

        const auto flags = [this, one_time_submit]() {
            auto flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            if (!one_time_submit) flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            if (m_level == CommandBufferLevel::SECONDARY)
                flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;

            return flags;
        }();

        const auto begin_info = VkCommandBufferBeginInfo {
            .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext            = nullptr,
            .flags            = flags,
            .pInheritanceInfo = &vk_inheritance_info,
        };

        return vk_call(m_vk_device_table->vkBeginCommandBuffer, m_vk_handle, &begin_info)
          .transform([this] noexcept { m_state = State::RECORDING; })
          .transform_error(monadic::from_vk<Result>());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto CommandBuffer::begin_render_pass(const RenderPass&           render_pass,
                                          const FrameBuffer&          framebuffer,
                                          std::span<const ClearValue> clear_values,
                                          bool secondary_commandbuffers) noexcept
      -> CommandBuffer& {
        EXPECTS(m_state == State::RECORDING);

        const auto vk_clear_values
          = clear_values
            | stdv::transform(core::monadic::either(
              [](const ClearColor& clear_color) static noexcept -> decltype(auto) {
                  return VkClearValue {
                      .color = VkClearColorValue { .float32 = { clear_color.color.red,
                                                                clear_color.color.blue,
                                                                clear_color.color.green,
                                                                clear_color.color.alpha } },
                  };
              },
              [](const ClearDepthStencil& clear_depth_stencil) static noexcept -> decltype(auto) {
                  return VkClearValue {
                      .depthStencil = VkClearDepthStencilValue { .depth = clear_depth_stencil.depth,
                                                                .stencil = clear_depth_stencil
                                                                              .stencil },
                  };
              }))
            | stdr::to<std::vector>();

        const auto begin_info = VkRenderPassBeginInfo {
            .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext           = nullptr,
            .renderPass      = to_vk(render_pass),
            .framebuffer     = to_vk(framebuffer),
            .renderArea      = VkRect2D { .offset = { 0, 0 },
                                         .extent = { framebuffer.extent().width,
                                                      framebuffer.extent().height } },
            .clearValueCount = as<u32>(stdr::size(vk_clear_values)),
            .pClearValues    = stdr::data(vk_clear_values),
        };

        const auto subpass_content = secondary_commandbuffers
                                       ? VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
                                       : VK_SUBPASS_CONTENTS_INLINE;

        vk_call(m_vk_device_table->vkCmdBeginRenderPass, m_vk_handle, &begin_info, subpass_content);
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto CommandBuffer::set_viewport(u32                       first_viewport,
                                     std::span<const Viewport> viewports) noexcept
      -> CommandBuffer& {
        EXPECTS(m_state == State::RECORDING);

        const auto vk_viewports = viewports
                                  | stdv::transform(monadic::to_vk())
                                  | stdr::to<std::vector>();

        vk_call(m_vk_device_table->vkCmdSetViewport,
                m_vk_handle,
                first_viewport,
                stdr::size(vk_viewports),
                stdr::data(vk_viewports));
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto CommandBuffer::set_scissor(u32 first_scissor, std::span<const Scissor> scissors) noexcept
      -> CommandBuffer& {
        EXPECTS(m_state == State::RECORDING);

        const auto vk_scissors = scissors
                                 | stdv::transform(monadic::to_vk())
                                 | stdr::to<std::vector>();

        vk_call(m_vk_device_table->vkCmdSetScissor,
                m_vk_handle,
                first_scissor,
                stdr::size(vk_scissors),
                stdr::data(vk_scissors));
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto CommandBuffer::bind_vertex_buffers(std::span<const Ref<const Buffer>> buffers,
                                            std::span<const u64>               offsets) noexcept
      -> CommandBuffer& {
        EXPECTS(m_state == State::RECORDING);
        EXPECTS(not std::empty(buffers));
        EXPECTS(std::size(buffers) == std::size(offsets));

        const auto vk_buffers = buffers
                                | stdv::transform(monadic::to_vk())
                                | stdr::to<std::vector>();

        vk_call(m_vk_device_table->vkCmdBindVertexBuffers,
                m_vk_handle,
                0,
                stdr::size(vk_buffers),
                stdr::data(vk_buffers),
                stdr::data(offsets));
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto CommandBuffer::bind_descriptor_sets(const Pipeline&       pipeline,
                                             const PipelineLayout& layout,
                                             std::span<const Ref<const DescriptorSet>>
                                                                  descriptor_sets,
                                             std::span<const u32> dynamic_offsets) noexcept
      -> CommandBuffer& {
        EXPECTS(m_state == State::RECORDING);

        const auto bind_point = (pipeline.type() == Pipeline::Type::RASTER)
                                  ? VK_PIPELINE_BIND_POINT_GRAPHICS
                                  : VK_PIPELINE_BIND_POINT_COMPUTE;

        const auto vk_descriptor_sets = descriptor_sets
                                        | stdv::transform(monadic::to_vk())
                                        | stdr::to<std::vector>();

        vk_call(m_vk_device_table->vkCmdBindDescriptorSets,
                m_vk_handle,
                bind_point,
                to_vk(layout),
                0,
                stdr::size(vk_descriptor_sets),
                stdr::data(vk_descriptor_sets),
                stdr::size(dynamic_offsets),
                stdr::data(dynamic_offsets));
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto CommandBuffer::copy_buffer(const Buffer& src,
                                    const Buffer& dst,
                                    usize         size,
                                    u64           src_offset,
                                    u64           dst_offset) noexcept -> CommandBuffer& {
        const auto vk_copy_buffers = std::array {
            VkBufferCopy { .srcOffset = src_offset, .dstOffset = dst_offset, .size = size }
        };

        vk_call(m_vk_device_table->vkCmdCopyBuffer,
                m_vk_handle,
                to_vk(src),
                to_vk(dst),
                stdr::size(vk_copy_buffers),
                stdr::data(vk_copy_buffers));
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto CommandBuffer::copy_buffer_to_image(const Buffer& src,
                                             const Image&  dst,
                                             std::span<const BufferImageCopy>
                                               buffer_image_copies) noexcept -> CommandBuffer& {
        EXPECTS(m_state == State::RECORDING);

        const auto DEFAULT_COPY = std::array {
            BufferImageCopy { 0, 0, 0, {}, { 0, 0, 0 }, dst.extent() }
        };

        if (stdr::empty(buffer_image_copies)) buffer_image_copies = DEFAULT_COPY;

        const auto vk_copy_regions
          = buffer_image_copies
            | stdv::transform([](auto&& buffer_image_copy) noexcept {
                  const auto image_subresource = VkImageSubresourceLayers {
                      .aspectMask     = to_vk<VkImageAspectFlagBits>(buffer_image_copy
                                                                   .subresource_layers.aspect_mask),
                      .mipLevel       = buffer_image_copy.subresource_layers.mip_level,
                      .baseArrayLayer = buffer_image_copy.subresource_layers.base_array_layer,
                      .layerCount     = buffer_image_copy.subresource_layers.layer_count,
                  };

                  return VkBufferImageCopy {
                      .bufferOffset      = buffer_image_copy.buffer_offset,
                      .bufferRowLength   = buffer_image_copy.buffer_row_length,
                      .bufferImageHeight = buffer_image_copy.buffer_image_height,
                      .imageSubresource  = image_subresource,
                      .imageOffset       = to_vk<VkOffset3D>(buffer_image_copy.offset),
                      .imageExtent       = to_vk(buffer_image_copy.extent)
                  };
              })
            | stdr::to<std::vector>();

        vk_call(m_vk_device_table->vkCmdCopyBufferToImage,
                m_vk_handle,
                to_vk(src),
                to_vk(dst),
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                stdr::size(vk_copy_regions),
                stdr::data(vk_copy_regions));
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto CommandBuffer::copy_image_to_buffer(const Image&  src,
                                             const Buffer& dst,
                                             std::span<const BufferImageCopy>
                                               buffer_image_copies) noexcept -> CommandBuffer& {
        EXPECTS(m_state == State::RECORDING);

        const auto DEFAULT_COPY = std::array {
            BufferImageCopy { 0, 0, 0, {}, { 0, 0, 0 }, src.extent() }
        };

        if (stdr::empty(buffer_image_copies)) buffer_image_copies = DEFAULT_COPY;

        const auto vk_copy_regions
          = buffer_image_copies
            | stdv::transform([](auto&& buffer_image_copy) noexcept {
                  const auto image_subresource = VkImageSubresourceLayers {
                      .aspectMask     = to_vk<VkImageAspectFlagBits>(buffer_image_copy
                                                                   .subresource_layers.aspect_mask),
                      .mipLevel       = buffer_image_copy.subresource_layers.mip_level,
                      .baseArrayLayer = buffer_image_copy.subresource_layers.base_array_layer,
                      .layerCount     = buffer_image_copy.subresource_layers.layer_count,
                  };

                  return VkBufferImageCopy {
                      .bufferOffset      = buffer_image_copy.buffer_offset,
                      .bufferRowLength   = buffer_image_copy.buffer_row_length,
                      .bufferImageHeight = buffer_image_copy.buffer_image_height,
                      .imageSubresource  = image_subresource,
                      .imageOffset       = to_vk<VkOffset3D>(buffer_image_copy.offset),
                      .imageExtent       = to_vk(buffer_image_copy.extent)
                  };
              })
            | stdr::to<std::vector>();

        vk_call(m_vk_device_table->vkCmdCopyImageToBuffer,
                m_vk_handle,
                to_vk(src),
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                to_vk(dst),
                stdr::size(vk_copy_regions),
                stdr::data(vk_copy_regions));
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto CommandBuffer::copy_image(const Image&                  src,
                                   const Image&                  dst,
                                   ImageLayout                   src_layout,
                                   ImageLayout                   dst_layout,
                                   const ImageSubresourceLayers& src_subresource_layers,
                                   const ImageSubresourceLayers& dst_subresource_layers,
                                   const math::Extent3<u32>& extent) noexcept -> CommandBuffer& {
        EXPECTS(m_state == State::RECORDING);

        const auto vk_src_subresource_layers = VkImageSubresourceLayers {
            .aspectMask     = to_vk<VkImageAspectFlagBits>(src_subresource_layers.aspect_mask),
            .mipLevel       = src_subresource_layers.mip_level,
            .baseArrayLayer = src_subresource_layers.base_array_layer,
            .layerCount     = src_subresource_layers.layer_count
        };

        const auto vk_dst_subresource_layers = VkImageSubresourceLayers {
            .aspectMask     = to_vk<VkImageAspectFlagBits>(dst_subresource_layers.aspect_mask),
            .mipLevel       = dst_subresource_layers.mip_level,
            .baseArrayLayer = dst_subresource_layers.base_array_layer,
            .layerCount     = dst_subresource_layers.layer_count
        };

        const auto vk_regions = std::array {
            VkImageCopy { .srcSubresource = vk_src_subresource_layers,
                         .srcOffset      = { 0, 0, 0 },
                         .dstSubresource = vk_dst_subresource_layers,
                         .dstOffset      = { 0, 0, 0 },
                         .extent         = to_vk(extent) }
        };

        vk_call(m_vk_device_table->vkCmdCopyImage,
                m_vk_handle,
                to_vk(src),
                to_vk<VkImageLayout>(src_layout),
                to_vk(dst),
                to_vk<VkImageLayout>(dst_layout),
                stdr::size(vk_regions),
                stdr::data(vk_regions));
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto CommandBuffer::resolve_image(const Image&                  src,
                                      const Image&                  dst,
                                      ImageLayout                   src_layout,
                                      ImageLayout                   dst_layout,
                                      const ImageSubresourceLayers& src_subresource_layers,
                                      const ImageSubresourceLayers& dst_subresource_layers) noexcept
      -> CommandBuffer& {
        EXPECTS(m_state == State::RECORDING);

        const auto vk_extent = to_vk(dst.extent());

        const auto vk_src_subresource_layers = VkImageSubresourceLayers {
            .aspectMask     = to_vk<VkImageAspectFlagBits>(src_subresource_layers.aspect_mask),
            .mipLevel       = src_subresource_layers.mip_level,
            .baseArrayLayer = src_subresource_layers.base_array_layer,
            .layerCount     = src_subresource_layers.layer_count
        };

        const auto vk_dst_subresource_layers = VkImageSubresourceLayers {
            .aspectMask     = to_vk<VkImageAspectFlagBits>(dst_subresource_layers.aspect_mask),
            .mipLevel       = dst_subresource_layers.mip_level,
            .baseArrayLayer = dst_subresource_layers.base_array_layer,
            .layerCount     = dst_subresource_layers.layer_count
        };

        const auto vk_regions = std::array {
            VkImageResolve { .srcSubresource = vk_src_subresource_layers,
                            .srcOffset      = { 0, 0, 0 },
                            .dstSubresource = vk_dst_subresource_layers,
                            .dstOffset      = { 0, 0, 0 },
                            .extent         = vk_extent }
        };

        vk_call(m_vk_device_table->vkCmdResolveImage,
                m_vk_handle,
                to_vk(src),
                to_vk<VkImageLayout>(src_layout),
                to_vk(dst),
                to_vk<VkImageLayout>(dst_layout),
                stdr::size(vk_regions),
                stdr::data(vk_regions));
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto CommandBuffer::blit_image(const Image&                src,
                                   const Image&                dst,
                                   ImageLayout                 src_layout,
                                   ImageLayout                 dst_layout,
                                   std::span<const BlitRegion> regions,
                                   Filter                      filter) noexcept -> CommandBuffer& {
        EXPECTS(m_state == State::RECORDING);

        const auto vk_regions
          = regions
            | stdv::transform([](auto&& region) noexcept {
                  const auto vk_src_subresource_layers = VkImageSubresourceLayers {
                      .aspectMask     = to_vk<VkImageAspectFlagBits>(region.src.aspect_mask),
                      .mipLevel       = region.src.mip_level,
                      .baseArrayLayer = region.src.base_array_layer,
                      .layerCount     = region.src.layer_count
                  };

                  const auto vk_dst_subresource_layers = VkImageSubresourceLayers {
                      .aspectMask     = to_vk<VkImageAspectFlagBits>(region.dst.aspect_mask),
                      .mipLevel       = region.dst.mip_level,
                      .baseArrayLayer = region.dst.base_array_layer,
                      .layerCount     = region.dst.layer_count
                  };

                  return VkImageBlit {
                      .srcSubresource = vk_src_subresource_layers,
                      .srcOffsets     = { to_vk<VkOffset3D>(region.src_offset[0]),
                                         to_vk<VkOffset3D>(region.src_offset[1]) },
                      .dstSubresource = vk_dst_subresource_layers,
                      .dstOffsets     = { to_vk<VkOffset3D>(region.dst_offset[0]),
                                         to_vk<VkOffset3D>(region.dst_offset[1]) },
                  };
              })
            | stdr::to<std::vector>();

        vk_call(m_vk_device_table->vkCmdBlitImage,
                m_vk_handle,
                to_vk(src),
                to_vk<VkImageLayout>(src_layout),
                to_vk(dst),
                to_vk<VkImageLayout>(dst_layout),
                stdr::size(vk_regions),
                stdr::data(vk_regions),
                to_vk<VkFilter>(filter));
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto CommandBuffer::transition_image_layout(const Image& image,
                                                ImageLayout  src_layout,
                                                ImageLayout  dst_layout,
                                                const ImageSubresourceRange&
                                                  subresource_range) noexcept -> CommandBuffer& {
        EXPECTS(m_state == State::RECORDING);

        const auto vk_src_layout = to_vk<VkImageLayout>(src_layout);
        const auto vk_dst_layout = to_vk<VkImageLayout>(dst_layout);

        const auto& src_access = OLD_LAYOUT_ACCESS_MAP.find(vk_src_layout);
        const auto& dst_access = NEW_LAYOUT_ACCESS_MAP.find(vk_dst_layout);

        const auto src_stage = src_access->second.second;
        const auto dst_stage = dst_access->second.second;

        const auto vk_subresource_range = VkImageSubresourceRange {
            .aspectMask     = to_vk<VkImageAspectFlagBits>(subresource_range.aspect_mask),
            .baseMipLevel   = subresource_range.base_mip_level,
            .levelCount     = subresource_range.level_count,
            .baseArrayLayer = subresource_range.base_array_layer,
            .layerCount     = subresource_range.layer_count,
        };

        const auto barriers = std::array {
            VkImageMemoryBarrier {
                                  .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                  .pNext               = nullptr,
                                  .srcAccessMask       = src_access->second.first,
                                  .dstAccessMask       = dst_access->second.first,
                                  .oldLayout           = vk_src_layout,
                                  .newLayout           = vk_dst_layout,
                                  .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                  .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                  .image               = to_vk(image),
                                  .subresourceRange    = vk_subresource_range

            },
        };

        vk_call(m_vk_device_table->vkCmdPipelineBarrier,
                m_vk_handle,
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
    auto CommandBuffer::
      pipeline_barrier(PipelineStageFlag                    src_mask,
                       PipelineStageFlag                    dst_mask,
                       DependencyFlag                       dependency,
                       std::span<const MemoryBarrier>       memory_barriers,
                       std::span<const BufferMemoryBarrier> buffer_memory_barriers,
                       std::span<const ImageMemoryBarrier>  image_memory_barriers) noexcept
      -> CommandBuffer& {
        const auto vk_memory_barriers = memory_barriers
                                        | stdv::transform([](auto&& barrier) noexcept
                                                            -> decltype(auto) {
                                              return VkMemoryBarrier {
                                                  .sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
                                                  .pNext         = nullptr,
                                                  .srcAccessMask = to_vk<VkAccessFlagBits>(barrier
                                                                                             .src),
                                                  .dstAccessMask = to_vk<VkAccessFlagBits>(barrier
                                                                                             .dst),
                                              };
                                          })
                                        | stdr::to<std::vector>();

        const auto vk_buffer_memory_barriers
          = buffer_memory_barriers
            | stdv::transform([](auto&& barrier) noexcept -> decltype(auto) {
                  return VkBufferMemoryBarrier {
                      .sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                      .pNext               = nullptr,
                      .srcAccessMask       = to_vk<VkAccessFlagBits>(barrier.src),
                      .dstAccessMask       = to_vk<VkAccessFlagBits>(barrier.dst),
                      .srcQueueFamilyIndex = barrier.src_queue_family_index,
                      .dstQueueFamilyIndex = barrier.dst_queue_family_index,
                      .buffer              = to_vk(barrier.buffer),
                      .offset              = barrier.offset,
                      .size                = barrier.size
                  };
              })
            | stdr::to<std::vector>();

        const auto vk_image_memory_barriers
          = image_memory_barriers
            | stdv::transform([](auto&& barrier) noexcept -> decltype(auto) {
                  const auto vk_subresource_range = VkImageSubresourceRange {
                      .aspectMask     = to_vk<VkImageAspectFlagBits>(barrier.range.aspect_mask),
                      .baseMipLevel   = barrier.range.base_mip_level,
                      .levelCount     = barrier.range.level_count,
                      .baseArrayLayer = barrier.range.base_array_layer,
                      .layerCount     = barrier.range.layer_count
                  };

                  return VkImageMemoryBarrier {
                      .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                      .pNext               = nullptr,
                      .srcAccessMask       = to_vk<VkAccessFlagBits>(barrier.src),
                      .dstAccessMask       = to_vk<VkAccessFlagBits>(barrier.dst),
                      .oldLayout           = to_vk<VkImageLayout>(barrier.old_layout),
                      .newLayout           = to_vk<VkImageLayout>(barrier.new_layout),
                      .srcQueueFamilyIndex = barrier.src_queue_family_index,
                      .dstQueueFamilyIndex = barrier.dst_queue_family_index,
                      .image               = to_vk(barrier.image),
                      .subresourceRange    = vk_subresource_range
                  };
              })
            | stdr::to<std::vector>();

        vk_call(m_vk_device_table->vkCmdPipelineBarrier,
                m_vk_handle,
                to_vk<VkPipelineStageFlagBits>(src_mask),
                to_vk<VkPipelineStageFlagBits>(dst_mask),
                to_vk<VkDependencyFlagBits>(dependency),
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
    auto CommandBuffer::push_constants(const PipelineLayout& pipeline_layout,
                                       ShaderStageFlag       stage,
                                       std::span<const Byte> data,
                                       u32                   offset) noexcept -> CommandBuffer& {
        EXPECTS(m_state == State::RECORDING);
        EXPECTS(not std::empty(data));

        vk_call(m_vk_device_table->vkCmdPushConstants,
                m_vk_handle,
                to_vk(pipeline_layout),
                to_vk<VkShaderStageFlagBits>(stage),
                offset,
                stdr::size(data),
                stdr::data(data));
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto CommandBuffer::execute_sub_command_buffers(std::span<const Ref<const CommandBuffer>>
                                                      commandbuffers) noexcept -> CommandBuffer& {
        EXPECTS(m_state == State::RECORDING);

        constexpr auto EXPECTS_secondary = [](auto&& cmb) noexcept -> decltype(auto) {
            EXPECTS(cmb.level() == CommandBufferLevel::SECONDARY);
            return cmb;
        };

        const auto vk_command_buffers = commandbuffers
                                        | stdv::transform(core::monadic::unref())
                                        | stdv::transform(core::monadic::map(EXPECTS_secondary,
                                                                             monadic::to_vk()))
                                        | stdr::to<std::vector>();

        vk_call(m_vk_device_table->vkCmdExecuteCommands,
                m_vk_handle,
                stdr::size(vk_command_buffers),
                stdr::data(vk_command_buffers));
        return *this;
    }
} // namespace stormkit::gpu
