module;

#include <stormkit/core/flags_macro.hpp>
#include <stormkit/core/platform_macro.hpp>

#include <stormkit/gpu/vulkan.hpp>

export module stormkit.gpu.core:vulkan.structs;

import std;
import stormkit.core;

import :structs;

import :vulkan.volk;

export namespace stormkit::gpu::vk {
    template<typename T>
        requires(
          requires { std::declval<T>().native_handle(); } or requires { std::declval<T>()->native_handle(); })
    [[nodiscard]]
    auto to_vk(const T& value) noexcept -> decltype(auto);

    template<typename Out>
    [[nodiscard]]
    constexpr auto to_vk(const math::ivec2& vector) noexcept -> Out;

    template<typename Out>
    [[nodiscard]]
    constexpr auto to_vk(const math::ivec3& vector) noexcept -> Out;

    // template<>
    // [[nodiscard]]
    // constexpr auto to_vk<VkOffset2D, math::ivec2>(const math::ivec2& vector) noexcept
    //   -> VkOffset2D;

    [[nodiscard]]
    constexpr auto from_vk(const VkFormatProperties& properties) noexcept -> FormatProperties;

    [[nodiscard]]
    constexpr auto from_vk(const VkOffset2D& vector) noexcept -> math::ivec2;

    [[nodiscard]]
    constexpr auto from_vk(const VkOffset3D& vector) noexcept -> math::ivec3;

    [[nodiscard]]
    constexpr auto to_vk(const Viewport& viewport) noexcept -> VkViewport;

    [[nodiscard]]
    constexpr auto to_vk(const math::irect& rect) noexcept -> VkRect2D;

    [[nodiscard]]
    constexpr auto to_vk(const Scissor& viewport) noexcept -> VkRect2D;

    [[nodiscard]]
    constexpr auto from_vk(const VkViewport& viewport) noexcept -> Viewport;

    template<typename Out = VkExtent2D, math::meta::IsExtent2 Extent>
    [[nodiscard]]
    constexpr auto to_vk(const Extent& extent) noexcept -> Out;

    template<typename Out = VkExtent3D, math::meta::IsExtent3 Extent>
    [[nodiscard]]
    constexpr auto to_vk(const Extent& extent) noexcept -> Out;

    template<math::meta::IsExtent2 Out = math::uextent2>
    [[nodiscard]]
    constexpr auto from_vk(const VkExtent2D& viewport) noexcept -> Out;

    template<math::meta::IsExtent3 Out = math::uextent3>
    [[nodiscard]]
    constexpr auto from_vk(const VkExtent3D& viewport) noexcept -> Out;
} // namespace stormkit::gpu::vk

namespace stormkit::gpu::vk {

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
        requires(
          requires { std::declval<T>().native_handle(); } or requires { std::declval<T>()->native_handle(); })
    STORMKIT_FORCE_INLINE
    STORMKIT_PURE
    inline auto to_vk(const T& value) noexcept -> decltype(auto) {
        if constexpr (core::meta::IsPointer<T>) return value->native_handle();
        else
            return value.native_handle();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    constexpr auto to_vk(const math::irect& rect) noexcept -> VkRect2D {
        return VkRect2D {
            .offset = { rect.x,                    rect.y                     },
            .extent = { as<u32>(rect.width.value), as<u32>(rect.height.value) }
        };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto to_vk(const Viewport& viewport) noexcept -> VkViewport {
        return VkViewport {
            .x        = viewport.position.x,
            .y        = viewport.position.y,
            .width    = viewport.extent.width,
            .height   = viewport.extent.height,
            .minDepth = viewport.depth.x,
            .maxDepth = viewport.depth.y,
        };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto to_vk(const Scissor& scissor) noexcept -> VkRect2D {
        return VkRect2D {
            .offset = { scissor.offset.x,     scissor.offset.y      },
            .extent = { scissor.extent.width, scissor.extent.height }
        };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto from_vk(const VkViewport& viewport) noexcept -> Viewport {
        return Viewport {
            .position = { viewport.x,              viewport.y                },
            .extent   = { .width = viewport.width, .height = viewport.height },
            .depth    = { viewport.minDepth,       viewport.maxDepth         },
        };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out>
        STORMKIT_FORCE_INLINE
        STORMKIT_PURE
    constexpr auto to_vk(const math::ivec2& vector) noexcept -> Out {
        return Out { .x = vector.x, .y = vector.y };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out>
        STORMKIT_FORCE_INLINE
        STORMKIT_PURE
    constexpr auto to_vk(const math::ivec3& vector) noexcept -> Out {
        return Out { .x = vector.x, .y = vector.y, .z = vector.z };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto from_vk(const VkFormatProperties& properties) noexcept -> FormatProperties {
        return {
            .linear_tiling_features  = from_vk<FormatFeatureFlag>(properties.linearTilingFeatures),
            .optimal_tiling_features = from_vk<FormatFeatureFlag>(properties.optimalTilingFeatures),
            .buffer_features         = from_vk<FormatFeatureFlag>(properties.bufferFeatures),
        };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto from_vk(const VkOffset2D& offset) noexcept -> math::ivec2 {
        return math::ivec2 { offset.x, offset.y };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto from_vk(const VkOffset3D& offset) noexcept -> math::ivec3 {
        return math::ivec3 { offset.x, offset.y, offset.z };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, math::meta::IsExtent2 Extent>
        STORMKIT_FORCE_INLINE
        STORMKIT_PURE
    constexpr auto to_vk(const Extent& extent) noexcept -> Out {
        if constexpr (stormkit::meta::Is<Out, VkExtent2D>)
            return VkExtent2D { .width = as<u32>(extent.width), .height = as<u32>(extent.height) };
        else
            return Out { .x = extent.width, .y = extent.height };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, math::meta::IsExtent3 Extent>
        STORMKIT_FORCE_INLINE
        STORMKIT_PURE
    constexpr auto to_vk(const Extent& extent) noexcept -> Out {
        if constexpr (stormkit::meta::Is<Out, VkExtent3D>)
            return VkExtent3D { .width  = as<u32>(extent.width),
                                .height = as<u32>(extent.height),
                                .depth  = as<u32>(extent.depth) };
        else
            return Out { .x = extent.width, .y = extent.height, .z = extent.depth };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<math::meta::IsExtent2 Out>
        STORMKIT_FORCE_INLINE
        STORMKIT_PURE
    constexpr auto from_vk(const VkExtent2D& extent) noexcept -> Out {
        using T = typename Out::ValueType;
        return Out { .width = as<T>(extent.width), .height = as<T>(extent.height) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<math::meta::IsExtent3 Out>
        STORMKIT_FORCE_INLINE
        STORMKIT_PURE
    constexpr auto from_vk(const VkExtent3D& extent) noexcept -> Out {
        using T = typename Out::ValueType;
        return Out { .width = as<T>(extent.width), .height = as<T>(extent.height) };
    }
} // namespace stormkit::gpu::vk
