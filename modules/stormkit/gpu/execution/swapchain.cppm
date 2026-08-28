// Copyright (C) 2023 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#include <stormkit/gpu/api.hpp>
#include <stormkit/gpu/vulkan.hpp>

export module stormkit.gpu.execution:swapchain;

import std;

import stormkit.core;
import stormkit.gpu.core;
import stormkit.gpu.resource;

import :objects;

namespace cmeta = stormkit::core::meta;

namespace stormkit::gpu {
    struct SwapChainInterfaceBase {
        using ImageID = u32;

        struct NextImage {
            Result  result;
            ImageID id;
        };

        struct CreateInfo {
            view::Surface  surface;
            math::uextent2 extent;
            VkSwapchainKHR old = VK_NULL_HANDLE;
        };
    };

    export template<typename Base>
    class STORMKIT_GPU_API SwapChainInterface final: public DeviceObject<Base>, public SwapChainInterfaceBase {
      public:
        using DeviceObject<Base>::DeviceObject;
        using DeviceObject<Base>::operator=;
        using TagType = SwapChainTag;

        using SwapChainInterfaceBase::ImageID;
        using SwapChainInterfaceBase::NextImage;

        [[nodiscard]]
        auto pixel_format() const noexcept -> PixelFormat;
        [[nodiscard]]
        auto images() const noexcept -> array_view<const Image>;
        auto acquire_next_image(std::chrono::nanoseconds wait, view::Semaphore image_available) const noexcept
          -> Expected<NextImage>;
    };

    class STORMKIT_GPU_API
      SwapChainImplementation: public GpuObjectImplementation<SwapChainTag, const SwapChainInterfaceBase::CreateInfo&> {
      public:
        using ImageID    = SwapChainInterfaceBase::ImageID;
        using NextImage  = SwapChainInterfaceBase::NextImage;
        using CreateInfo = SwapChainInterfaceBase::CreateInfo;

        SwapChainImplementation(PrivateTag, view::Device&&) noexcept;
        ~SwapChainImplementation() noexcept;

        SwapChainImplementation(const SwapChainImplementation&)                    = delete;
        auto operator=(const SwapChainImplementation&) -> SwapChainImplementation& = delete;

        SwapChainImplementation(SwapChainImplementation&&) noexcept;
        auto operator=(SwapChainImplementation&&) noexcept -> SwapChainImplementation&;

        auto do_init(PrivateTag, const CreateInfo&) noexcept -> Expected<void>;

      protected:
        math::uextent2 m_extent;
        PixelFormat    m_pixel_format;
        u32            m_image_count;

        dyn_array<Image> m_images;
    };

    namespace view {
        class SwapChainImplementation: public GpuObjectViewImplementation<SwapChainTag> {
          public:
            using ImageID   = SwapChainInterfaceBase::ImageID;
            using NextImage = SwapChainInterfaceBase::NextImage;

            SwapChainImplementation(const gpu::SwapChain& of) noexcept;
            template<cmeta::IsContainerOrPointerOf<gpu::SwapChain> TContainerOrPointer>
            SwapChainImplementation(const TContainerOrPointer&) noexcept;
            ~SwapChainImplementation() noexcept;

            SwapChainImplementation(const SwapChainImplementation&) noexcept;
            auto operator=(const SwapChainImplementation&) noexcept -> SwapChainImplementation&;

            SwapChainImplementation(SwapChainImplementation&&) noexcept;
            auto operator=(SwapChainImplementation&&) noexcept -> SwapChainImplementation&;

          protected:
            math::uextent2 m_extent;
            PixelFormat    m_pixel_format;
            u32            m_image_count;

            array_view<const gpu::Image> m_images;
        };
    } // namespace view
} // namespace stormkit::gpu

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::gpu {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto SwapChainInterface<Base>::pixel_format() const noexcept -> PixelFormat {
        return Base::m_pixel_format;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto SwapChainInterface<Base>::images() const noexcept -> array_view<const Image> {
        return Base::m_images;
    }

    STORMKIT_FORCE_INLINE
    inline SwapChainImplementation::SwapChainImplementation(PrivateTag, view::Device&& device) noexcept
        : GpuObjectImplementation { std::move(device), &VolkDeviceTable::vkDestroySwapchainKHR } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline SwapChainImplementation::~SwapChainImplementation() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline SwapChainImplementation::SwapChainImplementation(SwapChainImplementation&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto SwapChainImplementation::operator=(SwapChainImplementation&&) noexcept -> SwapChainImplementation& = default;

    namespace view {
        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline SwapChainImplementation::SwapChainImplementation(const gpu::SwapChain& of) noexcept
            : GpuObjectViewImplementation { of }, m_pixel_format { of.pixel_format() }, m_images { of.images() } {
        }

        /////////////////////////////////////
        /////////////////////////////////////
        template<cmeta::IsContainerOrPointerOf<gpu::SwapChain> TContainerOrPointer>
        STORMKIT_FORCE_INLINE
        inline SwapChainImplementation::SwapChainImplementation(const TContainerOrPointer& of) noexcept
            : SwapChainImplementation { *of } {
        }

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline SwapChainImplementation::~SwapChainImplementation() noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline SwapChainImplementation::SwapChainImplementation(const SwapChainImplementation&) noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto SwapChainImplementation::operator=(const SwapChainImplementation&) noexcept
          -> SwapChainImplementation& = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline SwapChainImplementation::SwapChainImplementation(SwapChainImplementation&&) noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto SwapChainImplementation::operator=(SwapChainImplementation&&) noexcept -> SwapChainImplementation& = default;
    } // namespace view
} // namespace stormkit::gpu
