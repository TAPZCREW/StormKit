// Copyright (C) 2023 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>
#include <stormkit/core/try_expected.hpp>

#include <stormkit/gpu/api.hpp>
#include <stormkit/gpu/vulkan.hpp>

export module stormkit.gpu.core:sync;

import std;

import stormkit.core;

import :vulkan;
import :base;

import :structs;
import :objects;

import :instance;
import :device;

namespace stormkit::gpu {
    export {
        struct FenceInterfaceBase {
            struct CreateInfo {
                bool signaled = false;
            };

            enum class Status {
                SIGNALED,
                UNSIGNALED,
            };
        };

        template<typename Base>
        class STORMKIT_GPU_API FenceInterface final: public FenceInterfaceBase, public DeviceObject<Base> {
          public:
            using DeviceObject<Base>::DeviceObject;
            using DeviceObject<Base>::operator=;
            using TagType = FenceTag;

            using Status = FenceInterfaceBase::Status;

            auto status() const noexcept -> Expected<Status>;
            auto wait(const std::chrono::milliseconds& wait_for = std::chrono::milliseconds::max()) const noexcept
              -> Expected<Result>;
            auto reset() const noexcept -> Expected<void>;
        };

        template<typename Base>
        class SemaphoreInterface final: public DeviceObject<Base> {
          public:
            using DeviceObject<Base>::DeviceObject;
            using DeviceObject<Base>::operator=;
            using TagType = SemaphoreTag;
        };
    }

    class STORMKIT_GPU_API FenceImplementation: public GpuObjectImplementation<FenceTag, const FenceInterfaceBase::CreateInfo&> {
      public:
        using CreateInfo = FenceInterfaceBase::CreateInfo;

        static auto create(view::Device device) noexcept -> Expected<Fence>;
        static auto allocate(view::Device device) noexcept -> Expected<Heap<Fence>>;
        static auto create_signaled(view::Device device) noexcept -> Expected<Fence>;
        static auto allocate_signaled(view::Device device) noexcept -> Expected<Heap<Fence>>;

        FenceImplementation(PrivateTag, view::Device&&) noexcept;

        auto do_init(PrivateTag, const CreateInfo&) noexcept -> Expected<void>;
    };

    namespace view {
        class FenceImplementation: public GpuObjectViewImplementation<FenceTag> {
          public:
            using GpuObjectViewImplementation<FenceTag>::GpuObjectViewImplementation;
            using GpuObjectViewImplementation<FenceTag>::operator=;
        };
    } // namespace view

    class STORMKIT_GPU_API SemaphoreImplementation: public GpuObjectImplementation<SemaphoreTag> {
      public:
        SemaphoreImplementation(PrivateTag, view::Device&&) noexcept;

        auto do_init(PrivateTag) noexcept -> Expected<void>;
    };

    namespace view {
        class SemaphoreImplementation: public GpuObjectViewImplementation<SemaphoreTag> {
          public:
            using GpuObjectViewImplementation<SemaphoreTag>::GpuObjectViewImplementation;
            using GpuObjectViewImplementation<SemaphoreTag>::operator=;
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
    inline auto DeviceInterface<Base>::wait_for_fence(view::Fence fence, const std::chrono::milliseconds& timeout) const noexcept
      -> Expected<Result> {
        return wait_for_fences(as_views(std::move(fence)), true, timeout);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto DeviceInterface<Base>::reset_fence(view::Fence fence) const noexcept -> Expected<void> {
        return reset_fences(as_views(std::move(fence)));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline FenceImplementation::FenceImplementation(PrivateTag, view::Device&& device) noexcept
        : GpuObjectImplementation { std::move(device), &VolkDeviceTable::vkDestroyFence } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto FenceImplementation::create(view::Device device) noexcept -> Expected<Fence> {
        return NamedConstructor::create(std::move(device), {});
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto FenceImplementation::allocate(view::Device device) noexcept -> Expected<Heap<Fence>> {
        return NamedConstructor::allocate(std::move(device), {});
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto FenceImplementation::create_signaled(view::Device device) noexcept -> Expected<Fence> {
        return NamedConstructor::create(std::move(device), { true });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto FenceImplementation::allocate_signaled(view::Device device) noexcept -> Expected<Heap<Fence>> {
        return NamedConstructor::allocate(std::move(device), { true });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline SemaphoreImplementation::SemaphoreImplementation(PrivateTag, view::Device&& device) noexcept
        : GpuObjectImplementation { std::move(device), &VolkDeviceTable::vkDestroySemaphore } {
    }
} // namespace stormkit::gpu
