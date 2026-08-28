// Copyright (C) 2023 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>
#include <stormkit/core/try_expected.hpp>

#include <stormkit/gpu/api.hpp>
#include <stormkit/gpu/vulkan.hpp>

export module stormkit.gpu.execution:descriptors;

import std;

import stormkit.core;
import stormkit.gpu.core;
import stormkit.gpu.resource;

import :objects;

namespace stdr = std::ranges;
namespace stdv = std::views;

namespace cmonadic = stormkit::core::monadic;
namespace cmeta    = stormkit::core::meta;

namespace stormkit::gpu {
    export {
        struct BufferDescriptor {
            DescriptorType     type = DescriptorType::UNIFORM_BUFFER;
            u32                binding;
            view::Buffer       buffer;
            std::optional<u32> range  = std::nullopt;
            u32                offset = 0;
        };

        struct ImageDescriptor {
            DescriptorType  type = DescriptorType::COMBINED_IMAGE_SAMPLER;
            u32             binding;
            ImageLayout     layout;
            view::ImageView image_view;
            view::Sampler   sampler;
        };

        using Descriptor = std::variant<BufferDescriptor, ImageDescriptor>;

        struct DescriptorSetLayoutBinding {
            u32             binding;
            DescriptorType  type;
            ShaderStageFlag stages;
            usize           descriptor_count;
        };

        template<typename Base>
        class DescriptorSetInterface final: public DeviceObject<Base> {
          public:
            using DeviceObject<Base>::DeviceObject;
            using DeviceObject<Base>::operator=;
            using TagType = DescriptorSetTag;

            auto update(array_view<const Descriptor> descriptors) const noexcept -> void;
        };
    }

    using DescriptorSetDeleter = std::function<void(VkDescriptorSet)>;

    struct DescriptorSetLayoutInterfaceBase {
        struct Size {
            DescriptorType type;
            u32            descriptor_count;
        };
    };

    export {
        template<typename Base>
        class DescriptorSetLayoutInterface final: public DeviceObject<Base>, public DescriptorSetLayoutInterfaceBase {
          public:
            using DeviceObject<Base>::DeviceObject;
            using DeviceObject<Base>::operator=;
            using TagType = DescriptorSetLayoutTag;

            using DescriptorSetLayoutInterfaceBase::Size;

            [[nodiscard]]
            auto bindings() const noexcept -> array_view<const DescriptorSetLayoutBinding>;
        };

        template<typename Base>
        class DescriptorPoolInterface final: public DeviceObject<Base> {
          public:
            using DeviceObject<Base>::DeviceObject;
            using DeviceObject<Base>::operator=;
            using TagType = DescriptorPoolTag;

            auto create_descriptor_set(this const auto&, view::DescriptorSetLayout layout) noexcept -> Expected<DescriptorSet>;
            auto create_descriptor_sets(this const auto&, usize count, view::DescriptorSetLayout layout) noexcept
              -> Expected<dyn_array<DescriptorSet>>;

            auto allocate_descriptor_set(this const auto&, view::DescriptorSetLayout layout) noexcept
              -> Expected<Heap<DescriptorSet>>;
            auto allocate_descriptor_sets(this const auto&, usize count, view::DescriptorSetLayout layout) noexcept
              -> Expected<dyn_array<Heap<DescriptorSet>>>;

          private:
            auto create_vk_descriptor_sets(usize, view::DescriptorSetLayout&&) const noexcept
              -> Expected<dyn_array<VkDescriptorSet>>;

            static auto delete_vk_descriptor_set(view::Device, view::DescriptorPool, VkDescriptorSet) noexcept -> void;
        };
    }

    class STORMKIT_GPU_API
      DescriptorSetImplementation: public GpuObjectImplementation<DescriptorSetTag, VkDescriptorSet&&, DescriptorSetDeleter&&> {
      public:
        DescriptorSetImplementation(PrivateTag, view::Device&&) noexcept;
        ~DescriptorSetImplementation() noexcept;

        DescriptorSetImplementation(const DescriptorSetImplementation&)                    = delete;
        auto operator=(const DescriptorSetImplementation&) -> DescriptorSetImplementation& = delete;

        DescriptorSetImplementation(DescriptorSetImplementation&&) noexcept;
        auto operator=(DescriptorSetImplementation&&) noexcept -> DescriptorSetImplementation&;

        auto do_init(PrivateTag, VkDescriptorSet&&, DescriptorSetDeleter&&) noexcept -> Expected<void>;

      protected:
        using NamedConstructor::allocate;
        using NamedConstructor::create;

        DescriptorSetDeleter m_deleter;

        friend class DescriptorPoolInterface<DescriptorPoolImplementation>;
        friend class DescriptorPoolInterface<view::DescriptorPoolImplementation>;
    };

    namespace view {
        class DescriptorSetImplementation: public GpuObjectViewImplementation<DescriptorSetTag> {
          public:
            using GpuObjectViewImplementation<DescriptorSetTag>::GpuObjectViewImplementation;
            using GpuObjectViewImplementation<DescriptorSetTag>::operator=;
        };
    } // namespace view

    class STORMKIT_GPU_API DescriptorSetLayoutImplementation
        : public GpuObjectImplementation<DescriptorSetLayoutTag, dyn_array<DescriptorSetLayoutBinding>> {
      public:
        DescriptorSetLayoutImplementation(PrivateTag, view::Device&&) noexcept;
        ~DescriptorSetLayoutImplementation() noexcept;

        DescriptorSetLayoutImplementation(const DescriptorSetLayoutImplementation&)                    = delete;
        auto operator=(const DescriptorSetLayoutImplementation&) -> DescriptorSetLayoutImplementation& = delete;

        DescriptorSetLayoutImplementation(DescriptorSetLayoutImplementation&&) noexcept;
        auto operator=(DescriptorSetLayoutImplementation&&) noexcept -> DescriptorSetLayoutImplementation&;

        auto do_init(PrivateTag, dyn_array<DescriptorSetLayoutBinding>&&) noexcept -> Expected<void>;

      protected:
        dyn_array<DescriptorSetLayoutBinding> m_bindings;
    };

    namespace view {
        class DescriptorSetLayoutImplementation: public GpuObjectViewImplementation<DescriptorSetLayoutTag> {
          public:
            DescriptorSetLayoutImplementation(const gpu::DescriptorSetLayout& of) noexcept;
            template<cmeta::IsContainerOrPointerOf<gpu::DescriptorSetLayout> TContainerOrPointer>
            DescriptorSetLayoutImplementation(const TContainerOrPointer&) noexcept;
            ~DescriptorSetLayoutImplementation() noexcept;

            DescriptorSetLayoutImplementation(const DescriptorSetLayoutImplementation&) noexcept;
            auto operator=(const DescriptorSetLayoutImplementation&) noexcept -> DescriptorSetLayoutImplementation&;

            DescriptorSetLayoutImplementation(DescriptorSetLayoutImplementation&&) noexcept;
            auto operator=(DescriptorSetLayoutImplementation&&) noexcept -> DescriptorSetLayoutImplementation&;

          protected:
            array_view<const DescriptorSetLayoutBinding> m_bindings;
        };
    } // namespace view

    class STORMKIT_GPU_API DescriptorPoolImplementation
        : public GpuObjectImplementation<DescriptorPoolTag, array_view<const DescriptorSetLayoutInterfaceBase::Size>, u32> {
      public:
        using Size = DescriptorSetLayoutInterfaceBase::Size;

        DescriptorPoolImplementation(PrivateTag, view::Device&&) noexcept;
        ~DescriptorPoolImplementation() noexcept;

        DescriptorPoolImplementation(const DescriptorPoolImplementation&)                    = delete;
        auto operator=(const DescriptorPoolImplementation&) -> DescriptorPoolImplementation& = delete;

        DescriptorPoolImplementation(DescriptorPoolImplementation&&) noexcept;
        auto operator=(DescriptorPoolImplementation&&) noexcept -> DescriptorPoolImplementation&;

        auto do_init(PrivateTag, array_view<const Size>&&, u32) noexcept -> Expected<void>;
    };

    namespace view {
        class DescriptorPoolImplementation: public GpuObjectViewImplementation<DescriptorPoolTag> {
          public:
            using GpuObjectViewImplementation<DescriptorPoolTag>::GpuObjectViewImplementation;
            using GpuObjectViewImplementation<DescriptorPoolTag>::operator=;
        };
    } // namespace view

    template<core::meta::HashType Ret = hash32>
    constexpr auto hasher(view::DescriptorSetLayout value) noexcept -> Ret;
    template<core::meta::HashType Ret = hash32>
    constexpr auto hasher(const DescriptorSetLayoutBinding& value) noexcept -> Ret;
    template<core::meta::HashType Ret = hash32>
    constexpr auto hasher(const BufferDescriptor& value) noexcept -> Ret;
    template<core::meta::HashType Ret = hash32>
    constexpr auto hasher(const ImageDescriptor& value) noexcept -> Ret;
    template<core::meta::HashType Ret = hash32>
    constexpr auto hasher(const Descriptor& value) noexcept -> Ret;
} // namespace stormkit::gpu

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::gpu {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto DescriptorSetLayoutInterface<Base>::bindings() const noexcept -> array_view<const DescriptorSetLayoutBinding> {
        return Base::m_bindings;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto DescriptorPoolInterface<Base>::create_descriptor_set(this const auto&          self,
                                                                     view::DescriptorSetLayout layout) noexcept
      -> Expected<DescriptorSet> {
        auto   device    = self.owner();
        auto   vk_handle = Try(self.create_vk_descriptor_sets(1, std::move(layout))).front();
        Return DescriptorSet::create(device,
                                     std::move(vk_handle),
                                     bind_front(self.delete_vk_descriptor_set, std::move(device), gpu::as_view(self)));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto DescriptorPoolInterface<Base>::create_descriptor_sets(this const auto&          self,
                                                                      usize                     count,
                                                                      view::DescriptorSetLayout layout) noexcept
      -> Expected<dyn_array<DescriptorSet>> {
        auto   device = self.owner();
        Return transform(Try(self.create_vk_descriptor_sets(count, std::move(layout))), [&self, device](auto vk_handle) noexcept {
            return DescriptorSet::create(device,
                                         std::move(vk_handle),
                                         bind_front(self.delete_vk_descriptor_set, std::move(device), gpu::as_view(self)));
        });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto DescriptorPoolInterface<Base>::allocate_descriptor_set(this const auto&          self,
                                                                       view::DescriptorSetLayout layout) noexcept
      -> Expected<Heap<DescriptorSet>> {
        auto   device    = self.owner();
        auto   vk_handle = Try(self.create_vk_descriptor_sets(1, std::move(layout))).front();
        Return DescriptorSet::allocate(device,
                                       std::move(vk_handle),
                                       bind_front(self.delete_vk_descriptor_set, std::move(device), gpu::as_view(self)));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto DescriptorPoolInterface<Base>::allocate_descriptor_sets(this const auto&          self,
                                                                        usize                     count,
                                                                        view::DescriptorSetLayout layout) noexcept
      -> Expected<dyn_array<Heap<DescriptorSet>>> {
        auto   device = self.owner();
        Return transform(Try(self.create_vk_descriptor_sets(count, std::move(layout))), [&self, device](auto vk_handle) noexcept {
            return DescriptorSet::allocate(device,
                                           std::move(vk_handle),
                                           bind_front(self.delete_vk_descriptor_set, std::move(device), gpu::as_view(self)));
        });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline DescriptorSetLayoutImplementation::DescriptorSetLayoutImplementation(PrivateTag, view::Device&& device) noexcept
        : GpuObjectImplementation { std::move(device), &VolkDeviceTable::vkDestroyDescriptorSetLayout } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline DescriptorSetLayoutImplementation::~DescriptorSetLayoutImplementation() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline DescriptorSetLayoutImplementation::DescriptorSetLayoutImplementation(DescriptorSetLayoutImplementation&&
                                                                                  other) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto DescriptorSetLayoutImplementation::operator=(DescriptorSetLayoutImplementation&& other) noexcept
      -> DescriptorSetLayoutImplementation& = default;

    namespace view {
        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline DescriptorSetLayoutImplementation ::DescriptorSetLayoutImplementation(const gpu::DescriptorSetLayout& of) noexcept
            : GpuObjectViewImplementation { of }, m_bindings { of.bindings() } {
        }

        /////////////////////////////////////
        /////////////////////////////////////
        template<cmeta::IsContainerOrPointerOf<gpu::DescriptorSetLayout> TContainerOrPointer>
        STORMKIT_FORCE_INLINE
        inline DescriptorSetLayoutImplementation::DescriptorSetLayoutImplementation(const TContainerOrPointer& of) noexcept
            : DescriptorSetLayoutImplementation { *of } {
        }

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline DescriptorSetLayoutImplementation ::~DescriptorSetLayoutImplementation() noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline DescriptorSetLayoutImplementation ::
          DescriptorSetLayoutImplementation(const DescriptorSetLayoutImplementation&) noexcept = default;
        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto DescriptorSetLayoutImplementation ::operator=(const DescriptorSetLayoutImplementation&) noexcept
          -> DescriptorSetLayoutImplementation& = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline DescriptorSetLayoutImplementation ::
          DescriptorSetLayoutImplementation(DescriptorSetLayoutImplementation&&) noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto DescriptorSetLayoutImplementation ::operator=(DescriptorSetLayoutImplementation&&) noexcept
          -> DescriptorSetLayoutImplementation& = default;
    } // namespace view

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline DescriptorSetImplementation::DescriptorSetImplementation(PrivateTag, view::Device&& device) noexcept
        : GpuObjectImplementation { std::move(device), cmonadic::noop() } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline DescriptorSetImplementation::~DescriptorSetImplementation() noexcept {
        if (m_vk_handle != VK_NULL_HANDLE) {
            m_deleter(m_vk_handle);
            m_vk_handle = VK_NULL_HANDLE;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline DescriptorSetImplementation::DescriptorSetImplementation(DescriptorSetImplementation&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto DescriptorSetImplementation::operator=(DescriptorSetImplementation&&) noexcept
      -> DescriptorSetImplementation& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline DescriptorPoolImplementation::DescriptorPoolImplementation(PrivateTag, view::Device&& device) noexcept
        : GpuObjectImplementation { std::move(device), &VolkDeviceTable::vkDestroyDescriptorPool } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline DescriptorPoolImplementation::~DescriptorPoolImplementation() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline DescriptorPoolImplementation::DescriptorPoolImplementation(DescriptorPoolImplementation&& other) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto DescriptorPoolImplementation::operator=(DescriptorPoolImplementation&& other) noexcept
      -> DescriptorPoolImplementation& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<core::meta::HashType Ret>
    STORMKIT_FORCE_INLINE
    constexpr auto hasher(view::DescriptorSetLayout value) noexcept -> Ret {
        auto out = Ret {};
        for (const auto binding : value.bindings()) out = hash_combine(out, binding);
        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<core::meta::HashType Ret>
    STORMKIT_FORCE_INLINE
    constexpr auto hasher(const DescriptorSetLayoutBinding& value) noexcept -> Ret {
        return hash<Ret>(value.binding, value.type, value.stages, value.descriptor_count);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<core::meta::HashType Ret>
    STORMKIT_FORCE_INLINE
    constexpr auto hasher(const BufferDescriptor& value) noexcept -> Ret {
        return hash<Ret>(value.type, value.binding, value.buffer, value.range, value.offset);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<core::meta::HashType Ret>
    STORMKIT_FORCE_INLINE
    constexpr auto hasher(const ImageDescriptor& value) noexcept -> Ret {
        return hash<Ret>(value.type, value.binding, value.layout, value.image_view, value.sampler);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<core::meta::HashType Ret>
    STORMKIT_FORCE_INLINE
    constexpr auto hasher(const Descriptor& value) noexcept -> Ret {
        return std::visit([](const auto& descriptor) static noexcept { return hash<Ret>(descriptor); }, value);
    }
} // namespace stormkit::gpu
