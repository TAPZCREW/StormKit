// Copyright (C) 2023 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>

#include <volk.h>

export module stormkit.gpu.execution:raster_pipeline;

import std;

import stormkit.core;

import stormkit.gpu.core;
import stormkit.gpu.resource;

import :descriptors;

export namespace stormkit::gpu {
    struct VertexBindingDescription {
        u32             binding;
        u32             stride;
        VertexInputRate input_rate = VertexInputRate::VERTEX;
    };

    struct VertexInputAttributeDescription {
        u32         location;
        u32         binding;
        PixelFormat format;
        u32         offset;
    };

    struct RasterPipelineVertexInputState {
        dyn_array<VertexBindingDescription>        binding_descriptions         = {};
        dyn_array<VertexInputAttributeDescription> input_attribute_descriptions = {};
    };

    struct RasterPipelineInputAssemblyState {
        PrimitiveTopology topology                 = PrimitiveTopology::TRIANGLE_LIST;
        bool              primitive_restart_enable = false;
    };

    struct RasterPipelineViewportState {
        dyn_array<Viewport> viewports = {};
        dyn_array<Scissor>  scissors  = {};
    };

    struct RasterPipelineRasterizationState {
        bool         depth_clamp_enable         = false;
        bool         rasterizer_discard_enable  = false;
        PolygonMode  polygon_mode               = PolygonMode::FILL;
        f32          line_width                 = 1;
        CullModeFlag cull_mode                  = CullModeFlag::BACK;
        FrontFace    front_face                 = FrontFace::CLOCKWISE;
        bool         depth_bias_enable          = false;
        f32          depth_bias_constant_factor = 0.f;
        f32          depth_bias_clamp           = 0.f;
        f32          depth_bias_slope_factor    = 0.f;
    };

    struct RasterPipelineMultiSampleState {
        bool            sample_shading_enable = false;
        SampleCountFlag rasterization_samples = SampleCountFlag::C1;
        f32             min_sample_shading    = 0.f;
    };

    struct RasterPipelineColorBlendAttachmentState {
        ColorComponentFlag color_write_mask = ColorComponentFlag::RGBA;
        bool               blend_enable     = false;

        BlendFactor    src_color_blend_factor = BlendFactor::ONE;
        BlendFactor    dst_color_blend_factor = BlendFactor::ZERO;
        BlendOperation color_blend_operation  = BlendOperation::ADD;

        BlendFactor    src_alpha_blend_factor = BlendFactor::ONE;
        BlendFactor    dst_alpha_blend_factor = BlendFactor::ZERO;
        BlendOperation alpha_blend_operation  = BlendOperation::SUBTRACT;
    };

    struct RasterPipelineColorBlendState {
        bool                                               logic_operation_enable = false;
        LogicOperation                                     logic_operation        = LogicOperation::COPY;
        dyn_array<RasterPipelineColorBlendAttachmentState> attachments;
        array<f32, 4>                                      blend_constants = { 0.f, 0.f, 0.f, 0.f };
    };

    using RasterPipelineDynamicState = dyn_array<DynamicState>;
    using RasterPipelineShaderState  = dyn_array<view::Shader>;

    struct RasterPipelineDepthStencilState {
        bool depth_test_enable  = false;
        bool depth_write_enable = false;

        CompareOperation depth_compare_op = CompareOperation::LESS;

        bool depth_bounds_test_enable = false;

        f32 min_depth_bounds = 0.f;
        f32 max_depth_bounds = 1.f;
    };

    struct RasterPipelineRenderingInfo {
        u32                        view_mask = 0u;
        dyn_array<PixelFormat>     color_attachment_formats;
        std::optional<PixelFormat> depth_attachment_format   = std::nullopt;
        std::optional<PixelFormat> stencil_attachment_format = std::nullopt;
    };

    struct RasterPipelineLayout {
        dyn_array<view::DescriptorSetLayout> descriptor_set_layouts = {};
        dyn_array<PushConstantRange>         push_constant_ranges   = {};
    };

    struct RasterPipelineState {
        RasterPipelineInputAssemblyState input_assembly_state = {};
        RasterPipelineViewportState      viewport_state       = {};
        RasterPipelineRasterizationState rasterization_state  = {};
        RasterPipelineMultiSampleState   multisample_state    = {};
        RasterPipelineColorBlendState    color_blend_state    = {};
        RasterPipelineDynamicState       dynamic_state        = {};
        RasterPipelineShaderState        shader_state         = {};
        RasterPipelineVertexInputState   vertex_input_state   = {};
        RasterPipelineDepthStencilState  depth_stencil_state  = {};
    };
} // namespace stormkit::gpu
