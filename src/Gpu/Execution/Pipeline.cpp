// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module stormkit.Gpu;

import std;

import stormkit.Core;

import stormkit.Gpu.Vulkan;
import :Core;
import :Execution.Pipeline;
import :Execution.RenderPass;
import :Execution.RasterPipelineState;

namespace stormkit::gpu {
    /////////////////////////////////////
    /////////////////////////////////////
    auto Pipeline::doInitRasterPipeline(const Device&                      device,
                                        const PipelineLayout&              layout,
                                        const RenderPass&                  render_pass,
                                        OptionalRef<const PipelineCache> pipeline_cache) noexcept
        -> VulkanExpected<void> {
        const auto& state = as<RasterPipelineState>(m_state);

        // clang-format off
        const auto binding_descriptions =
            state.vertex_input_state.binding_descriptions 
            | std::views::transform([](auto&& binding_description) static noexcept {
                return vk::VertexInputBindingDescription {
                    .binding   = binding_description.binding,
                    .stride    = binding_description.stride,
                    .inputRate = as<vk::VertexInputRate>(binding_description.input_rate)

                };
            }) 
            | std::ranges::to<std::vector>();

        const auto attribute_descriptions =
            state.vertex_input_state.input_attribute_descriptions 
            | std::views::transform([](auto&& input_attribute_description) static noexcept {
                return vk::VertexInputAttributeDescription {
                    .location = input_attribute_description.location,
                    .binding  = input_attribute_description.binding,
                    .format   = as<vk::Format>(input_attribute_description.format),
                    .offset   = input_attribute_description.offset
                };
            }) 
            | std::ranges::to<std::vector>();

        const auto vertex_input_info = vk::PipelineVertexInputStateCreateInfo {}
                                           .setVertexBindingDescriptions(binding_descriptions)
                                           .setVertexAttributeDescriptions(attribute_descriptions);

        const auto input_assembly =
            vk::PipelineInputAssemblyStateCreateInfo {
                .topology = as<vk::PrimitiveTopology>(state.input_assembly_state.topology),
                .primitiveRestartEnable = state.input_assembly_state.primitive_restart_enable
            };

        const auto viewports = state.viewport_state.viewports 
                               | std::views::transform(core::monadic::as<vk::Viewport>())
                               | std::ranges::to<std::vector>();

        const auto scissors = state.viewport_state.scissors 
                               | std::views::transform(core::monadic::as<vk::Rect2D>())
                               | std::ranges::to<std::vector>();

        const auto viewport_state =
            vk::PipelineViewportStateCreateInfo {}.setViewports(viewports).setScissors(scissors);

        const auto rasterizer =
            vk::PipelineRasterizationStateCreateInfo {
                .depthClampEnable = state.rasterization_state.depth_clamp_enable,
                .rasterizerDiscardEnable = state.rasterization_state.rasterizer_discard_enable,
                .polygonMode = as<vk::PolygonMode>(state.rasterization_state.polygon_mode),
                .cullMode = toVkFlags<vk::CullModeFlagBits>(state.rasterization_state.cull_mode),
                .frontFace = as<vk::FrontFace>(state.rasterization_state.front_face),
                .lineWidth = state.rasterization_state.line_width
            };

        const auto multisampling =
            vk::PipelineMultisampleStateCreateInfo {
                .rasterizationSamples = 
                    as<vk::SampleCountFlagBits>(state.multisample_state.rasterization_samples),
                .sampleShadingEnable = state.multisample_state.sample_shading_enable
            };

        const auto blend_attachments = state.color_blend_state.attachments
             | std::views::transform([](auto&& attachment) static noexcept {
                return vk::PipelineColorBlendAttachmentState {
                    .blendEnable         = attachment.blend_enable,
                    .srcColorBlendFactor = as<vk::BlendFactor>(attachment.src_color_blend_factor),
                    .dstColorBlendFactor = as<vk::BlendFactor>(attachment.dst_color_blend_factor),
                    .colorBlendOp        = as<vk::BlendOp>(attachment.color_blend_operation),
                    .srcAlphaBlendFactor = as<vk::BlendFactor>(attachment.src_alpha_blend_factor),
                    .dstAlphaBlendFactor = as<vk::BlendFactor>(attachment.dst_alpha_blend_factor),
                    .alphaBlendOp        = as<vk::BlendOp>(attachment.alpha_blend_operation),
                    .colorWriteMask      = toVkFlags<vk::ColorComponentFlagBits>(attachment.color_write_mask)
                };
            })
            | std::ranges::to<std::vector>();

        const auto color_blending = vk::PipelineColorBlendStateCreateInfo {
                .logicOpEnable  = state.color_blend_state.logic_operation_enable,
                .logicOp        = as<vk::LogicOp>(state.color_blend_state.logic_operation),
        }
        .setBlendConstants({ state.color_blend_state.blend_constants[0],
                             state.color_blend_state.blend_constants[1],
                             state.color_blend_state.blend_constants[2],
                             state.color_blend_state.blend_constants[3] })
        .setAttachments(blend_attachments);
       

        const auto states = state.dynamic_state.dynamics 
            | std::views::transform(core::monadic::as<vk::DynamicState>())
            | std::ranges::to<std::vector>();

        const auto dynamic_state = vk::PipelineDynamicStateCreateInfo {}.setDynamicStates(states);

        const auto shaders = state.shader_state.shaders
                             | std::views::transform([](auto&& shader) static noexcept {
                                 static constexpr auto NAME = "main";
                                 return vk::PipelineShaderStageCreateInfo {
                                     .stage  = as<vk::ShaderStageFlagBits>(shader->type()),
                                     .module = toVkHandle(shader),
                                     .pName  = NAME
                                 };
                             })
                             | std::ranges::to<std::vector>();

        const auto depth_stencil = vk::PipelineDepthStencilStateCreateInfo {
          .depthTestEnable       = state.depth_stencil_state.depth_test_enable,
          .depthWriteEnable      = state.depth_stencil_state.depth_write_enable,
          .depthCompareOp        = as<vk::CompareOp>(state.depth_stencil_state.depth_compare_op),  
          .depthBoundsTestEnable = state.depth_stencil_state.depth_bounds_test_enable,
          .stencilTestEnable     = false, 
          .minDepthBounds        = state.depth_stencil_state.min_depth_bounds, 
          .maxDepthBounds        = state.depth_stencil_state.max_depth_bounds
        };

        const auto create_info = vk::GraphicsPipelineCreateInfo {
         .pVertexInputState   = &vertex_input_info,
         .pInputAssemblyState = &input_assembly,
         .pViewportState      = &viewport_state,
         .pRasterizationState = &rasterizer,
         .pMultisampleState   = &multisampling,
         .pDepthStencilState  = &depth_stencil,
         .pColorBlendState    = &color_blending,
         .pDynamicState       = &dynamic_state,
         .layout              = toVkHandle(layout),
         .renderPass          = toVkHandle(render_pass),
         .subpass             = 0,
         .basePipelineHandle  = nullptr,
         .basePipelineIndex   = -1
        }
        .setStages(shaders);
        // clang-format on

        using PipelineCacheOpt = vk::Optional<const vk::raii::PipelineCache>;

        // const auto vk_pipeline_cache = [&pipeline_cache] -> const vk::raii::PipelineCache& {
        //     if (pipeline_cache) return toRaiiVkHandle(*pipeline_cache);
        //     return nullptr;
        // };

        const auto vk_pipeline_cache =
            core::either(pipeline_cache,
                         core::monadic::map(monadic::toRaiiVkHandle(),
                                            core::monadic::init<PipelineCacheOpt>()),
                         core::monadic::init<PipelineCacheOpt>(nullptr));

        return toRaiiVkHandle(device)
            .createGraphicsPipelines(vk_pipeline_cache, create_info)
            .transform([this](auto&& pipelines) noexcept {
                m_vk_pipeline = std::move(pipelines.front());
            });
    }
} // namespace stormkit::gpu
