// Copyright (C) 2023 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>
#include <stormkit/core/try_expected.hpp>

#include <stormkit/gpu/api.hpp>
#include <stormkit/gpu/vulkan.hpp>

export module stormkit.gpu.resource:buffer;

import std;

import stormkit.core;
import stormkit.gpu.core;

import :objects;

namespace stdr = std::ranges;

namespace cmeta    = stormkit::core::meta;
namespace cmonadic = stormkit::core::monadic;

namespace stormkit::gpu {
    struct BufferInterfaceBase {
        struct CreateInfo {
            BufferUsageFlag    usages;
            usize              size;
            MemoryPropertyFlag properties = MemoryPropertyFlag::HOST_VISIBLE | MemoryPropertyFlag::HOST_COHERENT;

            bool persistently_mapped = false;
        };
    };

    export template<typename Base>
    class STORMKIT_GPU_API BufferInterface final: public DeviceObject<Base>, public BufferInterfaceBase {
      public:
        using DeviceObject<Base>::DeviceObject;
        using DeviceObject<Base>::operator=;
        using TagType = BufferTag;

        BufferInterface(const BufferInterface&) noexcept
            requires(cmeta::IsCopyConstructible<DeviceObject<Base>>);
        auto operator=(const BufferInterface&) noexcept -> BufferInterface&
            requires(cmeta::IsCopyAssignable<DeviceObject<Base>>);

        BufferInterface(BufferInterface&&) noexcept;
        auto operator=(BufferInterface&&) noexcept -> BufferInterface&;
        ~BufferInterface() noexcept;

        [[nodiscard]]
        auto usages() const noexcept -> BufferUsageFlag;
        [[nodiscard]]
        auto size() const noexcept -> usize;
        [[nodiscard]]
        auto memory_properties() const noexcept -> MemoryPropertyFlag;
        [[nodiscard]]
        auto is_persistently_mapped() const noexcept -> bool;

        auto map(ioffset offset) noexcept -> Expected<byte*>;
        auto map(ioffset offset, usize size) noexcept -> Expected<byte_mut_view<>>;

        template<typename T>
        auto map_as(ioffset offset) noexcept -> Expected<ref<T>>;

        bool mapped() const noexcept;

        template<typename Self>
        [[nodiscard]]
        auto data(this Self& self) noexcept -> cmeta::ForwardConst<Self, byte>*;
        template<typename Self>
        [[nodiscard]]
        auto data(this Self& self, usize size) noexcept -> cmeta::If<cmeta::IsConst<Self>, byte_mut_view<>, byte_view<>>;

        template<typename T>
        [[nodiscard]]
        auto data_as(this auto& self) noexcept -> ref<T>;

        auto flush(ioffset offset, usize size) const noexcept -> Expected<void>;
        auto unmap() noexcept -> void;

        auto upload(byte_view<> data, ioffset offset = 0) noexcept -> Expected<void>;

        template<typename T>
            requires(not stormkit::meta::IsStdSpan<T>)
        auto upload(const T& data, ioffset offset = 0) noexcept -> Expected<void>;

        [[nodiscard]]
        auto allocation() const noexcept -> vk::Observer<VmaAllocation>;
    };

    class STORMKIT_GPU_API
      BufferImplementation: public GpuObjectImplementation<BufferTag, const BufferInterfaceBase::CreateInfo&> {
      public:
        using CreateInfo = BufferInterfaceBase::CreateInfo;

        BufferImplementation(PrivateTag, view::Device&&) noexcept;
        ~BufferImplementation() noexcept;

        BufferImplementation(const BufferImplementation&) noexcept                    = delete;
        auto operator=(const BufferImplementation&) noexcept -> BufferImplementation& = delete;

        BufferImplementation(BufferImplementation&&) noexcept;
        auto operator=(BufferImplementation&&) noexcept -> BufferImplementation&;

        auto do_init(PrivateTag, const CreateInfo&) noexcept -> Expected<void>;

      protected:
        BufferUsageFlag    m_usages            = {};
        usize              m_size              = 0;
        MemoryPropertyFlag m_memory_properties = {};

        bool  m_is_persistently_mapped = false;
        byte* m_mapped_pointer         = nullptr;

        vk::Owned<VmaAllocation> m_vma_allocation = { cmonadic::discard() };
    };

    namespace view {
        class BufferImplementation: public GpuObjectViewImplementation<BufferTag> {
          public:
            BufferImplementation(const gpu::Buffer&) noexcept;
            template<cmeta::IsContainerOrPointerOf<gpu::Buffer> TContainerOrPointer>
            BufferImplementation(const TContainerOrPointer&) noexcept;
            ~BufferImplementation() noexcept;

            BufferImplementation(const BufferImplementation&) noexcept;
            auto operator=(const BufferImplementation&) noexcept -> BufferImplementation&;

            BufferImplementation(BufferImplementation&&) noexcept;
            auto operator=(BufferImplementation&&) noexcept -> BufferImplementation&;

          protected:
            BufferUsageFlag    m_usages            = {};
            usize              m_size              = 0;
            MemoryPropertyFlag m_memory_properties = {};

            bool  m_is_persistently_mapped = false;
            byte* m_mapped_pointer         = nullptr;

            vk::Observer<VmaAllocation> m_vma_allocation;
        };
    } // namespace view

    export {
        struct BufferMemoryBarrier {
            AccessFlag src;
            AccessFlag dst;

            u32 src_queue_family_index = QUEUE_FAMILY_IGNORED;
            u32 dst_queue_family_index = QUEUE_FAMILY_IGNORED;

            view::Buffer buffer;
            usize        size;
            u64          offset = 0;
        };

        template<core::meta::HashType Ret = hash32>
        constexpr auto hasher(const Buffer::CreateInfo& value) noexcept -> Ret;
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
    inline BufferInterface<Base>::BufferInterface(const BufferInterface& other) noexcept
        requires(cmeta::IsCopyConstructible<DeviceObject<Base>>)
        : DeviceObject<Base> { other } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
        STORMKIT_FORCE_INLINE
    inline auto BufferInterface<Base>::operator=(const BufferInterface& other) noexcept -> BufferInterface&
        requires(cmeta::IsCopyAssignable<DeviceObject<Base>>)
    = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline BufferInterface<Base>::BufferInterface(BufferInterface&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
        STORMKIT_FORCE_INLINE
    inline auto BufferInterface<Base>::operator=(BufferInterface&&) noexcept -> BufferInterface& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline BufferInterface<Base>::~BufferInterface() noexcept {
        if (allocation()) unmap();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto BufferInterface<Base>::usages() const noexcept -> BufferUsageFlag {
        EXPECTS(Base::m_vma_allocation and Base::m_vk_handle);
        return Base::m_usages;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto BufferInterface<Base>::size() const noexcept -> usize {
        EXPECTS(Base::m_vma_allocation and Base::m_vk_handle);
        return Base::m_size;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto BufferInterface<Base>::memory_properties() const noexcept -> MemoryPropertyFlag {
        return Base::m_memory_properties;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto BufferInterface<Base>::is_persistently_mapped() const noexcept -> bool {
        return Base::m_is_persistently_mapped;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto BufferInterface<Base>::map(ioffset offset, usize size) noexcept -> Expected<byte_mut_view<>> {
        auto   ptr = Try(map(offset));
        Return as_bytes_mut(ptr, size);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline auto BufferInterface<Base>::map_as(ioffset offset) noexcept -> Expected<ref<T>> {
        const auto ptr = Try(map(offset));
        Return     from_bytes_mut<T>(ptr);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto BufferInterface<Base>::mapped() const noexcept -> bool {
        return Base::m_mapped_pointer != nullptr;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    inline auto BufferInterface<Base>::data(this Self& self) noexcept -> cmeta::ForwardConst<Self, byte>* {
        EXPECTS(self.m_vma_allocation and self.m_vk_handle);
        EXPECTS(self.m_mapped_pointer);

        using Out = cmeta::ForwardConst<decltype(self), byte>*;
        return std::bit_cast<Out>(self.m_mapped_pointer);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    inline auto BufferInterface<Base>::data(this Self& self, usize size) noexcept
      -> cmeta::If<cmeta::IsConst<Self>, byte_mut_view<>, byte_view<>> {
        using Out = array_view<cmeta::ForwardConst<Self, byte>>;
        return Out { std::bit_cast<typename Out::element_type>(self.data()), size };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    template<typename T, typename Self>
    STORMKIT_FORCE_INLINE
    inline auto BufferInterface<Base>::data_as(this Self& self) noexcept -> ref<T> {
        using Type = cmeta::ForwardConst<decltype(self), T>*;
        return as_ref_like<Self>(std::bit_cast<Type>(self.data()));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    template<typename T>
        requires(not stormkit::meta::IsStdSpan<T>)
    STORMKIT_FORCE_INLINE
    inline auto BufferInterface<Base>::upload(const T& data, ioffset offset) noexcept -> Expected<void> {
        const auto bytes = as_bytes(data);
        return upload(bytes, offset);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto BufferInterface<Base>::allocation() const noexcept -> vk::Observer<VmaAllocation> {
        return Base::m_vma_allocation;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline BufferImplementation::BufferImplementation(PrivateTag, view::Device&& device) noexcept
        : GpuObjectImplementation { std::move(device), &VolkDeviceTable::vkDestroyBuffer } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline BufferImplementation::~BufferImplementation() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline BufferImplementation::BufferImplementation(BufferImplementation&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto BufferImplementation::operator=(BufferImplementation&&) noexcept -> BufferImplementation& = default;

    namespace view {
        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline BufferImplementation::BufferImplementation(const gpu::Buffer& of) noexcept
            : GpuObjectViewImplementation { of },
              m_usages { of.usages() },
              m_size { of.size() },
              m_memory_properties { of.memory_properties() },
              m_is_persistently_mapped { of.is_persistently_mapped() },
              m_mapped_pointer { nullptr },
              m_vma_allocation { of.allocation() } {
            if (of.is_persistently_mapped()) m_mapped_pointer = std::bit_cast<byte*>(of.data());
        }

        /////////////////////////////////////
        /////////////////////////////////////
        template<cmeta::IsContainerOrPointerOf<gpu::Buffer> TContainerOrPointer>
        STORMKIT_FORCE_INLINE
        inline BufferImplementation::BufferImplementation(const TContainerOrPointer& of) noexcept
            : BufferImplementation { *of } {
        }

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline BufferImplementation::~BufferImplementation() noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline BufferImplementation::BufferImplementation(const BufferImplementation&) noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto BufferImplementation::operator=(const BufferImplementation&) noexcept -> BufferImplementation& = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline BufferImplementation::BufferImplementation(BufferImplementation&&) noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto BufferImplementation::operator=(BufferImplementation&&) noexcept -> BufferImplementation& = default;
    } // namespace view

    ///////////////////////////////////
    ///////////////////////////////////
    template<cmeta::HashType Ret = hash32>
    constexpr auto hasher(const Buffer::CreateInfo& create_info) noexcept -> Ret {
        return hash(create_info.usages, create_info.size, create_info.properties);
    }
} // namespace stormkit::gpu
