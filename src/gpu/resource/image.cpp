// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
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
        // constexpr auto to_pixel_format(image::Image::Format format) noexcept {
        //     switch (format) {
        //         case image::Image::Format::R8_SNORM: return PixelFormat::R8_SNORM;
        //         case image::Image::Format::R8_UNORM: return PixelFormat::R8_UNORM;
        //         case image::Image::Format::RG8_SNORM: return PixelFormat::RG8_SNORM;
        //         case image::Image::Format::RG8_UNORM: return PixelFormat::RG8_UNORM;
        //         case image::Image::Format::R8I: return PixelFormat::R8I;
        //         case image::Image::Format::R8U: return PixelFormat::R8U;
        //         case image::Image::Format::RG8I: return PixelFormat::RG8I;
        //         case image::Image::Format::RG8U: return PixelFormat::RG8U;
        //         case image::Image::Format::RGB8_SNORM: return PixelFormat::RGB8_SNORM;
        //         case image::Image::Format::RGB8_UNORM: return PixelFormat::RGB8_UNORM;
        //         case image::Image::Format::BGR8_UNORM: return PixelFormat::BGR8_UNORM;
        //         case image::Image::Format::RGB8I: return PixelFormat::RGB8I;
        //         case image::Image::Format::RGB8U: return PixelFormat::RGB8U;
        //         case image::Image::Format::RGBA8_SNORM: return PixelFormat::RGBA8_SNORM;
        //         case image::Image::Format::RGBA8_UNORM: return PixelFormat::RGBA8_UNORM;
        //         case image::Image::Format::RGBA16_SNORM: return PixelFormat::RGBA16_SNORM;
        //         case image::Image::Format::BGRA8_UNORM: return PixelFormat::BGRA8_UNORM;
        //         case image::Image::Format::SRGB8: return PixelFormat::SRGB8;
        //         case image::Image::Format::SBGR8: return PixelFormat::SBGR8;
        //         case image::Image::Format::SRGBA8: return PixelFormat::SRGBA8;
        //         case image::Image::Format::SBGRA8: return PixelFormat::SBGRA8;

        //    case image::Image::Format::R16_SNORM: return PixelFormat::R16_SNORM;
        //    case image::Image::Format::R16_UNORM: return PixelFormat::R16_UNORM;
        //    case image::Image::Format::R16I: return PixelFormat::R16I;
        //    case image::Image::Format::R16U: return PixelFormat::R16U;
        //    case image::Image::Format::RG16_SNORM: return PixelFormat::RG16_SNORM;
        //    case image::Image::Format::RG16_UNORM: return PixelFormat::RG16_UNORM;
        //    case image::Image::Format::RG16I: return PixelFormat::RG16I;
        //    case image::Image::Format::RG16U: return PixelFormat::RG16U;
        //    case image::Image::Format::RG16F: return PixelFormat::RG16F;
        //    case image::Image::Format::RGB16I: return PixelFormat::RGB16I;
        //    case image::Image::Format::RGB16U: return PixelFormat::RGB16U;
        //    case image::Image::Format::RGB16F: return PixelFormat::RGB16F;
        //    case image::Image::Format::RGBA16I: return PixelFormat::RGBA16I;
        //    case image::Image::Format::RGBA16U: return PixelFormat::RGBA16U;
        //    case image::Image::Format::RGBA16F: return PixelFormat::RGBA16F;
        //    case image::Image::Format::R16F: return PixelFormat::R16F;

        //    case image::Image::Format::R32I: return PixelFormat::R32I;
        //    case image::Image::Format::R32U: return PixelFormat::R32U;
        //    case image::Image::Format::R32F: return PixelFormat::R32F;
        //    case image::Image::Format::RG32I: return PixelFormat::RG32I;
        //    case image::Image::Format::RG32U: return PixelFormat::RG32U;
        //    case image::Image::Format::RG32F: return PixelFormat::RG32F;
        //    case image::Image::Format::RGB16_SNORM: return PixelFormat::RGB16_SNORM;
        //    case image::Image::Format::RGB32I: return PixelFormat::RGB32I;
        //    case image::Image::Format::RGB32U: return PixelFormat::RGB32U;
        //    case image::Image::Format::RGB32F: return PixelFormat::RGB32F;
        //    case image::Image::Format::RGBA8I: return PixelFormat::RGBA8I;
        //    case image::Image::Format::RGBA8U: return PixelFormat::RGBA8U;
        //    case image::Image::Format::RGBA32I: return PixelFormat::RGBA32I;
        //    case image::Image::Format::RGBA32U: return PixelFormat::RGBA32U;
        //    case image::Image::Format::RGBA32F: return PixelFormat::RGBA32F;

        //    default: return PixelFormat::UNDEFINED;
        // }

        //    return PixelFormat::UNDEFINED;
        // }
    } // namespace

    template class ImageInterface<ImageImplementation>;
    template class ImageInterface<view::ImageImplementation>;

    /////////////////////////////////////
    /////////////////////////////////////
    auto ImageImplementation::do_init(PrivateTag, const CreateInfo& _create_info) noexcept -> Expected<void> {
        m_extent     = _create_info.extent;
        m_format     = _create_info.format;
        m_layers     = _create_info.layers;
        m_faces      = 1;
        m_mip_levels = _create_info.mip_levels;
        m_type       = _create_info.type;
        m_flags      = _create_info.flags;
        m_samples    = _create_info.samples;
        m_usages     = _create_info.usages;

        if (core::check_flag_bit(m_flags, gpu::ImageCreateFlag::CUBE_COMPATIBLE)) m_faces = 6u;
        const auto create_info = VkImageCreateInfo {
            .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext                 = nullptr,
            .flags                 = vk::to_vk<VkImageCreateFlags>(m_flags),
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

        m_vk_handle = Try(vk::call_checked<VkImage>(device.device_table().vkCreateImage, device, &create_info, nullptr));

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
        Try(vk::call_checked(vmaAllocateMemoryForImage, allocator, m_vk_handle, &vma_create_info, &out, nullptr));
        m_vma_allocation = { [allocator](VmaAllocation handle) noexcept {
            if (handle != VK_NULL_HANDLE) vk::call(vmaFreeMemory, allocator, handle);
        } };
        m_vma_allocation = std::move(out);

        Try(vk::call_checked(vmaBindImageMemory, allocator, m_vma_allocation, m_vk_handle));

        Return {};
    }
} // namespace stormkit::gpu
