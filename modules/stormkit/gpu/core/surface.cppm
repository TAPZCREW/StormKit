// Copyright (C) 2023 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>
#include <stormkit/core/try_expected.hpp>

#include <stormkit/gpu/api.hpp>
#include <stormkit/gpu/vulkan.hpp>

export module stormkit.gpu.core:surface;

import std;

import stormkit.core;
import stormkit.wsi;

import :vulkan;
import :base;

import :structs;
import :objects;

import :instance;

namespace stormkit::gpu {
    export {
        struct SurfaceInterfaceBase {
            struct OffscreenCreateInfo {};

            struct CreateInfo {
                ref<const wsi::Window> window;
            };
        };

        template<typename Base>
        class SurfaceInterface final: public InstanceObject<Base> {
          public:
            using InstanceObject<Base>::InstanceObject;
            using InstanceObject<Base>::operator=;
            using TagType = SurfaceTag;
        };
    }

    class STORMKIT_GPU_API SurfaceImplementation
        : public GpuObjectImplementation<SurfaceTag, const SurfaceInterfaceBase::CreateInfo&>,
          public core::NamedConstructor<SurfaceImplementation,
                                        ConstructorArgs<view::Instance>,
                                        DoInitArgs<const SurfaceInterfaceBase::OffscreenCreateInfo&>> {
      public:
        using CreateInfo                = SurfaceInterfaceBase::CreateInfo;
        using OffscreenCreateInfo       = SurfaceInterfaceBase::OffscreenCreateInfo;
        using OffscreenNamedConstructor = core::
          NamedConstructor<SurfaceImplementation, ConstructorArgs<view::Instance>, DoInitArgs<const OffscreenCreateInfo&>>;

        using OffscreenNamedConstructor::allocate;
        using OffscreenNamedConstructor::create;

        SurfaceImplementation(PrivateTag, view::Instance&&) noexcept;
        ~SurfaceImplementation() noexcept;

        SurfaceImplementation(const SurfaceImplementation&) noexcept                    = delete;
        auto operator=(const SurfaceImplementation&) noexcept -> SurfaceImplementation& = delete;

        SurfaceImplementation(SurfaceImplementation&&) noexcept;
        auto operator=(SurfaceImplementation&&) noexcept -> SurfaceImplementation&;

#if false
        [[nodiscard]]
        static auto create_offscreen(view::Instance instance) noexcept 
          -> Expected<Surface>;
        [[nodiscard]]
        static auto allocate_offscreen(view::Instance instance) noexcept
          -> Expected<Heap<Surface>>;
#endif

        [[nodiscard]]
        static auto create_from_window(view::Instance instance, const wsi::Window& window) noexcept -> Expected<Surface>;
        [[nodiscard]]
        static auto allocate_from_window(view::Instance instance, const wsi::Window& window) noexcept -> Expected<Heap<Surface>>;

        auto do_init(PrivateTag, const CreateInfo&) noexcept -> Expected<void>;
        auto do_init(PrivateTag, const OffscreenCreateInfo&) noexcept -> Expected<void>;
    };

    namespace view {
        class SurfaceImplementation: public GpuObjectViewImplementation<SurfaceTag> {
          public:
            using GpuObjectViewImplementation<SurfaceTag>::GpuObjectViewImplementation;
            using GpuObjectViewImplementation<SurfaceTag>::operator=;
        };
    } // namespace view
} // namespace stormkit::gpu

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::gpu {
    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline SurfaceImplementation::SurfaceImplementation(PrivateTag, view::Instance&& instance) noexcept
        : GpuObjectImplementation { std::move(instance), auto(vkDestroySurfaceKHR) } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline SurfaceImplementation::~SurfaceImplementation() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline SurfaceImplementation::SurfaceImplementation(SurfaceImplementation&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto SurfaceImplementation::operator=(SurfaceImplementation&&) noexcept -> SurfaceImplementation& = default;

#if false
    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE inline auto SurfaceImplementation::create_offscreen(view::Instance instance) noexcept
        -> Expected<Surface> {
        return OffScreenNamedConstructor::create(std::move(instance), OffscreenCreateInfo{});
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE inline auto SurfaceImplementation::allocate_offscreen(view::Instance instance) noexcept
        -> Expected<Heap<Surface>> {
        return OffScreenNamedConstructor::allocate(std::move(instance), OffscreenCreateInfo{});
    }
#endif

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto SurfaceImplementation::create_from_window(view::Instance instance, const wsi::Window& window) noexcept
      -> Expected<Surface> {
        return GpuObjectImplementation::create(std::move(instance), CreateInfo { as_ref(window) });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto SurfaceImplementation::allocate_from_window(view::Instance instance, const wsi::Window& window) noexcept
      -> Expected<Heap<Surface>> {
        return GpuObjectImplementation::allocate(std::move(instance), CreateInfo { as_ref(window) });
    }
} // namespace stormkit::gpu
