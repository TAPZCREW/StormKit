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
                typename trait::GpuObject<Tag>::ValueType;
                typename trait::GpuObject<Tag>::DeleterType;
                typename trait::GpuObject<Tag>::ObjectType;
                typename trait::GpuObject<Tag>::ViewType;

                { trait::GpuObject<Tag>::DEBUG_TYPE } -> cmeta::SameAs<const DebugObjectType&>;
            };

            template<typename T>
            concept HasDoInitReturnType = GpuObjectHasTraitDefined<T> and requires() {
                typename trait::GpuObject<T>::DoInitReturnType;
            };

            template<typename T>
            concept HasOwnerType = GpuObjectHasTraitDefined<T> and requires() { typename trait::GpuObject<T>::OwnerType; };

            template<typename T>
            concept HasTagType = requires() { typename T::TagType; };
        }

        namespace details {
            template<typename T>
            struct GpuObjectDoInitReturnType {
                using Type = Expected<void>;
            };

            template<HasDoInitReturnType T>
            struct GpuObjectDoInitReturnType<T> {
                using Type = typename trait::GpuObject<cmeta::CanonicalType<T>>::DoInitReturnType;
            };
        } // namespace details

        export {
            template<typename T>
            using GpuObjectDoInitReturnType = details::GpuObjectDoInitReturnType<T>::Type;

            template<typename T>
            concept IsGpuObject = HasTagType<T>
                                  and GpuObjectHasTraitDefined<typename T::TagType>
                                  and cmeta::SameAs<T, typename trait::GpuObject<typename T::TagType>::ObjectType>;

            template<typename T>
            concept IsGpuView = HasTagType<T>
                                and GpuObjectHasTraitDefined<typename T::TagType>
                                and cmeta::SameAs<T, typename trait::GpuObject<typename T::TagType>::ViewType>;

            template<typename T>
            concept IsGpuObjectOrView = IsGpuObject<T> or IsGpuView<T>;
        }
    } // namespace meta
} // namespace stormkit::gpu
