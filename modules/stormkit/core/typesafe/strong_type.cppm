// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.typesafe.strong_type;

import std;

import stormkit.core.meta;
import stormkit.core.hash;
import stormkit.core.string.static_string;
import stormkit.core.meta.concepts;
import stormkit.core.meta.algorithms;
import stormkit.core.string.format;

namespace stdr = std::ranges;

export namespace stormkit { inline namespace core {
    template<meta::is_decayed T, typename Tag, static_string NAME, template<class> typename... Capabilities>
        requires(meta::destructible<T>)
    class strong_type;

    namespace meta {
        template<typename T>
        concept is_strong_type = specialization_of_nttp_ttvtcs<T, strong_type>;

        template<typename T, typename Capability>
        concept has_capability = derived_from<T, Capability>;
    } // namespace meta

    namespace capabilities {
        template<typename T>
        struct arithmetic {
            using type       = T;
            using param_type = meta::in<type>;

            template<typename Self>
            constexpr auto operator+(this Self self, param_type other) noexcept -> Self
                requires(meta::prefer_pass_by_value<Self>);
            template<typename Self>
            constexpr auto operator+(this const Self& self, param_type other) noexcept -> const Self&
                requires(meta::prefer_pass_by_ref<Self>);
            template<typename Self>
            constexpr auto operator+(this Self self, Self other) noexcept -> Self
                requires(meta::prefer_pass_by_value<Self>);
            template<typename Self>
            constexpr auto operator+(this const Self& self, const Self& other) noexcept -> Self
                requires(meta::prefer_pass_by_ref<Self>);
            template<typename Self>
            constexpr auto operator+=(this Self self, param_type other) noexcept -> Self&
                requires(meta::prefer_pass_by_value<Self>);
            template<typename Self>
            constexpr auto operator+=(this Self& self, param_type other) noexcept -> Self&
                requires(meta::prefer_pass_by_ref<Self>);
            template<typename Self>
            constexpr auto operator+=(this Self self, Self other) noexcept -> Self&
                requires(meta::prefer_pass_by_value<Self>);
            template<typename Self>
            constexpr auto operator+=(this Self& self, const Self& other) noexcept -> Self&
                requires(meta::prefer_pass_by_ref<Self>);

            template<typename Self>
            constexpr auto operator-(this Self self, param_type other) noexcept -> Self
                requires(meta::prefer_pass_by_value<Self>);
            template<typename Self>
            constexpr auto operator-(this const Self& self, param_type other) noexcept -> const Self&
                requires(meta::prefer_pass_by_ref<Self>);
            template<typename Self>
            constexpr auto operator-(this Self self, Self other) noexcept -> Self
                requires(meta::prefer_pass_by_value<Self>);
            template<typename Self>
            constexpr auto operator-(this const Self& self, const Self& other) noexcept -> Self
                requires(meta::prefer_pass_by_ref<Self>);
            template<typename Self>
            constexpr auto operator-=(this Self self, param_type other) noexcept -> Self&
                requires(meta::prefer_pass_by_value<Self>);
            template<typename Self>
            constexpr auto operator-=(this Self& self, param_type other) noexcept -> Self&
                requires(meta::prefer_pass_by_ref<Self>);
            template<typename Self>
            constexpr auto operator-=(this Self self, Self other) noexcept -> Self&
                requires(meta::prefer_pass_by_value<Self>);
            template<typename Self>
            constexpr auto operator-=(this Self& self, const Self& other) noexcept -> Self&
                requires(meta::prefer_pass_by_ref<Self>);

            template<typename Self>
            constexpr auto operator*(this Self self, param_type other) noexcept -> Self
                requires(meta::prefer_pass_by_value<Self>);
            template<typename Self>
            constexpr auto operator*(this const Self& self, param_type other) noexcept -> const Self&
                requires(meta::prefer_pass_by_ref<Self>);
            template<typename Self>
            constexpr auto operator*(this Self self, Self other) noexcept -> Self
                requires(meta::prefer_pass_by_value<Self>);
            template<typename Self>
            constexpr auto operator*(this const Self& self, const Self& other) noexcept -> Self
                requires(meta::prefer_pass_by_ref<Self>);
            template<typename Self>
            constexpr auto operator*=(this Self self, param_type other) noexcept -> Self&
                requires(meta::prefer_pass_by_value<Self>);
            template<typename Self>
            constexpr auto operator*=(this Self& self, param_type other) noexcept -> Self&
                requires(meta::prefer_pass_by_ref<Self>);
            template<typename Self>
            constexpr auto operator*=(this Self self, Self other) noexcept -> Self&
                requires(meta::prefer_pass_by_value<Self>);
            template<typename Self>
            constexpr auto operator*=(this Self& self, const Self& other) noexcept -> Self&
                requires(meta::prefer_pass_by_ref<Self>);

            template<typename Self>
            constexpr auto operator/(this Self self, param_type other) noexcept -> Self
                requires(meta::prefer_pass_by_value<Self>);
            template<typename Self>
            constexpr auto operator/(this const Self& self, param_type other) noexcept -> const Self&
                requires(meta::prefer_pass_by_ref<Self>);
            template<typename Self>
            constexpr auto operator/(this Self self, Self other) noexcept -> Self
                requires(meta::prefer_pass_by_value<Self>);
            template<typename Self>
            constexpr auto operator/(this const Self& self, const Self& other) noexcept -> Self
                requires(meta::prefer_pass_by_ref<Self>);
            template<typename Self>
            constexpr auto operator/=(this Self self, param_type other) noexcept -> Self&
                requires(meta::prefer_pass_by_value<Self>);
            template<typename Self>
            constexpr auto operator/=(this Self& self, param_type other) noexcept -> Self&
                requires(meta::prefer_pass_by_ref<Self>);
            template<typename Self>
            constexpr auto operator/=(this Self self, Self other) noexcept -> Self&
                requires(meta::prefer_pass_by_value<Self>);
            template<typename Self>
            constexpr auto operator/=(this Self& self, const Self& other) noexcept -> Self&
                requires(meta::prefer_pass_by_ref<Self>);
        };

        template<typename T>
        struct implicit_convertion {};

        namespace meta {
            template<template<class> typename... Capabilities>
            concept has_arithmetic = core::meta::is_any_of<arithmetic<int>, Capabilities<int>...>;

            template<template<class> typename... Capabilities>
            concept has_implicit_convertion = core::meta::is_any_of<implicit_convertion<int>, Capabilities<int>...>;
        } // namespace meta
    } // namespace capabilities

    template<meta::is_decayed T, typename Tag, static_string NAME_, template<class> typename... Capabilities>
        requires(meta::destructible<T>)
    class STORMKIT_OWNER strong_type final: public Capabilities<T>... {
      public:
        using value_type = T;
        using param_type = meta::in<T>;

        static constexpr auto NAME = std::string_view { stdr::data(NAME_), stdr::size(NAME_) };

        constexpr explicit strong_type(meta::take<value_type>
                                         value) noexcept(meta::noexcept_constructible_from<value_type, meta::take<value_type>>);

        template<class... Ts>
        constexpr explicit(sizeof...(Ts) == 1)
          strong_type(std::in_place_t, Ts&&... args) noexcept(meta::noexcept_constructible_from<value_type, Ts...>)
            requires(not meta::decayed::is<meta::first_type<Ts...>, T> and meta::constructible_from<value_type, Ts...>);

        constexpr ~strong_type() noexcept(meta::noexcept_destructible<value_type>);

        constexpr strong_type(const strong_type&) noexcept(meta::noexcept_copyable<value_type>)
            requires(meta::copyable<value_type>);
        constexpr strong_type(strong_type&&) noexcept(meta::noexcept_movable<value_type>)
            requires(meta::movable<value_type>);

        constexpr auto operator=(const strong_type&) noexcept(meta::noexcept_copy_assignable<value_type>) -> strong_type&
            requires(meta::copy_assignable<value_type>);
        constexpr auto operator=(strong_type&&) noexcept(meta::noexcept_move_assignable<value_type>) -> strong_type&
            requires(meta::move_assignable<value_type>);

        template<typename Self>
        [[nodiscard]]
        constexpr explicit(not capabilities::meta::has_implicit_convertion<Capabilities...>) operator meta::
          forward_like<Self, value_type>(STORMKIT_LIFETIMEBOUND this Self&& self) noexcept;

        template<typename Self>
        [[nodiscard]]
        constexpr auto value(STORMKIT_LIFETIMEBOUND this Self&& self) noexcept -> meta::forward_like<Self, value_type>;

      private:
        friend class Capabilities<T>...;

        value_type m_value;
    };

    template<typename CharT,
             typename FormatContext,
             meta::is_decayed T,
             typename Tag,
             static_string NAME_,
             template<class> typename... Capabilities>
    constexpr auto tag_invoke(format_as_fn<CharT>,
                              meta::in<strong_type<T, Tag, NAME_, Capabilities...>> value,
                              FormatContext&                                        ctx) -> decltype(ctx.out());
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, typename Tag, static_string NAME_, template<class> typename... Capabilities>
        requires(meta::destructible<T>)
    STORMKIT_FORCE_INLINE
    constexpr strong_type<T, Tag, NAME_, Capabilities...>::
      strong_type(meta::take<value_type> value) noexcept(meta::noexcept_constructible_from<value_type, meta::take<value_type>>)
        : m_value { std::forward<meta::take<value_type>>(value) } {
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, typename Tag, static_string NAME_, template<class> typename... Capabilities>
        requires(meta::destructible<T>)
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    constexpr strong_type<T, Tag, NAME_, Capabilities...>::
      strong_type(std::in_place_t, Ts&&... args) noexcept(meta::noexcept_constructible_from<value_type, Ts...>)
        requires(not meta::decayed::is<meta::first_type<Ts...>, T> and meta::constructible_from<value_type, Ts...>)
        : m_value { std::forward<Ts>(args)... } {
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, typename Tag, static_string NAME_, template<class> typename... Capabilities>
        requires(meta::destructible<T>)
    STORMKIT_FORCE_INLINE
    constexpr strong_type<T, Tag, NAME_, Capabilities...>::
      ~strong_type() noexcept(meta::noexcept_destructible<value_type>) = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, typename Tag, static_string NAME_, template<class> typename... Capabilities>
        requires(meta::destructible<T>)
    STORMKIT_FORCE_INLINE
    constexpr strong_type<T, Tag, NAME_, Capabilities...>::
      strong_type(const strong_type&) noexcept(meta::noexcept_copyable<value_type>)
        requires(meta::copyable<value_type>)
    = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, typename Tag, static_string NAME_, template<class> typename... Capabilities>
        requires(meta::destructible<T>)
    STORMKIT_FORCE_INLINE
    constexpr strong_type<T, Tag, NAME_, Capabilities...>::strong_type(strong_type&&) noexcept(meta::noexcept_movable<value_type>)
        requires(meta::movable<value_type>)
    = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, typename Tag, static_string NAME_, template<class> typename... Capabilities>
        requires(meta::destructible<T>)
    STORMKIT_FORCE_INLINE
    constexpr auto strong_type<T, Tag, NAME_, Capabilities...>::
      operator=(const strong_type&) noexcept(meta::noexcept_copy_assignable<value_type>) -> strong_type&
        requires(meta::copy_assignable<value_type>)
    = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, typename Tag, static_string NAME_, template<class> typename... Capabilities>
        requires(meta::destructible<T>)
    STORMKIT_FORCE_INLINE
    constexpr auto strong_type<T, Tag, NAME_, Capabilities...>::
      operator=(strong_type&&) noexcept(meta::noexcept_move_assignable<value_type>) -> strong_type&
        requires(meta::move_assignable<value_type>)
    = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, typename Tag, static_string NAME_, template<class> typename... Capabilities>
        requires(meta::destructible<T>)
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr strong_type<T, Tag, NAME_, Capabilities...>::operator meta::forward_like<Self,
                                                                                       value_type>(this Self&& self) noexcept {
        return std::forward_like<Self>(self.m_value);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, typename Tag, static_string NAME_, template<class> typename... Capabilities>
        requires(meta::destructible<T>)
    template<typename Self>
    STORMKIT_FORCE_INLINE
    constexpr auto strong_type<T, Tag, NAME_, Capabilities...>::value(this Self&& self) noexcept
      -> meta::forward_like<Self, value_type> {
        return std::forward_like<Self>(self.m_value);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename CharT,
             typename FormatContext,
             meta::is_decayed T,
             typename Tag,
             static_string NAME_,
             template<class> typename... Capabilities>
             STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(format_as_fn<CharT>,
                              meta::in<strong_type<T, Tag, NAME_, Capabilities...>> value,
                              FormatContext&                                        ctx) -> decltype(ctx.out()) {
        return std::format_to(ctx.out(), "{{{} {}}}", NAME_, value.value());
    }

    namespace capabilities {
        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator+(this Self self, param_type other) noexcept -> Self
            requires(core::meta::prefer_pass_by_value<Self>)
        {
            return Self { self.m_value + other };
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator+(this const Self& self, param_type other) noexcept -> const Self&
            requires(core::meta::prefer_pass_by_ref<Self>)
        {
            return Self { self.m_value + other };
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator+(this Self self, Self other) noexcept -> Self
            requires(core::meta::prefer_pass_by_value<Self>)
        {
            return Self { self.m_value + other.m_value };
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator+(this const Self& self, const Self& other) noexcept -> Self
            requires(core::meta::prefer_pass_by_ref<Self>)
        {
            return Self { self.m_value + other.m_value };
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator+=(this Self self, param_type other) noexcept -> Self&
            requires(core::meta::prefer_pass_by_value<Self>)
        {
            self.m_value += other;
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator+=(this Self& self, param_type other) noexcept -> Self&
            requires(core::meta::prefer_pass_by_ref<Self>)
        {
            self.m_value += other;
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator+=(this Self self, Self other) noexcept -> Self&
            requires(core::meta::prefer_pass_by_value<Self>)
        {
            self.m_value += other.m_value;
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator+=(this Self& self, const Self& other) noexcept -> Self&
            requires(core::meta::prefer_pass_by_ref<Self>)
        {
            self.m_value += other.m_value;
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator-(this Self self, param_type other) noexcept -> Self
            requires(core::meta::prefer_pass_by_value<Self>)
        {
            return Self { self.m_value - other };
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator-(this const Self& self, param_type other) noexcept -> const Self&
            requires(core::meta::prefer_pass_by_ref<Self>)
        {
            return Self { self.m_value - other };
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator-(this Self self, Self other) noexcept -> Self
            requires(core::meta::prefer_pass_by_value<Self>)
        {
            return Self { self.m_value - other.m_value };
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator-(this const Self& self, const Self& other) noexcept -> Self
            requires(core::meta::prefer_pass_by_ref<Self>)
        {
            return Self { self.m_value - other.m_value };
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator-=(this Self self, param_type other) noexcept -> Self&
            requires(core::meta::prefer_pass_by_value<Self>)
        {
            self.m_value -= other;
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator-=(this Self& self, param_type other) noexcept -> Self&
            requires(core::meta::prefer_pass_by_ref<Self>)
        {
            self.m_value -= other;
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator-=(this Self self, Self other) noexcept -> Self&
            requires(core::meta::prefer_pass_by_value<Self>)
        {
            self.m_value -= other.m_value;
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator-=(this Self& self, const Self& other) noexcept -> Self&
            requires(core::meta::prefer_pass_by_ref<Self>)
        {
            self.m_value -= other.m_value;
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator*(this Self self, param_type other) noexcept -> Self
            requires(core::meta::prefer_pass_by_value<Self>)
        {
            return Self { self.m_value * other };
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator*(this const Self& self, param_type other) noexcept -> const Self&
            requires(core::meta::prefer_pass_by_ref<Self>)
        {
            return Self { self.m_value * other };
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator*(this Self self, Self other) noexcept -> Self
            requires(core::meta::prefer_pass_by_value<Self>)
        {
            return Self { self.m_value * other.m_value };
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator*(this const Self& self, const Self& other) noexcept -> Self
            requires(core::meta::prefer_pass_by_ref<Self>)
        {
            return Self { self.m_value * other.m_value };
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator*=(this Self self, param_type other) noexcept -> Self&
            requires(core::meta::prefer_pass_by_value<Self>)
        {
            self.m_value *= other;
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator*=(this Self& self, param_type other) noexcept -> Self&
            requires(core::meta::prefer_pass_by_ref<Self>)
        {
            self.m_value *= other;
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator*=(this Self self, Self other) noexcept -> Self&
            requires(core::meta::prefer_pass_by_value<Self>)
        {
            self.m_value *= other.m_value;
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator*=(this Self& self, const Self& other) noexcept -> Self&
            requires(core::meta::prefer_pass_by_ref<Self>)
        {
            self.m_value *= other.m_value;
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator/(this Self self, param_type other) noexcept -> Self
            requires(core::meta::prefer_pass_by_value<Self>)
        {
            return Self { self.m_value / other };
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator/(this const Self& self, param_type other) noexcept -> const Self&
            requires(core::meta::prefer_pass_by_ref<Self>)
        {
            return Self { self.m_value / other };
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator/(this Self self, Self other) noexcept -> Self
            requires(core::meta::prefer_pass_by_value<Self>)
        {
            return Self { self.m_value / other.m_value };
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator/(this const Self& self, const Self& other) noexcept -> Self
            requires(core::meta::prefer_pass_by_ref<Self>)
        {
            return Self { self.m_value / other.m_value };
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator/=(this Self self, param_type other) noexcept -> Self&
            requires(core::meta::prefer_pass_by_value<Self>)
        {
            self.m_value /= other;
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator/=(this Self& self, param_type other) noexcept -> Self&
            requires(core::meta::prefer_pass_by_ref<Self>)
        {
            self.m_value /= other;
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator/=(this Self self, Self other) noexcept -> Self&
            requires(core::meta::prefer_pass_by_value<Self>)
        {
            self.m_value /= other.m_value;
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        template<typename Self>
            STORMKIT_FORCE_INLINE
        constexpr auto arithmetic<T>::operator/=(this Self& self, const Self& other) noexcept -> Self&
            requires(core::meta::prefer_pass_by_ref<Self>)
        {
            self.m_value /= other.m_value;
        }
    } // namespace capabilities
}} // namespace stormkit::core
