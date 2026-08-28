// Copyright (C) 2023 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/gpu/vulkan.hpp>

export module stormkit.gpu.resource:objects;

import std;

import stormkit.core;
import stormkit.gpu.core;

namespace stormkit::gpu {
    class ShaderImplementation;
    class BufferImplementation;
    class ImageImplementation;
    class ImageViewImplementation;
    class SamplerImplementation;

    namespace view {
        class ShaderImplementation;
        class BufferImplementation;
        class ImageImplementation;
        class ImageViewImplementation;
        class SamplerImplementation;
    } // namespace view

    export {
        class ShaderTag;
        template<typename>
        class ShaderInterface;

        class BufferTag;
        template<typename>
        class BufferInterface;

        class ImageTag;
        template<typename>
        class ImageInterface;

        class ImageViewTag;
        template<typename>
        class ImageViewInterface;

        class SamplerTag;
        template<typename>
        class SamplerInterface;

        using Shader    = ShaderInterface<ShaderImplementation>;
        using Buffer    = BufferInterface<BufferImplementation>;
        using Image     = ImageInterface<ImageImplementation>;
        using ImageView = ImageViewInterface<ImageViewImplementation>;
        using Sampler   = SamplerInterface<SamplerImplementation>;

        namespace view {
            using Shader    = ShaderInterface<ShaderImplementation>;
            using Buffer    = BufferInterface<BufferImplementation>;
            using Image     = ImageInterface<ImageImplementation>;
            using ImageView = ImageViewInterface<ImageViewImplementation>;
            using Sampler   = SamplerInterface<SamplerImplementation>;
        } // namespace view

        namespace trait {
            template<>
            struct GpuObject<ShaderTag> {
                using ValueType   = VkShaderModule;
                using DeleterType = PFN_vkDestroyShaderModule VolkDeviceTable::*;
                using ObjectType  = Shader;
                using ViewType    = view::Shader;
                using OwnerType   = Device;

                static constexpr auto DEBUG_TYPE = DebugObjectType::SHADER_MODULE;
            };

            template<>
            struct GpuObject<BufferTag> {
                using ValueType   = VkBuffer;
                using DeleterType = PFN_vkDestroyBuffer VolkDeviceTable::*;
                using ObjectType  = Buffer;
                using ViewType    = view::Buffer;
                using OwnerType   = Device;

                static constexpr auto DEBUG_TYPE = DebugObjectType::BUFFER;
            };

            template<>
            struct GpuObject<ImageTag> {
                using ValueType   = VkImage;
                using DeleterType = PFN_vkDestroyImage VolkDeviceTable::*;
                using ObjectType  = Image;
                using ViewType    = view::Image;
                using OwnerType   = Device;

                static constexpr auto DEBUG_TYPE = DebugObjectType::IMAGE;
            };

            template<>
            struct GpuObject<ImageViewTag> {
                using ValueType   = VkImageView;
                using DeleterType = PFN_vkDestroyImageView VolkDeviceTable::*;
                using ObjectType  = ImageView;
                using ViewType    = view::ImageView;
                using OwnerType   = Device;

                static constexpr auto DEBUG_TYPE = DebugObjectType::IMAGE_VIEW;
            };

            template<>
            struct GpuObject<SamplerTag> {
                using ValueType   = VkSampler;
                using DeleterType = PFN_vkDestroySampler VolkDeviceTable::*;
                using ObjectType  = Sampler;
                using ViewType    = view::Sampler;
                using OwnerType   = Device;

                static constexpr auto DEBUG_TYPE = DebugObjectType::SAMPLER;
            };
        } // namespace trait
    }
} // namespace stormkit::gpu
