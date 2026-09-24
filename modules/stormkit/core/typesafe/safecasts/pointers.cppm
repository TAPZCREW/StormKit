// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/api.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.typesafe.safecasts:pointers;

import :as_cpo;
import :is_cpo;

import stormkit.core.meta.algorithms;
import stormkit.core.typesafe.ref_ptr;

export namespace stormkit { inline namespace core {
    template<meta::polymorphic_type T, meta::negate<meta::pointer> U>
    [[nodiscard]]
    constexpr auto tag_invoke(meta::tag<is<T>>, const U& value) noexcept -> bool
        requires(meta::polymorphic_reference<U&>);

    template<meta::polymorphic_type T, meta::polymorphic_pointer U>
    [[nodiscard]]
    constexpr auto tag_invoke(meta::tag<is<T>>, const U& value) noexcept -> bool;

    template<meta::polymorphic_type To, meta::negate<meta::pointer> From>
    [[nodiscard]]
    constexpr auto tag_invoke(meta::tag<as<To>>, From& value, source_location_arg = std::source_location::current()) noexcept
      -> To&
        requires(meta::polymorphic_reference<From&>);

    template<meta::polymorphic_type To, meta::polymorphic_pointer From>
    [[nodiscard]]
    constexpr auto tag_invoke(meta::tag<as<To>>,
                              const From& value,
                              source_location_arg = std::source_location::current()) noexcept -> ref_ptr<To>
        requires(not meta::same_as<To, From>);
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::polymorphic_type T, meta::negate<meta::pointer> U>
        STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(meta::tag<is<T>>, const U& value) noexcept -> bool
        requires(meta::polymorphic_reference<U&>)
    {
        if constexpr (meta::plain::is<T, U>) return dynamic_cast<const T*>(&value) != nullptr;
        else
            return false;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::polymorphic_type T, meta::polymorphic_pointer U>
        STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(meta::tag<is<T>>, const U& value) noexcept -> bool {
        if constexpr (meta::plain::is<T, U>) {
            if constexpr (meta::raw_pointer<U>) return dynamic_cast<const T*>(value) != nullptr;
            else
                return dynamic_cast<const T*>(value.get()) != nullptr;
        } else
            return false;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::polymorphic_type To, meta::negate<meta::pointer> From>
        STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(meta::tag<as<To>>, From& value, source_location_arg location) noexcept -> To&
        requires(meta::polymorphic_reference<From&>)
    {
        expects(is<To>(value), "Invalid polymorphic cast!", location);
        return dynamic_cast<meta::forward_const_to<From, To>&>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::polymorphic_type To, meta::polymorphic_pointer From>
        STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(meta::tag<as<To>>, const From& value, source_location_arg location) noexcept -> ref_ptr<To> {
        expects(is<To>(value), "Invalid polymorphic cast!", location);

        if constexpr (meta::raw_pointer<From>) return dynamic_cast<meta::forward_const_to<From, To>*>(value);
        else
            return dynamic_cast<meta::forward_const_to<From, To>*>(value.get());
    }
}} // namespace stormkit::core
