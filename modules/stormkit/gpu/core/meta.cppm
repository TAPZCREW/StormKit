// Copyright (C) 2023 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

export module stormkit.gpu.core:meta;

import std;

import stormkit.core;

import :vulkan;

namespace cmeta = stormkit::core::meta;

namespace stormkit::gpu {
    export namespace trait {
        template<typename T>
        struct GpuObject;
    };

    namespace meta {
        export {
            template<typename Tag>
            concept GpuObjectHasTraitDefined = requires() {
                trait::GpuObject<Tag> {};
                typename trait::GpuObject<Tag>::value_type;
                typename trait::GpuObject<Tag>::DeleterType;
                typename trait::GpuObject<Tag>::ObjectType;
                typename trait::GpuObject<Tag>::ViewType;

                { trait::GpuObject<Tag>::DEBUG_TYPE } -> cmeta::same_as<const DebugObjectType&>;
            };

            template<typename T>
            concept HasDoInitreturn_type = GpuObjectHasTraitDefined<T> and requires() {
                typename trait::GpuObject<T>::DoInitreturn_type;
            };

            template<typename T>
            concept HasOwnerType = GpuObjectHasTraitDefined<T> and requires() { typename trait::GpuObject<T>::OwnerType; };

            template<typename T>
            concept HasTagType = requires() { typename T::TagType; };
        }

        namespace details {
            template<typename T>
            struct GpuObjectDoInitreturn_type {
                using type = expected<void>;
            };

            template<HasDoInitreturn_type T>
            struct GpuObjectDoInitreturn_type<T> {
                using type = typename trait::GpuObject<cmeta::to_plain_type<T>>::DoInitreturn_type;
            };
        } // namespace details

        export {
            template<typename T>
            using GpuObjectDoInitreturn_type = details::GpuObjectDoInitreturn_type<T>::type;

            template<typename T>
            concept IsGpuObject = HasTagType<T>
                                  and GpuObjectHasTraitDefined<typename T::TagType>
                                  and cmeta::same_as<T, typename trait::GpuObject<typename T::TagType>::ObjectType>;

            template<typename T>
            concept IsGpuView = HasTagType<T>
                                and GpuObjectHasTraitDefined<typename T::TagType>
                                and cmeta::same_as<T, typename trait::GpuObject<typename T::TagType>::ViewType>;

            template<typename T>
            concept IsGpuObjectOrView = IsGpuObject<T> or IsGpuView<T>;
        }
    } // namespace meta
} // namespace stormkit::gpu
