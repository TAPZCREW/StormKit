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

namespace cmonadic = stormkit::core::monadic;

using namespace std::literals;

namespace stormkit::gpu {
    namespace {
        struct PipelineData {
            dyn_array<VkVertexInputBindingDescription>     binding_descriptions;
            dyn_array<VkVertexInputAttributeDescription>   input_attribute_descriptions;
            VkPipelineVertexInputStateCreateInfo           vertex_input_info;
            VkPipelineInputAssemblyStateCreateInfo         input_assembly;
            dyn_array<VkViewport>                          viewports;
            dyn_array<VkRect2D>                            scissors;
            VkPipelineViewportStateCreateInfo              viewport_state;
            VkPipelineRasterizationStateCreateInfo         rasterizer;
            VkPipelineMultisampleStateCreateInfo           multisample;
            dyn_array<VkPipelineColorBlendAttachmentState> blend_attachments;
            VkPipelineColorBlendStateCreateInfo            color_blending;
            dyn_array<VkDynamicState>                      states;
            VkPipelineDynamicStateCreateInfo               dynamic_state;
            dyn_array<VkPipelineShaderStageCreateInfo>     shaders;
            VkPipelineDepthStencilStateCreateInfo          depth_stencil;
        };

        auto do_init(const RasterPipelineState& state) noexcept -> PipelineData {
            auto out                 = PipelineData {};
            out.binding_descriptions = transform(state.vertex_input_state.binding_descriptions,
                                                 [](const auto& binding_description) static noexcept {
                                                     return VkVertexInputBindingDescription {
                                                         .binding   = binding_description.binding,
                                                         .stride    = binding_description.stride,
                                                         .inputRate = vk::to_vk<VkVertexInputRate>(binding_description.input_rate)

                                                     };
                                                 });

            out.input_attribute_descriptions = transform(state.vertex_input_state.input_attribute_descriptions,
                                                         [](auto&& input_attribute_description) static noexcept {
                                                             return VkVertexInputAttributeDescription {
                                                                 .location = input_attribute_description.location,
                                                                 .binding  = input_attribute_description.binding,
                                                                 .format   = vk::to_vk<
                                                                   VkFormat>(input_attribute_description.format),
                                                                 .offset = input_attribute_description.offset
                                                             };
                                                         });
            out.vertex_input_info            = VkPipelineVertexInputStateCreateInfo {
                .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .pNext                           = nullptr,
                .flags                           = 0,
                .vertexBindingDescriptionCount   = as<u32>(stdr::size(out.binding_descriptions)),
                .pVertexBindingDescriptions      = std::data(out.binding_descriptions),
                .vertexAttributeDescriptionCount = as<u32>(stdr::size(out.input_attribute_descriptions)),
                .pVertexAttributeDescriptions    = stdr::data(out.input_attribute_descriptions),
            };

            out.input_assembly = VkPipelineInputAssemblyStateCreateInfo {
                .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                .pNext                  = nullptr,
                .flags                  = 0,
                .topology               = vk::to_vk<VkPrimitiveTopology>(state.input_assembly_state.topology),
                .primitiveRestartEnable = state.input_assembly_state.primitive_restart_enable
            };

            out.viewports = transform(state.viewport_state.viewports, vk::monadic::to_vk());

            out.scissors = transform(state.viewport_state.scissors, vk::monadic::to_vk());

            out.viewport_state = VkPipelineViewportStateCreateInfo {
                .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .pNext         = nullptr,
                .flags         = 0,
                .viewportCount = as<u32>(stdr::size(out.viewports)),
                .pViewports    = stdr::data(out.viewports),
                .scissorCount  = as<u32>(stdr::size(out.scissors)),
                .pScissors     = stdr::data(out.scissors),
            };

            out.rasterizer = VkPipelineRasterizationStateCreateInfo {
                .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                .pNext                   = nullptr,
                .flags                   = 0,
                .depthClampEnable        = state.rasterization_state.depth_clamp_enable,
                .rasterizerDiscardEnable = state.rasterization_state.rasterizer_discard_enable,
                .polygonMode             = vk::to_vk<VkPolygonMode>(state.rasterization_state.polygon_mode),
                .cullMode                = vk::to_vk<VkCullModeFlags>(state.rasterization_state.cull_mode),
                .frontFace               = vk::to_vk<VkFrontFace>(state.rasterization_state.front_face),
                .depthBiasEnable         = state.rasterization_state.depth_bias_enable,
                .depthBiasConstantFactor = state.rasterization_state.depth_bias_constant_factor,
                .depthBiasClamp          = state.rasterization_state.depth_bias_clamp,
                .depthBiasSlopeFactor    = state.rasterization_state.depth_bias_slope_factor,
                .lineWidth               = state.rasterization_state.line_width,
            };

            out.multisample = VkPipelineMultisampleStateCreateInfo {
                .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                .pNext                 = nullptr,
                .flags                 = 0,
                .rasterizationSamples  = vk::to_vk<VkSampleCountFlagBits>(state.multisample_state.rasterization_samples),
                .sampleShadingEnable   = state.multisample_state.sample_shading_enable,
                .minSampleShading      = state.multisample_state.min_sample_shading,
                .pSampleMask           = nullptr,
                .alphaToCoverageEnable = false,
                .alphaToOneEnable      = false,
            };

            out.blend_attachments = transform(state.color_blend_state.attachments, [](auto&& attachment) static noexcept {
                return VkPipelineColorBlendAttachmentState {
                    .blendEnable         = attachment.blend_enable,
                    .srcColorBlendFactor = vk::to_vk<VkBlendFactor>(attachment.src_color_blend_factor),
                    .dstColorBlendFactor = vk::to_vk<VkBlendFactor>(attachment.dst_color_blend_factor),
                    .colorBlendOp        = vk::to_vk<VkBlendOp>(attachment.color_blend_operation),
                    .srcAlphaBlendFactor = vk::to_vk<VkBlendFactor>(attachment.src_alpha_blend_factor),
                    .dstAlphaBlendFactor = vk::to_vk<VkBlendFactor>(attachment.dst_alpha_blend_factor),
                    .alphaBlendOp        = vk::to_vk<VkBlendOp>(attachment.alpha_blend_operation),
                    .colorWriteMask      = vk::to_vk<VkColorComponentFlags>(attachment.color_write_mask)
                };
            });

            out.color_blending = VkPipelineColorBlendStateCreateInfo {
                .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                .pNext           = nullptr,
                .flags           = 0,
                .logicOpEnable   = state.color_blend_state.logic_operation_enable,
                .logicOp         = vk::to_vk<VkLogicOp>(state.color_blend_state.logic_operation),
                .attachmentCount = as<u32>(stdr::size(out.blend_attachments)),
                .pAttachments    = stdr::data(out.blend_attachments),
                .blendConstants  = { state.color_blend_state.blend_constants[0],
                                    state.color_blend_state.blend_constants[1],
                                    state.color_blend_state.blend_constants[2],
                                    state.color_blend_state.blend_constants[3] },
            };

            out.states = transform(state.dynamic_state, vk::monadic::to_vk<VkDynamicState>());

            out.dynamic_state = VkPipelineDynamicStateCreateInfo {
                .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                .pNext             = nullptr,
                .flags             = 0,
                .dynamicStateCount = as<u32>(stdr::size(out.states)),
                .pDynamicStates    = stdr::data(out.states),
            };

            out.shaders = transform(state.shader_state, [](const auto& shader) static noexcept {
                const auto name = [](const auto& shader) static noexcept {
                    if (shader.type() == ShaderStageFlag::VERTEX) return "vert_main"sv;
                    else if (shader.type() == ShaderStageFlag::FRAGMENT)
                        return "frag_main"sv;
                    else
                        return "main"sv;
                }(shader);

                return VkPipelineShaderStageCreateInfo {
                    .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .pNext               = nullptr,
                    .flags               = 0,
                    .stage               = vk::to_vk<VkShaderStageFlagBits>(shader.type()),
                    .module              = shader,
                    .pName               = stdr::data(name),
                    .pSpecializationInfo = nullptr,
                };
            });

            out.depth_stencil = VkPipelineDepthStencilStateCreateInfo {
                .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                .pNext                 = nullptr,
                .flags                 = 0,
                .depthTestEnable       = state.depth_stencil_state.depth_test_enable,
                .depthWriteEnable      = state.depth_stencil_state.depth_write_enable,
                .depthCompareOp        = vk::to_vk<VkCompareOp>(state.depth_stencil_state.depth_compare_op),
                .depthBoundsTestEnable = state.depth_stencil_state.depth_bounds_test_enable,
                .stencilTestEnable     = false,
                .front                 = {},
                .back                  = {},
                .minDepthBounds        = state.depth_stencil_state.min_depth_bounds,
                .maxDepthBounds        = state.depth_stencil_state.max_depth_bounds
            };

            return out;
        }
    } // namespace

    template class PipelineInterface<PipelineImplementation>;
    template class PipelineInterface<view::PipelineImplementation>;

    /////////////////////////////////////
    /////////////////////////////////////
    auto PipelineImplementation::do_init(PrivateTag, const RasterizationCreateInfo& create_info) noexcept -> Expected<void> {
        m_type  = Type::RASTER;
        m_state = core::allocate_unsafe<StateVariant>(create_info.state);

        const auto& state = as<RasterPipelineState>(*m_state);

        const auto [binding_descriptions,
                    attribute_descriptions,
                    vertex_input_info,
                    input_assembly,
                    viewports,
                    scissors,
                    viewport_state,
                    rasterizer,
                    multisampling,
                    blend_attachments,
                    color_blending,
                    states,
                    dynamic_state,
                    shaders,
                    depth_stencil] = gpu::do_init(state);

        const auto formats = transform(create_info.rendering_info.color_attachment_formats, vk::monadic::to_vk<VkFormat>());

        const auto rendering_info = [&] noexcept {
            auto info = VkPipelineRenderingCreateInfo {
                .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                .pNext                   = nullptr,
                .viewMask                = create_info.rendering_info.view_mask,
                .colorAttachmentCount    = as<u32>(stdr::size(formats)),
                .pColorAttachmentFormats = stdr::data(formats),
                .depthAttachmentFormat   = {},
                .stencilAttachmentFormat = {}
            };

            if (create_info.rendering_info.depth_attachment_format)
                info.depthAttachmentFormat = vk::to_vk<VkFormat>(*create_info.rendering_info.depth_attachment_format);

            if (create_info.rendering_info.stencil_attachment_format)
                info.stencilAttachmentFormat = vk::to_vk<VkFormat>(*create_info.rendering_info.stencil_attachment_format);

            return info;
        }();

        const auto vk_create_info = VkGraphicsPipelineCreateInfo {
            .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext               = &rendering_info,
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
            .layout              = vk::to_vk(create_info.layout),
            .renderPass          = VK_NULL_HANDLE,
            .subpass             = 0,
            .basePipelineHandle  = nullptr,
            .basePipelineIndex   = -1,
        };

        const auto vk_pipeline_cache = either(create_info.cache, vk::monadic::to_vk(), cmonadic::init<VkPipelineCache>(nullptr));

        const auto& device       = owner();
        const auto& device_table = device.device_table();

        m_vk_handle = Try(vk::call_checked<VkPipeline>(device_table.vkCreateGraphicsPipelines,
                                                       device,
                                                       vk_pipeline_cache,
                                                       1,
                                                       &vk_create_info,
                                                       nullptr));
        Return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto PipelineImplementation::do_init(PrivateTag, const LegacyRasterizationCreateInfo& create_info) noexcept
      -> Expected<void> {
        m_type  = Type::RASTER;
        m_state = core::allocate_unsafe<StateVariant>(create_info.state);

        const auto& state = as<RasterPipelineState>(*m_state);

        const auto [binding_descriptions,
                    attribute_descriptions,
                    vertex_input_info,
                    input_assembly,
                    viewports,
                    scissors,
                    viewport_state,
                    rasterizer,
                    multisampling,
                    blend_attachments,
                    color_blending,
                    states,
                    dynamic_state,
                    shaders,
                    depth_stencil] = gpu::do_init(state);

        const auto vk_create_info = VkGraphicsPipelineCreateInfo {
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
            .layout              = vk::to_vk(create_info.layout),
            .renderPass          = create_info.render_pass,
            .subpass             = 0,
            .basePipelineHandle  = nullptr,
            .basePipelineIndex   = -1,
        };

        const auto vk_pipeline_cache = either(create_info.cache, vk::monadic::to_vk(), cmonadic::init<VkPipelineCache>(nullptr));

        const auto& device       = owner();
        const auto& device_table = device.device_table();

        m_vk_handle = Try(vk::call_checked<VkPipeline>(device_table.vkCreateGraphicsPipelines,
                                                       device,
                                                       vk_pipeline_cache,
                                                       1,
                                                       &vk_create_info,
                                                       nullptr));
        Return {};
    }
} // namespace stormkit::gpu
