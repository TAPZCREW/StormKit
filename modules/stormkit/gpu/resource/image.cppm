// Copyright (C) 2023 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>

#include <stormkit/gpu/api.hpp>
#include <stormkit/gpu/vulkan.hpp>

export module stormkit.gpu.resource:image;

import std;

import stormkit.core;
import stormkit.image;
import stormkit.gpu.core;

import :objects;

namespace cmeta    = stormkit::core::meta;
namespace cmonadic = stormkit::core::monadic;

namespace stormkit::gpu {
    struct SamplerInterfaceBase {
        struct Settings {
            Filter mag_filter = Filter::LINEAR;
            Filter min_filter = Filter::LINEAR;

            SamplerAddressMode address_mode_u = SamplerAddressMode::REPEAT;
            SamplerAddressMode address_mode_v = SamplerAddressMode::REPEAT;
            SamplerAddressMode address_mode_w = SamplerAddressMode::REPEAT;

            bool enable_anisotropy = false;
            f32  max_anisotropy    = 0.f;

            BorderColor border_color = BorderColor::INT_OPAQUE_BLACK;

            bool unnormalized_coordinates = false;

            bool             compare_enable    = false;
            CompareOperation compare_operation = CompareOperation::ALWAYS;

            SamplerMipmapMode mipmap_mode  = SamplerMipmapMode::LINEAR;
            f32               mip_lod_bias = 0.f;

            f32 min_lod = 0.f;
            f32 max_lod = 0.f;
        };
    };

    struct ImageInterfaceBase {
        struct CreateInfo {
            math::uextent3     extent;
            PixelFormat        format     = PixelFormat::RGBA8_UNORM;
            u32                layers     = 1u;
            u32                mip_levels = 1u;
            ImageType          type       = ImageType::T2D;
            ImageCreateFlag    flags      = ImageCreateFlag::NONE;
            SampleCountFlag    samples    = SampleCountFlag::C1;
            ImageUsageFlag     usages     = ImageUsageFlag::SAMPLED | ImageUsageFlag::TRANSFER_DST | ImageUsageFlag::TRANSFER_SRC;
            ImageTiling        tiling     = ImageTiling::OPTIMAL;
            MemoryPropertyFlag properties = MemoryPropertyFlag::DEVICE_LOCAL;
        };
    };

    export {
        template<typename Base>
        class SamplerInterface: public DeviceObject<Base>, public SamplerInterfaceBase {
          public:
            using DeviceObject<Base>::DeviceObject;
            using DeviceObject<Base>::operator=;
            using TagType = SamplerTag;

            [[nodiscard]]
            auto settings() const noexcept -> const Settings&;
        };

        template<typename Base>
        class ImageInterface: public DeviceObject<Base>, public ImageInterfaceBase {
          public:
            using DeviceObject<Base>::DeviceObject;
            using DeviceObject<Base>::operator=;
            using TagType = ImageTag;

            [[nodiscard]]
            auto extent() const noexcept -> const math::uextent3&;
            [[nodiscard]]
            auto format() const noexcept -> PixelFormat;
            [[nodiscard]]
            auto type() const noexcept -> ImageType;
            [[nodiscard]]
            auto samples() const noexcept -> SampleCountFlag;
            [[nodiscard]]
            auto layers() const noexcept -> u32;
            [[nodiscard]]
            auto faces() const noexcept -> u32;
            [[nodiscard]]
            auto mip_levels() const noexcept -> u32;
            [[nodiscard]]
            auto usages() const noexcept -> ImageUsageFlag;
            [[nodiscard]]
            auto allocation() const noexcept -> vk::Observer<VmaAllocation>;
        };
    }

    class STORMKIT_GPU_API ImageImplementation: public GpuObjectImplementation<ImageTag, const ImageInterfaceBase::CreateInfo&> {
      public:
        using CreateInfo = ImageInterfaceBase::CreateInfo;

        ImageImplementation(PrivateTag, view::Device&&) noexcept;
        ~ImageImplementation() noexcept;

        ImageImplementation(const ImageImplementation&) noexcept                    = delete;
        auto operator=(const ImageImplementation&) noexcept -> ImageImplementation& = delete;

        ImageImplementation(ImageImplementation&&) noexcept;
        auto operator=(ImageImplementation&&) noexcept -> ImageImplementation&;

        static auto from_existing(view::Device device, const CreateInfo& create_info, VkImage image) noexcept -> Image;

        auto do_init(PrivateTag, const CreateInfo&) noexcept -> Expected<void>;

      protected:
        bool m_no_delete = false;

        math::uextent3  m_extent     = { 0, 0, 0 };
        PixelFormat     m_format     = {};
        u32             m_layers     = 0;
        u32             m_faces      = 0;
        u32             m_mip_levels = 0;
        ImageType       m_type       = {};
        ImageCreateFlag m_flags      = {};
        SampleCountFlag m_samples    = {};
        ImageUsageFlag  m_usages     = {};

        vk::Owned<VmaAllocation> m_vma_allocation = { cmonadic::discard() };

        friend class view::ImageImplementation;
    };

    namespace view {
        class ImageImplementation: public GpuObjectViewImplementation<ImageTag> {
          public:
            ImageImplementation(const gpu::Image&) noexcept;
            template<cmeta::IsContainerOrPointerOf<gpu::Image> TContainerOrPointer>
            ImageImplementation(const TContainerOrPointer&) noexcept;
            ~ImageImplementation() noexcept;

            ImageImplementation(const ImageImplementation&) noexcept;
            auto operator=(const ImageImplementation&) noexcept -> ImageImplementation&;

            ImageImplementation(ImageImplementation&&) noexcept;
            auto operator=(ImageImplementation&&) noexcept -> ImageImplementation&;

          protected:
            math::uextent3 m_extent     = { 0, 0, 0 };
            PixelFormat    m_format     = {};
            u32            m_layers     = 0;
            u32            m_faces      = 0;
            u32            m_mip_levels = 0;
            ImageType      m_type       = {};
            STORMKIT_PUSH_WARNINGS
#pragma clang diagnostic ignored "-Wunused-private-field"
            ImageCreateFlag m_flags = {};
            STORMKIT_POP_WARNINGS
            SampleCountFlag m_samples = {};
            ImageUsageFlag  m_usages  = {};

            vk::Observer<VmaAllocation> m_vma_allocation = VK_NULL_HANDLE;
        };
    } // namespace view

    struct ImageViewInterfaceBase {
        struct CreateInfo {
            view::Image           image;
            ImageViewType         type              = ImageViewType::T2D;
            ImageSubresourceRange subresource_range = {};
        };
    };

    export template<typename Base>
    class ImageViewInterface: public DeviceObject<Base>, public ImageViewInterfaceBase {
      public:
        using CreateInfo = ImageViewInterfaceBase::CreateInfo;

        using DeviceObject<Base>::DeviceObject;
        using DeviceObject<Base>::operator=;
        using TagType = ImageViewTag;

        [[nodiscard]]
        auto type() const noexcept -> ImageViewType;
        [[nodiscard]]
        auto subresource_range() const noexcept -> const ImageSubresourceRange&;
    };

    class STORMKIT_GPU_API
      SamplerImplementation: public GpuObjectImplementation<SamplerTag, const SamplerInterfaceBase::Settings&> {
      public:
        using Settings = SamplerInterfaceBase::Settings;

        SamplerImplementation(PrivateTag, view::Device&&) noexcept;
        ~SamplerImplementation() noexcept;

        SamplerImplementation(const SamplerImplementation&) noexcept                    = delete;
        auto operator=(const SamplerImplementation&) noexcept -> SamplerImplementation& = delete;

        SamplerImplementation(SamplerImplementation&&) noexcept;
        auto operator=(SamplerImplementation&&) noexcept -> SamplerImplementation&;

        auto do_init(PrivateTag, const Settings&) noexcept -> Expected<void>;

      protected:
        Settings m_settings = {};
    };

    namespace view {
        class SamplerImplementation: public GpuObjectViewImplementation<SamplerTag> {
          public:
            using Settings = SamplerInterfaceBase::Settings;

            SamplerImplementation(const gpu::Sampler&) noexcept;
            template<cmeta::IsContainerOrPointerOf<gpu::Sampler> TContainerOrPointer>
            SamplerImplementation(const TContainerOrPointer&) noexcept;
            ~SamplerImplementation() noexcept;

            SamplerImplementation(const SamplerImplementation&) noexcept;
            auto operator=(const SamplerImplementation&) noexcept -> SamplerImplementation&;

            SamplerImplementation(SamplerImplementation&&) noexcept;
            auto operator=(SamplerImplementation&&) noexcept -> SamplerImplementation&;

          protected:
            Settings m_settings;
        };
    } // namespace view

    class STORMKIT_GPU_API
      ImageViewImplementation: public GpuObjectImplementation<ImageViewTag, const ImageViewInterfaceBase::CreateInfo&> {
      public:
        using CreateInfo = ImageViewInterfaceBase::CreateInfo;

        ImageViewImplementation(PrivateTag, view::Device&&) noexcept;
        ~ImageViewImplementation() noexcept;

        ImageViewImplementation(const ImageViewImplementation&) noexcept                    = delete;
        auto operator=(const ImageViewImplementation&) noexcept -> ImageViewImplementation& = delete;

        ImageViewImplementation(ImageViewImplementation&&) noexcept;
        auto operator=(ImageViewImplementation&&) noexcept -> ImageViewImplementation&;

        auto do_init(PrivateTag, const CreateInfo&) noexcept -> Expected<void>;

      protected:
        ImageViewType         m_type              = {};
        ImageSubresourceRange m_subresource_range = {};
    };

    namespace view {
        class ImageViewImplementation: public GpuObjectViewImplementation<ImageViewTag> {
          public:
            ImageViewImplementation(const gpu::ImageView&) noexcept;
            template<cmeta::IsContainerOrPointerOf<gpu::ImageView> TContainerOrPointer>
            ImageViewImplementation(const TContainerOrPointer&) noexcept;
            ~ImageViewImplementation() noexcept;

            ImageViewImplementation(const ImageViewImplementation&) noexcept;
            auto operator=(const ImageViewImplementation&) noexcept -> ImageViewImplementation&;

            ImageViewImplementation(ImageViewImplementation&&) noexcept;
            auto operator=(ImageViewImplementation&&) noexcept -> ImageViewImplementation&;

          protected:
            ImageViewType         m_type              = {};
            ImageSubresourceRange m_subresource_range = {};
        };
    } // namespace view

    export {
        struct ImageMemoryBarrier {
            AccessFlag src;
            AccessFlag dst;

            ImageLayout old_layout;
            ImageLayout new_layout;

            u32 src_queue_family_index = QUEUE_FAMILY_IGNORED;
            u32 dst_queue_family_index = QUEUE_FAMILY_IGNORED;

            view::Image           image;
            ImageSubresourceRange range;
        };

        template<core::meta::HashType Ret = hash32>
        constexpr auto hasher(const Image::CreateInfo& value) noexcept -> Ret;
    }
} // namespace stormkit::gpu

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::gpu {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto SamplerInterface<Base>::settings() const noexcept -> const Settings& {
        return Base::m_settings;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto ImageInterface<Base>::extent() const noexcept -> const math::uextent3& {
        return Base::m_extent;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto ImageInterface<Base>::format() const noexcept -> PixelFormat {
        return Base::m_format;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto ImageInterface<Base>::type() const noexcept -> ImageType {
        return Base::m_type;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto ImageInterface<Base>::samples() const noexcept -> SampleCountFlag {
        return Base::m_samples;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto ImageInterface<Base>::layers() const noexcept -> u32 {
        return Base::m_layers;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto ImageInterface<Base>::faces() const noexcept -> u32 {
        return Base::m_faces;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto ImageInterface<Base>::mip_levels() const noexcept -> u32 {
        return Base::m_mip_levels;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto ImageInterface<Base>::usages() const noexcept -> ImageUsageFlag {
        return Base::m_usages;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto ImageInterface<Base>::allocation() const noexcept -> vk::Observer<VmaAllocation> {
        EXPECTS(Base::m_vma_allocation != VK_NULL_HANDLE);
        return Base::m_vma_allocation;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto ImageViewInterface<Base>::type() const noexcept -> ImageViewType {
        return Base::m_type;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto ImageViewInterface<Base>::subresource_range() const noexcept -> const ImageSubresourceRange& {
        return Base::m_subresource_range;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline SamplerImplementation::SamplerImplementation(PrivateTag, view::Device&& device) noexcept
        : GpuObjectImplementation { std::move(device), &VolkDeviceTable::vkDestroySampler } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline SamplerImplementation::~SamplerImplementation() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline SamplerImplementation::SamplerImplementation(SamplerImplementation&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto SamplerImplementation::operator=(SamplerImplementation&&) noexcept -> SamplerImplementation& = default;

    namespace view {
        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline SamplerImplementation::SamplerImplementation(const gpu::Sampler& of) noexcept
            : GpuObjectViewImplementation { of }, m_settings { of.settings() } {
        }

        /////////////////////////////////////
        /////////////////////////////////////
        template<cmeta::IsContainerOrPointerOf<gpu::Sampler> TContainerOrPointer>
        STORMKIT_FORCE_INLINE
        inline SamplerImplementation::SamplerImplementation(const TContainerOrPointer& of) noexcept
            : SamplerImplementation { *of } {
        }

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline SamplerImplementation::~SamplerImplementation() noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline SamplerImplementation::SamplerImplementation(const SamplerImplementation&) noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto SamplerImplementation::operator=(const SamplerImplementation&) noexcept -> SamplerImplementation& = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline SamplerImplementation::SamplerImplementation(SamplerImplementation&&) noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto SamplerImplementation::operator=(SamplerImplementation&&) noexcept -> SamplerImplementation& = default;
    } // namespace view

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline ImageImplementation::ImageImplementation(PrivateTag, view::Device&& device) noexcept
        : GpuObjectImplementation { std::move(device), &VolkDeviceTable::vkDestroyImage } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline ImageImplementation::~ImageImplementation() noexcept {
        if (m_no_delete) [[unlikely]]
            m_vk_handle = VK_NULL_HANDLE;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline ImageImplementation::ImageImplementation(ImageImplementation&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto ImageImplementation::operator=(ImageImplementation&&) noexcept -> ImageImplementation& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto ImageImplementation::from_existing(view::Device device, const CreateInfo& create_info, VkImage vk_image) noexcept
      -> Image {
        auto image         = Image { NamedConstructor::PRIVATE, std::move(device) };
        image.m_extent     = create_info.extent;
        image.m_format     = create_info.format;
        image.m_layers     = create_info.layers;
        image.m_faces      = 1;
        image.m_mip_levels = create_info.mip_levels;
        image.m_type       = create_info.type;
        image.m_flags      = create_info.flags;
        image.m_samples    = create_info.samples;
        image.m_usages     = create_info.usages;

        image.m_vma_allocation = { core::monadic::noop() };
        image.m_vk_handle      = std::move(vk_image);
        image.m_no_delete      = true;

        return image;
    }

    namespace view {
        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline ImageImplementation::ImageImplementation(const gpu::Image& of) noexcept
            : GpuObjectViewImplementation { of },
              m_extent { of.extent() },
              m_format { of.format() },
              m_layers { of.layers() },
              m_faces { of.faces() },
              m_mip_levels { of.mip_levels() },
              m_type { of.type() },
              m_flags { of.m_flags },
              m_samples { of.samples() },
              m_usages { of.usages() } {
            if (not of.m_no_delete) m_vma_allocation = of.allocation();
        }

        /////////////////////////////////////
        /////////////////////////////////////
        template<cmeta::IsContainerOrPointerOf<gpu::Image> TContainerOrPointer>
        STORMKIT_FORCE_INLINE
        inline ImageImplementation::ImageImplementation(const TContainerOrPointer& of) noexcept
            : ImageImplementation { *of } {
        }

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline ImageImplementation::~ImageImplementation() noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline ImageImplementation::ImageImplementation(const ImageImplementation&) noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto ImageImplementation::operator=(const ImageImplementation&) noexcept -> ImageImplementation& = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline ImageImplementation::ImageImplementation(ImageImplementation&&) noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto ImageImplementation::operator=(ImageImplementation&&) noexcept -> ImageImplementation& = default;
    } // namespace view

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline ImageViewImplementation::ImageViewImplementation(PrivateTag, view::Device&& device) noexcept
        : GpuObjectImplementation { std::move(device), &VolkDeviceTable::vkDestroyImageView } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline ImageViewImplementation::~ImageViewImplementation() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline ImageViewImplementation::ImageViewImplementation(ImageViewImplementation&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto ImageViewImplementation::operator=(ImageViewImplementation&&) noexcept -> ImageViewImplementation& = default;

    namespace view {
        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline ImageViewImplementation::ImageViewImplementation(const gpu::ImageView& of) noexcept
            : GpuObjectViewImplementation { of }, m_type { of.type() }, m_subresource_range { of.subresource_range() } {
        }

        /////////////////////////////////////
        /////////////////////////////////////
        template<cmeta::IsContainerOrPointerOf<gpu::ImageView> TContainerOrPointer>
        STORMKIT_FORCE_INLINE
        inline ImageViewImplementation::ImageViewImplementation(const TContainerOrPointer& of) noexcept
            : ImageViewImplementation { *of } {
        }

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline ImageViewImplementation::~ImageViewImplementation() noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline ImageViewImplementation::ImageViewImplementation(const ImageViewImplementation&) noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto ImageViewImplementation::operator=(const ImageViewImplementation&) noexcept
          -> ImageViewImplementation& = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline ImageViewImplementation::ImageViewImplementation(ImageViewImplementation&&) noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto ImageViewImplementation::operator=(ImageViewImplementation&&) noexcept -> ImageViewImplementation& = default;
    } // namespace view

    /////////////////////////////////////
    /////////////////////////////////////
    template<core::meta::HashType Ret = hash32>
    constexpr auto hasher(const Image::CreateInfo& create_info) noexcept -> Ret {
        return hash(create_info.extent,
                    create_info.format,
                    create_info.layers,
                    create_info.mip_levels,
                    create_info.type,
                    create_info.flags,
                    create_info.samples,
                    create_info.usages,
                    create_info.tiling,
                    create_info.properties);
    }
} // namespace stormkit::gpu
