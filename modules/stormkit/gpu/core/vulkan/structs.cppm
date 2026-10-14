module;

#include <stormkit/core/platform_macro.hpp>

#include <stormkit/gpu/vulkan.hpp>

export module stormkit.gpu.core:vulkan.structs;

import std;
import stormkit.core;

import :structs;

import :vulkan.volk;

namespace cmeta = stormkit::core::meta;
namespace mmeta = stormkit::math::meta;

export namespace stormkit::gpu {
    template<typename Out>
    [[nodiscard]]
    constexpr auto tag_invoke(cmeta::tag<core::as<Out>>, const math::ivec2& vector, core::source_location_arg) noexcept -> Out;

    template<typename Out>
    [[nodiscard]]
    constexpr auto tag_invoke(cmeta::tag<core::as<Out>>, const math::ivec3& vector, core::source_location_arg) noexcept -> Out;

    [[nodiscard]]
    constexpr auto tag_invoke(cmeta::tag<core::as<format_properties>>,
                              const VkFormatProperties& properties,
                              core::source_location_arg) noexcept -> format_properties;

    [[nodiscard]]
    constexpr auto tag_invoke(cmeta::tag<core::as<math::ivec2>>, const VkOffset2D& vector, core::source_location_arg) noexcept
      -> math::ivec2;

    [[nodiscard]]
    constexpr auto tag_invoke(cmeta::tag<core::as<math::ivec3>>, const VkOffset3D& vector, core::source_location_arg) noexcept
      -> math::ivec3;

    [[nodiscard]]
    constexpr auto tag_invoke(cmeta::tag<core::as<VkViewport>>, const viewport& viewport, core::source_location_arg) noexcept
      -> VkViewport;

    [[nodiscard]]
    constexpr auto tag_invoke(cmeta::tag<core::as<VkRect2D>>, const math::irect& rect, core::source_location_arg) noexcept
      -> VkRect2D;

    [[nodiscard]]
    constexpr auto tag_invoke(cmeta::tag<core::as<VkRect2D>>, const scissor& viewport, core::source_location_arg) noexcept
      -> VkRect2D;

    [[nodiscard]]
    constexpr auto tag_invoke(cmeta::tag<core::as<viewport>>, const VkViewport& viewport, core::source_location_arg) noexcept
      -> gpu::viewport;

    template<typename Out = VkExtent2D, mmeta::is_extent2 Extent>
    [[nodiscard]]
    constexpr auto tag_invoke(cmeta::tag<core::as<Out>>, const Extent& extent, core::source_location_arg) noexcept -> Out;

    template<typename Out = VkExtent3D, mmeta::is_extent3 Extent>
    [[nodiscard]]
    constexpr auto tag_invoke(cmeta::tag<core::as<Out>>, const Extent& extent, core::source_location_arg) noexcept -> Out;

    template<mmeta::is_extent2 Out = math::uextent2>
    [[nodiscard]]
    constexpr auto tag_invoke(cmeta::tag<core::as<Out>>, const VkExtent2D& viewport, core::source_location_arg) noexcept -> Out;

    template<mmeta::is_extent3 Out = math::uextent3>
    [[nodiscard]]
    constexpr auto tag_invoke(cmeta::tag<core::as<Out>>, const VkExtent3D& viewport, core::source_location_arg) noexcept -> Out;
} // namespace stormkit::gpu

namespace stormkit::gpu {
    /////////////////////////////////////
    /////////////////////////////////////
    constexpr auto tag_invoke(cmeta::tag<core::as<VkRect2D>>,
                              const math::irect&        rect,
                              core::source_location_arg location) noexcept -> VkRect2D {
        return VkRect2D {
            .offset = { rect.x,                    rect.y                     },
            .extent = { as<u32>(rect.width.value), as<u32>(rect.height.value) }
        };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto tag_invoke(cmeta::tag<core::as<VkViewport>>, const viewport& viewport, core::source_location_arg) noexcept
      -> VkViewport {
        return VkViewport {
            .x        = viewport.position.x(),
            .y        = viewport.position.y(),
            .width    = viewport.extent.width,
            .height   = viewport.extent.height,
            .minDepth = viewport.depth.x(),
            .maxDepth = viewport.depth.y(),
        };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto tag_invoke(cmeta::tag<core::as<VkRect2D>>, const scissor& scissor, core::source_location_arg) noexcept
      -> VkRect2D {
        return VkRect2D {
            .offset = { scissor.offset.x(),   scissor.offset.y()    },
            .extent = { scissor.extent.width, scissor.extent.height }
        };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto tag_invoke(cmeta::tag<core::as<viewport>>, const VkViewport& viewport, core::source_location_arg) noexcept
      -> gpu::viewport {
        return gpu::viewport {
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
    constexpr auto tag_invoke(cmeta::tag<core::as<Out>>, const math::ivec2& vector, core::source_location_arg) noexcept -> Out {
        return Out { .x = vector.x, .y = vector.y };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out>
        STORMKIT_FORCE_INLINE
        STORMKIT_PURE
    constexpr auto tag_invoke(cmeta::tag<core::as<Out>>, const math::ivec3& vector, core::source_location_arg) noexcept -> Out {
        return Out { .x = vector.x, .y = vector.y, .z = vector.z };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto tag_invoke(cmeta::tag<core::as<format_properties>>,
                              const VkFormatProperties& properties,
                              core::source_location_arg) noexcept -> format_properties {
        return {
            .linear_tiling_features  = as<format_feature_flag>(properties.linearTilingFeatures),
            .optimal_tiling_features = as<format_feature_flag>(properties.optimalTilingFeatures),
            .buffer_features         = as<format_feature_flag>(properties.bufferFeatures),
        };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto tag_invoke(cmeta::tag<core::as<math::ivec2>>, const VkOffset2D& offset, core::source_location_arg) noexcept
      -> math::ivec2 {
        return math::ivec2 { offset.x, offset.y };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto tag_invoke(cmeta::tag<core::as<math::ivec3>>, const VkOffset3D& offset, core::source_location_arg) noexcept
      -> math::ivec3 {
        return math::ivec3 { offset.x, offset.y, offset.z };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, mmeta::is_extent2 Extent>
        STORMKIT_FORCE_INLINE
        STORMKIT_PURE
    constexpr auto tag_invoke(cmeta::tag<core::as<Out>>, const Extent& extent, core::source_location_arg) noexcept -> Out {
        if constexpr (stormkit::meta::is<Out, VkExtent2D>)
            return VkExtent2D { .width = as<u32>(extent.width), .height = as<u32>(extent.height) };
        else
            return Out { .x = extent.width, .y = extent.height };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, mmeta::is_extent3 Extent>
        STORMKIT_FORCE_INLINE
        STORMKIT_PURE
    constexpr auto tag_invoke(cmeta::tag<core::as<Out>>, const Extent& extent, core::source_location_arg) noexcept -> Out {
        if constexpr (stormkit::meta::is<Out, VkExtent3D>)
            return VkExtent3D { .width  = as<u32>(extent.width),
                                .height = as<u32>(extent.height),
                                .depth  = as<u32>(extent.depth) };
        else
            return Out { .x = extent.width, .y = extent.height, .z = extent.depth };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<mmeta::is_extent2 Out>
        STORMKIT_FORCE_INLINE
        STORMKIT_PURE
    constexpr auto tag_invoke(cmeta::tag<core::as<Out>>, const VkExtent2D& extent, core::source_location_arg) noexcept -> Out {
        using T = typename Out::value_type;
        return Out { .width = as<T>(extent.width), .height = as<T>(extent.height) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<mmeta::is_extent3 Out>
        STORMKIT_FORCE_INLINE
        STORMKIT_PURE
    constexpr auto tag_invoke(cmeta::tag<core::as<Out>>, const VkExtent3D& extent, core::source_location_arg) noexcept -> Out {
        using T = typename Out::value_type;
        return Out { .width = as<T>(extent.width), .height = as<T>(extent.height) };
    }
} // namespace stormkit::gpu
