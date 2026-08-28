// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/try_expected.hpp>

#include <stormkit/gpu/api.hpp>
#include <stormkit/gpu/vulkan.hpp>

module stormkit.gpu.core;

import std;

import stormkit.core;

import :vulkan;

using namespace std::literals;

namespace stdr = std::ranges;
namespace stdv = std::views;

namespace cm = stormkit::core::meta;

namespace stormkit::gpu {
    namespace {
        constexpr auto RAYTRACING_EXTENSIONS = array {
            "VK_KHR_ray_tracing_pipeline"sv,     "VK_KHR_acceleration_structure"sv, "VK_KHR_buffer_device_address"sv,
            "VK_KHR_deferred_host_operations"sv, "VK_EXT_descriptor_indexing"sv,    "VK_KHR_spirv_1_4"sv,
            "VK_KHR_shader_float_controls"sv
        };

        /////////////////////////////////////
        /////////////////////////////////////
        auto vendor_name_by_id(u64 ID) -> string_view {
            switch (ID) {
                case 0x1002: return "AMD";
                case 0x1010: return "ImgTex";
                case 0x10DE: return "NVidia";
                case 0x8086: return "Intel";
                case 0x5143: return "Qualcomm";
                case 0x13B5: return "ARM";
            }

            return "UNKNOWN";
        }

        auto info(const PhysicalDeviceImplementation& physical_device) noexcept -> PhysicalDeviceInfo {
            const auto& handle = physical_device.native_handle();

            auto device_info = PhysicalDeviceInfo {};

            const auto properties = vk::call<VkPhysicalDeviceProperties>(vkGetPhysicalDeviceProperties, handle);
            const auto vendor_id  = properties.vendorID;

            device_info.device_id = properties.deviceID;

            const auto device_name_size = std::char_traits<char>::length(properties.deviceName);

            device_info.device_name.resize(device_name_size);
            stdr::copy(string_view { properties.deviceName, device_name_size }, std::begin(device_info.device_name));

            device_info.vendor_id         = vendor_id;
            device_info.vendor_name       = vendor_name_by_id(vendor_id);
            device_info.api_major_version = vk::version_major(properties.apiVersion);
            device_info.api_minor_version = vk::version_minor(properties.apiVersion);
            device_info.api_patch_version = vk::version_patch(properties.apiVersion);

            device_info.driver_major_version = vk::version_major(properties.driverVersion);
            device_info.driver_minor_version = vk::version_minor(properties.driverVersion);
            device_info.driver_patch_version = vk::version_patch(properties.driverVersion);
            stdr::copy(properties.pipelineCacheUUID, stdr::begin(device_info.pipeline_cache_uuid));

            device_info.type = vk::from_vk<PhysicalDeviceType>(properties.deviceType);

            return device_info;
        }

        auto capabilities(const PhysicalDeviceImplementation& physical_device) noexcept -> RenderCapabilities {
            const auto& handle = physical_device.native_handle();

            const auto properties = vk::call<VkPhysicalDeviceProperties>(vkGetPhysicalDeviceProperties, handle);
            // TODO port to vkGetPhysicalDeviceFeatures2
            const auto features = vk::call<VkPhysicalDeviceFeatures>(vkGetPhysicalDeviceFeatures, handle);

            auto capabilities                                            = RenderCapabilities {};
            capabilities.limits.max_image_dimension_1D                   = properties.limits.maxImageDimension1D;
            capabilities.limits.max_image_dimension_2D                   = properties.limits.maxImageDimension2D;
            capabilities.limits.max_image_dimension_3D                   = properties.limits.maxImageDimension3D;
            capabilities.limits.max_image_dimension_cube                 = properties.limits.maxImageDimensionCube;
            capabilities.limits.max_image_array_layers                   = properties.limits.maxImageArrayLayers;
            capabilities.limits.max_texel_buffer_elements                = properties.limits.maxTexelBufferElements;
            capabilities.limits.max_uniform_buffer_range                 = properties.limits.maxUniformBufferRange;
            capabilities.limits.max_storage_buffer_range                 = properties.limits.maxStorageBufferRange;
            capabilities.limits.max_push_constants_size                  = properties.limits.maxPushConstantsSize;
            capabilities.limits.max_memory_allocation_count              = properties.limits.maxMemoryAllocationCount;
            capabilities.limits.max_sampler_allocation_count             = properties.limits.maxSamplerAllocationCount;
            capabilities.limits.buffer_image_granularity                 = properties.limits.bufferImageGranularity;
            capabilities.limits.sparse_address_space_size                = properties.limits.sparseAddressSpaceSize;
            capabilities.limits.max_bound_descriptor_sets                = properties.limits.maxBoundDescriptorSets;
            capabilities.limits.max_per_stage_descriptor_samplers        = properties.limits.maxPerStageDescriptorSamplers;
            capabilities.limits.max_per_stage_descriptor_uniform_buffers = properties.limits.maxPerStageDescriptorUniformBuffers;
            capabilities.limits.max_per_stage_descriptor_storage_buffers = properties.limits.maxPerStageDescriptorStorageBuffers;
            capabilities.limits.max_per_stage_descriptor_sampled_images  = properties.limits.maxPerStageDescriptorSampledImages;
            capabilities.limits.max_per_stage_descriptor_storage_images  = properties.limits.maxPerStageDescriptorStorageImages;
            capabilities.limits.max_per_stage_descriptor_input_attachments
              = properties.limits.maxPerStageDescriptorInputAttachments;
            capabilities.limits.max_per_stage_resources            = properties.limits.maxPerStageResources;
            capabilities.limits.max_descriptor_set_samplers        = properties.limits.maxDescriptorSetSamplers;
            capabilities.limits.max_descriptor_set_uniform_buffers = properties.limits.maxDescriptorSetUniformBuffers;
            capabilities.limits.max_descriptor_set_uniform_buffers_dynamic
              = properties.limits.maxDescriptorSetUniformBuffersDynamic;
            capabilities.limits.max_descriptor_set_storage_buffers = properties.limits.maxDescriptorSetStorageBuffers;
            capabilities.limits.max_descriptor_set_storage_buffers_dynamic
              = properties.limits.maxDescriptorSetStorageBuffersDynamic;
            capabilities.limits.max_descriptor_set_sampled_images    = properties.limits.maxDescriptorSetSampledImages;
            capabilities.limits.max_descriptor_set_storage_images    = properties.limits.maxDescriptorSetStorageImages;
            capabilities.limits.max_descriptor_set_input_attachments = properties.limits.maxDescriptorSetInputAttachments;
            capabilities.limits.max_vertex_input_attributes          = properties.limits.maxVertexInputAttributes;
            capabilities.limits.max_vertex_input_bindings            = properties.limits.maxVertexInputBindings;
            capabilities.limits.max_vertex_input_attribute_offset    = properties.limits.maxVertexInputAttributeOffset;
            capabilities.limits.max_vertex_input_binding_stride      = properties.limits.maxVertexInputBindingStride;
            capabilities.limits.max_vertex_output_components         = properties.limits.maxVertexOutputComponents;
            capabilities.limits.max_tessellation_generation_level    = properties.limits.maxTessellationGenerationLevel;
            capabilities.limits.max_tessellation_patch_size          = properties.limits.maxTessellationPatchSize;
            capabilities.limits.max_tessellation_control_per_vertex_input_components
              = properties.limits.maxTessellationControlPerVertexInputComponents;
            capabilities.limits.max_tessellation_control_per_vertex_output_components
              = properties.limits.maxTessellationControlPerVertexOutputComponents;
            capabilities.limits.max_tessellation_control_per_patch_output_components
              = properties.limits.maxTessellationControlPerPatchOutputComponents;
            capabilities.limits.max_tessellation_control_total_output_components
              = properties.limits.maxTessellationControlTotalOutputComponents;
            capabilities.limits.max_tessellation_evaluation_input_components
              = properties.limits.maxTessellationEvaluationInputComponents;
            capabilities.limits.max_tessellation_evaluation_output_components
              = properties.limits.maxTessellationEvaluationOutputComponents;
            capabilities.limits.max_geometry_shader_invocations        = properties.limits.maxGeometryShaderInvocations;
            capabilities.limits.max_geometry_input_components          = properties.limits.maxGeometryInputComponents;
            capabilities.limits.max_geometry_output_components         = properties.limits.maxGeometryOutputComponents;
            capabilities.limits.max_geometry_output_vertices           = properties.limits.maxGeometryOutputVertices;
            capabilities.limits.max_geometry_total_output_components   = properties.limits.maxGeometryTotalOutputComponents;
            capabilities.limits.max_fragment_input_components          = properties.limits.maxFragmentInputComponents;
            capabilities.limits.max_fragment_output_attachments        = properties.limits.maxFragmentOutputAttachments;
            capabilities.limits.max_fragment_dual_src_attachments      = properties.limits.maxFragmentDualSrcAttachments;
            capabilities.limits.max_fragment_combined_output_resources = properties.limits.maxFragmentCombinedOutputResources;
            capabilities.limits.max_compute_shared_memory_size         = properties.limits.maxComputeSharedMemorySize;
            stdr::copy(properties.limits.maxComputeWorkGroupCount, stdr::begin(capabilities.limits.max_compute_work_group_count));
            capabilities.limits.max_compute_work_group_invocations = properties.limits.maxComputeWorkGroupInvocations;
            stdr::copy(properties.limits.maxComputeWorkGroupSize, stdr::begin(capabilities.limits.max_compute_work_group_size));
            capabilities.limits.sub_pixel_precision_bits     = properties.limits.subPixelPrecisionBits;
            capabilities.limits.sub_texel_precision_bits     = properties.limits.subTexelPrecisionBits;
            capabilities.limits.mipmap_precision_bits        = properties.limits.mipmapPrecisionBits;
            capabilities.limits.max_draw_indexed_index_value = properties.limits.maxDrawIndexedIndexValue;
            capabilities.limits.max_draw_indirect_count      = properties.limits.maxDrawIndirectCount;
            capabilities.limits.max_sampler_lod_bias         = properties.limits.maxSamplerLodBias;
            capabilities.limits.max_sampler_anisotropy       = properties.limits.maxSamplerAnisotropy;
            capabilities.limits.max_viewports                = properties.limits.maxViewports;
            stdr::copy(properties.limits.maxViewportDimensions, stdr::begin(capabilities.limits.max_viewport_dimensions));
            stdr::copy(properties.limits.viewportBoundsRange, stdr::begin(capabilities.limits.viewport_bounds_range));
            capabilities.limits.viewport_sub_pixel_bits             = properties.limits.viewportSubPixelBits;
            capabilities.limits.min_memory_map_alignment            = properties.limits.minMemoryMapAlignment;
            capabilities.limits.min_texel_buffer_offset_alignment   = properties.limits.minTexelBufferOffsetAlignment;
            capabilities.limits.min_uniform_buffer_offset_alignment = properties.limits.minUniformBufferOffsetAlignment;
            capabilities.limits.min_storage_buffer_offset_alignment = properties.limits.minStorageBufferOffsetAlignment;
            capabilities.limits.min_texel_offset                    = properties.limits.minTexelOffset;
            capabilities.limits.max_texel_offset                    = properties.limits.maxTexelOffset;
            capabilities.limits.min_texel_gather_offset             = properties.limits.minTexelGatherOffset;
            capabilities.limits.max_texel_gather_offset             = properties.limits.maxTexelGatherOffset;
            capabilities.limits.min_interpolation_offset            = properties.limits.minInterpolationOffset;
            capabilities.limits.max_interpolation_offset            = properties.limits.maxInterpolationOffset;
            capabilities.limits.sub_pixel_interpolation_offset_bits = properties.limits.subPixelInterpolationOffsetBits;
            capabilities.limits.max_framebuffer_width               = properties.limits.maxFramebufferWidth;
            capabilities.limits.max_framebuffer_height              = properties.limits.maxFramebufferHeight;
            capabilities.limits.max_framebuffer_layers              = properties.limits.maxFramebufferLayers;
            capabilities.limits.framebuffer_color_sample_counts     = narrow<
              SampleCountFlag>(properties.limits.framebufferColorSampleCounts);
            capabilities.limits.framebuffer_depth_sample_counts = narrow<
              SampleCountFlag>(properties.limits.framebufferDepthSampleCounts);
            capabilities.limits.framebuffer_stencil_sample_counts = narrow<
              SampleCountFlag>(properties.limits.framebufferStencilSampleCounts);
            capabilities.limits.framebuffer_no_attachments_sample_counts = narrow<
              SampleCountFlag>(properties.limits.framebufferNoAttachmentsSampleCounts);
            capabilities.limits.max_color_attachments             = properties.limits.maxColorAttachments;
            capabilities.limits.sampled_image_color_sample_counts = narrow<
              SampleCountFlag>(properties.limits.sampledImageColorSampleCounts);
            capabilities.limits.sampled_image_integer_sample_counts = narrow<
              SampleCountFlag>(properties.limits.sampledImageIntegerSampleCounts);
            capabilities.limits.sampled_image_depth_sample_counts = narrow<
              SampleCountFlag>(properties.limits.sampledImageDepthSampleCounts);
            capabilities.limits.sampled_image_stencil_sample_counts = narrow<
              SampleCountFlag>(properties.limits.sampledImageStencilSampleCounts);
            capabilities.limits.storage_image_sample_counts = narrow<SampleCountFlag>(properties.limits.storageImageSampleCounts);
            capabilities.limits.max_sample_mask_words       = properties.limits.maxSampleMaskWords;
            capabilities.limits.timestamp_compute_and_engine         = properties.limits.timestampComputeAndGraphics;
            capabilities.limits.timestamp_period                     = properties.limits.timestampPeriod;
            capabilities.limits.max_clip_distances                   = properties.limits.maxClipDistances;
            capabilities.limits.max_cull_distances                   = properties.limits.maxCullDistances;
            capabilities.limits.max_combined_clip_and_cull_distances = properties.limits.maxCombinedClipAndCullDistances;
            capabilities.limits.discrete_queue_priorities            = properties.limits.discreteQueuePriorities;
            stdr::copy(properties.limits.pointSizeRange, stdr::begin(capabilities.limits.point_size_range));
            stdr::copy(properties.limits.lineWidthRange, stdr::begin(capabilities.limits.line_width_range));
            capabilities.limits.point_size_granularity                  = properties.limits.pointSizeGranularity;
            capabilities.limits.line_width_granularity                  = properties.limits.lineWidthGranularity;
            capabilities.limits.strict_lines                            = properties.limits.strictLines;
            capabilities.limits.standard_sample_locations               = properties.limits.standardSampleLocations;
            capabilities.limits.optimal_buffer_copy_offset_alignment    = properties.limits.optimalBufferCopyOffsetAlignment;
            capabilities.limits.optimal_buffer_copy_row_pitch_alignment = properties.limits.optimalBufferCopyRowPitchAlignment;
            capabilities.limits.non_coherent_atom_size                  = properties.limits.nonCoherentAtomSize;

            capabilities.features.robust_buffer_access                         = features.robustBufferAccess;
            capabilities.features.full_draw_index_uint32                       = features.fullDrawIndexUint32;
            capabilities.features.image_cube_array                             = features.imageCubeArray;
            capabilities.features.independent_blend                            = features.independentBlend;
            capabilities.features.geometry_shader                              = features.geometryShader;
            capabilities.features.tessellation_shader                          = features.tessellationShader;
            capabilities.features.sampler_rate_shading                         = features.sampleRateShading;
            capabilities.features.dual_src_blend                               = features.dualSrcBlend;
            capabilities.features.logic_op                                     = features.logicOp;
            capabilities.features.multi_draw_indirect                          = features.multiDrawIndirect;
            capabilities.features.draw_indirect_first_instance                 = features.drawIndirectFirstInstance;
            capabilities.features.depth_clamp                                  = features.depthClamp;
            capabilities.features.depth_bias_clamp                             = features.depthBiasClamp;
            capabilities.features.fill_Mode_non_solid                          = features.fillModeNonSolid;
            capabilities.features.depth_bounds                                 = features.depthBounds;
            capabilities.features.wide_lines                                   = features.wideLines;
            capabilities.features.large_points                                 = features.largePoints;
            capabilities.features.alpha_to_one                                 = features.alphaToOne;
            capabilities.features.multi_viewport                               = features.multiViewport;
            capabilities.features.sampler_anisotropy                           = features.samplerAnisotropy;
            capabilities.features.texture_compression_etc2                     = features.textureCompressionETC2;
            capabilities.features.texture_compression_astc_ldr                 = features.textureCompressionASTC_LDR;
            capabilities.features.texture_compression_bc                       = features.textureCompressionBC;
            capabilities.features.occlusion_query_precise                      = features.occlusionQueryPrecise;
            capabilities.features.pipeline_statistics_query                    = features.pipelineStatisticsQuery;
            capabilities.features.vertex_pipeline_stores_and_atomics           = features.vertexPipelineStoresAndAtomics;
            capabilities.features.fragment_stores_and_atomics                  = features.fragmentStoresAndAtomics;
            capabilities.features.shader_tessellation_and_geometry_point_size  = features.shaderTessellationAndGeometryPointSize;
            capabilities.features.shader_image_gather_extended                 = features.shaderImageGatherExtended;
            capabilities.features.shader_storage_image_extended_formats        = features.shaderStorageImageExtendedFormats;
            capabilities.features.shader_storage_image_multisample             = features.shaderStorageImageMultisample;
            capabilities.features.shader_storage_image_read_without_format     = features.shaderStorageImageReadWithoutFormat;
            capabilities.features.shader_storage_image_write_without_format    = features.shaderStorageImageWriteWithoutFormat;
            capabilities.features.shader_uniform_buffer_array_dynamic_indexing = features.shaderUniformBufferArrayDynamicIndexing;
            capabilities.features.shader_sampled_image_array_dynamic_indexing  = features.shaderSampledImageArrayDynamicIndexing;
            capabilities.features.shader_storage_buffer_array_dynamic_indexing = features.shaderStorageBufferArrayDynamicIndexing;
            capabilities.features.shader_storage_image_array_dynamic_indexing  = features.shaderStorageImageArrayDynamicIndexing;
            capabilities.features.shader_clip_distance                         = features.shaderClipDistance;
            capabilities.features.shader_cull_distance                         = features.shaderCullDistance;
            capabilities.features.shader_float_64                              = features.shaderFloat64;
            capabilities.features.shader_int_64                                = features.shaderInt64;
            capabilities.features.shader_int_16                                = features.shaderInt16;
            capabilities.features.shader_resource_residency                    = features.shaderResourceResidency;
            capabilities.features.shader_resource_min_lod                      = features.shaderResourceMinLod;
            capabilities.features.sparse_binding                               = features.sparseBinding;
            capabilities.features.sparse_residency_buffer                      = features.sparseResidencyBuffer;
            capabilities.features.sparse_residency_image_2D                    = features.sparseResidencyImage2D;
            capabilities.features.sparse_residency_image_3D                    = features.sparseResidencyImage3D;
            capabilities.features.sparse_residency_2_samples                   = features.sparseResidency2Samples;
            capabilities.features.sparse_residency_4_samples                   = features.sparseResidency4Samples;
            capabilities.features.sparse_residency_8_samples                   = features.sparseResidency8Samples;
            capabilities.features.sparse_residency_16_samples                  = features.sparseResidency16Samples;
            capabilities.features.sparse_residency_aliased                     = features.sparseResidencyAliased;
            capabilities.features.variable_multisample_rate                    = features.variableMultisampleRate;
            capabilities.features.inherited_queries                            = features.inheritedQueries;

            return capabilities;
        }

        auto memory_types(const PhysicalDeviceImplementation& physical_device) noexcept -> dyn_array<MemoryPropertyFlag> {
            const auto& handle               = physical_device.native_handle();
            const auto  vk_memory_properties = vk::call<VkPhysicalDeviceMemoryProperties>(vkGetPhysicalDeviceMemoryProperties,
                                                                                          handle);

            return transform(array_view { vk_memory_properties.memoryTypes, 32 }, [](const auto& type) static noexcept {
                return narrow<MemoryPropertyFlag>(type.propertyFlags);
            });
        }

        auto queue_families(const PhysicalDeviceImplementation& physical_device) noexcept -> dyn_array<QueueFamily> {
            const auto& handle = physical_device.native_handle();
            return transform(vk::enumerate<VkQueueFamilyProperties>(vkGetPhysicalDeviceQueueFamilyProperties, handle),
                             [](const auto& family) static noexcept {
                                 return QueueFamily { .flags = narrow<QueueFlag>(family.queueFlags), .count = family.queueCount };
                             });
        }

        auto extensions(const PhysicalDeviceImplementation& physical_device, const PhysicalDeviceInfo& info) noexcept
          -> dyn_array<string> {
            const auto& handle     = physical_device.native_handle();
            const auto  extensions = TryAssert(vk::enumerate_checked<VkExtensionProperties>(vkEnumerateDeviceExtensionProperties,
                                                                                            handle,
                                                                                            nullptr),
                                               format("Failed to enumerate device {} extensions properties", info.device_name));

            Return transform(extensions, [](const auto& extension) static noexcept {
                const auto string_size = std::char_traits<char>::length(extension.extensionName);

                return string { extension.extensionName, string_size };
            });
        }

        auto formats_properties(const PhysicalDeviceImplementation& physical_device) noexcept
          -> dyn_array<std::pair<PixelFormat, FormatProperties>> {
            const auto& handle = physical_device.native_handle();
            return transform(cm::enumerate<PixelFormat>(), [&handle](const auto val) noexcept {
                return std::make_pair(val,
                                      vk::from_vk(vk::call<VkFormatProperties>(vkGetPhysicalDeviceFormatProperties,
                                                                               handle,
                                                                               vk::to_vk<VkFormat>(val))));
            });
        }
    } // namespace

    /////////////////////////////////////
    /////////////////////////////////////
    auto score_physical_device(view::PhysicalDevice physical_device) noexcept -> u64 {
        const auto support_raytracing = physical_device.check_extension_support(RAYTRACING_EXTENSIONS);

        auto score = u64 { 0u };

        const auto& info         = physical_device.info();
        const auto& capabilities = physical_device.capabilities();

        if (info.type == PhysicalDeviceType::DISCRETE_GPU) score += 10000000u;
        else if (info.type == PhysicalDeviceType::VIRTUAL_GPU)
            score += 5000000u;
        else if (info.type == PhysicalDeviceType::INTEGRATED_GPU)
            score += 250000u;

        score += capabilities.limits.max_image_dimension_1D;
        score += capabilities.limits.max_image_dimension_2D;
        score += capabilities.limits.max_image_dimension_3D;
        score += capabilities.limits.max_image_dimension_cube;
        score += capabilities.limits.max_uniform_buffer_range;
        score += info.api_major_version * 10000000u;
        score += info.api_minor_version * 10000u;
        score += info.api_patch_version * 100u;

        if (support_raytracing) score += 10000000u;

        return score;
    }

    // TODO implement
    // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_KHR_driver_properties.html

    template class PhysicalDeviceInterface<PhysicalDeviceImplementation>;
    template class PhysicalDeviceInterface<view::PhysicalDeviceImplementation>;

    /////////////////////////////////////
    /////////////////////////////////////
    auto PhysicalDeviceImplementation::do_init(PrivateTag, VkPhysicalDevice&& physical_device) noexcept -> void {
        m_vk_handle = std::move(physical_device);

        m_data = core::allocate_unsafe<Data>(Data {
          .device_info  = gpu::info(*this),
          .capabilities = gpu::capabilities(*this),
        });

        m_memory_types      = gpu::memory_types(*this);
        m_extensions        = gpu::extensions(*this, m_data->device_info);
        m_queue_families    = gpu::queue_families(*this);
        m_format_properties = gpu::formats_properties(*this);
    }
} // namespace stormkit::gpu
