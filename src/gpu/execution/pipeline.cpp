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
    /////////////////////////////////////
    /////////////////////////////////////
    auto Pipeline::do_init(const PipelineLayout&            layout,
                           const RenderPass&                render_pass,
                           OptionalRef<const PipelineCache> pipeline_cache) noexcept
      -> Expected<void> {
        const auto& state = as<RasterPipelineState>(m_state);

        const auto binding_descriptions
          = state.vertex_input_state.binding_descriptions
            | stdv::transform([](auto&& binding_description) static noexcept {
                  return VkVertexInputBindingDescription {
                      .binding   = binding_description.binding,
                      .stride    = binding_description.stride,
                      .inputRate = to_vk<VkVertexInputRate>(binding_description.input_rate)

                  };
              })
            | stdr::to<std::vector>();

        const auto attribute_descriptions
          = state.vertex_input_state.input_attribute_descriptions
            | stdv::transform([](auto&& input_attribute_description) static noexcept {
                  return VkVertexInputAttributeDescription {
                      .location = input_attribute_description.location,
                      .binding  = input_attribute_description.binding,
                      .format   = to_vk<VkFormat>(input_attribute_description.format),
                      .offset   = input_attribute_description.offset
                  };
              })
            | stdr::to<std::vector>();

        const auto vertex_input_info = VkPipelineVertexInputStateCreateInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .vertexBindingDescriptionCount   = as<u32>(stdr::size(binding_descriptions)),
            .pVertexBindingDescriptions      = std::data(binding_descriptions),
            .vertexAttributeDescriptionCount = as<u32>(stdr::size(attribute_descriptions)),
            .pVertexAttributeDescriptions    = stdr::data(attribute_descriptions),
        };

        const auto input_assembly = VkPipelineInputAssemblyStateCreateInfo {
            .sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext    = nullptr,
            .flags    = 0,
            .topology = to_vk<VkPrimitiveTopology>(state.input_assembly_state.topology),
            .primitiveRestartEnable = state.input_assembly_state.primitive_restart_enable
        };

        const auto viewports = state.viewport_state.viewports
                               | stdv::transform(monadic::to_vk())
                               | stdr::to<std::vector>();

        const auto scissors = state.viewport_state.scissors
                              | stdv::transform(monadic::to_vk())
                              | stdr::to<std::vector>();

        const auto viewport_state = VkPipelineViewportStateCreateInfo {
            .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext         = nullptr,
            .flags         = 0,
            .viewportCount = as<u32>(stdr::size(viewports)),
            .pViewports    = stdr::data(viewports),
            .scissorCount  = as<u32>(stdr::size(scissors)),
            .pScissors     = stdr::data(scissors),
        };

        const auto rasterizer = VkPipelineRasterizationStateCreateInfo {
            .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext                   = nullptr,
            .flags                   = 0,
            .depthClampEnable        = state.rasterization_state.depth_clamp_enable,
            .rasterizerDiscardEnable = state.rasterization_state.rasterizer_discard_enable,
            .polygonMode             = to_vk<VkPolygonMode>(state.rasterization_state.polygon_mode),
            .cullMode                = to_vk<VkCullModeFlags>(state.rasterization_state.cull_mode),
            .frontFace               = to_vk<VkFrontFace>(state.rasterization_state.front_face),
            .depthBiasEnable         = state.rasterization_state.depth_bias_enable,
            .depthBiasConstantFactor = state.rasterization_state.depth_bias_constant_factor,
            .depthBiasClamp          = state.rasterization_state.depth_bias_clamp,
            .depthBiasSlopeFactor    = state.rasterization_state.depth_bias_slope_factor,
            .lineWidth               = state.rasterization_state.line_width,
        };

        const auto multisampling = VkPipelineMultisampleStateCreateInfo {
            .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext                 = nullptr,
            .flags                 = 0,
            .rasterizationSamples  = to_vk<VkSampleCountFlagBits>(state.multisample_state
                                                                   .rasterization_samples),
            .sampleShadingEnable   = state.multisample_state.sample_shading_enable,
            .minSampleShading      = state.multisample_state.min_sample_shading,
            .pSampleMask           = nullptr,
            .alphaToCoverageEnable = false,
            .alphaToOneEnable      = false,
        };

        const auto blend_attachments
          = state.color_blend_state.attachments
            | stdv::transform([](auto&& attachment) static noexcept {
                  return VkPipelineColorBlendAttachmentState {
                      .blendEnable         = attachment.blend_enable,
                      .srcColorBlendFactor = to_vk<VkBlendFactor>(attachment
                                                                    .src_color_blend_factor),
                      .dstColorBlendFactor = to_vk<VkBlendFactor>(attachment
                                                                    .dst_color_blend_factor),
                      .colorBlendOp        = to_vk<VkBlendOp>(attachment.color_blend_operation),
                      .srcAlphaBlendFactor = to_vk<VkBlendFactor>(attachment
                                                                    .src_alpha_blend_factor),
                      .dstAlphaBlendFactor = to_vk<VkBlendFactor>(attachment
                                                                    .dst_alpha_blend_factor),
                      .alphaBlendOp        = to_vk<VkBlendOp>(attachment.alpha_blend_operation),
                      .colorWriteMask = to_vk<VkColorComponentFlags>(attachment.color_write_mask)
                  };
              })
            | stdr::to<std::vector>();

        const auto color_blending = VkPipelineColorBlendStateCreateInfo {
            .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext           = nullptr,
            .flags           = 0,
            .logicOpEnable   = state.color_blend_state.logic_operation_enable,
            .logicOp         = to_vk<VkLogicOp>(state.color_blend_state.logic_operation),
            .attachmentCount = as<u32>(stdr::size(blend_attachments)),
            .pAttachments    = stdr::data(blend_attachments),
            .blendConstants  = { state.color_blend_state.blend_constants[0],
                                state.color_blend_state.blend_constants[1],
                                state.color_blend_state.blend_constants[2],
                                state.color_blend_state.blend_constants[3] },
        };

        const auto states = state.dynamic_state
                            | stdv::transform(monadic::to_vk<VkDynamicState>())
                            | stdr::to<std::vector>();

        const auto dynamic_state = VkPipelineDynamicStateCreateInfo {
            .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext             = nullptr,
            .flags             = 0,
            .dynamicStateCount = as<u32>(stdr::size(states)),
            .pDynamicStates    = stdr::data(states),
        };

        const auto shaders = state.shader_state
                             | stdv::transform(core::monadic::unref())
                             | stdv::transform([](auto&& shader) static noexcept {
                                   static constexpr auto NAME = "main";
                                   return VkPipelineShaderStageCreateInfo {
                                       .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                       .pNext = nullptr,
                                       .flags = 0,
                                       .stage = to_vk<VkShaderStageFlagBits>(shader.type()),
                                       .module              = to_vk(shader),
                                       .pName               = NAME,
                                       .pSpecializationInfo = nullptr,
                                   };
                               })
                             | stdr::to<std::vector>();

        const auto depth_stencil = VkPipelineDepthStencilStateCreateInfo {
            .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .pNext                 = nullptr,
            .flags                 = 0,
            .depthTestEnable       = state.depth_stencil_state.depth_test_enable,
            .depthWriteEnable      = state.depth_stencil_state.depth_write_enable,
            .depthCompareOp        = to_vk<VkCompareOp>(state.depth_stencil_state.depth_compare_op),
            .depthBoundsTestEnable = state.depth_stencil_state.depth_bounds_test_enable,
            .stencilTestEnable     = false,
            .front                 = {},
            .back                  = {},
            .minDepthBounds        = state.depth_stencil_state.min_depth_bounds,
            .maxDepthBounds        = state.depth_stencil_state.max_depth_bounds
        };

        const auto create_info = VkGraphicsPipelineCreateInfo {
            .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext               = nullptr,
            .flags               = 0,
            .stageCount          = as<u32>(stdr::size(shaders)),
            .pStages             = stdr::data(shaders),
            .pVertexInputState   = &vertex_input_info,
            .pInputAssemblyState = &input_assembly,
            .pTessellationState  = nullptr,
            .pViewportState      = &viewport_state,
            .pRasterizationState = &rasterizer,
            .pMultisampleState   = &multisampling,
            .pDepthStencilState  = &depth_stencil,
            .pColorBlendState    = &color_blending,
            .pDynamicState       = &dynamic_state,
            .layout              = to_vk(layout),
            .renderPass          = to_vk(render_pass),
            .subpass             = 0,
            .basePipelineHandle  = nullptr,
            .basePipelineIndex   = -1,
        };

        using namespace core::monadic;
        const auto vk_pipeline_cache = core::either(pipeline_cache,
                                                    monadic::to_vk(),
                                                    init<VkPipelineCache>(nullptr));

        return vk_call<VkPipeline>(m_vk_device_table->vkCreateGraphicsPipelines,
                                   m_vk_device,
                                   vk_pipeline_cache,
                                   1,
                                   &create_info,
                                   nullptr)
          .transform(set(m_vk_handle))
          .transform_error(monadic::from_vk<Result>());
    }
} // namespace stormkit::gpu
