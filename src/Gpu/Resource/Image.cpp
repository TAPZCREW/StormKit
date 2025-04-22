// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module stormkit.Gpu;

import std;

import stormkit.core;
import stormkit.Image;

import :Core;
import stormkit.Gpu.Vulkan;

import :Resource.Image;

// #include <gli/load.hpp>

namespace stormkit::gpu {
    /* constexpr auto toPixelFormat(gli::format format) {
    switch (format) {
        case gli::format_RGBA16_SFLOAT_PACK16: return gpu::PixelFormat::RGBA16F;
        default: STORMKIT_ENSURES(false);
    }*/

    constexpr auto toPixelFormat(image::Image::Format format) noexcept {
        switch (format) {
            case image::Image::Format::R8_SNorm: return PixelFormat::R8_SNorm;
            case image::Image::Format::R8_UNorm: return PixelFormat::R8_UNorm;
            case image::Image::Format::RG8_SNorm: return PixelFormat::RG8_SNorm;
            case image::Image::Format::RG8_UNorm: return PixelFormat::RG8_UNorm;
            case image::Image::Format::R8I: return PixelFormat::R8I;
            case image::Image::Format::R8U: return PixelFormat::R8U;
            case image::Image::Format::RG8I: return PixelFormat::RG8I;
            case image::Image::Format::RG8U: return PixelFormat::RG8U;
            case image::Image::Format::RGB8_SNorm: return PixelFormat::RGB8_SNorm;
            case image::Image::Format::RGB8_UNorm: return PixelFormat::RGB8_UNorm;
            case image::Image::Format::BGR8_UNorm: return PixelFormat::BGR8_UNorm;
            case image::Image::Format::RGB8I: return PixelFormat::RGB8I;
            case image::Image::Format::RGB8U: return PixelFormat::RGB8U;
            case image::Image::Format::RGBA8_SNorm: return PixelFormat::RGBA8_SNorm;
            case image::Image::Format::RGBA8_UNorm: return PixelFormat::RGBA8_UNorm;
            case image::Image::Format::RGBA16_SNorm: return PixelFormat::RGBA16_SNorm;
            case image::Image::Format::BGRA8_UNorm: return PixelFormat::BGRA8_UNorm;
            case image::Image::Format::sRGB8: return PixelFormat::sRGB8;
            case image::Image::Format::sBGR8: return PixelFormat::sBGR8;
            case image::Image::Format::sRGBA8: return PixelFormat::sRGBA8;
            case image::Image::Format::sBGRA8: return PixelFormat::sBGRA8;

            case image::Image::Format::R16_SNorm: return PixelFormat::R16_SNorm;
            case image::Image::Format::R16_UNorm: return PixelFormat::R16_UNorm;
            case image::Image::Format::R16I: return PixelFormat::R16I;
            case image::Image::Format::R16U: return PixelFormat::R16U;
            case image::Image::Format::RG16_SNorm: return PixelFormat::RG16_SNorm;
            case image::Image::Format::RG16_UNorm: return PixelFormat::RG16_UNorm;
            case image::Image::Format::RG16I: return PixelFormat::RG16I;
            case image::Image::Format::RG16U: return PixelFormat::RG16U;
            case image::Image::Format::RG16F: return PixelFormat::RG16F;
            case image::Image::Format::RGB16I: return PixelFormat::RGB16I;
            case image::Image::Format::RGB16U: return PixelFormat::RGB16U;
            case image::Image::Format::RGB16F: return PixelFormat::RGB16F;
            case image::Image::Format::RGBA16I: return PixelFormat::RGBA16I;
            case image::Image::Format::RGBA16U: return PixelFormat::RGBA16U;
            case image::Image::Format::RGBA16F: return PixelFormat::RGBA16F;
            case image::Image::Format::R16F: return PixelFormat::R16F;

            case image::Image::Format::R32I: return PixelFormat::R32I;
            case image::Image::Format::R32U: return PixelFormat::R32U;
            case image::Image::Format::R32F: return PixelFormat::R32F;
            case image::Image::Format::RG32I: return PixelFormat::RG32I;
            case image::Image::Format::RG32U: return PixelFormat::RG32U;
            case image::Image::Format::RG32F: return PixelFormat::RG32F;
            case image::Image::Format::RGB16_SNorm: return PixelFormat::RGB16_SNorm;
            case image::Image::Format::RGB32I: return PixelFormat::RGB32I;
            case image::Image::Format::RGB32U: return PixelFormat::RGB32U;
            case image::Image::Format::RGB32F: return PixelFormat::RGB32F;
            case image::Image::Format::RGBA8I: return PixelFormat::RGBA8I;
            case image::Image::Format::RGBA8U: return PixelFormat::RGBA8U;
            case image::Image::Format::RGBA32I: return PixelFormat::RGBA32I;
            case image::Image::Format::RGBA32U: return PixelFormat::RGBA32U;
            case image::Image::Format::RGBA32F: return PixelFormat::RGBA32F;

            default: return PixelFormat::Undefined;
        }

        return PixelFormat::Undefined;
    }

} // namespace stormkit::gpu
