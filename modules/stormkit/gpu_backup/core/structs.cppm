// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#include <stormkit/gpu/vulkan.hpp>

export module stormkit.gpu.core:structs;

import std;

import stormkit.core;

import :vulkan.enums;

export {
    namespace stormkit::gpu {
        struct surface_format {
            pixel_format format;
            color_space  color_space;
        };

        struct memory_barrier {
            access_flag src;
            access_flag dst;
        };

        struct render_capabilities {
            struct {
                bool robust_buffer_access;
                bool full_draw_index_uint32;
                bool image_cube_array;
                bool independent_blend;
                bool geometry_shader;
                bool tessellation_shader;
                bool sampler_rate_shading;
                bool dual_src_blend;
                bool logic_op;
                bool multi_draw_indirect;
                bool draw_indirect_first_instance;
                bool depth_clamp;
                bool depth_bias_clamp;
                bool fill_Mode_non_solid;
                bool depth_bounds;
                bool wide_lines;
                bool large_points;
                bool alpha_to_one;
                bool multi_viewport;
                bool sampler_anisotropy;
                bool texture_compression_etc2;
                bool texture_compression_astc_ldr;
                bool texture_compression_bc;
                bool occlusion_query_precise;
                bool pipeline_statistics_query;
                bool vertex_pipeline_stores_and_atomics;
                bool fragment_stores_and_atomics;
                bool shader_tessellation_and_geometry_point_size;
                bool shader_image_gather_extended;
                bool shader_storage_image_extended_formats;
                bool shader_storage_image_multisample;
                bool shader_storage_image_read_without_format;
                bool shader_storage_image_write_without_format;
                bool shader_uniform_buffer_array_dynamic_indexing;
                bool shader_sampled_image_array_dynamic_indexing;
                bool shader_storage_buffer_array_dynamic_indexing;
                bool shader_storage_image_array_dynamic_indexing;
                bool shader_clip_distance;
                bool shader_cull_distance;
                bool shader_float_64;
                bool shader_int_64;
                bool shader_int_16;
                bool shader_resource_residency;
                bool shader_resource_min_lod;
                bool sparse_binding;
                bool sparse_residency_buffer;
                bool sparse_residency_image_2D;
                bool sparse_residency_image_3D;
                bool sparse_residency_2_samples;
                bool sparse_residency_4_samples;
                bool sparse_residency_6_samples;
                bool sparse_residency_8_samples;
                bool sparse_residency_16_samples;
                bool sparse_residency_aliased;
                bool variable_multisample_rate;
                bool inherited_queries;
            } features;

            struct {
                u32                  max_image_dimension_1D;
                u32                  max_image_dimension_2D;
                u32                  max_image_dimension_3D;
                u32                  max_image_dimension_cube;
                u32                  max_image_array_layers;
                u32                  max_texel_buffer_elements;
                u32                  max_uniform_buffer_range;
                std::optional<u32>   max_storage_buffer_range;
                u32                  max_push_constants_size;
                std::optional<u32>   max_memory_allocation_count;
                std::optional<u32>   max_sampler_allocation_count;
                std::optional<u64>   buffer_image_granularity;
                std::optional<u64>   sparse_address_space_size;
                std::optional<u32>   max_bound_descriptor_sets;
                u32                  max_per_stage_descriptor_samplers;
                u32                  max_per_stage_descriptor_uniform_buffers;
                u32                  max_per_stage_descriptor_storage_buffers;
                u32                  max_per_stage_descriptor_sampled_images;
                u32                  max_per_stage_descriptor_storage_images;
                std::optional<u32>   max_per_stage_descriptor_input_attachments;
                std::optional<u32>   max_per_stage_resources;
                u32                  max_descriptor_set_samplers;
                u32                  max_descriptor_set_uniform_buffers;
                u32                  max_descriptor_set_uniform_buffers_dynamic;
                u32                  max_descriptor_set_storage_buffers;
                u32                  max_descriptor_set_storage_buffers_dynamic;
                u32                  max_descriptor_set_sampled_images;
                u32                  max_descriptor_set_storage_images;
                std::optional<u32>   max_descriptor_set_input_attachments;
                u32                  max_vertex_input_attributes;
                u32                  max_vertex_input_bindings;
                u32                  max_vertex_input_attribute_offset;
                std::optional<u32>   max_vertex_input_binding_stride;
                u32                  max_vertex_output_components;
                u32                  max_tessellation_generation_level;
                u32                  max_tessellation_patch_size;
                u32                  max_tessellation_control_per_vertex_input_components;
                u32                  max_tessellation_control_per_vertex_output_components;
                u32                  max_tessellation_control_per_patch_output_components;
                u32                  max_tessellation_control_total_output_components;
                u32                  max_tessellation_evaluation_input_components;
                u32                  max_tessellation_evaluation_output_components;
                u32                  max_geometry_shader_invocations;
                u32                  max_geometry_input_components;
                u32                  max_geometry_output_components;
                u32                  max_geometry_output_vertices;
                u32                  max_geometry_total_output_components;
                u32                  max_fragment_input_components;
                u32                  max_fragment_output_attachments;
                u32                  max_fragment_dual_src_attachments;
                u32                  max_fragment_combined_output_resources;
                u32                  max_compute_shared_memory_size;
                array<u32, 3>        max_compute_work_group_count;
                u32                  max_compute_work_group_invocations;
                array<u32, 3>        max_compute_work_group_size;
                std::optional<u32>   sub_pixel_precision_bits;
                std::optional<u32>   sub_texel_precision_bits;
                std::optional<u32>   mipmap_precision_bits;
                u32                  max_draw_indexed_index_value;
                std::optional<u32>   max_draw_indirect_count;
                f32                  max_sampler_lod_bias;
                f32                  max_sampler_anisotropy;
                u32                  max_viewports;
                array<u32, 2>        max_viewport_dimensions;
                array<f32, 2>        viewport_bounds_range;
                std::optional<u32>   viewport_sub_pixel_bits;
                std::optional<usize> min_memory_map_alignment;
                std::optional<u64>   min_texel_buffer_offset_alignment;
                u64                  min_uniform_buffer_offset_alignment;
                u64                  min_storage_buffer_offset_alignment;
                i32                  min_texel_offset;
                u32                  max_texel_offset;
                i32                  min_texel_gather_offset;
                u32                  max_texel_gather_offset;
                f32                  min_interpolation_offset;
                f32                  max_interpolation_offset;
                std::optional<u32>   sub_pixel_interpolation_offset_bits;
                u32                  max_framebuffer_width;
                u32                  max_framebuffer_height;
                u32                  max_framebuffer_layers;
                sample_count_flag      framebuffer_color_sample_counts;
                sample_count_flag      framebuffer_depth_sample_counts;
                sample_count_flag      framebuffer_stencil_sample_counts;
                sample_count_flag      framebuffer_no_attachments_sample_counts;
                u32                  max_color_attachments;
                sample_count_flag      sampled_image_color_sample_counts;
                sample_count_flag      sampled_image_integer_sample_counts;
                sample_count_flag      sampled_image_depth_sample_counts;
                sample_count_flag      sampled_image_stencil_sample_counts;
                sample_count_flag      storage_image_sample_counts;
                u32                  max_sample_mask_words;
                bool                 timestamp_compute_and_engine;
                f32                  timestamp_period;
                u32                  max_clip_distances;
                u32                  max_cull_distances;
                u32                  max_combined_clip_and_cull_distances;
                u32                  discrete_queue_priorities;
                array<f32, 2>        point_size_range;
                array<f32, 2>        line_width_range;
                f32                  point_size_granularity;
                f32                  line_width_granularity;
                bool                 strict_lines;
                bool                 standard_sample_locations;
                std::optional<u64>   optimal_buffer_copy_offset_alignment;
                std::optional<u64>   optimal_buffer_copy_row_pitch_alignment;
                u64                  non_coherent_atom_size;
            } limits;
        };

        struct format_properties {
            format_feature_flag linear_tiling_features;
            format_feature_flag optimal_tiling_features;
            format_feature_flag buffer_features;
        };

        struct image_subresource_range {
            image_aspect_flag aspect_mask = image_aspect_flag::COLOR;

            u32 base_mip_level   = 0u;
            u32 level_count      = 1u;
            u32 base_array_layer = 0u;
            u32 layer_count      = 1u;
        };

        struct image_subresource_layers {
            image_aspect_flag aspect_mask = image_aspect_flag::COLOR;

            u32 mip_level        = 0u;
            u32 base_array_layer = 0u;
            u32 layer_count      = 1u;
        };

        struct viewport {
            math::fvec2        position;
            math::extent2<f32> extent;
            math::fvec2        depth;
        };

        struct scissor {
            math::ivec2    offset;
            math::uextent2 extent;
        };

        struct clear_color {
            fcolor_rgba color = stormkit::colors::silver<f32>;

            constexpr auto operator==(const clear_color& other) const noexcept -> bool;
        };

        struct clear_depth {
            f32            depth = 1.f;
            constexpr auto operator==(const clear_depth& other) const noexcept -> bool;
        };

        struct clear_stencil {
            u32            stencil = 0u;
            constexpr auto operator==(const clear_stencil& other) const noexcept -> bool;
        };

        struct clear_depth_stencil: clear_depth, clear_stencil {
            constexpr auto operator==(const clear_depth_stencil& other) const noexcept -> bool;
        };

        using clear_value = std::variant<clear_color, clear_depth_stencil>;

        struct buffer_image_copy {
            u32 buffer_offset;
            u32 buffer_row_length;
            u32 buffer_image_height;

            image_subresource_layers subresource_layers;

            math::ivec3    offset;
            math::uextent3 extent;
        };

        struct blit_region {
            image_subresource_layers src;
            image_subresource_layers dst;

            struct {
                math::ivec3    position;
                math::iextent3 extent;
            } src_offset;

            struct {
                math::ivec3    position;
                math::iextent3 extent;
            } dst_offset;
        };

        struct push_constant_range {
            shader_stage_flag stages;
            u32             offset;
            usize           size;
        };

        struct physical_device_info {
            u64    device_id;
            string device_name;
            u64    vendor_id;
            string vendor_name;

            u32 api_major_version;
            u32 api_minor_version;
            u32 api_patch_version;

            u32 driver_major_version;
            u32 driver_minor_version;
            u32 driver_patch_version;

            array<u8, 16> pipeline_cache_uuid;

            physical_device_type type;
        };

        struct queue_family {
            queue_flag flags;
            u32       count;
        };

        using clear_value = std::variant<clear_color, clear_depth_stencil>;
        using spirv_id    = u32;

        template<typename T>
        using expected = std::expected<T, Result>;

        [[nodiscard]]
        constexpr auto compute_mip_level(const math::uextent2& extent) noexcept -> u32;
        [[nodiscard]]
        constexpr auto compute_uniform_buffer_offset_align(usize size, const render_capabilities& capabilities) noexcept -> usize;

        [[nodiscard]]
        auto to_string(const physical_device_info& data) noexcept;

        template<core::meta::hash Ret = hash32>
        constexpr auto hasher(const viewport& value) noexcept -> Ret;

        template<core::meta::hash Ret = hash32>
        constexpr auto hasher(const scissor& value) noexcept -> Ret;

        template<core::meta::hash Ret = hash32>
        constexpr auto hasher(const clear_color& value) noexcept -> Ret;

        template<core::meta::hash Ret = hash32>
        constexpr auto hasher(const clear_depth_stencil& value) noexcept -> Ret;
    } // namespace stormkit::gpu
}

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::gpu {
    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto clear_color::operator==(const clear_color& other) const noexcept -> bool {
        return color == other.color;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto clear_depth::operator==(const clear_depth& other) const noexcept -> bool {
        return depth == other.depth;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto clear_stencil::operator==(const clear_stencil& other) const noexcept -> bool {
        return stencil == other.stencil;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto clear_depth_stencil::operator==(const clear_depth_stencil& other) const noexcept -> bool {
        return depth == other.depth and stencil == other.stencil;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto compute_mip_level(const math::uextent2& extent) noexcept -> u32 {
        const auto as_float = extent.to<f32>();
        return as<u32>(math::floor(math::log2(math::max(as_float.width, as_float.height)))) + 1;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto compute_uniform_buffer_offset_align(usize size, const render_capabilities& capabilities) noexcept -> usize {
        const auto min_ubo_align = capabilities.limits.min_uniform_buffer_offset_alignment;
        if (min_ubo_align > 0) size = (size + min_ubo_align - 1) & ~(min_ubo_align - 1);

        return size;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto to_string(const physical_device_info& data) noexcept {
        return std::format("[physical_device_info:\n"
                           "   .device_id      = {:#06x},\n"
                           "   .device_name    = {},\n"
                           "   .vendor_id      = {:#06x},\n"
                           "   .vendor_name    = {},\n"
                           "   .api_version    = {}.{}.{},\n"
                           "   .driver_version = {}.{}.{},\n"
                           "   .type           = {}]",
                           data.device_id,
                           data.device_name,
                           data.vendor_id,
                           data.vendor_name,
                           data.api_major_version,
                           data.api_minor_version,
                           data.api_patch_version,
                           data.driver_major_version,
                           data.driver_minor_version,
                           data.driver_patch_version,
                           data.type);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<core::meta::hash Ret>
    constexpr auto hasher(const viewport& value) noexcept -> Ret {
        return hash(value.position, value.extent, value.depth);
    }

    /////////////////////////////////////
    /////////////////////////////////////

    template<core::meta::hash Ret>
    constexpr auto hasher(const scissor& value) noexcept -> Ret {
        return hash(value.offset, value.extent);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<core::meta::hash Ret>
    constexpr auto hasher(const clear_color& value) noexcept -> Ret {
        return hash(value.color);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<core::meta::hash Ret>
    constexpr auto hasher(const clear_depth_stencil& value) noexcept -> Ret {
        return hash(value.depth, value.stencil);
    }
} // namespace stormkit::gpu
