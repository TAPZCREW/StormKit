// Copyright (C) 2023 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>
#include <stormkit/core/try_expected.hpp>

#include <stormkit/gpu/api.hpp>
#include <stormkit/gpu/vulkan.hpp>

export module stormkit.gpu.core:instance;

import std;

import stormkit.core;
import stormkit.wsi;

import :vulkan;
import :base;

import :structs;
import :objects;

namespace cmonadic = stormkit::core::monadic;

namespace stormkit::gpu {
    export {
        template<typename Base>
        class STORMKIT_GPU_API PhysicalDeviceInterface final: public InstanceObject<Base> {
          public:
            using InstanceObject<Base>::InstanceObject;
            using InstanceObject<Base>::operator=;
            using TagType = PhysicalDeviceTag;

            [[nodiscard]]
            auto check_extension_support(string_view extension) const noexcept -> bool;
            [[nodiscard]]
            auto check_extension_support(array_view<const string_view> extensions) const noexcept
              -> std::optional<hash_set<string_view>>;
            [[nodiscard]]
            auto check_extension_support(array_view<const czstring> extensions) const noexcept
              -> std::optional<hash_set<string_view>>;

            [[nodiscard]]
            auto info() const noexcept -> const PhysicalDeviceInfo&;
            [[nodiscard]]
            auto capabilities() const noexcept -> const RenderCapabilities&;
            [[nodiscard]]
            auto memory_types() const noexcept -> array_view<const MemoryPropertyFlag>;
            [[nodiscard]]
            auto queue_families() const noexcept -> array_view<const QueueFamily>;
            [[nodiscard]]
            auto extensions() const noexcept -> array_view<const string>;
            [[nodiscard]]
            auto formats_properties() const noexcept -> array_view<const std::pair<PixelFormat, FormatProperties>>;
        };

        struct InstanceInterfaceBase {
            struct CreateInfo {
                string_view application_name;
                u32         application_version     = vk::make_version(0, 0, 0);
                bool        enable_validation_layers = (STORMKIT_BUILD_TYPE == "DEBUG");
            };
        };

        template<typename Base>
        class InstanceInterface final: public Base, public InstanceInterfaceBase {
          public:
            using Base::Base;
            using Base::operator=;
            using TagType = InstanceTag;

            [[nodiscard]]
            auto extensions() const noexcept -> array_view<const string>;

            [[nodiscard]]
            auto physical_devices() const noexcept -> array_view<const PhysicalDevice>;
        };
    }

    class STORMKIT_GPU_API
      InstanceImplementation: public GpuObjectImplementation<InstanceTag, const InstanceInterfaceBase::CreateInfo&> {
      public:
        using CreateInfo = InstanceInterfaceBase::CreateInfo;

        explicit InstanceImplementation(PrivateTag) noexcept;
        ~InstanceImplementation() noexcept;

        InstanceImplementation(const InstanceImplementation&) noexcept                    = delete;
        auto operator=(const InstanceImplementation&) noexcept -> InstanceImplementation& = delete;

        InstanceImplementation(InstanceImplementation&&) noexcept;
        auto operator=(InstanceImplementation&&) noexcept -> InstanceImplementation&;

        auto do_init(PrivateTag, const CreateInfo&) noexcept -> Expected<void>;

      protected:
        dyn_array<string>         m_extensions;
        dyn_array<PhysicalDevice> m_physical_devices;

        friend class view::InstanceImplementation;

      private:
        auto do_load_instance() noexcept -> Expected<void>;
        auto do_retrieve_physical_devices() noexcept -> Expected<void>;
    };

    namespace view {
        class InstanceImplementation: public GpuObjectViewImplementation<InstanceTag> {
          public:
            InstanceImplementation(const gpu::Instance&) noexcept;
            template<cmeta::IsContainerOrPointerOf<gpu::Instance> TContainerOrPointer>
            InstanceImplementation(const TContainerOrPointer&) noexcept;
            InstanceImplementation(const gpu::InstanceImplementation&) noexcept;
            ~InstanceImplementation() noexcept;

            InstanceImplementation(const InstanceImplementation&) noexcept;
            auto operator=(const InstanceImplementation&) noexcept -> InstanceImplementation&;

            InstanceImplementation(InstanceImplementation&&) noexcept;
            auto operator=(InstanceImplementation&&) noexcept -> InstanceImplementation&;

          protected:
            array_view<const string>              m_extensions;
            array_view<const gpu::PhysicalDevice> m_physical_devices;
        };
    } // namespace view

    class STORMKIT_GPU_API PhysicalDeviceImplementation: public GpuObjectImplementation<PhysicalDeviceTag, VkPhysicalDevice&&> {
      public:
        struct Data {
            PhysicalDeviceInfo device_info;
            RenderCapabilities capabilities;
        };

        PhysicalDeviceImplementation(PrivateTag, view::Instance&&) noexcept;
        ~PhysicalDeviceImplementation() noexcept;

        PhysicalDeviceImplementation(const PhysicalDeviceImplementation&) noexcept                    = delete;
        auto operator=(const PhysicalDeviceImplementation&) noexcept -> PhysicalDeviceImplementation& = delete;

        PhysicalDeviceImplementation(PhysicalDeviceImplementation&&) noexcept;
        auto operator=(PhysicalDeviceImplementation&&) noexcept -> PhysicalDeviceImplementation&;

        auto do_init(PrivateTag, VkPhysicalDevice&&) noexcept -> void;

      protected:
        Heap<Data>                                          m_data;
        dyn_array<MemoryPropertyFlag>                       m_memory_types;
        dyn_array<QueueFamily>                              m_queue_families;
        dyn_array<string>                                   m_extensions;
        dyn_array<std::pair<PixelFormat, FormatProperties>> m_format_properties;

        friend class InstanceInterface<InstanceImplementation>;
        friend class view::PhysicalDeviceImplementation;
    };

    namespace view {
        class PhysicalDeviceImplementation: public GpuObjectViewImplementation<PhysicalDeviceTag> {
          public:
            PhysicalDeviceImplementation(const gpu::PhysicalDevice&) noexcept;
            template<cmeta::IsContainerOrPointerOf<gpu::PhysicalDevice> TContainerOrPointer>
            PhysicalDeviceImplementation(const TContainerOrPointer&) noexcept;
            ~PhysicalDeviceImplementation() noexcept;

            PhysicalDeviceImplementation(const PhysicalDeviceImplementation&) noexcept;
            auto operator=(const PhysicalDeviceImplementation&) noexcept -> PhysicalDeviceImplementation&;

            PhysicalDeviceImplementation(PhysicalDeviceImplementation&&) noexcept;
            auto operator=(PhysicalDeviceImplementation&&) noexcept -> PhysicalDeviceImplementation&;

          protected:
            ref<const gpu::PhysicalDeviceImplementation::Data>         m_data;
            array_view<const MemoryPropertyFlag>                       m_memory_types;
            array_view<const QueueFamily>                              m_queue_families;
            array_view<const string>                                   m_extensions;
            array_view<const std::pair<PixelFormat, FormatProperties>> m_format_properties;
        };
    } // namespace view

    export {
        [[nodiscard]]
        STORMKIT_GPU_API auto score_physical_device(view::PhysicalDevice physical_device) noexcept -> u64;

        template<typename FormatContext>
        auto format_as(view::PhysicalDevice physical_device, FormatContext& ctx) noexcept -> decltype(ctx.out());
    }
} // namespace stormkit::gpu

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::gpu {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Interface>
    STORMKIT_FORCE_INLINE
    inline auto InstanceObject<Interface>::instance() const noexcept -> view::Instance {
        return Interface::owner();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto PhysicalDeviceInterface<Base>::check_extension_support(string_view extension) const noexcept -> bool {
        return stdr::any_of(extensions(), [extension](const auto& e) { return e == extension; });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    inline auto PhysicalDeviceInterface<Base>::check_extension_support(array_view<const string_view> extensions) const noexcept
      -> std::optional<hash_set<string_view>> {
        auto required_extensions = hash_set<string_view> { stdr::begin(extensions), stdr::end(extensions) };

        for (const auto& extension : this->extensions()) required_extensions.erase(extension);

        if (not required_extensions.empty()) return required_extensions;

        return std::nullopt;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    inline auto PhysicalDeviceInterface<Base>::check_extension_support(array_view<const czstring> extensions) const noexcept
      -> std::optional<hash_set<string_view>> {
        const auto ext = transform(extensions, cmonadic::init<string_view>());
        return check_extension_support(ext);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto PhysicalDeviceInterface<Base>::info() const noexcept -> const PhysicalDeviceInfo& {
        return Base::m_data->device_info;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto PhysicalDeviceInterface<Base>::capabilities() const noexcept -> const RenderCapabilities& {
        return Base::m_data->capabilities;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto PhysicalDeviceInterface<Base>::memory_types() const noexcept -> array_view<const MemoryPropertyFlag> {
        return Base::m_memory_types;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto PhysicalDeviceInterface<Base>::queue_families() const noexcept -> array_view<const QueueFamily> {
        return Base::m_queue_families;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto PhysicalDeviceInterface<Base>::extensions() const noexcept -> array_view<const string> {
        return Base::m_extensions;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto PhysicalDeviceInterface<Base>::formats_properties() const noexcept
      -> array_view<const std::pair<PixelFormat, FormatProperties>> {
        return Base::m_format_properties;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto InstanceInterface<Base>::extensions() const noexcept -> array_view<const string> {
        return Base::m_extensions;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto InstanceInterface<Base>::physical_devices() const noexcept -> array_view<const PhysicalDevice> {
        return Base::m_physical_devices;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline InstanceImplementation::InstanceImplementation(PrivateTag) noexcept
        : GpuObjectImplementation { auto(vkDestroyInstance) } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline InstanceImplementation::~InstanceImplementation() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline InstanceImplementation::InstanceImplementation(InstanceImplementation&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto InstanceImplementation::operator=(InstanceImplementation&&) noexcept -> InstanceImplementation& = default;

    namespace view {
        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline InstanceImplementation::InstanceImplementation(const gpu::Instance& of) noexcept
            : GpuObjectViewImplementation { of }, m_extensions { of.extensions() }, m_physical_devices { of.physical_devices() } {
        }

        /////////////////////////////////////
        /////////////////////////////////////
        template<cmeta::IsContainerOrPointerOf<gpu::Instance> TContainerOrPointer>
        STORMKIT_FORCE_INLINE
        inline InstanceImplementation::InstanceImplementation(const TContainerOrPointer& of) noexcept
            : InstanceImplementation { *of } {
        }

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline InstanceImplementation::InstanceImplementation(const gpu::InstanceImplementation& of) noexcept
            : GpuObjectViewImplementation { of }, m_extensions { of.m_extensions }, m_physical_devices { of.m_physical_devices } {
        }

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline InstanceImplementation::~InstanceImplementation() noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline InstanceImplementation::InstanceImplementation(const InstanceImplementation&) noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto InstanceImplementation::operator=(const InstanceImplementation&) noexcept
          -> InstanceImplementation& = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline InstanceImplementation::InstanceImplementation(InstanceImplementation&&) noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto InstanceImplementation::operator=(InstanceImplementation&&) noexcept -> InstanceImplementation& = default;
    } // namespace view

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline PhysicalDeviceImplementation::PhysicalDeviceImplementation(PrivateTag, view::Instance&& instance) noexcept
        : GpuObjectImplementation { std::move(instance), monadic::noop() } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline PhysicalDeviceImplementation::~PhysicalDeviceImplementation() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline PhysicalDeviceImplementation::PhysicalDeviceImplementation(PhysicalDeviceImplementation&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto PhysicalDeviceImplementation::operator=(PhysicalDeviceImplementation&&) noexcept
      -> PhysicalDeviceImplementation& = default;

    namespace view {
        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline PhysicalDeviceImplementation::PhysicalDeviceImplementation(const gpu::PhysicalDevice& of) noexcept
            : GpuObjectViewImplementation { of },
              m_data { as_ref(of.m_data) },
              m_memory_types { of.m_memory_types },
              m_queue_families { of.m_queue_families },
              m_extensions { of.m_extensions },
              m_format_properties { of.m_format_properties } {
        }

        /////////////////////////////////////
        /////////////////////////////////////
        template<cmeta::IsContainerOrPointerOf<gpu::PhysicalDevice> TContainerOrPointer>
        STORMKIT_FORCE_INLINE
        inline PhysicalDeviceImplementation::PhysicalDeviceImplementation(const TContainerOrPointer& of) noexcept
            : PhysicalDeviceImplementation { *of } {
        }

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline PhysicalDeviceImplementation::~PhysicalDeviceImplementation() noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline PhysicalDeviceImplementation::PhysicalDeviceImplementation(const PhysicalDeviceImplementation&) noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto PhysicalDeviceImplementation::operator=(const PhysicalDeviceImplementation& other) noexcept
          -> PhysicalDeviceImplementation& {
            if (&other == this) [[unlikely]]
                return *this;

            GpuObjectViewImplementation::operator=(other);

            m_data              = as_ref(other.m_data);
            m_memory_types      = other.m_memory_types;
            m_queue_families    = other.m_queue_families;
            m_extensions        = other.m_extensions;
            m_format_properties = other.m_format_properties;

            return *this;
        }

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline PhysicalDeviceImplementation::PhysicalDeviceImplementation(PhysicalDeviceImplementation&&) noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto PhysicalDeviceImplementation::operator=(PhysicalDeviceImplementation&&) noexcept
          -> PhysicalDeviceImplementation& = default;
    } // namespace view

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Interface>
    STORMKIT_FORCE_INLINE
    inline auto PhysicalDeviceObject<Interface>::instance() const noexcept -> view::Instance {
        return Interface::owner().instance();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Interface>
    STORMKIT_FORCE_INLINE
    inline auto PhysicalDeviceObject<Interface>::physical_device() const noexcept -> view::PhysicalDevice {
        return Interface::owner();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename FormatContext>
    inline auto format_as(view::PhysicalDevice physical_device, FormatContext& ctx) noexcept -> decltype(ctx.out()) {
        const auto& info = physical_device.info();
        return std::format_to(ctx.out(),
                              "PhysicalDevice[name: {}, vendor: {}, id: {}, vulkan: {}.{}.{}, driver version: "
                              "{}.{}.{}]",
                              info.device_name,
                              info.vendor_name,
                              info.device_id,
                              info.api_major_version,
                              info.api_minor_version,
                              info.api_patch_version,
                              info.driver_major_version,
                              info.driver_minor_version,
                              info.driver_patch_version);
    }
} // namespace stormkit::gpu
