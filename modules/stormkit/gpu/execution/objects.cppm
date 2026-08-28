// Copyright (C) 2023 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/gpu/vulkan.hpp>

export module stormkit.gpu.execution:objects;

import std;

import stormkit.core;
import stormkit.gpu.core;

namespace cmonadic = stormkit::core::monadic;

namespace stormkit::gpu {
    class SwapChainImplementation;
    class FrameBufferImplementation;
    class RenderPassImplementation;
    class PipelineCacheImplementation;
    class PipelineLayoutImplementation;
    class PipelineImplementation;
    class DescriptorSetImplementation;
    class DescriptorSetLayoutImplementation;
    class DescriptorPoolImplementation;
    class QueueImplementation;
    class CommandPoolImplementation;
    class CommandBufferImplementation;

    namespace view {
        class SwapChainImplementation;
        class FrameBufferImplementation;
        class RenderPassImplementation;
        class PipelineCacheImplementation;
        class PipelineLayoutImplementation;
        class PipelineImplementation;
        class DescriptorSetImplementation;
        class DescriptorSetLayoutImplementation;
        class DescriptorPoolImplementation;
        class QueueImplementation;
        class CommandPoolImplementation;
        class CommandBufferImplementation;
    } // namespace view

    export {
        class SwapChainTag;
        template<typename>
        class SwapChainInterface;

        class FrameBufferTag;
        template<typename>
        class FrameBufferInterface;

        class RenderPassTag;
        template<typename>
        class RenderPassInterface;

        class PipelineCacheTag;
        template<typename>
        class PipelineCacheInterface;

        class PipelineLayoutTag;
        template<typename>
        class PipelineLayoutInterface;

        class PipelineTag;
        template<typename>
        class PipelineInterface;

        class DescriptorSetTag;
        template<typename>
        class DescriptorSetInterface;

        class DescriptorSetLayoutTag;
        template<typename>
        class DescriptorSetLayoutInterface;

        class DescriptorPoolTag;
        template<typename>
        class DescriptorPoolInterface;

        class QueueTag;
        template<typename>
        class QueueInterface;

        class CommandBufferTag;
        template<typename>
        class CommandBufferInterface;

        class CommandPoolTag;
        template<typename>
        class CommandPoolInterface;

        using LoadSaveError = DecoratedError<std::variant<core::SystemError, Result>>;
        template<typename T>
        using LoadSaveExpected = core::Expected<T, LoadSaveError>;

        using SwapChain           = SwapChainInterface<SwapChainImplementation>;
        using FrameBuffer         = FrameBufferInterface<FrameBufferImplementation>;
        using RenderPass          = RenderPassInterface<RenderPassImplementation>;
        using PipelineCache       = PipelineCacheInterface<PipelineCacheImplementation>;
        using PipelineLayout      = PipelineLayoutInterface<PipelineLayoutImplementation>;
        using Pipeline            = PipelineInterface<PipelineImplementation>;
        using DescriptorSet       = DescriptorSetInterface<DescriptorSetImplementation>;
        using DescriptorSetLayout = DescriptorSetLayoutInterface<DescriptorSetLayoutImplementation>;
        using DescriptorPool      = DescriptorPoolInterface<DescriptorPoolImplementation>;
        using Queue               = QueueInterface<QueueImplementation>;
        using CommandBuffer       = CommandBufferInterface<CommandBufferImplementation>;
        using CommandPool         = CommandPoolInterface<CommandPoolImplementation>;

        namespace view {
            using SwapChain           = SwapChainInterface<SwapChainImplementation>;
            using FrameBuffer         = FrameBufferInterface<FrameBufferImplementation>;
            using RenderPass          = RenderPassInterface<RenderPassImplementation>;
            using PipelineCache       = PipelineCacheInterface<PipelineCacheImplementation>;
            using PipelineLayout      = PipelineLayoutInterface<PipelineLayoutImplementation>;
            using Pipeline            = PipelineInterface<PipelineImplementation>;
            using DescriptorSet       = DescriptorSetInterface<DescriptorSetImplementation>;
            using DescriptorSetLayout = DescriptorSetLayoutInterface<DescriptorSetLayoutImplementation>;
            using DescriptorPool      = DescriptorPoolInterface<DescriptorPoolImplementation>;
            using Queue               = QueueInterface<QueueImplementation>;
            using CommandBuffer       = CommandBufferInterface<CommandBufferImplementation>;
            using CommandPool         = CommandPoolInterface<CommandPoolImplementation>;
        } // namespace view

        namespace trait {
            template<>
            struct GpuObject<SwapChainTag> {
                using ValueType   = VkSwapchainKHR;
                using DeleterType = PFN_vkDestroySwapchainKHR VolkDeviceTable::*;
                using ObjectType  = SwapChain;
                using ViewType    = view::SwapChain;
                using OwnerType   = Device;

                static constexpr auto DEBUG_TYPE = DebugObjectType::SWAPCHAIN;
            };

            template<>
            struct GpuObject<FrameBufferTag> {
                using ValueType   = VkFramebuffer;
                using DeleterType = PFN_vkDestroyFramebuffer VolkDeviceTable::*;
                using ObjectType  = FrameBuffer;
                using ViewType    = view::FrameBuffer;
                using OwnerType   = Device;

                static constexpr auto DEBUG_TYPE = DebugObjectType::FRAMEBUFFER;
            };

            template<>
            struct GpuObject<RenderPassTag> {
                using ValueType   = VkRenderPass;
                using DeleterType = PFN_vkDestroyRenderPass VolkDeviceTable::*;
                using ObjectType  = RenderPass;
                using ViewType    = view::RenderPass;
                using OwnerType   = Device;

                static constexpr auto DEBUG_TYPE = DebugObjectType::RENDER_PASS;
            };

            template<>
            struct GpuObject<PipelineCacheTag> {
                using ValueType        = VkPipelineCache;
                using DeleterType      = PFN_vkDestroyPipelineCache VolkDeviceTable::*;
                using ObjectType       = PipelineCache;
                using ViewType         = view::PipelineCache;
                using OwnerType        = Device;
                using DoInitReturnType = LoadSaveExpected<void>;

                static constexpr auto DEBUG_TYPE = DebugObjectType::PIPELINE_CACHE;
            };

            template<>
            struct GpuObject<PipelineLayoutTag> {
                using ValueType   = VkPipelineLayout;
                using DeleterType = PFN_vkDestroyPipelineLayout VolkDeviceTable::*;
                using ObjectType  = PipelineLayout;
                using ViewType    = view::PipelineLayout;
                using OwnerType   = Device;

                static constexpr auto DEBUG_TYPE = DebugObjectType::PIPELINE_LAYOUT;
            };

            template<>
            struct GpuObject<PipelineTag> {
                using ValueType   = VkPipeline;
                using DeleterType = PFN_vkDestroyPipeline VolkDeviceTable::*;
                using ObjectType  = Pipeline;
                using ViewType    = view::Pipeline;
                using OwnerType   = Device;

                static constexpr auto DEBUG_TYPE = DebugObjectType::PIPELINE;
            };

            template<>
            struct GpuObject<DescriptorSetTag> {
                using ValueType        = VkDescriptorSet;
                using DeleterType      = decltype(cmonadic::noop());
                using ObjectType       = DescriptorSet;
                using ViewType         = view::DescriptorSet;
                using OwnerType        = Device;
                using DoInitReturnType = void;

                static constexpr auto DEBUG_TYPE = DebugObjectType::DESCRIPTOR_SET;
            };

            template<>
            struct GpuObject<DescriptorSetLayoutTag> {
                using ValueType   = VkDescriptorSetLayout;
                using DeleterType = PFN_vkDestroyDescriptorSetLayout VolkDeviceTable::*;
                using ObjectType  = DescriptorSetLayout;
                using ViewType    = view::DescriptorSetLayout;
                using OwnerType   = Device;

                static constexpr auto DEBUG_TYPE = DebugObjectType::DESCRIPTOR_SET_LAYOUT;
            };

            template<>
            struct GpuObject<DescriptorPoolTag> {
                using ValueType   = VkDescriptorPool;
                using DeleterType = PFN_vkDestroyDescriptorPool VolkDeviceTable::*;
                using ObjectType  = DescriptorPool;
                using ViewType    = view::DescriptorPool;
                using OwnerType   = Device;

                static constexpr auto DEBUG_TYPE = DebugObjectType::DESCRIPTOR_POOL;
            };

            template<>
            struct GpuObject<QueueTag> {
                using ValueType        = VkQueue;
                using DeleterType      = decltype(cmonadic::noop());
                using ObjectType       = Queue;
                using ViewType         = view::Queue;
                using OwnerType        = Device;
                using DoInitReturnType = void;

                static constexpr auto DEBUG_TYPE = DebugObjectType::QUEUE;
            };

            template<>
            struct GpuObject<CommandBufferTag> {
                using ValueType        = VkCommandBuffer;
                using DeleterType      = decltype(cmonadic::noop());
                using ObjectType       = CommandBuffer;
                using ViewType         = view::CommandBuffer;
                using OwnerType        = Device;
                using DoInitReturnType = void;

                static constexpr auto DEBUG_TYPE = DebugObjectType::COMMAND_BUFFER;
            };

            template<>
            struct GpuObject<CommandPoolTag> {
                using ValueType   = VkCommandPool;
                using DeleterType = PFN_vkDestroyCommandPool VolkDeviceTable::*;
                using ObjectType  = CommandPool;
                using ViewType    = view::CommandPool;
                using OwnerType   = Device;

                static constexpr auto DEBUG_TYPE = DebugObjectType::COMMAND_POOL;
            };

        } // namespace trait
    }
} // namespace stormkit::gpu
