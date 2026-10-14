// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/try_expected.hpp>

#include <stormkit/gpu/vulkan.hpp>

module stormkit.gpu.resource;

import std;

import stormkit.core;

import stormkit.gpu.core;

namespace stormkit::gpu {
    namespace {
        // constexpr auto to_pixel_format(image::image::Format format) noexcept {
        //     switch (format) {
        //         case image::image::Format::R8_SNORM: return pixel_format::R8_SNORM;
        //         case image::image::Format::R8_UNORM: return pixel_format::R8_UNORM;
        //         case image::image::Format::RG8_SNORM: return pixel_format::RG8_SNORM;
        //         case image::image::Format::RG8_UNORM: return pixel_format::RG8_UNORM;
        //         case image::image::Format::R8I: return pixel_format::R8I;
        //         case image::image::Format::R8U: return pixel_format::R8U;
        //         case image::image::Format::RG8I: return pixel_format::RG8I;
        //         case image::image::Format::RG8U: return pixel_format::RG8U;
        //         case image::image::Format::RGB8_SNORM: return pixel_format::RGB8_SNORM;
        //         case image::image::Format::RGB8_UNORM: return pixel_format::RGB8_UNORM;
        //         case image::image::Format::BGR8_UNORM: return pixel_format::BGR8_UNORM;
        //         case image::image::Format::RGB8I: return pixel_format::RGB8I;
        //         case image::image::Format::RGB8U: return pixel_format::RGB8U;
        //         case image::image::Format::RGBA8_SNORM: return pixel_format::RGBA8_SNORM;
        //         case image::image::Format::RGBA8_UNORM: return pixel_format::RGBA8_UNORM;
        //         case image::image::Format::RGBA16_SNORM: return pixel_format::RGBA16_SNORM;
        //         case image::image::Format::BGRA8_UNORM: return pixel_format::BGRA8_UNORM;
        //         case image::image::Format::SRGB8: return pixel_format::SRGB8;
        //         case image::image::Format::SBGR8: return pixel_format::SBGR8;
        //         case image::image::Format::SRGBA8: return pixel_format::SRGBA8;
        //         case image::image::Format::SBGRA8: return pixel_format::SBGRA8;

        //    case image::image::Format::R16_SNORM: return pixel_format::R16_SNORM;
        //    case image::image::Format::R16_UNORM: return pixel_format::R16_UNORM;
        //    case image::image::Format::R16I: return pixel_format::R16I;
        //    case image::image::Format::R16U: return pixel_format::R16U;
        //    case image::image::Format::RG16_SNORM: return pixel_format::RG16_SNORM;
        //    case image::image::Format::RG16_UNORM: return pixel_format::RG16_UNORM;
        //    case image::image::Format::RG16I: return pixel_format::RG16I;
        //    case image::image::Format::RG16U: return pixel_format::RG16U;
        //    case image::image::Format::RG16F: return pixel_format::RG16F;
        //    case image::image::Format::RGB16I: return pixel_format::RGB16I;
        //    case image::image::Format::RGB16U: return pixel_format::RGB16U;
        //    case image::image::Format::RGB16F: return pixel_format::RGB16F;
        //    case image::image::Format::RGBA16I: return pixel_format::RGBA16I;
        //    case image::image::Format::RGBA16U: return pixel_format::RGBA16U;
        //    case image::image::Format::RGBA16F: return pixel_format::RGBA16F;
        //    case image::image::Format::R16F: return pixel_format::R16F;

        //    case image::image::Format::R32I: return pixel_format::R32I;
        //    case image::image::Format::R32U: return pixel_format::R32U;
        //    case image::image::Format::R32F: return pixel_format::R32F;
        //    case image::image::Format::RG32I: return pixel_format::RG32I;
        //    case image::image::Format::RG32U: return pixel_format::RG32U;
        //    case image::image::Format::RG32F: return pixel_format::RG32F;
        //    case image::image::Format::RGB16_SNORM: return pixel_format::RGB16_SNORM;
        //    case image::image::Format::RGB32I: return pixel_format::RGB32I;
        //    case image::image::Format::RGB32U: return pixel_format::RGB32U;
        //    case image::image::Format::RGB32F: return pixel_format::RGB32F;
        //    case image::image::Format::RGBA8I: return pixel_format::RGBA8I;
        //    case image::image::Format::RGBA8U: return pixel_format::RGBA8U;
        //    case image::image::Format::RGBA32I: return pixel_format::RGBA32I;
        //    case image::image::Format::RGBA32U: return pixel_format::RGBA32U;
        //    case image::image::Format::RGBA32F: return pixel_format::RGBA32F;

        //    default: return pixel_format::UNDEFINED;
        // }

        //    return pixel_format::UNDEFINED;
        // }
    } // namespace

    template class ImageInterface<ImageImplementation>;
    template class ImageInterface<view::ImageImplementation>;

    /////////////////////////////////////
    /////////////////////////////////////
    auto ImageImplementation::do_init(PrivateTag, const CreateInfo& _create_info) noexcept -> expected<void> {
        m_extent     = _create_info.extent;
        m_format     = _create_info.format;
        m_layers     = _create_info.layers;
        m_faces      = 1;
        m_mip_levels = _create_info.mip_levels;
        m_type       = _create_info.type;
        flags_      = _create_info.flags;
        m_samples    = _create_info.samples;
        m_usages     = _create_info.usages;

        if (core::has_flag_bit(flags_, gpu::ImageCreateFlag::CUBE_COMPATIBLE)) m_faces = 6u;
        const auto create_info = VkImageCreateInfo {
            .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext                 = nullptr,
            .flags                 = vk::to_vk<VkImageCreateFlags>(flags_),
            .imageType             = vk::to_vk<VkImageType>(m_type),
            .format                = vk::to_vk<VkFormat>(m_format),
            .extent                = { m_extent.width, m_extent.height, m_extent.depth },
            .mipLevels             = m_mip_levels,
            .arrayLayers           = m_layers * m_faces,
            .samples               = vk::to_vk<VkSampleCountFlagBits>(m_samples),
            .tiling                = vk::to_vk<VkImageTiling>(_create_info.tiling),
            .usage                 = vk::to_vk<VkImageUsageFlags>(m_usages),
            .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0, // TODO CHECK IF VALID VALUE
            .pQueueFamilyIndices   = nullptr,
            .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        const auto& device = owner();

        m_vk_handle = TryX(vk::call_checked<VkImage>(device.device_table().vkCreateImage, device, &create_info, nullptr));

        const auto vma_create_info = VmaAllocationCreateInfo {
            .flags          = 0,
            .usage          = VMA_MEMORY_USAGE_UNKNOWN,
            .requiredFlags  = vk::to_vk<VkMemoryPropertyFlags>(_create_info.properties),
            .preferredFlags = 0,
            .memoryTypeBits = 0,
            .pool           = nullptr,
            .pUserData      = nullptr,
            .priority       = 0
        };

        const auto allocator = device.allocator();
        auto       out       = VmaAllocation { VK_NULL_HANDLE };
        TryX(vk::call_checked(vmaAllocateMemoryForImage, allocator, m_vk_handle, &vma_create_info, &out, nullptr));
        m_vma_allocation = { [allocator](VmaAllocation handle) noexcept {
            if (handle != VK_NULL_HANDLE) vk::call(vmaFreeMemory, allocator, handle);
        } };
        m_vma_allocation = std::move(out);

        TryX(vk::call_checked(vmaBindImageMemory, allocator, m_vma_allocation, m_vk_handle));

        Return {};
    }
} // namespace stormkit::gpu
