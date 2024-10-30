// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module stormkit.Gpu;

import std;

import stormkit.Core;

import stormkit.Gpu.Vulkan;

namespace stormkit::gpu {
    static auto vendorNameByID(UInt64 ID) -> std::string_view {
        switch (ID) {
            case 0x1002: return "AMD";
            case 0x1010: return "ImgTex";
            case 0x10DE: return "NVidia";
            case 0x8086: return "Intel";
            case 0x5143: return "Qualcomm";
            case 0x13B5: return "ARM";
        }

        return "Unknown";
    }

    // TODO implement
    // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_KHR_driver_properties.html
    /////////////////////////////////////
    /////////////////////////////////////
    PhysicalDevice::PhysicalDevice(vk::raii::PhysicalDevice physical_device)
        : m_vk_physical_device { std::move(physical_device) } {
        const auto properties = m_vk_physical_device.getProperties();
        const auto features   = m_vk_physical_device.getFeatures();

        const auto vendor_id = properties.vendorID;

        m_device_info.device_id = properties.deviceID;

        const auto device_name_size = std::char_traits<char>::length(properties.deviceName);

        m_device_info.device_name.resize(device_name_size);
        std::char_traits<char>::copy(std::data(m_device_info.device_name),
                                     std::data(properties.deviceName),
                                     device_name_size);
        m_device_info.device_name.shrink_to_fit();

        m_device_info.vendor_id         = vendor_id;
        m_device_info.vendor_name       = vendorNameByID(vendor_id);
        m_device_info.api_major_version = vkVersionMajor(properties.apiVersion);
        m_device_info.api_minor_version = vkVersionMinor(properties.apiVersion);
        m_device_info.api_patch_version = vkVersionPatch(properties.apiVersion);

        m_device_info.driver_major_version = vkVersionMajor(properties.driverVersion);
        m_device_info.driver_minor_version = vkVersionMinor(properties.driverVersion);
        m_device_info.driver_patch_version = vkVersionPatch(properties.driverVersion);
        std::ranges::copy(properties.pipelineCacheUUID,
                          std::ranges::begin(m_device_info.pipeline_cache_uuid));

        m_device_info.type = narrow<PhysicalDeviceType>(properties.deviceType);

        m_capabilities.limits.max_image_dimension_1D    = properties.limits.maxImageDimension1D;
        m_capabilities.limits.max_image_dimension_2D    = properties.limits.maxImageDimension2D;
        m_capabilities.limits.max_image_dimension_3D    = properties.limits.maxImageDimension3D;
        m_capabilities.limits.max_image_dimension_cube  = properties.limits.maxImageDimensionCube;
        m_capabilities.limits.max_image_array_layers    = properties.limits.maxImageArrayLayers;
        m_capabilities.limits.max_texel_buffer_elements = properties.limits.maxTexelBufferElements;
        m_capabilities.limits.max_uniform_buffer_range  = properties.limits.maxUniformBufferRange;
        m_capabilities.limits.max_storage_buffer_range  = properties.limits.maxStorageBufferRange;
        m_capabilities.limits.max_push_constants_size   = properties.limits.maxPushConstantsSize;
        m_capabilities.limits.max_memory_allocation_count
            = properties.limits.maxMemoryAllocationCount;
        m_capabilities.limits.max_sampler_allocation_count
            = properties.limits.maxSamplerAllocationCount;
        m_capabilities.limits.buffer_image_granularity  = properties.limits.bufferImageGranularity;
        m_capabilities.limits.sparse_address_space_size = properties.limits.sparseAddressSpaceSize;
        m_capabilities.limits.max_bound_descriptor_sets = properties.limits.maxBoundDescriptorSets;
        m_capabilities.limits.max_per_stage_descriptor_samplers
            = properties.limits.maxPerStageDescriptorSamplers;
        m_capabilities.limits.max_per_stage_descriptor_uniform_buffers
            = properties.limits.maxPerStageDescriptorUniformBuffers;
        m_capabilities.limits.max_per_stage_descriptor_storage_buffers
            = properties.limits.maxPerStageDescriptorStorageBuffers;
        m_capabilities.limits.max_per_stage_descriptor_sampled_images
            = properties.limits.maxPerStageDescriptorSampledImages;
        m_capabilities.limits.max_per_stage_descriptor_storage_images
            = properties.limits.maxPerStageDescriptorStorageImages;
        m_capabilities.limits.max_per_stage_descriptor_input_attachments
            = properties.limits.maxPerStageDescriptorInputAttachments;
        m_capabilities.limits.max_per_stage_resources = properties.limits.maxPerStageResources;
        m_capabilities.limits.max_descriptor_set_samplers
            = properties.limits.maxDescriptorSetSamplers;
        m_capabilities.limits.max_descriptor_set_uniform_buffers
            = properties.limits.maxDescriptorSetUniformBuffers;
        m_capabilities.limits.max_descriptor_set_uniform_buffers_dynamic
            = properties.limits.maxDescriptorSetUniformBuffersDynamic;
        m_capabilities.limits.max_descriptor_set_storage_buffers
            = properties.limits.maxDescriptorSetStorageBuffers;
        m_capabilities.limits.max_descriptor_set_storage_buffers_dynamic
            = properties.limits.maxDescriptorSetStorageBuffersDynamic;
        m_capabilities.limits.max_descriptor_set_sampled_images
            = properties.limits.maxDescriptorSetSampledImages;
        m_capabilities.limits.max_descriptor_set_storage_images
            = properties.limits.maxDescriptorSetStorageImages;
        m_capabilities.limits.max_descriptor_set_input_attachments
            = properties.limits.maxDescriptorSetInputAttachments;
        m_capabilities.limits.max_vertex_input_attributes
            = properties.limits.maxVertexInputAttributes;
        m_capabilities.limits.max_vertex_input_bindings = properties.limits.maxVertexInputBindings;
        m_capabilities.limits.max_vertex_input_attribute_offset
            = properties.limits.maxVertexInputAttributeOffset;
        m_capabilities.limits.max_vertex_input_binding_stride
            = properties.limits.maxVertexInputBindingStride;
        m_capabilities.limits.max_vertex_output_components
            = properties.limits.maxVertexOutputComponents;
        m_capabilities.limits.max_tessellation_generation_level
            = properties.limits.maxTessellationGenerationLevel;
        m_capabilities.limits.max_tessellation_patch_size
            = properties.limits.maxTessellationPatchSize;
        m_capabilities.limits.max_tessellation_control_per_vertex_input_components
            = properties.limits.maxTessellationControlPerVertexInputComponents;
        m_capabilities.limits.max_tessellation_control_per_vertex_output_components
            = properties.limits.maxTessellationControlPerVertexOutputComponents;
        m_capabilities.limits.max_tessellation_control_per_patch_output_components
            = properties.limits.maxTessellationControlPerPatchOutputComponents;
        m_capabilities.limits.max_tessellation_control_total_output_components
            = properties.limits.maxTessellationControlTotalOutputComponents;
        m_capabilities.limits.max_tessellation_evaluation_input_components
            = properties.limits.maxTessellationEvaluationInputComponents;
        m_capabilities.limits.max_tessellation_evaluation_output_components
            = properties.limits.maxTessellationEvaluationOutputComponents;
        m_capabilities.limits.max_geometry_shader_invocations
            = properties.limits.maxGeometryShaderInvocations;
        m_capabilities.limits.max_geometry_input_components
            = properties.limits.maxGeometryInputComponents;
        m_capabilities.limits.max_geometry_output_components
            = properties.limits.maxGeometryOutputComponents;
        m_capabilities.limits.max_geometry_output_vertices
            = properties.limits.maxGeometryOutputVertices;
        m_capabilities.limits.max_geometry_total_output_components
            = properties.limits.maxGeometryTotalOutputComponents;
        m_capabilities.limits.max_fragment_input_components
            = properties.limits.maxFragmentInputComponents;
        m_capabilities.limits.max_fragment_output_attachments
            = properties.limits.maxFragmentOutputAttachments;
        m_capabilities.limits.max_fragment_dual_src_attachments
            = properties.limits.maxFragmentDualSrcAttachments;
        m_capabilities.limits.max_fragment_combined_output_resources
            = properties.limits.maxFragmentCombinedOutputResources;
        m_capabilities.limits.max_compute_shared_memory_size
            = properties.limits.maxComputeSharedMemorySize;
        std::ranges::copy(properties.limits.maxComputeWorkGroupCount,
                          std::ranges::begin(m_capabilities.limits.max_compute_work_group_count));
        m_capabilities.limits.max_compute_work_group_invocations
            = properties.limits.maxComputeWorkGroupInvocations;
        std::ranges::copy(properties.limits.maxComputeWorkGroupSize,
                          std::ranges::begin(m_capabilities.limits.max_compute_work_group_size));
        m_capabilities.limits.sub_pixel_precision_bits = properties.limits.subPixelPrecisionBits;
        m_capabilities.limits.sub_texel_precision_bits = properties.limits.subTexelPrecisionBits;
        m_capabilities.limits.mipmap_precision_bits    = properties.limits.mipmapPrecisionBits;
        m_capabilities.limits.max_draw_indexed_index_value
            = properties.limits.maxDrawIndexedIndexValue;
        m_capabilities.limits.max_draw_indirect_count = properties.limits.maxDrawIndirectCount;
        m_capabilities.limits.max_sampler_lod_bias    = properties.limits.maxSamplerLodBias;
        m_capabilities.limits.max_sampler_anisotropy  = properties.limits.maxSamplerAnisotropy;
        m_capabilities.limits.max_viewports           = properties.limits.maxViewports;
        std::ranges::copy(properties.limits.maxViewportDimensions,
                          std::ranges::begin(m_capabilities.limits.max_viewport_dimensions));
        std::ranges::copy(properties.limits.viewportBoundsRange,
                          std::ranges::begin(m_capabilities.limits.viewport_bounds_range));
        m_capabilities.limits.viewport_sub_pixel_bits  = properties.limits.viewportSubPixelBits;
        m_capabilities.limits.min_memory_map_alignment = properties.limits.minMemoryMapAlignment;
        m_capabilities.limits.min_texel_buffer_offset_alignment
            = properties.limits.minTexelBufferOffsetAlignment;
        m_capabilities.limits.min_uniform_buffer_offset_alignment
            = properties.limits.minUniformBufferOffsetAlignment;
        m_capabilities.limits.min_storage_buffer_offset_alignment
            = properties.limits.minStorageBufferOffsetAlignment;
        m_capabilities.limits.min_texel_offset         = properties.limits.minTexelOffset;
        m_capabilities.limits.max_texel_offset         = properties.limits.maxTexelOffset;
        m_capabilities.limits.min_texel_gather_offset  = properties.limits.minTexelGatherOffset;
        m_capabilities.limits.max_texel_gather_offset  = properties.limits.maxTexelGatherOffset;
        m_capabilities.limits.min_interpolation_offset = properties.limits.minInterpolationOffset;
        m_capabilities.limits.max_interpolation_offset = properties.limits.maxInterpolationOffset;
        m_capabilities.limits.sub_pixel_interpolation_offset_bits
            = properties.limits.subPixelInterpolationOffsetBits;
        m_capabilities.limits.max_framebuffer_width  = properties.limits.maxFramebufferWidth;
        m_capabilities.limits.max_framebuffer_height = properties.limits.maxFramebufferHeight;
        m_capabilities.limits.max_framebuffer_layers = properties.limits.maxFramebufferLayers;
        m_capabilities.limits.framebuffer_color_sample_counts
            = narrow<SampleCountFlag>(properties.limits.framebufferColorSampleCounts
                                          .operator vk::SampleCountFlags::MaskType());
        m_capabilities.limits.framebuffer_depth_sample_counts
            = narrow<SampleCountFlag>(properties.limits.framebufferDepthSampleCounts
                                          .operator vk::SampleCountFlags::MaskType());
        m_capabilities.limits.framebuffer_stencil_sample_counts
            = narrow<SampleCountFlag>(properties.limits.framebufferStencilSampleCounts
                                          .operator vk::SampleCountFlags::MaskType());
        m_capabilities.limits.framebuffer_no_attachments_sample_counts
            = narrow<SampleCountFlag>(properties.limits.framebufferNoAttachmentsSampleCounts
                                          .operator vk::SampleCountFlags::MaskType());
        m_capabilities.limits.max_color_attachments = properties.limits.maxColorAttachments;
        m_capabilities.limits.sampled_image_color_sample_counts
            = narrow<SampleCountFlag>(properties.limits.sampledImageColorSampleCounts
                                          .operator vk::SampleCountFlags::MaskType());
        m_capabilities.limits.sampled_image_integer_sample_counts
            = narrow<SampleCountFlag>(properties.limits.sampledImageIntegerSampleCounts
                                          .operator vk::SampleCountFlags::MaskType());
        m_capabilities.limits.sampled_image_depth_sample_counts
            = narrow<SampleCountFlag>(properties.limits.sampledImageDepthSampleCounts
                                          .operator vk::SampleCountFlags::MaskType());
        m_capabilities.limits.sampled_image_stencil_sample_counts
            = narrow<SampleCountFlag>(properties.limits.sampledImageStencilSampleCounts
                                          .operator vk::SampleCountFlags::MaskType());
        m_capabilities.limits.storage_image_sample_counts = narrow<SampleCountFlag>(
            properties.limits.storageImageSampleCounts.operator vk::SampleCountFlags::MaskType());
        m_capabilities.limits.max_sample_mask_words = properties.limits.maxSampleMaskWords;
        m_capabilities.limits.timestamp_compute_and_engine
            = properties.limits.timestampComputeAndGraphics;
        m_capabilities.limits.timestamp_period   = properties.limits.timestampPeriod;
        m_capabilities.limits.max_clip_distances = properties.limits.maxClipDistances;
        m_capabilities.limits.max_cull_distances = properties.limits.maxCullDistances;
        m_capabilities.limits.max_combined_clip_and_cull_distances
            = properties.limits.maxCombinedClipAndCullDistances;
        m_capabilities.limits.discrete_queue_priorities = properties.limits.discreteQueuePriorities;
        std::ranges::copy(properties.limits.pointSizeRange,
                          std::ranges::begin(m_capabilities.limits.point_size_range));
        std::ranges::copy(properties.limits.lineWidthRange,
                          std::ranges::begin(m_capabilities.limits.line_width_range));
        m_capabilities.limits.point_size_granularity    = properties.limits.pointSizeGranularity;
        m_capabilities.limits.line_width_granularity    = properties.limits.lineWidthGranularity;
        m_capabilities.limits.strict_lines              = properties.limits.strictLines;
        m_capabilities.limits.standard_sample_locations = properties.limits.standardSampleLocations;
        m_capabilities.limits.optimal_buffer_copy_offset_alignment
            = properties.limits.optimalBufferCopyOffsetAlignment;
        m_capabilities.limits.optimal_buffer_copy_row_pitch_alignment
            = properties.limits.optimalBufferCopyRowPitchAlignment;
        m_capabilities.limits.non_coherent_atom_size = properties.limits.nonCoherentAtomSize;

        m_capabilities.features.robust_buffer_access         = features.robustBufferAccess;
        m_capabilities.features.full_draw_index_uint32       = features.fullDrawIndexUint32;
        m_capabilities.features.image_cube_array             = features.imageCubeArray;
        m_capabilities.features.independent_blend            = features.independentBlend;
        m_capabilities.features.geometry_shader              = features.geometryShader;
        m_capabilities.features.tessellation_shader          = features.tessellationShader;
        m_capabilities.features.sampler_rate_shading         = features.sampleRateShading;
        m_capabilities.features.dual_src_blend               = features.dualSrcBlend;
        m_capabilities.features.logic_op                     = features.logicOp;
        m_capabilities.features.multi_draw_indirect          = features.multiDrawIndirect;
        m_capabilities.features.draw_indirect_first_instance = features.drawIndirectFirstInstance;
        m_capabilities.features.depth_clamp                  = features.depthClamp;
        m_capabilities.features.depth_bias_clamp             = features.depthBiasClamp;
        m_capabilities.features.fill_Mode_non_solid          = features.fillModeNonSolid;
        m_capabilities.features.depth_bounds                 = features.depthBounds;
        m_capabilities.features.wide_lines                   = features.wideLines;
        m_capabilities.features.large_points                 = features.largePoints;
        m_capabilities.features.alpha_to_one                 = features.alphaToOne;
        m_capabilities.features.multi_viewport               = features.multiViewport;
        m_capabilities.features.sampler_anisotropy           = features.samplerAnisotropy;
        m_capabilities.features.texture_compression_etc2     = features.textureCompressionETC2;
        m_capabilities.features.texture_compression_astc_ldr = features.textureCompressionASTC_LDR;
        m_capabilities.features.texture_compression_bc       = features.textureCompressionBC;
        m_capabilities.features.occlusion_query_precise      = features.occlusionQueryPrecise;
        m_capabilities.features.pipeline_statistics_query    = features.pipelineStatisticsQuery;
        m_capabilities.features.vertex_pipeline_stores_and_atomics
            = features.vertexPipelineStoresAndAtomics;
        m_capabilities.features.fragment_stores_and_atomics = features.fragmentStoresAndAtomics;
        m_capabilities.features.shader_tessellation_and_geometry_point_size
            = features.shaderTessellationAndGeometryPointSize;
        m_capabilities.features.shader_image_gather_extended = features.shaderImageGatherExtended;
        m_capabilities.features.shader_storage_image_extended_formats
            = features.shaderStorageImageExtendedFormats;
        m_capabilities.features.shader_storage_image_multisample
            = features.shaderStorageImageMultisample;
        m_capabilities.features.shader_storage_image_read_without_format
            = features.shaderStorageImageReadWithoutFormat;
        m_capabilities.features.shader_storage_image_write_without_format
            = features.shaderStorageImageWriteWithoutFormat;
        m_capabilities.features.shader_uniform_buffer_array_dynamic_indexing
            = features.shaderUniformBufferArrayDynamicIndexing;
        m_capabilities.features.shader_sampled_image_array_dynamic_indexing
            = features.shaderSampledImageArrayDynamicIndexing;
        m_capabilities.features.shader_storage_buffer_array_dynamic_indexing
            = features.shaderStorageBufferArrayDynamicIndexing;
        m_capabilities.features.shader_storage_image_array_dynamic_indexing
            = features.shaderStorageImageArrayDynamicIndexing;
        m_capabilities.features.shader_clip_distance        = features.shaderClipDistance;
        m_capabilities.features.shader_cull_distance        = features.shaderCullDistance;
        m_capabilities.features.shader_float_64             = features.shaderFloat64;
        m_capabilities.features.shader_int_64               = features.shaderInt64;
        m_capabilities.features.shader_int_16               = features.shaderInt16;
        m_capabilities.features.shader_resource_residency   = features.shaderResourceResidency;
        m_capabilities.features.shader_resource_min_lod     = features.shaderResourceMinLod;
        m_capabilities.features.sparse_binding              = features.sparseBinding;
        m_capabilities.features.sparse_residency_buffer     = features.sparseResidencyBuffer;
        m_capabilities.features.sparse_residency_image_2D   = features.sparseResidencyImage2D;
        m_capabilities.features.sparse_residency_image_3D   = features.sparseResidencyImage3D;
        m_capabilities.features.sparse_residency_2_samples  = features.sparseResidency2Samples;
        m_capabilities.features.sparse_residency_4_samples  = features.sparseResidency4Samples;
        m_capabilities.features.sparse_residency_8_samples  = features.sparseResidency8Samples;
        m_capabilities.features.sparse_residency_16_samples = features.sparseResidency16Samples;
        m_capabilities.features.sparse_residency_aliased    = features.sparseResidencyAliased;
        m_capabilities.features.variable_multisample_rate   = features.variableMultisampleRate;
        m_capabilities.features.inherited_queries           = features.inheritedQueries;

        m_extensions = m_vk_physical_device.enumerateDeviceExtensionProperties()
                       | std::views::transform([](auto&& extension) noexcept {
                             const auto string_size
                                 = std::char_traits<char>::length(extension.extensionName);

                             auto string = std::string {};
                             string.resize(string_size);
                             std::char_traits<char>::copy(std::data(string),
                                                          std::data(extension.extensionName),
                                                          string_size);
                             string.shrink_to_fit();
                             return string;
                         })
                       | std::ranges::to<std::vector>();

        m_vk_memory_properties = m_vk_physical_device.getMemoryProperties();
        m_memory_properties
            = m_vk_memory_properties.memoryTypes
              | std::views::transform([](auto&& property) noexcept {
                    return narrow<MemoryPropertyFlag>(
                        property.propertyFlags.operator vk::MemoryPropertyFlags::MaskType());
                })
              | std::ranges::to<std::vector>();

        m_queue_families
            = m_vk_physical_device.getQueueFamilyProperties()
              | std::views::transform([](auto&& family) noexcept {
                    return QueueFamily { .flags = narrow<QueueFlag>(
                                             family.queueFlags.operator vk::QueueFlags::MaskType()),
                                         .count = family.queueCount };
                })
              | std::ranges::to<std::vector>();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    PhysicalDevice::~PhysicalDevice() = default;

    /////////////////////////////////////
    /////////////////////////////////////
    PhysicalDevice::PhysicalDevice(PhysicalDevice&& other) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto PhysicalDevice::operator=(PhysicalDevice&& other) noexcept -> PhysicalDevice& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto PhysicalDevice::checkExtensionSupport(std::string_view extension) const noexcept -> bool {
        return std::ranges::any_of(m_extensions,
                                   [extension](const auto& e) { return e == extension; });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto PhysicalDevice::checkExtensionSupport(
        std::span<const std::string_view> extensions) const noexcept -> bool {
        auto required_extensions = HashSet<std::string_view> { std::ranges::begin(extensions),
                                                               std::ranges::end(extensions) };
        // HashSet<std::string_view> { std::ranges::begin(extensions),
        // std::ranges::end(extensions) };

        for (const auto& extension : m_extensions) required_extensions.erase(extension);

        return std::ranges::empty(required_extensions);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto PhysicalDevice::checkExtensionSupport(std::span<const CZString> extensions) const noexcept
        -> bool {
        auto required_extensions = HashSet<std::string_view> { std::ranges::begin(extensions),
                                                               std::ranges::end(extensions) };

        for (const auto& extension : m_extensions) required_extensions.erase(extension);

        return std::ranges::empty(required_extensions);
    }
} // namespace stormkit::gpu
