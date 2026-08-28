// Copyright (C) 2023 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/try_expected.hpp>

#include <stormkit/gpu/api.hpp>
#include <stormkit/gpu/vulkan.hpp>

export module stormkit.gpu.core:base;

import std;

import stormkit.core;

import :meta;
import :structs;
import :vulkan;

namespace stdr = std::ranges;
namespace stdv = std::views;

namespace cmeta = stormkit::core::meta;

export namespace stormkit::gpu {
    STORMKIT_GPU_API
    auto initialize_backend() -> Expected<void>;

    template<meta::GpuObjectHasTraitDefined>
    class GpuObjectViewImplementation;

    template<meta::GpuObjectHasTraitDefined Tag>
    class GpuObjectBase {
      public:
        using TagType    = Tag;
        using TraitType  = trait::GpuObject<TagType>;
        using ValueType  = TraitType::ValueType;
        using ObjectType = TraitType::ObjectType;
        using ViewType   = TraitType::ViewType;

        ~GpuObjectBase() noexcept;

        GpuObjectBase(const GpuObjectBase&) noexcept;
        auto operator=(const GpuObjectBase&) noexcept -> GpuObjectBase&;

        GpuObjectBase(GpuObjectBase&&) noexcept;
        auto operator=(GpuObjectBase&&) noexcept -> GpuObjectBase&;

        [[nodiscard]]
        auto native_handle() const noexcept -> ValueType;

        [[nodiscard]]
        operator ValueType() const noexcept;

      protected:
        GpuObjectBase() noexcept;

        ValueType m_vk_handle = VK_NULL_HANDLE;

        friend class GpuObjectViewImplementation<Tag>;
    };

    template<meta::GpuObjectHasTraitDefined Tag>
        requires(meta::HasOwnerType<Tag>)
    class GpuObjectBase<Tag> {
      public:
        using TagType       = Tag;
        using TraitType     = trait::GpuObject<TagType>;
        using ValueType     = TraitType::ValueType;
        using ObjectType    = TraitType::ObjectType;
        using ViewType      = TraitType::ViewType;
        using OwnerType     = TraitType::OwnerType;
        using OwnerViewType = typename OwnerType::ViewType;

        ~GpuObjectBase() noexcept;

        GpuObjectBase(const GpuObjectBase&) noexcept;
        auto operator=(const GpuObjectBase&) noexcept -> GpuObjectBase&;

        GpuObjectBase(GpuObjectBase&&) noexcept;
        auto operator=(GpuObjectBase&&) noexcept -> GpuObjectBase&;

        [[nodiscard]]
        auto native_handle() const noexcept -> ValueType;

        [[nodiscard]]
        operator ValueType() const noexcept;

        [[nodiscard]]
        auto owner() const noexcept -> OwnerViewType;

      protected:
        GpuObjectBase(OwnerViewType&&) noexcept;

        ValueType     m_vk_handle = VK_NULL_HANDLE;
        OwnerViewType m_owner;

        friend class GpuObjectViewImplementation<Tag>;
    };

    template<typename...>
    class GpuObjectImplementation;

    template<meta::GpuObjectHasTraitDefined Tag>
    class GpuObjectViewImplementation: public GpuObjectBase<Tag> {
        using Base = GpuObjectBase<Tag>;

      public:
        using TagType    = Base::TagType;
        using TraitType  = Base::TraitType;
        using ValueType  = Base::ValueType;
        using ObjectType = Base::ObjectType;
        using ViewType   = Base::ViewType;

        GpuObjectViewImplementation(const cmeta::IsSpecializationOf<GpuObjectImplementation> auto&) noexcept;
        template<cmeta::IsContainerOrPointer TContainerOrPointer>
        GpuObjectViewImplementation(const TContainerOrPointer&) noexcept;
        ~GpuObjectViewImplementation() noexcept;

        GpuObjectViewImplementation(const GpuObjectViewImplementation&) noexcept;
        auto operator=(const GpuObjectViewImplementation&) noexcept -> GpuObjectViewImplementation&;

        GpuObjectViewImplementation(GpuObjectViewImplementation&&) noexcept;
        auto operator=(GpuObjectViewImplementation&&) noexcept -> GpuObjectViewImplementation&;
    };

    template<meta::GpuObjectHasTraitDefined Tag>
        requires(meta::HasOwnerType<Tag>)
    class GpuObjectViewImplementation<Tag>: public GpuObjectBase<Tag> {
        using Base = GpuObjectBase<Tag>;

      public:
        using TagType       = Base::TagType;
        using TraitType     = Base::TraitType;
        using ValueType     = Base::ValueType;
        using ObjectType    = Base::ObjectType;
        using ViewType      = Base::ViewType;
        using OwnerType     = Base::OwnerType;
        using OwnerViewType = Base::OwnerViewType;

        GpuObjectViewImplementation(const cmeta::IsSpecializationOf<GpuObjectImplementation> auto&) noexcept;
        template<cmeta::IsContainerOrPointer TContainerOrPointer>
        GpuObjectViewImplementation(const TContainerOrPointer&) noexcept;
        ~GpuObjectViewImplementation() noexcept;

        GpuObjectViewImplementation(const GpuObjectViewImplementation&) noexcept;
        auto operator=(const GpuObjectViewImplementation&) noexcept -> GpuObjectViewImplementation&;

        GpuObjectViewImplementation(GpuObjectViewImplementation&&) noexcept;
        auto operator=(GpuObjectViewImplementation&&) noexcept -> GpuObjectViewImplementation&;
    };

    template<typename Tag, typename... TDoInitArgs>
        requires(meta::GpuObjectHasTraitDefined<Tag>)
    class GpuObjectImplementation<Tag, TDoInitArgs...>
        : public GpuObjectBase<Tag>,
          public core::NamedConstructor<typename trait::GpuObject<Tag>::ObjectType, DoInitArgs<TDoInitArgs...>> {
        using Base = GpuObjectBase<Tag>;

      protected:
        using NamedConstructorBase = core::NamedConstructor<typename trait::GpuObject<Tag>::ObjectType,
                                                            DoInitArgs<TDoInitArgs...>>;

      public:
        using TagType     = Base::TagType;
        using TraitType   = Base::TraitType;
        using ValueType   = Base::ValueType;
        using ObjectType  = Base::ObjectType;
        using ViewType    = Base::ViewType;
        using DeleterType = TraitType::DeleterType;

        GpuObjectImplementation(DeleterType&&) noexcept;
        ~GpuObjectImplementation() noexcept;

        GpuObjectImplementation(const GpuObjectImplementation&) noexcept                    = delete;
        auto operator=(const GpuObjectImplementation&) noexcept -> GpuObjectImplementation& = delete;

        GpuObjectImplementation(GpuObjectImplementation&&) noexcept;
        auto operator=(GpuObjectImplementation&&) noexcept -> GpuObjectImplementation&;

        using NamedConstructorBase::allocate;
        using NamedConstructorBase::create;

      protected:
        DeleterType m_deleter_ptr;
    };

    template<typename Tag, typename... TDoInitArgs>
        requires(meta::GpuObjectHasTraitDefined<Tag> and meta::HasOwnerType<Tag>)
    class GpuObjectImplementation<Tag, TDoInitArgs...>
        : public GpuObjectBase<Tag>,
          public core::NamedConstructor<typename trait::GpuObject<Tag>::ObjectType,
                                        ConstructorArgs<typename trait::GpuObject<Tag>::OwnerType::ViewType>,
                                        DoInitArgs<TDoInitArgs...>> {
        using Base = GpuObjectBase<Tag>;

      protected:
        using NamedConstructorBase = core::NamedConstructor<typename trait::GpuObject<Tag>::ObjectType,
                                                            ConstructorArgs<typename trait::GpuObject<Tag>::OwnerType::ViewType>,
                                                            DoInitArgs<TDoInitArgs...>>;

      public:
        using TagType       = Base::TagType;
        using TraitType     = Base::TraitType;
        using ValueType     = Base::ValueType;
        using ObjectType    = Base::ObjectType;
        using ViewType      = Base::ViewType;
        using DeleterType   = TraitType::DeleterType;
        using OwnerType     = Base::OwnerType;
        using OwnerViewType = Base::OwnerViewType;

        GpuObjectImplementation(OwnerViewType&&, DeleterType&&) noexcept;
        ~GpuObjectImplementation() noexcept;

        GpuObjectImplementation(const GpuObjectImplementation&) noexcept                    = delete;
        auto operator=(const GpuObjectImplementation&) noexcept -> GpuObjectImplementation& = delete;

        GpuObjectImplementation(GpuObjectImplementation&&) noexcept;
        auto operator=(GpuObjectImplementation&&) noexcept -> GpuObjectImplementation&;

        using NamedConstructorBase::allocate;
        using NamedConstructorBase::create;

      protected:
        DeleterType m_deleter_ptr;
    };

    template<typename T>
        requires(meta::IsGpuView<cmeta::CanonicalType<T>>)
    auto as_view(T&& value) noexcept -> decltype(auto);

    template<meta::IsGpuObject T>
    auto as_view(const T& value) noexcept -> trait::GpuObject<typename T::TagType>::ViewType;

    template<cmeta::IsPointer T>
    auto as_view(const T& value) noexcept
      -> trait::GpuObject<typename cmeta::CanonicalType<cmeta::PointedType<T>>::TagType>::ViewType;

    template<cmeta::IsContainer T>
    auto as_view(const T& value) noexcept
      -> trait::GpuObject<typename cmeta::CanonicalType<cmeta::ContainedType<T>>::TagType>::ViewType;

    template<template<typename, std::size_t> class Out = array, typename... Args>
        requires(not stdr::range<Args> and ...)
    auto as_views(Args&&... args) noexcept -> decltype(auto);

    template<template<typename...> class Out = dyn_array, typename... Args>
        requires(not stdr::range<Args> and ...)
    auto to_views(Args&&... args) noexcept -> decltype(auto);

    template<template<typename...> class Out = dyn_array, stdr::range Range>
    auto to_views(const Range& range) noexcept -> decltype(auto);

    template<meta::IsGpuObject T, typename FormatContext>
    auto format_as(const T& object, FormatContext& ctx) noexcept -> decltype(ctx.out());
} // namespace stormkit::gpu

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::gpu {
    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
    STORMKIT_FORCE_INLINE
    inline GpuObjectBase<Tag>::GpuObjectBase() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
    STORMKIT_FORCE_INLINE
    inline GpuObjectBase<Tag>::~GpuObjectBase() noexcept {
        m_vk_handle = VK_NULL_HANDLE;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
    STORMKIT_FORCE_INLINE
    inline GpuObjectBase<Tag>::GpuObjectBase(const GpuObjectBase&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
        STORMKIT_FORCE_INLINE
    inline auto GpuObjectBase<Tag>::operator=(const GpuObjectBase&) noexcept -> GpuObjectBase& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
    STORMKIT_FORCE_INLINE
    inline GpuObjectBase<Tag>::GpuObjectBase(GpuObjectBase&& other) noexcept
        : m_vk_handle { std::exchange(other.m_vk_handle, VK_NULL_HANDLE) } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
    STORMKIT_FORCE_INLINE
    inline auto GpuObjectBase<Tag>::operator=(GpuObjectBase&& other) noexcept -> GpuObjectBase& {
        if (&other == this) [[unlikely]]
            return *this;

        m_vk_handle = std::exchange(other.m_vk_handle, VK_NULL_HANDLE);

        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
    STORMKIT_FORCE_INLINE
    inline auto GpuObjectBase<Tag>::native_handle() const noexcept -> ValueType {
        EXPECTS(m_vk_handle != VK_NULL_HANDLE);
        return m_vk_handle;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
    STORMKIT_FORCE_INLINE
    inline GpuObjectBase<Tag>::operator ValueType() const noexcept {
        return native_handle();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
        requires(meta::HasOwnerType<Tag>)
    STORMKIT_FORCE_INLINE
    inline GpuObjectBase<Tag>::GpuObjectBase(OwnerViewType&& owner) noexcept
        : m_owner { std::move(owner) } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
        requires(meta::HasOwnerType<Tag>)
    STORMKIT_FORCE_INLINE
    inline GpuObjectBase<Tag>::~GpuObjectBase() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
        requires(meta::HasOwnerType<Tag>)
    STORMKIT_FORCE_INLINE
    inline GpuObjectBase<Tag>::GpuObjectBase(const GpuObjectBase&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
        requires(meta::HasOwnerType<Tag>)
        STORMKIT_FORCE_INLINE
    inline auto GpuObjectBase<Tag>::operator=(const GpuObjectBase&) noexcept -> GpuObjectBase& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
        requires(meta::HasOwnerType<Tag>)
    STORMKIT_FORCE_INLINE
    inline GpuObjectBase<Tag>::GpuObjectBase(GpuObjectBase&& other) noexcept
        : m_vk_handle { std::exchange(other.m_vk_handle, VK_NULL_HANDLE) }, m_owner { std::move(other.m_owner) } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
        requires(meta::HasOwnerType<Tag>)
    STORMKIT_FORCE_INLINE
    inline auto GpuObjectBase<Tag>::operator=(GpuObjectBase&& other) noexcept -> GpuObjectBase& {
        if (&other == this) [[unlikely]]
            return *this;

        m_vk_handle = std::exchange(other.m_vk_handle, VK_NULL_HANDLE);
        m_owner     = std::move(other.m_owner);

        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
        requires(meta::HasOwnerType<Tag>)
    STORMKIT_FORCE_INLINE
    inline auto GpuObjectBase<Tag>::native_handle() const noexcept -> ValueType {
        EXPECTS(m_vk_handle != VK_NULL_HANDLE);
        return m_vk_handle;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
        requires(meta::HasOwnerType<Tag>)
    STORMKIT_FORCE_INLINE
    inline GpuObjectBase<Tag>::operator ValueType() const noexcept {
        return native_handle();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
        requires(meta::HasOwnerType<Tag>)
    STORMKIT_FORCE_INLINE
    inline auto GpuObjectBase<Tag>::owner() const noexcept -> OwnerViewType {
        return m_owner;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
    STORMKIT_FORCE_INLINE
    inline GpuObjectViewImplementation<
      Tag>::GpuObjectViewImplementation(const cmeta::IsSpecializationOf<GpuObjectImplementation> auto& object) noexcept
        : GpuObjectBase<Tag> {} {
        GpuObjectBase<Tag>::m_vk_handle = object.m_vk_handle;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
    template<cmeta::IsContainerOrPointer TContainerOrPointer>
    STORMKIT_FORCE_INLINE
    inline GpuObjectViewImplementation<Tag>::GpuObjectViewImplementation(const TContainerOrPointer& object) noexcept
        : GpuObjectBase<Tag> {} {
        GpuObjectBase<Tag>::m_vk_handle = (*object).m_vk_handle;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
    STORMKIT_FORCE_INLINE
    inline GpuObjectViewImplementation<Tag>::~GpuObjectViewImplementation() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
    STORMKIT_FORCE_INLINE
    inline GpuObjectViewImplementation<Tag>::GpuObjectViewImplementation(const GpuObjectViewImplementation&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
    STORMKIT_FORCE_INLINE
    inline auto GpuObjectViewImplementation<Tag>::operator=(const GpuObjectViewImplementation&) noexcept
      -> GpuObjectViewImplementation& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
    STORMKIT_FORCE_INLINE
    inline GpuObjectViewImplementation<Tag>::GpuObjectViewImplementation(GpuObjectViewImplementation&& other) noexcept
        : GpuObjectBase<Tag> {} {
        GpuObjectBase<Tag>::m_vk_handle = other.m_vk_handle;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
    STORMKIT_FORCE_INLINE
    inline auto GpuObjectViewImplementation<Tag>::operator=(GpuObjectViewImplementation&& other) noexcept
      -> GpuObjectViewImplementation& {
        if (this == &other) [[unlikely]]
            return *this;

        GpuObjectBase<Tag>::m_vk_handle = other.m_vk_handle;

        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
        requires(meta::HasOwnerType<Tag>)
    STORMKIT_FORCE_INLINE
    inline GpuObjectViewImplementation<
      Tag>::GpuObjectViewImplementation(const cmeta::IsSpecializationOf<GpuObjectImplementation> auto& object) noexcept
        : GpuObjectBase<Tag> { object.owner() } {
        GpuObjectBase<Tag>::m_vk_handle = object.m_vk_handle;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
        requires(meta::HasOwnerType<Tag>)
    template<cmeta::IsContainerOrPointer TContainerOrPointer>
    STORMKIT_FORCE_INLINE
    inline GpuObjectViewImplementation<Tag>::GpuObjectViewImplementation(const TContainerOrPointer& object) noexcept
        : GpuObjectBase<Tag> { (*object).owner() } {
        GpuObjectBase<Tag>::m_vk_handle = (*object).m_vk_handle;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
        requires(meta::HasOwnerType<Tag>)
    STORMKIT_FORCE_INLINE
    inline GpuObjectViewImplementation<Tag>::~GpuObjectViewImplementation() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
        requires(meta::HasOwnerType<Tag>)
    STORMKIT_FORCE_INLINE
    inline GpuObjectViewImplementation<
      Tag>::GpuObjectViewImplementation(const GpuObjectViewImplementation& other) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
        requires(meta::HasOwnerType<Tag>)
    STORMKIT_FORCE_INLINE
    inline auto GpuObjectViewImplementation<Tag>::operator=(const GpuObjectViewImplementation&) noexcept
      -> GpuObjectViewImplementation& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
        requires(meta::HasOwnerType<Tag>)
    STORMKIT_FORCE_INLINE
    inline GpuObjectViewImplementation<Tag>::GpuObjectViewImplementation(GpuObjectViewImplementation&& other) noexcept
        : GpuObjectBase<Tag> { other.owner() } {
        GpuObjectBase<Tag>::m_vk_handle = other.m_vk_handle;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::GpuObjectHasTraitDefined Tag>
        requires(meta::HasOwnerType<Tag>)
    STORMKIT_FORCE_INLINE
    inline auto GpuObjectViewImplementation<Tag>::operator=(GpuObjectViewImplementation&& other) noexcept
      -> GpuObjectViewImplementation& {
        if (this == &other) [[unlikely]]
            return *this;

        GpuObjectBase<Tag>::m_vk_handle = other.m_vk_handle;
        GpuObjectBase<Tag>::m_owner     = other.m_owner;

        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Tag, typename... TDoInitArgs>
        requires(meta::GpuObjectHasTraitDefined<Tag>)
    STORMKIT_FORCE_INLINE
    inline GpuObjectImplementation<Tag, TDoInitArgs...>::GpuObjectImplementation(DeleterType&& deleter_ptr) noexcept
        : GpuObjectBase<Tag> {}, m_deleter_ptr { std::move(deleter_ptr) } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Tag, typename... TDoInitArgs>
        requires(meta::GpuObjectHasTraitDefined<Tag>)
    STORMKIT_FORCE_INLINE inline GpuObjectImplementation<Tag, TDoInitArgs...>::~GpuObjectImplementation() noexcept {
        if constexpr (cmeta::SameAs<DeleterType, void (*)(ValueType, const VkAllocationCallbacks*)>) {
            if (m_deleter_ptr != nullptr and Base::m_vk_handle != VK_NULL_HANDLE)
                vk::call(m_deleter_ptr, Base::m_vk_handle, nullptr);
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Tag, typename... TDoInitArgs>
        requires(meta::GpuObjectHasTraitDefined<Tag>)
    STORMKIT_FORCE_INLINE
    inline GpuObjectImplementation<Tag, TDoInitArgs...>::GpuObjectImplementation(GpuObjectImplementation&& other) noexcept
        : GpuObjectBase<Tag> { std::move(other) }, m_deleter_ptr { std::exchange(other.m_deleter_ptr, {}) } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Tag, typename... TDoInitArgs>
        requires(meta::GpuObjectHasTraitDefined<Tag>)
    STORMKIT_FORCE_INLINE
    inline auto GpuObjectImplementation<Tag, TDoInitArgs...>::operator=(GpuObjectImplementation&& other) noexcept
      -> GpuObjectImplementation& {
        if (&other == this) [[unlikely]]
            return *this;

        GpuObjectBase<Tag>::operator=(std::move(other));

        m_deleter_ptr = std::exchange(other.m_deleter_ptr, {});

        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Tag, typename... TDoInitArgs>
        requires(meta::GpuObjectHasTraitDefined<Tag> and meta::HasOwnerType<Tag>)
    STORMKIT_FORCE_INLINE
    inline GpuObjectImplementation<Tag, TDoInitArgs...>::GpuObjectImplementation(OwnerViewType&& owner,
                                                                                 DeleterType&&   deleter_ptr) noexcept
        : GpuObjectBase<Tag> { std::move(owner) }, m_deleter_ptr { std::move(deleter_ptr) } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Tag, typename... TDoInitArgs>
        requires(meta::GpuObjectHasTraitDefined<Tag> and meta::HasOwnerType<Tag>)
    STORMKIT_FORCE_INLINE inline GpuObjectImplementation<Tag, TDoInitArgs...>::~GpuObjectImplementation() noexcept {
        using OwnerValueType = OwnerType::ValueType;

        if constexpr (cmeta::SameAs<DeleterType, void (*)(OwnerValueType, ValueType, const VkAllocationCallbacks*)>) {
            if constexpr (cmeta::SameAs<OwnerValueType, VkInstance>) {
                if (m_deleter_ptr != nullptr and Base::m_vk_handle != VK_NULL_HANDLE)
                    vk::call(m_deleter_ptr, Base::m_owner, Base::m_vk_handle, nullptr);
            } else {
                const auto& device       = this->device();
                const auto& device_table = device.device_table();
                if (m_deleter_ptr != nullptr and Base::m_vk_handle != VK_NULL_HANDLE)
                    vk::call(device_table.*m_deleter_ptr, device, Base::m_vk_handle, nullptr);
            }
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Tag, typename... TDoInitArgs>
        requires(meta::GpuObjectHasTraitDefined<Tag> and meta::HasOwnerType<Tag>)
    STORMKIT_FORCE_INLINE
    inline GpuObjectImplementation<Tag, TDoInitArgs...>::GpuObjectImplementation(GpuObjectImplementation&& other) noexcept
        : GpuObjectBase<Tag> { std::move(other) }, m_deleter_ptr { std::exchange(other.m_deleter_ptr, {}) } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Tag, typename... TDoInitArgs>
        requires(meta::GpuObjectHasTraitDefined<Tag> and meta::HasOwnerType<Tag>)
    STORMKIT_FORCE_INLINE
    inline auto GpuObjectImplementation<Tag, TDoInitArgs...>::operator=(GpuObjectImplementation&& other) noexcept
      -> GpuObjectImplementation& {
        if (&other == this) [[unlikely]]
            return *this;

        GpuObjectBase<Tag>::operator=(std::move(other));

        m_deleter_ptr = std::exchange(other.m_deleter_ptr, {});

        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
        requires(meta::IsGpuView<cmeta::CanonicalType<T>>)
    STORMKIT_FORCE_INLINE
    inline auto as_view(T&& value) noexcept -> decltype(auto) {
        return std::forward<T>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::IsGpuObject T>
    STORMKIT_FORCE_INLINE
    inline auto as_view(const T& value) noexcept -> trait::GpuObject<typename T::TagType>::ViewType {
        using Out = trait::GpuObject<typename T::TagType>::ViewType;
        return Out { value };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<cmeta::IsPointer T>
    STORMKIT_FORCE_INLINE
    inline auto as_view(const T& value) noexcept
      -> trait::GpuObject<typename cmeta::CanonicalType<cmeta::PointedType<T>>::TagType>::ViewType {
        return as_view(unref(value));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<cmeta::IsContainer T>
    STORMKIT_FORCE_INLINE
    inline auto as_view(const T& value) noexcept
      -> trait::GpuObject<typename cmeta::CanonicalType<cmeta::ContainedType<T>>::TagType>::ViewType {
        return as_view(value.value());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<typename, std::size_t> class Out, typename... Args>
        requires(not stdr::range<Args> and ...)
    STORMKIT_FORCE_INLINE
    inline auto as_views(Args&&... args) noexcept -> decltype(auto) {
        using ValueType = std::common_type_t<cmeta::ContainedOrPointedOrTType<cmeta::RemoveIndirectionsType<Args>>...>;
        using TagType   = typename ValueType::TagType;
        using ViewType  = trait::GpuObject<TagType>::ViewType;

        return Out<ViewType, sizeof...(Args)> { gpu::as_view(std::forward<Args>(args))... };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<typename...> class Out, typename... Args>
        requires(not stdr::range<Args> and ...)
    STORMKIT_FORCE_INLINE
    inline auto to_views(Args&&... args) noexcept -> decltype(auto) {
        using ValueType = std::common_type_t<cmeta::ContainedOrPointedOrTType<cmeta::RemoveIndirectionsType<Args>>...>;
        using TagType   = typename ValueType::TagType;
        using ViewType  = trait::GpuObject<TagType>::ViewType;

        return Out<ViewType> { gpu::as_view(std::forward<Args>(args))... };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<typename...> class Out, stdr::range Range>
    STORMKIT_FORCE_INLINE
    inline auto to_views(const Range& range) noexcept -> decltype(auto) {
        using ValueType = stdr::range_value_t<Range>;
        using TagType   = typename ValueType::TagType;
        using ViewType  = trait::GpuObject<TagType>::ViewType;

        return range
               | stdv::transform([]<typename T>(T&& val) static noexcept { return gpu::as_view(std::forward<T>(val)); })
               | stdr::to<Out<ViewType>>();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::IsGpuObject T, typename FormatContext>
    STORMKIT_FORCE_INLINE
    inline auto format_as(const T& object, FormatContext& ctx) noexcept -> decltype(ctx.out()) {
        return format_as(as_view(object), ctx);
    }
} // namespace stormkit::gpu
