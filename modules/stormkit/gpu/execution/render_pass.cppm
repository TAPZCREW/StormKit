// Copyright (C) 2023 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>
#include <stormkit/core/try_expected.hpp>

#include <stormkit/gpu/api.hpp>
#include <stormkit/gpu/vulkan.hpp>

export module stormkit.gpu.execution:render_pass;

import std;

import stormkit.core;
import stormkit.gpu.core;
import stormkit.gpu.resource;

import :objects;

namespace cmeta = stormkit::core::meta;

namespace stormkit::gpu {
    export {
        struct AttachmentDescription {
            PixelFormat     format;
            SampleCountFlag samples = SampleCountFlag::C1;

            AttachmentLoadOperation  load_op  = AttachmentLoadOperation::CLEAR;
            AttachmentStoreOperation store_op = AttachmentStoreOperation::STORE;

            AttachmentLoadOperation  stencil_load_op  = AttachmentLoadOperation::DONT_CARE;
            AttachmentStoreOperation stencil_store_op = AttachmentStoreOperation::DONT_CARE;

            ImageLayout source_layout      = ImageLayout::UNDEFINED;
            ImageLayout destination_layout = ImageLayout::PRESENT_SRC;

            bool resolve = false;
        };

        using AttachmentDescriptions = dyn_array<AttachmentDescription>;

        struct Subpass {
            struct Ref {
                u32 attachment_id;

                ImageLayout layout = ImageLayout::COLOR_ATTACHMENT_OPTIMAL;
            };

            PipelineBindPoint  bind_point;
            dyn_array<Ref>     color_attachment_refs   = {};
            dyn_array<Ref>     resolve_attachment_refs = {};
            std::optional<Ref> depth_attachment_ref    = {};
        };

        using Subpasses = dyn_array<Subpass>;

        struct RenderPassDescription {
            AttachmentDescriptions attachments;
            Subpasses              subpasses;

            auto is_compatible(const RenderPassDescription& description) const noexcept -> bool;
        };

        template<typename Base>
        class FrameBufferInterface final: public DeviceObject<Base> {
          public:
            using DeviceObject<Base>::DeviceObject;
            using DeviceObject<Base>::operator=;
            using TagType = FrameBufferTag;

            [[nodiscard]]
            auto extent() const noexcept -> const math::uextent2&;
            [[nodiscard]]
            auto attachments() const noexcept -> array_view<const view::ImageView>;
        };

        template<typename Base>
        class RenderPassInterface final: public DeviceObject<Base> {
          public:
            using DeviceObject<Base>::DeviceObject;
            using DeviceObject<Base>::operator=;
            using TagType = RenderPassTag;

            auto create_framebuffer(this const auto&,
                                    view::Device               device,
                                    const math::uextent2&      extent,
                                    dyn_array<view::ImageView> attachments) noexcept -> Expected<FrameBuffer>;
            auto allocate_framebuffer(this const auto&,
                                      view::Device               device,
                                      const math::uextent2&      extent,
                                      dyn_array<view::ImageView> attachments) noexcept -> Expected<Heap<FrameBuffer>>;

            [[nodiscard]]
            auto is_compatible(view::RenderPass render_pass) const noexcept -> bool;
            [[nodiscard]]
            auto is_compatible(const RenderPassDescription& description) const noexcept -> bool;

            [[nodiscard]]
            auto description() const noexcept -> const RenderPassDescription&;
        };
    }

    class STORMKIT_GPU_API FrameBufferImplementation
        : public GpuObjectImplementation<FrameBufferTag, view::RenderPass, const math::uextent2&, dyn_array<view::ImageView>> {
      public:
        FrameBufferImplementation(PrivateTag, view::Device&&) noexcept;
        ~FrameBufferImplementation() noexcept;

        FrameBufferImplementation(const FrameBufferImplementation&)                    = delete;
        auto operator=(const FrameBufferImplementation&) -> FrameBufferImplementation& = delete;

        FrameBufferImplementation(FrameBufferImplementation&&) noexcept;
        auto operator=(FrameBufferImplementation&&) noexcept -> FrameBufferImplementation&;

        auto do_init(PrivateTag, view::RenderPass&&, const math::uextent2&, dyn_array<view::ImageView>&&) noexcept
          -> Expected<void>;

      protected:
        using NamedConstructor::allocate;
        using NamedConstructor::create;

        math::uextent2             m_extent = { 0, 0 };
        dyn_array<view::ImageView> m_attachments;

        friend class RenderPassInterface<RenderPassImplementation>;
        friend class RenderPassInterface<view::RenderPassImplementation>;
    };

    namespace view {
        class FrameBufferImplementation: public GpuObjectViewImplementation<FrameBufferTag> {
          public:
            FrameBufferImplementation(const gpu::FrameBuffer& of) noexcept;
            template<cmeta::IsContainerOrPointerOf<gpu::FrameBuffer> TContainerOrPointer>
            FrameBufferImplementation(const TContainerOrPointer&) noexcept;
            ~FrameBufferImplementation() noexcept;

            FrameBufferImplementation(const FrameBufferImplementation&) noexcept;
            auto operator=(const FrameBufferImplementation&) noexcept -> FrameBufferImplementation&;

            FrameBufferImplementation(FrameBufferImplementation&&) noexcept;
            auto operator=(FrameBufferImplementation&&) noexcept -> FrameBufferImplementation&;

          protected:
            math::uextent2                    m_extent;
            array_view<const view::ImageView> m_attachments;
        };
    } // namespace view

    class STORMKIT_GPU_API RenderPassImplementation: public GpuObjectImplementation<RenderPassTag, const RenderPassDescription&> {
      public:
        RenderPassImplementation(PrivateTag, view::Device&&) noexcept;
        ~RenderPassImplementation() noexcept;

        RenderPassImplementation(const RenderPassImplementation&)                    = delete;
        auto operator=(const RenderPassImplementation&) -> RenderPassImplementation& = delete;

        RenderPassImplementation(RenderPassImplementation&&) noexcept;
        auto operator=(RenderPassImplementation&&) noexcept -> RenderPassImplementation&;

        auto do_init(PrivateTag, const RenderPassDescription&) noexcept -> Expected<void>;

      protected:
        Heap<RenderPassDescription> m_description = {};
    };

    namespace view {
        class RenderPassImplementation: public GpuObjectViewImplementation<RenderPassTag> {
          public:
            RenderPassImplementation(const gpu::RenderPass& of) noexcept;
            template<cmeta::IsContainerOrPointerOf<gpu::RenderPass> TContainerOrPointer>
            RenderPassImplementation(const TContainerOrPointer&) noexcept;
            ~RenderPassImplementation() noexcept;

            RenderPassImplementation(const RenderPassImplementation&) noexcept;
            auto operator=(const RenderPassImplementation&) noexcept -> RenderPassImplementation&;

            RenderPassImplementation(RenderPassImplementation&&) noexcept;
            auto operator=(RenderPassImplementation&&) noexcept -> RenderPassImplementation&;

          protected:
            ref<const RenderPassDescription> m_description;

            friend class RenderPassInterface<RenderPassImplementation>;
        };
    } // namespace view

    template<core::meta::HashType Ret = hash32>
    constexpr auto hasher(const AttachmentDescription& value) noexcept -> Ret;
    template<core::meta::HashType Ret = hash32>
    constexpr auto hasher(const Subpass::Ref& value) noexcept -> Ret;
    template<core::meta::HashType Ret = hash32>
    constexpr auto hasher(const Subpass& value) noexcept -> Ret;
    template<core::meta::HashType Ret = hash32>
    constexpr auto hasher(const RenderPassDescription& value) noexcept -> Ret;
} // namespace stormkit::gpu

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::gpu {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto FrameBufferInterface<Base>::extent() const noexcept -> const math::uextent2& {
        return Base::m_extent;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto FrameBufferInterface<Base>::attachments() const noexcept -> array_view<const view::ImageView> {
        return Base::m_attachments;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto RenderPassInterface<Base>::create_framebuffer(this const auto&           self,
                                                              view::Device               device,
                                                              const math::uextent2&      extent,
                                                              dyn_array<view::ImageView> attachments) noexcept
      -> Expected<FrameBuffer> {
        return FrameBuffer::create(std::move(device), gpu::as_view(self), extent, std::move(attachments));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto RenderPassInterface<Base>::allocate_framebuffer(this const auto&           self,
                                                                view::Device               device,
                                                                const math::uextent2&      extent,
                                                                dyn_array<view::ImageView> attachments) noexcept
      -> Expected<Heap<FrameBuffer>> {
        return FrameBuffer::allocate(std::move(device), gpu::as_view(self), extent, std::move(attachments));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto RenderPassInterface<Base>::is_compatible(view::RenderPass) const noexcept -> bool {
        // TODO implement proper compatibility check
        // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/chap7.html#renderpass-compatibility

        return false;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto RenderPassInterface<Base>::description() const noexcept -> const RenderPassDescription& {
        EXPECTS(Base::m_description != nullptr);
        return *Base::m_description;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline FrameBufferImplementation::FrameBufferImplementation(PrivateTag, view::Device&& device) noexcept
        : GpuObjectImplementation { std::move(device), &VolkDeviceTable::vkDestroyFramebuffer } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline FrameBufferImplementation::~FrameBufferImplementation() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline FrameBufferImplementation::FrameBufferImplementation(FrameBufferImplementation&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto FrameBufferImplementation::operator=(FrameBufferImplementation&&) noexcept
      -> FrameBufferImplementation& = default;

    namespace view {
        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline FrameBufferImplementation::FrameBufferImplementation(const gpu::FrameBuffer& of) noexcept
            : GpuObjectViewImplementation { of }, m_extent { of.extent() }, m_attachments { of.attachments() } {
        }

        /////////////////////////////////////
        /////////////////////////////////////
        template<cmeta::IsContainerOrPointerOf<gpu::FrameBuffer> TContainerOrPointer>
        STORMKIT_FORCE_INLINE
        inline FrameBufferImplementation::FrameBufferImplementation(const TContainerOrPointer& of) noexcept
            : FrameBufferImplementation { *of } {
        }

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline FrameBufferImplementation::~FrameBufferImplementation() noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline FrameBufferImplementation::FrameBufferImplementation(const FrameBufferImplementation&) noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto FrameBufferImplementation::operator=(const FrameBufferImplementation&) noexcept
          -> FrameBufferImplementation& = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline FrameBufferImplementation::FrameBufferImplementation(FrameBufferImplementation&&) noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto FrameBufferImplementation::operator=(FrameBufferImplementation&&) noexcept
          -> FrameBufferImplementation& = default;
    } // namespace view

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline RenderPassImplementation::RenderPassImplementation(PrivateTag, view::Device&& device) noexcept
        : GpuObjectImplementation { std::move(device), &VolkDeviceTable::vkDestroyRenderPass } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline RenderPassImplementation::~RenderPassImplementation() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline RenderPassImplementation::RenderPassImplementation(RenderPassImplementation&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto RenderPassImplementation::operator=(RenderPassImplementation&&) noexcept -> RenderPassImplementation& = default;

    namespace view {
        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline RenderPassImplementation::RenderPassImplementation(const gpu::RenderPass& of) noexcept
            : GpuObjectViewImplementation { of }, m_description { as_ref(of.description()) } {
        }

        /////////////////////////////////////
        /////////////////////////////////////
        template<cmeta::IsContainerOrPointerOf<gpu::RenderPass> TContainerOrPointer>
        STORMKIT_FORCE_INLINE
        inline RenderPassImplementation::RenderPassImplementation(const TContainerOrPointer& of) noexcept
            : RenderPassImplementation { *of } {
        }

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline RenderPassImplementation::~RenderPassImplementation() noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline RenderPassImplementation::RenderPassImplementation(const RenderPassImplementation&) noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto RenderPassImplementation::operator=(const RenderPassImplementation& other) noexcept
          -> RenderPassImplementation& {
            if (&other == this) [[unlikely]]
                return *this;

            m_description = as_ref(other.m_description);

            return *this;
        }

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline RenderPassImplementation::RenderPassImplementation(RenderPassImplementation&&) noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto RenderPassImplementation::operator=(RenderPassImplementation&&) noexcept
          -> RenderPassImplementation& = default;
    } // namespace view

    /////////////////////////////////////
    /////////////////////////////////////
    template<core::meta::HashType Ret>
    STORMKIT_FORCE_INLINE
    constexpr auto hasher(const AttachmentDescription& value) noexcept -> Ret {
        return hash<Ret>(value.format,
                         value.samples,
                         value.load_op,
                         value.store_op,
                         value.stencil_load_op,
                         value.stencil_store_op,
                         value.source_layout,
                         value.destination_layout,
                         value.resolve);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<core::meta::HashType Ret>
    STORMKIT_FORCE_INLINE
    constexpr auto hasher(const Subpass::Ref& value) noexcept -> Ret {
        return hash<Ret>(value.attachment_id, value.layout);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<core::meta::HashType Ret>
    STORMKIT_FORCE_INLINE
    constexpr auto hasher(const Subpass& value) noexcept -> Ret {
        return hash<Ret>(value.bind_point,
                         value.color_attachment_refs,
                         value.depth_attachment_ref,
                         value.resolve_attachment_refs);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<core::meta::HashType Ret>
    STORMKIT_FORCE_INLINE
    constexpr auto hasher(const RenderPassDescription& value) noexcept -> Ret {
        return hash<Ret>(value.attachments, value.subpasses);
    }
} // namespace stormkit::gpu
