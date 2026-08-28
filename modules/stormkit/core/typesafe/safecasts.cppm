// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/api.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.typesafe.safecasts;

import std;

import stormkit.core.types;

import stormkit.core.contract;
import stormkit.core.meta.concepts;
import stormkit.core.meta.type_manipulation;
import stormkit.core.meta.tag_invoke;
import stormkit.core.meta.type_query;
import stormkit.core.meta.algorithms;
import stormkit.core.typesafe.ref_ptr;

export {
    namespace stormkit { inline namespace core {
        struct underlying;
        struct empty;
        struct equal;
        struct error;

        template<typename To>
        struct as_fn final {
          private:
            template<typename... Ts>
            using invoke_result = meta::tag_invoke_result<as_fn<To>, Ts..., source_location_arg>;

            template<typename... Ts>
            static constexpr auto IS_TAG_INVOKABLE = meta::tag_invocable<as_fn<To>, Ts..., source_location_arg>;

          public:
            template<typename... Ts>
            static constexpr auto operator()(Ts&&...) noexcept
                requires(not IS_TAG_INVOKABLE<Ts...>)
            = delete ("As caster not defined for these types!");

            template<typename... Ts>
            [[nodiscard]]
            static constexpr auto operator()(Ts&&... args) noexcept -> invoke_result<Ts...>
                requires(sizeof...(Ts) >= 1 and IS_TAG_INVOKABLE<Ts...>);
        };

        template<template<class...> typename To>
        struct as_nttp_fn final {
          private:
            template<typename... Ts>
            using invoke_result = meta::tag_invoke_result<as_nttp_fn<To>, Ts..., source_location_arg>;

            template<typename... Ts>
            static constexpr auto IS_TAG_INVOKABLE = meta::tag_invocable<as_nttp_fn<To>, Ts..., source_location_arg>;

          public:
            template<typename... Ts>
            static constexpr auto operator()(Ts&&...) noexcept
                requires(not IS_TAG_INVOKABLE<Ts...>)
            = delete ("As caster not defined for these types!");

            template<typename... Ts>
            [[nodiscard]]
            static constexpr auto operator()(Ts&&... args) noexcept -> invoke_result<Ts...>
                requires(sizeof...(Ts) >= 1 and IS_TAG_INVOKABLE<Ts...>);
        };

        template<template<class, usize, class...> typename To>
        struct as_nttp_v_fn final {
          private:
            template<typename... Ts>
            using invoke_result = meta::tag_invoke_result<as_nttp_v_fn<To>, Ts..., source_location_arg>;

            template<typename... Ts>
            static constexpr auto IS_TAG_INVOKABLE = meta::tag_invocable<as_nttp_v_fn<To>, Ts..., source_location_arg>;

          public:
            template<typename... Ts>
            static constexpr auto operator()(Ts&&...) noexcept
                requires(not IS_TAG_INVOKABLE<Ts...>)
            = delete ("As caster not defined for these types!");

            template<typename... Ts>
            [[nodiscard]]
            static constexpr auto operator()(Ts&&... args) noexcept -> invoke_result<Ts...>
                requires(sizeof...(Ts) >= 1 and IS_TAG_INVOKABLE<Ts...>);
        };

        template<typename To, typename... Ts>
        constexpr auto as(Ts&&... args) -> decltype(auto);

        template<template<class...> typename To, typename... Ts>
        constexpr auto as(Ts&&... args) -> decltype(auto);

        template<template<class, usize, class...> typename To, typename... Ts>
        constexpr auto as(Ts&&... args) -> decltype(auto);

        template<typename To>
        struct try_as_fn final {
          private:
            template<typename... Ts>
            using invoke_result = meta::tag_invoke_result<try_as_fn<To>, Ts..., source_location_arg>;

            template<typename... Ts>
            static constexpr auto IS_TAG_INVOKABLE = meta::tag_invocable<try_as_fn<To>, Ts..., source_location_arg>;

          public:
            template<typename... Ts>
            static constexpr auto operator()(Ts&&...) noexcept
                requires(not IS_TAG_INVOKABLE<Ts...>)
            = delete ("Try_as caster not defined for these types!");

            template<typename... Ts>
            [[nodiscard]]
            static constexpr auto operator()(Ts&&... args) noexcept -> invoke_result<Ts...>
                requires(sizeof...(Ts) >= 1 and IS_TAG_INVOKABLE<Ts...>);
        };

        template<template<class...> typename To>
        struct try_as_nttp_fn final {
          private:
            template<typename... Ts>
            using invoke_result = meta::tag_invoke_result<try_as_nttp_fn<To>, Ts..., source_location_arg>;

            template<typename... Ts>
            static constexpr auto IS_TAG_INVOKABLE = meta::tag_invocable<try_as_nttp_fn<To>, Ts..., source_location_arg>;

          public:
            template<typename... Ts>
            static constexpr auto operator()(Ts&&...) noexcept
                requires(not IS_TAG_INVOKABLE<Ts...>)
            = delete ("Try_as caster not defined for these types!");

            template<typename... Ts>
            [[nodiscard]]
            static constexpr auto operator()(Ts&&... args) noexcept -> invoke_result<Ts...>
                requires(sizeof...(Ts) >= 1 and IS_TAG_INVOKABLE<Ts...>);
        };

        template<template<class, usize, class...> typename To>
        struct try_as_nttp_v_fn final {
          private:
            template<typename... Ts>
            using invoke_result = meta::tag_invoke_result<try_as_nttp_v_fn<To>, Ts..., source_location_arg>;

            template<typename... Ts>
            static constexpr auto IS_TAG_INVOKABLE = meta::tag_invocable<try_as_nttp_v_fn<To>, Ts..., source_location_arg>;

          public:
            template<typename... Ts>
            static constexpr auto operator()(Ts&&...) noexcept
                requires(not IS_TAG_INVOKABLE<Ts...>)
            = delete ("Try_as caster not defined for these types!");

            template<typename... Ts>
            [[nodiscard]]
            static constexpr auto operator()(Ts&&... args) noexcept -> invoke_result<Ts...>
                requires(sizeof...(Ts) >= 1 and IS_TAG_INVOKABLE<Ts...>);
        };

        template<typename To, typename... Ts>
        constexpr auto try_as(Ts&&... args) -> decltype(auto);

        template<template<class...> typename To, typename... Ts>
        constexpr auto try_as(Ts&&... args) -> decltype(auto);

        template<template<class, usize, class...> typename To, typename... Ts>
        constexpr auto try_as(Ts&&... args) -> decltype(auto);

        template<template<class...> typename To>
        struct into_nttp_fn final {
          private:
            template<typename... Ts>
            using invoke_result = meta::tag_invoke_result<into_nttp_fn<To>, Ts...>;

            template<typename... Ts>
            static constexpr auto IS_TAG_INVOKABLE = meta::tag_invocable<into_nttp_fn<To>, Ts...>;

          public:
            template<typename... Ts>
            static constexpr auto operator()(Ts&&...) noexcept
                requires(not IS_TAG_INVOKABLE<Ts...>)
            = delete ("Into caster not defined for these types!");

            template<typename T, usize N>
            [[nodiscard]]
            static constexpr auto operator()(T (&values)[N]) noexcept -> invoke_result<decltype(values)>
                requires(IS_TAG_INVOKABLE<decltype(values)>);

            template<typename T, usize N>
            [[nodiscard]]
            static constexpr auto operator()(T (&&values)[N]) noexcept -> invoke_result<decltype(values)>
                requires(IS_TAG_INVOKABLE<decltype(values)>);

            template<typename... Ts>
            [[nodiscard]]
            static constexpr auto operator()(Ts&&... args) noexcept -> invoke_result<Ts...>
                requires(IS_TAG_INVOKABLE<Ts...>);
        };

        template<template<class, usize, class...> typename To>
        struct into_nttp_v_fn final {
          private:
            template<typename... Ts>
            using invoke_result = meta::tag_invoke_result<into_nttp_v_fn<To>, Ts...>;

            template<typename... Ts>
            static constexpr auto IS_TAG_INVOKABLE = meta::tag_invocable<into_nttp_v_fn<To>, Ts...>;

          public:
            template<typename... Ts>
            static constexpr auto operator()(Ts&&...) noexcept
                requires(not IS_TAG_INVOKABLE<Ts...>)
            = delete ("Into caster not defined for these types!");

            template<typename T, usize N>
            [[nodiscard]]
            static constexpr auto operator()(T (&values)[N]) noexcept -> invoke_result<decltype(values)>
                requires(IS_TAG_INVOKABLE<decltype(values)>);

            template<typename T, usize N>
            [[nodiscard]]
            static constexpr auto operator()(T (&&values)[N]) noexcept -> invoke_result<decltype(values)>
                requires(IS_TAG_INVOKABLE<decltype(values)>);

            template<typename... Ts>
            [[nodiscard]]
            static constexpr auto operator()(Ts&&... args) noexcept -> invoke_result<Ts...>
                requires(IS_TAG_INVOKABLE<Ts...>);
        };

        template<template<class...> typename To, typename T, usize N>
        constexpr auto into(T (&values)[N]) -> decltype(auto);

        template<template<class, usize, class...> typename To, typename T, usize N>
        constexpr auto into(T (&values)[N]) -> decltype(auto);

        template<template<class...> typename To, typename T, usize N>
        constexpr auto into(T (&&values)[N]) -> decltype(auto);

        template<template<class, usize, class...> typename To, typename T, usize N>
        constexpr auto into(T (&&values)[N]) -> decltype(auto);

        template<template<class...> typename To, typename... Ts>
        constexpr auto into(Ts&&... args) -> decltype(auto);

        template<template<class, usize, class...> typename To, typename... Ts>
        constexpr auto into(Ts&&... args) -> decltype(auto);

        template<typename T>
        struct is_fn final {
          private:
            template<typename... Ts>
            static constexpr auto IS_TAG_INVOKABLE = meta::tag_invocable<is_fn<T>, Ts...>;

          public:
            template<typename... Ts>
            static constexpr auto operator()(Ts&&...) noexcept
                requires(not IS_TAG_INVOKABLE<Ts...>)
            = delete ("Is queryier not defined for these types!");

            template<typename U>
            [[nodiscard]]
            static constexpr auto operator()(const U& value) noexcept -> bool
                requires(IS_TAG_INVOKABLE<const U&>);

            template<typename First, typename Second>
            [[nodiscard]]
            static constexpr auto operator()(const First& first, const Second& second) noexcept -> bool
                requires(not IS_TAG_INVOKABLE<const First&, const Second&>
                         and meta::has_equality_operator<First, Second>
                         and meta::same_as<T, equal>);

            template<typename First, typename Second>
            [[nodiscard]]
            static constexpr auto operator()(const First& first, const Second& second) noexcept -> bool
                requires(IS_TAG_INVOKABLE<const First&, const Second&> and meta::same_as<T, equal>);
        };

        template<typename T>
        inline constexpr auto is_cpo = is_fn<T> {};

        template<meta::plain::prefer_pass_by_value Value>
        [[nodiscard]]
        auto take(Value value) -> Value
            requires(not meta::const_type<Value>);

        template<meta::plain::prefer_pass_by_ref Value>
        [[nodiscard]]
        auto take(Value&& value) -> meta::to_plain_type<Value>&&
            requires(not meta::const_type<meta::remove_refs_of<Value>>);

        template<typename T, typename U>
        [[nodiscard]]
        constexpr auto is(const U& value) noexcept -> bool;

        template<typename First, typename Second>
        [[nodiscard]]
        constexpr auto is(const First& first, const Second& second) noexcept -> bool;

        template<typename To, typename From>
        [[nodiscard]]
        constexpr auto unchecked_narrow(From from) noexcept -> To
            requires(meta::is_narrowing<To, From> or (meta::arithmetic<To> and meta::arithmetic<From>));

        template<typename T, typename U>
        [[nodiscard]]
        constexpr auto tag_invoke(is_fn<T>, const U& value) noexcept -> bool;

        template<typename To, meta::plain::same_as<To> From>
        [[nodiscard]]
        constexpr auto tag_invoke(as_fn<To>, From&& value, source_location_arg = std::source_location::current()) noexcept
          -> meta::forward_like<From, To>;

        ////////////////////////////////////////////////////////////////////
        ///                      PREDICATE                               ///
        ////////////////////////////////////////////////////////////////////
        template<typename T, meta::plain::unary_predicate<T> Predicate>
        [[nodiscard]]
        constexpr auto tag_invoke(is_fn<equal>, T&& value, Predicate&& predicate) noexcept -> bool;

        ////////////////////////////////////////////////////////////////////
        ///                      POINTERS                                ///
        ////////////////////////////////////////////////////////////////////
        template<meta::polymorphic_type T, meta::negate<meta::pointer> U>
        [[nodiscard]]
        constexpr auto tag_invoke(is_fn<T>, const U& value) noexcept -> bool
            requires(meta::polymorphic_reference<U&>);

        template<meta::polymorphic_type T, meta::polymorphic_pointer U>
        [[nodiscard]]
        constexpr auto tag_invoke(is_fn<T>, const U& value) noexcept -> bool;

        template<meta::polymorphic_type To, meta::negate<meta::pointer> From>
        [[nodiscard]]
        constexpr auto tag_invoke(as_fn<To>, From& value, source_location_arg = std::source_location::current()) noexcept -> To&
            requires(meta::polymorphic_reference<From&>);

        template<meta::polymorphic_type To, meta::polymorphic_pointer From>
        [[nodiscard]]
        constexpr auto tag_invoke(as_fn<To>, const From& value, source_location_arg = std::source_location::current()) noexcept
          -> ref_ptr<To>;

        ////////////////////////////////////////////////////////////////////
        ///                      ARITHMETIC                              ///
        ////////////////////////////////////////////////////////////////////
        template<meta::floating_point First, meta::floating_point Second>
        [[nodiscard]]
        constexpr auto tag_invoke(is_fn<equal>, First first, Second second) noexcept -> bool;

        template<meta::floating_point First, meta::integral Second>
        [[nodiscard]]
        constexpr auto tag_invoke(is_fn<equal>, First first, Second second) noexcept -> bool;

        template<meta::integral First, meta::integral Second>
        [[nodiscard]]
        constexpr auto tag_invoke(is_fn<equal>, First first, Second second) noexcept -> bool;

        template<meta::arithmetic To, meta::arithmetic From>
        [[nodiscard]]
        constexpr auto tag_invoke(as_fn<To>, From value, source_location_arg = std::source_location::current()) noexcept -> To
            requires(not meta::same_as<To, From>);

        ////////////////////////////////////////////////////////////////////
        ///                           BYTES                              ///
        ////////////////////////////////////////////////////////////////////
        template<meta::arithmetic From>
        [[nodiscard]]
        constexpr auto tag_invoke(as_fn<byte>, From value, source_location_arg = std::source_location::current()) noexcept
          -> byte;

        template<meta::arithmetic To>
        [[nodiscard]]
        constexpr auto tag_invoke(as_fn<To>, byte value, source_location_arg = std::source_location::current()) noexcept -> To;

        template<meta::enumeration From>
        [[nodiscard]]
        constexpr auto tag_invoke(as_fn<byte>, From value, source_location_arg = std::source_location::current()) noexcept
          -> byte;

        template<meta::enumeration To>
        [[nodiscard]]
        constexpr auto tag_invoke(as_fn<To>, byte value, source_location_arg = std::source_location::current()) noexcept -> To;

        ////////////////////////////////////////////////////////////////////
        ///                       ENUMERATION                            ///
        ////////////////////////////////////////////////////////////////////
        template<meta::arithmetic To, meta::enumeration From>
        [[nodiscard]]
        constexpr auto tag_invoke(as_fn<To>, From value, source_location_arg = std::source_location::current()) noexcept -> To;

        template<meta::enumeration From>
        [[nodiscard]]
        constexpr auto tag_invoke(as_fn<underlying>, From value, source_location_arg = std::source_location::current()) noexcept
          -> std::underlying_type_t<From>;

        template<meta::enumeration To, meta::arithmetic From>
        [[nodiscard]]
        constexpr auto tag_invoke(as_fn<To>, From value, source_location_arg = std::source_location::current()) noexcept -> To;

        ////////////////////////////////////////////////////////////////////
        ///                          STL                                 ///
        ////////////////////////////////////////////////////////////////////
        template<typename T, meta::std_variant Variant>
        [[nodiscard]]
        constexpr auto tag_invoke(is_fn<T>, const Variant& variant) noexcept -> bool;

        template<typename To, meta::plain::apply_to<meta::std_variant> From>
        [[nodiscard]]
        constexpr auto tag_invoke(as_fn<To>, From&& variant, source_location_arg = std::source_location::current()) noexcept
          -> meta::forward_like<From, To>;

        template<typename T, meta::std_optional Optional>
        [[nodiscard]]
        constexpr auto tag_invoke(is_fn<T>, Optional& optional) noexcept -> bool;

        template<typename To, meta::plain::apply_to<meta::std_optional> From>
        [[nodiscard]]
        constexpr auto tag_invoke(as_fn<To>, From&& optional, source_location_arg = std::source_location::current()) noexcept
          -> meta::forward_like<From, To>;

        template<meta::same_as<empty> T, meta::std_expected Expected>
        [[nodiscard]]
        constexpr auto tag_invoke(is_fn<T>, const Expected& expected) noexcept -> bool;

        template<meta::same_as<error> T, meta::std_expected Expected>
        [[nodiscard]]
        constexpr auto tag_invoke(is_fn<T>, const Expected& expected) noexcept -> bool;

        template<typename T, meta::std_expected Expected>
        [[nodiscard]]
        constexpr auto tag_invoke(is_fn<T>, const Expected& expected) noexcept -> bool
            requires(meta::same_as<T, meta::value_type<Expected>>);

        template<typename T, meta::std_expected Expected>
        [[nodiscard]]
        constexpr auto tag_invoke(is_fn<T>, const Expected& expected) noexcept -> bool
            requires(meta::same_as<T, std::unexpected<meta::error_type<Expected>>>);

        template<typename To, meta::plain::apply_to<meta::std_expected> Expected>
        [[nodiscard]]
        constexpr auto tag_invoke(as_fn<To>, Expected&& expected, source_location_arg = std::source_location::current()) noexcept
          -> meta::forward_like<Expected, To>
            requires(meta::same_as<To, meta::value_type<Expected>>);

        template<typename To, meta::plain::apply_to<meta::std_expected> Expected>
        [[nodiscard]]
        constexpr auto tag_invoke(as_fn<To>, Expected&& expected, source_location_arg = std::source_location::current()) noexcept
          -> meta::forward_like<Expected, To>
            requires(meta::same_as<To, std::unexpected<meta::error_type<Expected>>>);

        template<meta::negate<meta::plain::const_type> To, meta::plain::prefer_pass_by_value Value>
        constexpr auto take(Value value) noexcept -> To
            requires(meta::wrapped_value_of<Value, To>);

        template<meta::negate<meta::plain::const_type> To, meta::plain::prefer_pass_by_ref Value>
        constexpr auto take(Value&& value) noexcept -> To&&
            requires(meta::plain::wrapped_value_of<Value, To> and not meta::const_type<meta::remove_refs_of<Value>>);
    }} // namespace stormkit::core
}

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    namespace details {
        /////////////////////////////////////
        /////////////////////////////////////
        template<typename To, typename From>
        STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
        constexpr auto is_safe_narrowing(const From& from) noexcept -> bool
            requires((meta::arithmetic<To> or meta::is<To, byte> or meta::enumeration<To>)
                     and (meta::arithmetic<From> or meta::is<From, byte> or meta::enumeration<From>))
        {
            if constexpr (meta::arithmetic<From> and meta::arithmetic<To>)
                return (static_cast<From>(static_cast<To>(from)) == from)
                       or (meta::signed_type<To> != meta::unsigned_type<From>
                           and ((static_cast<To>(from) < To {}) == (from < From {})));
            else
                return (static_cast<From>(static_cast<To>(from)) == from);
        }
    } // namespace details

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To>
    template<typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto as_fn<To>::operator()(Ts&&... args) noexcept -> invoke_result<Ts...>
        requires(sizeof...(Ts) >= 1 and IS_TAG_INVOKABLE<Ts...>)
    {
        return meta::tag_invoke_cpo(as_fn<To> {}, std::forward<Ts>(args)..., std::source_location::current());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class...> typename To>
    template<typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto as_nttp_fn<To>::operator()(Ts&&... args) noexcept -> invoke_result<Ts...>
        requires(sizeof...(Ts) >= 1 and IS_TAG_INVOKABLE<Ts...>)
    {
        return meta::tag_invoke_cpo(as_nttp_fn<To> {}, std::forward<Ts>(args)..., std::source_location::current());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class, usize, class...> typename To>
    template<typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto as_nttp_v_fn<To>::operator()(Ts&&... args) noexcept -> invoke_result<Ts...>
        requires(sizeof...(Ts) >= 1 and IS_TAG_INVOKABLE<Ts...>)
    {
        return meta::tag_invoke_cpo(as_nttp_v_fn<To> {}, std::forward<Ts>(args)..., std::source_location::current());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To, typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto as(Ts&&... args) -> decltype(auto) {
        return as_fn<To> {}(std::forward<Ts>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class...> typename To, typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto as(Ts&&... args) -> decltype(auto) {
        return as_nttp_fn<To> {}(std::forward<Ts>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class, usize, class...> typename To, typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto as(Ts&&... args) -> decltype(auto) {
        return as_nttp_v_fn<To> {}(std::forward<Ts>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To>
    template<typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto try_as_fn<To>::operator()(Ts&&... args) noexcept -> invoke_result<Ts...>
        requires(sizeof...(Ts) >= 1 and IS_TAG_INVOKABLE<Ts...>)
    {
        return meta::tag_invoke_cpo(try_as_fn<To> {}, std::forward<Ts>(args)..., std::source_location::current());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class...> typename To>
    template<typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto try_as_nttp_fn<To>::operator()(Ts&&... args) noexcept -> invoke_result<Ts...>
        requires(sizeof...(Ts) >= 1 and IS_TAG_INVOKABLE<Ts...>)
    {
        return meta::tag_invoke_cpo(try_as_nttp_fn<To> {}, std::forward<Ts>(args)..., std::source_location::current());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class, usize, class...> typename To>
    template<typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto try_as_nttp_v_fn<To>::operator()(Ts&&... args) noexcept -> invoke_result<Ts...>
        requires(sizeof...(Ts) >= 1 and IS_TAG_INVOKABLE<Ts...>)
    {
        return meta::tag_invoke_cpo(try_as_nttp_v_fn<To> {}, std::forward<Ts>(args)..., std::source_location::current());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To, typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto try_as(Ts&&... args) -> decltype(auto) {
        return try_as_fn<To> {}(std::forward<Ts>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class...> typename To, typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto try_as(Ts&&... args) -> decltype(auto) {
        return try_as_nttp_fn<To> {}(std::forward<Ts>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class, usize, class...> typename To, typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto try_as(Ts&&... args) -> decltype(auto) {
        return try_as_nttp_v_fn<To> {}(std::forward<Ts>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class...> typename To>
    template<typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto into_nttp_fn<To>::operator()(T (&values)[N]) noexcept -> invoke_result<decltype(values)>
        requires(IS_TAG_INVOKABLE<decltype(values)>)
    {
        return meta::tag_invoke_cpo(into_nttp_fn<To> {}, values);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class...> typename To>
    template<typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto into_nttp_fn<To>::operator()(T (&&values)[N]) noexcept -> invoke_result<decltype(values)>
        requires(IS_TAG_INVOKABLE<decltype(values)>)
    {
        return meta::tag_invoke_cpo(into_nttp_fn<To> {}, values);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class...> typename To>
    template<typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto into_nttp_fn<To>::operator()(Ts&&... args) noexcept -> invoke_result<Ts...>
        requires(IS_TAG_INVOKABLE<Ts...>)
    {
        return meta::tag_invoke_cpo(into_nttp_fn<To> {}, std::forward<Ts>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class, usize, class...> typename To>
    template<typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto into_nttp_v_fn<To>::operator()(T (&values)[N]) noexcept -> invoke_result<decltype(values)>
        requires(IS_TAG_INVOKABLE<decltype(values)>)
    {
        return meta::tag_invoke_cpo(into_nttp_v_fn<To> {}, values);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class, usize, class...> typename To>
    template<typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto into_nttp_v_fn<To>::operator()(T (&&values)[N]) noexcept -> invoke_result<decltype(values)>
        requires(IS_TAG_INVOKABLE<decltype(values)>)
    {
        return meta::tag_invoke_cpo(into_nttp_v_fn<To> {}, values);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class, usize, class...> typename To>
    template<typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto into_nttp_v_fn<To>::operator()(Ts&&... args) noexcept -> invoke_result<Ts...>
        requires(IS_TAG_INVOKABLE<Ts...>)
    {
        return meta::tag_invoke_cpo(into_nttp_v_fn<To> {}, std::forward<Ts>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class...> typename To, typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto into(T (&values)[N]) -> decltype(auto) {
        return into_nttp_fn<To> {}(values);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class, usize, class...> typename To, typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto into(T (&values)[N]) -> decltype(auto) {
        return into_nttp_v_fn<To> {}(values);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class...> typename To, typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto into(T (&&values)[N]) -> decltype(auto) {
        return into_nttp_fn<To> {}(std::move(values));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class, usize, class...> typename To, typename T, usize N>
    STORMKIT_FORCE_INLINE
    constexpr auto into(T (&&values)[N]) -> decltype(auto) {
        return into_nttp_v_fn<To> {}(std::move(values));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class...> typename To, typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto into(Ts&&... args) -> decltype(auto) {
        return into_nttp_fn<To> {}(std::forward<Ts>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class, usize, class...> typename To, typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto into(Ts&&... args) -> decltype(auto) {
        return into_nttp_v_fn<To> {}(std::forward<Ts>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    template<typename U>
    STORMKIT_FORCE_INLINE
    constexpr auto is_fn<T>::operator()(const U& value) noexcept -> bool
        requires(IS_TAG_INVOKABLE<const U&>)
    {
        return meta::tag_invoke_cpo(is_fn<T> {}, value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    template<typename First, typename Second>
    STORMKIT_FORCE_INLINE
    constexpr auto is_fn<T>::operator()(const First& first, const Second& second) noexcept -> bool
        requires(not IS_TAG_INVOKABLE<const First&, const Second&>
                 and meta::has_equality_operator<First, Second>
                 and meta::same_as<T, equal>)
    {
        return first == second;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    template<typename First, typename Second>
    STORMKIT_FORCE_INLINE
    constexpr auto is_fn<T>::operator()(const First& first, const Second& second) noexcept -> bool
        requires(IS_TAG_INVOKABLE<const First&, const Second&> and meta::same_as<T, equal>)
    {
        return meta::tag_invoke_cpo(is_fn<T> {}, first, second);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, typename U>
    STORMKIT_FORCE_INLINE
    constexpr auto is(const U& value) noexcept -> bool {
        return is_cpo<T>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename First, typename Second>
    STORMKIT_FORCE_INLINE
    constexpr auto is(const First& first, const Second& second) noexcept -> bool {
        return is_cpo<equal>(first, second);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To, typename From>
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto unchecked_narrow(From from) noexcept -> To
        requires(meta::is_narrowing<To, From> or (meta::arithmetic<To> and meta::arithmetic<From>))
    {
        return static_cast<To>(from);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To, meta::plain::same_as<To> From>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(as_fn<To>, From&& value, source_location_arg) noexcept -> meta::forward_like<From, To> {
        return std::forward<From>(value);
    }

    ////////////////////////////////////////////////////////////////////
    ///                      PREDICATE                               ///
    ////////////////////////////////////////////////////////////////////
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, meta::plain::unary_predicate<T> Predicate>
        STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(is_fn<equal>, T&& value, Predicate&& predicate) noexcept -> bool {
        return std::forward<Predicate>(predicate)(std::forward<T>(value));
    }

    ////////////////////////////////////////////////////////////////////
    ///                      POINTERS                                ///
    ////////////////////////////////////////////////////////////////////
    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::polymorphic_type T, meta::negate<meta::pointer> U>
        STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(is_fn<T>, const U& value) noexcept -> bool
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
    constexpr auto tag_invoke(is_fn<T>, const U& value) noexcept -> bool {
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
    constexpr auto tag_invoke(as_fn<To>, From& value, source_location_arg location) noexcept -> To&
        requires(meta::polymorphic_reference<From&>)
    {
        expects(is<To>(value), "Invalid polymorphic cast!", location);
        return dynamic_cast<meta::forward_const_to<From, To>&>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::polymorphic_type To, meta::polymorphic_pointer From>
        STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(as_fn<To>, const From& value, source_location_arg location) noexcept -> ref_ptr<To> {
        expects(is<To>(value), "Invalid polymorphic cast!", location);

        if constexpr (meta::raw_pointer<From>) return dynamic_cast<meta::forward_const_to<From, To>*>(value);
        else
            return dynamic_cast<meta::forward_const_to<From, To>*>(value.get());
    }

    ////////////////////////////////////////////////////////////////////
    ///                      ARITHMETIC                              ///
    ////////////////////////////////////////////////////////////////////
    static constexpr auto
      NARROWING_ERROR_MSG = "Lossy narrowing detected, use unchecked_narrow() to explicitly force the conversion and possibly "
                            "lose information!";

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::floating_point First, meta::floating_point Second>
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto tag_invoke(is_fn<equal>, First first, Second second) noexcept -> bool {
        const auto second_ = as<First>(second);

        const auto diff = std::abs(first - second_);
        const auto a    = std::abs(first);
        const auto b    = std::abs(second_);
        const auto ab   = (a > b) ? b : a;
        return diff <= (ab * std::numeric_limits<First>::epsilon());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::floating_point First, meta::integral Second>
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto tag_invoke(is_fn<equal> tag, First first, Second second) noexcept -> bool {
        return tag_invoke(tag, first, as<First>(second));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::integral First, meta::integral Second>
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto tag_invoke(is_fn<equal>, First first, Second second) noexcept -> bool {
        if constexpr (meta::is_narrowing<First, Second>) {
            using SafeT        = meta::safe_narrow_type<First, Second>;
            const auto _first  = as<SafeT>(first);
            const auto _second = as<SafeT>(second);
            return _first == _second;
        } else
            return first == second;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::arithmetic To, meta::arithmetic From>
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto tag_invoke(as_fn<To>, From value, [[maybe_unused]] source_location_arg location) noexcept -> To
        requires(not meta::same_as<To, From>)
    {
        if constexpr (meta::is_narrowing<To, From>) expects(details::is_safe_narrowing<To>(value), NARROWING_ERROR_MSG, location);

        return static_cast<To>(value);
    }

    ////////////////////////////////////////////////////////////////////
    ///                      BYTES                                   ///
    ////////////////////////////////////////////////////////////////////
    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::arithmetic From>
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto tag_invoke(as_fn<byte>, From value, [[maybe_unused]] source_location_arg location) noexcept -> byte {
        if constexpr (meta::is_narrowing<byte, From>)
            expects(details::is_safe_narrowing<byte>(value), NARROWING_ERROR_MSG, location);

        return static_cast<byte>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::arithmetic To>
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto tag_invoke(as_fn<To>, byte value, source_location_arg) noexcept -> To {
        return static_cast<To>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::enumeration From>
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto tag_invoke(as_fn<byte>, From value, [[maybe_unused]] source_location_arg location) noexcept -> byte {
        if constexpr (meta::is_narrowing<byte, From>)
            expects(details::is_safe_narrowing<byte>(value), NARROWING_ERROR_MSG, location);

        return static_cast<byte>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::enumeration To>
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto tag_invoke(as_fn<To>, byte value, source_location_arg) noexcept -> To {
        return static_cast<To>(value);
    }

    ////////////////////////////////////////////////////////////////////
    ///                       ENUMERATION                            ///
    ////////////////////////////////////////////////////////////////////
    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::arithmetic To, meta::enumeration From>
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto tag_invoke(as_fn<To>, From value, [[maybe_unused]] source_location_arg location) noexcept -> To {
        using From_ = meta::to_plain_type<From>;

        if constexpr (meta::is_narrowing<To, From_>)
            expects(details::is_safe_narrowing<To>(value), NARROWING_ERROR_MSG, location);

        return static_cast<To>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::enumeration From>
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto tag_invoke(as_fn<underlying>, From value, source_location_arg) noexcept -> std::underlying_type_t<From> {
        return std::to_underlying(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::enumeration To, meta::arithmetic From>
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto tag_invoke(as_fn<To>, From value, [[maybe_unused]] source_location_arg location) noexcept -> To {
        if constexpr (meta::is_narrowing<std::underlying_type_t<To>, From>)
            expects(details::is_safe_narrowing<std::underlying_type_t<To>>(value), NARROWING_ERROR_MSG, location);

        return static_cast<To>(value);
    }

    ////////////////////////////////////////////////////////////////////
    ///                          STL                                 ///
    ////////////////////////////////////////////////////////////////////
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, meta::std_variant Variant>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(is_fn<T>, const Variant& value) noexcept -> bool {
        if constexpr (meta::same_as<T, empty>) {
            if constexpr (requires { variant_contains_type<std::monostate>(std::declval<Variant>()) == true; })
                return std::get_if<std::monostate>(&value) != nullptr;
            else
                return false;
        } else {
            return meta::variant_type_find_if(value,
                                              [&value]<typename It>(const It&) noexcept -> bool {
                                                  if (value.index() == It::INDEX)
                                                      return meta::same_as<T, std::variant_alternative_t<It::INDEX, Variant>>;
                                                  return false;
                                              })
                   != std::variant_npos;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To, meta::plain::apply_to<meta::std_variant> Variant>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(as_fn<To>, Variant&& value, source_location_arg location) noexcept
      -> meta::forward_like<Variant, To> {
        auto ptr = raw_ptr<meta::forward_const_to<Variant, To>> { nullptr };
        meta::variant_type_find_if(std::forward<Variant>(value), [&ptr, &value]<typename It>(const It&) noexcept -> bool {
            if constexpr (meta::same_as<typename It::type, To>) {
                if (std::forward<Variant>(value).index() == It::INDEX) ptr = &std::get<It::index>(value);
                return true;
            }

            return false;
        });

        ensures(ptr != nullptr, "Bad variant access!", location);
        return std::forward_like<Variant>(*ptr);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, meta::std_optional Optional>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(is_fn<T>, const Optional& value) noexcept -> bool {
        if (not value.has_value()) return meta::same_as<T, empty>;
        return meta::is<T, meta::value_type<Optional>>;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To, meta::plain::apply_to<meta::std_optional> Optional>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(as_fn<To>, Optional&& value, source_location_arg location) noexcept
      -> meta::forward_like<Optional, To> {
        ensures(is<To>(value), "Bad optional access!", location);

        return std::forward_like<Optional>(value.value());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::same_as<empty> T, meta::std_expected Expected>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(is_fn<T>, const Expected& value) noexcept -> bool {
        return not value.has_value();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::same_as<error> T, meta::std_expected Expected>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(is_fn<T>, const Expected& value) noexcept -> bool {
        return not value.has_value();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, meta::std_expected Expected>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(is_fn<T>, const Expected& value) noexcept -> bool
        requires(meta::same_as<T, meta::value_type<Expected>>)
    {
        return value.has_value();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, meta::std_expected Expected>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(is_fn<T>, const Expected& value) noexcept -> bool
        requires(meta::same_as<T, std::unexpected<meta::error_type<Expected>>>)
    {
        return not value.has_value();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To, meta::plain::apply_to<meta::std_expected> Expected>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(as_fn<To>, Expected&& value, source_location_arg location) noexcept
      -> meta::forward_like<Expected, To>
        requires(meta::same_as<To, meta::value_type<Expected>>)
    {
        ensures(value.has_value(), "Bad expected access!", location);

        return std::forward_like<Expected>(value.value());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename To, meta::plain::apply_to<meta::std_expected> Expected>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(as_fn<To>, Expected&& value, source_location_arg location) noexcept
      -> meta::forward_like<Expected, To>
        requires(meta::same_as<To, std::unexpected<meta::error_type<Expected>>>)
    {
        ensures(not value.has_value(), "Bad expected access!", location);

        return std::forward_like<Expected>(value.error());
    }
}} // namespace stormkit::core
