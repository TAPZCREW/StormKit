// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <volk.h>

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

    auto Image::do_init(const VkImageCreateInfo& create_info,
                        MemoryPropertyFlag       memory_properties) noexcept -> Expected<void> {
        return vk_call<VkImage>(m_vk_device_table->vkCreateImage,
                                m_vk_device,
                                &create_info,
                                nullptr)
          .transform(core::monadic::set(m_vk_handle))
          .and_then([this, memory_properties] noexcept -> VulkanExpected<VmaAllocation> {
              const auto create_info = VmaAllocationCreateInfo {
                  .flags          = 0,
                  .usage          = VMA_MEMORY_USAGE_UNKNOWN,
                  .requiredFlags  = to_vk<VkMemoryPropertyFlags>(memory_properties),
                  .preferredFlags = 0,
                  .memoryTypeBits = 0,
                  .pool           = nullptr,
                  .pUserData      = nullptr,
                  .priority       = 0
              };

              auto out    = VulkanExpected<VmaAllocation> { std::in_place, nullptr };
              auto result = vmaAllocateMemoryForImage(m_vma_allocator,
                                                      m_vk_handle,
                                                      &create_info,
                                                      &*out,
                                                      nullptr);
              if (result != VK_SUCCESS) out = std::unexpected { result };
              else {
                  m_vma_allocation = { [vma_allocator = m_vma_allocator](auto handle) noexcept {
                      vmaFreeMemory(vma_allocator, handle);
                  } };
              }
              return out;
          })
          .transform(core::monadic::set(m_vma_allocation))
          .and_then([this] noexcept {
              return vk_call(vmaBindImageMemory, m_vma_allocator, m_vma_allocation, m_vk_handle);
          })
          .transform_error(monadic::from_vk<Result>());
    }
} // namespace stormkit::gpu
