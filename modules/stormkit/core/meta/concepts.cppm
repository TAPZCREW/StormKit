// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.meta.concepts;

import std;

import stormkit.core.types;

namespace stormkit { inline namespace core { namespace meta::details {
    template<template<class...> typename T, typename... Ts>
    constexpr auto is_specialization_of_helper(const T<Ts...>&) noexcept -> std::true_type;
    template<template<typename, auto...> typename T, typename T2, auto... Ts>
    constexpr auto is_specialization_of_with_nttp_helper(const T<T2, Ts...>&) noexcept -> std::true_type;
    template<template<typename, auto...> typename T, typename T1, auto... Ts>
    constexpr auto is_specialization_of_helper_nttp_tv(const T<T1, Ts...>&) noexcept -> std::true_type;
    template<template<class, class, auto...> typename T, typename T1, typename T2, auto... Ts>
    constexpr auto is_specialization_of_helper_nttp_ttv(const T<T1, T2, Ts...>&) noexcept -> std::true_type;
    template<template<class, class, auto, class...> typename T, typename T1, typename T2, auto Arg, typename... Ts>
    constexpr auto is_specialization_of_helper_nttp_ttvts(const T<T1, T2, Arg, Ts...>&) noexcept -> std::true_type;
    template<template<class, class, auto, template<class> class...> typename T,
             typename T1,
             typename T2,
             auto Arg,
             template<class> typename... Ts>
    constexpr auto is_specialization_of_helper_nttp_ttvtcs(const T<T1, T2, Arg, Ts...>&) noexcept -> std::true_type;
}}} // namespace stormkit::core::meta::details

namespace stdr = std::ranges;

export namespace stormkit { inline namespace core { namespace meta {
    using std::same_as;

    using std::derived_from;

    template<typename T, typename U>
    concept is = same_as<T, U> or (std::is_polymorphic_v<T> and std::is_polymorphic_v<U> and std::derived_from<T, U>);

    template<typename T, typename... U>
    concept is_any_of = (is<T, U> or ...);

    template<typename T, typename... U>
    concept same_as_any_of = (same_as<T, U> or ...);

    template<typename T, typename... Ts>
    concept same_types_as = (same_as<T, Ts> and ...);

    using std::convertible_to;

    template<typename From, typename To>
    concept explicitly_convertible_to = convertible_to<From, To> or requires(From val) { static_cast<To>(val); };

    template<typename T>
    concept is_decayed = same_as<T, std::decay_t<T>>;

    template<typename T>
    concept boolean_testable = convertible_to<T, bool>;

    template<typename S, template<class...> typename T>
    concept specialization_of = requires(S&& s) {
        { details::is_specialization_of_helper<T>(std::forward<S>(s)) } -> is<std::true_type>;
    };

    template<typename S, template<class, auto...> typename T>
    concept specialization_of_nttp_tv = requires(S&& s) {
        { details::is_specialization_of_helper_nttp_tv<T>(std::forward<S>(s)) } -> is<std::true_type>;
    };

    template<typename S, template<class, class, auto> typename T>
    concept specialization_of_nttp_ttv = requires(S&& s) {
        { details::is_specialization_of_helper_nttp_ttv<T>(std::forward<S>(s)) } -> is<std::true_type>;
    };

    template<typename S, template<class, class, auto, class...> typename T>
    concept specialization_of_nttp_ttvts = requires(S&& s) {
        { details::is_specialization_of_helper_nttp_ttvts<T>(std::forward<S>(s)) } -> is<std::true_type>;
    };

    template<typename S, template<class, class, auto, template<class> class...> typename T>
    concept specialization_of_nttp_ttvtcs = requires(S&& s) {
        { details::is_specialization_of_helper_nttp_ttvtcs<T>(std::forward<S>(s)) } -> is<std::true_type>;
    };

    template<typename T>
    concept std_optional = specialization_of<T, std::optional>;

    template<typename T>
    concept std_expected = specialization_of<T, std::expected>;

    template<typename T>
    concept std_variant = specialization_of<T, std::variant>;

    template<typename T>
    concept std_span = specialization_of_nttp_tv<T, std::span>;

    template<typename T>
    concept std_mdspan = specialization_of<T, std::mdspan>;

    template<typename T>
    concept std_array = specialization_of_nttp_tv<T, std::array>;

    template<typename T>
    concept std_reference_wrapper = specialization_of<T, std::reference_wrapper>;

    template<typename T>
    concept lvalue_ref = std::is_lvalue_reference_v<T>;

    template<typename T>
    concept rvalue_ref = std::is_rvalue_reference_v<T>;

    template<typename T>
    concept reference = lvalue_ref<T> or rvalue_ref<T>;

    template<typename T, typename U>
    concept reference_to = reference<T> and is<std::remove_reference_t<T>, U>;

    template<typename T>
    concept raw_pointer = std::is_pointer_v<T>;

    template<typename T>
    concept owning_pointer = requires { typename std::pointer_traits<T>::element_type; } and requires(T a) {
        { a.operator->() } -> convertible_to<decltype(&*a)>;
        { a.operator*() };
        { a == nullptr } -> boolean_testable;
        { a.reset() };
    };

    template<typename T>
    concept view_pointer = not owning_pointer<T> and (raw_pointer<T> or (requires {
        typename std::pointer_traits<T>::element_type;
    } and requires(T a) {
        { a.operator->() } -> convertible_to<decltype(&*a)>;
        { a.operator*() };
        { a == nullptr } -> boolean_testable;
    }) or std_reference_wrapper<T>);

    template<typename T>
    concept pointer = view_pointer<T> or owning_pointer<T>;

    template<typename T, typename U>
    concept pointer_to = pointer<T> and is<typename std::pointer_traits<T>::element_type, U>;

    template<typename T>
    concept indirection = reference<T> or pointer<T>;

    template<typename T, typename U>
    concept indirection_to = indirection<T> and (pointer_to<T, U> or reference_to<T, U>);

    template<typename T>
    concept raw_indirection = reference<T> or raw_pointer<T>;

    template<typename T, typename U>
    concept raw_indirection_to = raw_indirection<T> and (pointer_to<T, U> or reference_to<T, U>);

    template<typename T>
    concept has_element_type = requires() { typename T::element_type; };

    template<typename T>
    concept has_value_type = requires() { typename T::value_type; };

    template<typename T>
    concept has_error_type = requires() { typename T::error_type; };

    template<typename T>
    concept has_expected_type = requires() { typename T::expected_type; } and std_expected<typename T::expected_type>;

    template<typename T>
    concept wrapped_value = has_value_type<T> and requires(T& val) {
        { val.value() } -> reference_to<typename T::value_type>;
        { val.operator*() } -> reference_to<typename T::value_type>;
        { val.operator->() } -> reference_to<typename T::value_type*>;
    };

    template<typename T, typename U>
    concept wrapped_value_of = wrapped_value<T> and is<typename T::value_type, U>;

    template<typename T>
    concept polymorphic_type = std::is_polymorphic_v<T>;

    template<typename T>
    concept polymorphic_pointer = pointer<T> and polymorphic_type<typename std::pointer_traits<T>::element_type>;

    template<typename T>
    concept polymorphic_reference = reference<T> and polymorphic_type<std::remove_reference_t<T>>;

    template<typename T>
    concept polymorphic_indirection = polymorphic_reference<T> or polymorphic_pointer<T>;

    template<typename T>
    concept scoped_enum = std::is_scoped_enum_v<T> and not is<T, byte>;

    template<typename T>
    concept c_enum = not scoped_enum<T> and std::is_enum_v<T> and not is<T, byte>;

    template<typename T>
    concept enumeration = std::is_enum_v<T> and not is<T, std::byte>;

    template<typename T>
    concept integral = (std::integral<T> and not is<T, bool> and not is<T, byte>)
                       or is<T, stdr::range_difference_t<stdr::iota_view<longlong, longlong>>>
                       or is<T, stdr::range_difference_t<stdr::iota_view<ulonglong, ulonglong>>>
                       or is<T, i128>
                       or is<T, u128>;

    using std::floating_point;

    template<typename T>
    concept arithmetic = (integral<T> or floating_point<T>) and not pointer<T> and not enumeration<T>;

    template<typename T>
    concept scalar = arithmetic<T> or raw_pointer<T> or enumeration<T>;

    using std::predicate;

    template<typename T, typename U>
    concept unary_predicate = predicate<T, const U&>;

    template<typename T, typename U>
    concept binary_predicate = predicate<T, const U&, const U&>;

    template<typename T>
    concept char_type = is_any_of<T, char, wchar_t, char8_t, char16_t, char32_t>;

    template<typename T>
    concept const_type = std::is_const_v<T>;

    template<typename T>
    concept volatile_type = std::is_volatile_v<T>;

    template<class From, typename To>
    concept brace_initializable_to = requires(From&& from) { To { std::forward<From>(from) }; };

    template<typename T>
    concept structural_type = requires { []<T t> {}; };

    template<typename T>
    concept unsigned_type = std::is_unsigned_v<T> or is<T, u128>;

    template<typename T>
    concept signed_type = std::is_signed_v<T> or is<T, i128>;

    template<typename T, typename U>
    concept has_same_signeness = (signed_type<T> and signed_type<U>) or (unsigned_type<T> and unsigned_type<U>);

    template<typename To, typename From>
    concept is_sign_narrowing = (signed_type<From> ? not signed_type<To> : signed_type<To> and sizeof(From) == sizeof(To));

    template<typename To, typename From>
    concept is_byte_narrowing = ((arithmetic<To> and is<From, byte>) or (is<To, byte> and arithmetic<From>))
                                and (is<To, byte> and sizeof(To) != sizeof(From));

    template<typename To, typename From>
    concept is_narrowing = (floating_point<From> and integral<To>)
                           or (floating_point<From> and meta::floating_point<To> and sizeof(From) > sizeof(To))
                           or ((integral<From> or enumeration<From>) and meta::floating_point<To>)
                           or (integral<From> and integral<To> and (sizeof(From) > sizeof(To) or is_sign_narrowing<To, From>))
                           or (enumeration<From>
                               and integral<To>
                               and (sizeof(From) > sizeof(To) or is_sign_narrowing<std::underlying_type_t<From>, From>))
                           or (pointer<From> and is<To, bool>)
                           or (arithmetic<From> and is<To, byte>);

    template<typename T1, typename T2>
    concept has_equality_operator = requires(const T1& first, const T2& second) {
        { first == second } -> boolean_testable;
    };

    template<typename T>
    concept default_constructible = std::default_initializable<T>;

    using std::assignable_from;

    using std::constructible_from;

    using std::destructible;

    using std::copyable;

    using std::movable;

    template<typename T>
    concept copy_assignable = assignable_from<T&, const T&>;

    template<typename T>
    concept move_assignable = assignable_from<T&, T&&>;

    template<typename T>
    concept trivially_copyable = copyable<T> and std::is_trivially_copy_constructible_v<T>;

    template<typename T>
    concept trivially_movable = movable<T> and std::is_trivially_move_constructible_v<T>;

    template<typename T>
    concept trivially_destructible = destructible<T> and std::is_trivially_destructible_v<T>;

    template<typename T>
    concept trivially_relocatable = (trivially_copyable<T> and trivially_destructible<T>)
#if defined(__has_builtin)
    #if __has_builtin(__builtin_is_cpp_trivially_relocatable)
                                    or __builtin_is_cpp_trivially_relocatable(T)
    #elif __has_builtin(__is_trivially_relocatable)
                                    or __is_trivially_relocatable(T)
    #endif
#endif
      ;
    template<typename T, typename U>
    concept trivially_assignable_from = assignable_from<T, U> and std::is_trivially_assignable_v<std::remove_reference_t<T>, U>;

    template<typename T>
    concept trivially_copy_assignable = assignable_from<T&, const T&> and std::is_trivially_copy_assignable_v<T>;

    template<typename T>
    concept trivially_move_assignable = assignable_from<T&, T&&> and std::is_trivially_move_assignable_v<T>;

    template<typename T>
    concept noexcept_default_constructible = default_constructible<T> and std::is_nothrow_default_constructible_v<T>;

    template<typename T, typename... Ts>
    concept noexcept_constructible_from = constructible_from<T, Ts...> and std::is_nothrow_constructible_v<T, Ts...>;

    template<typename T>
    concept noexcept_copyable = copyable<T> and std::is_nothrow_copy_constructible_v<T>;

    template<typename T>
    concept noexcept_movable = movable<T> and std::is_nothrow_move_constructible_v<T>;

    template<typename T>
    concept noexcept_copy_assignable = copy_assignable<T> and std::is_nothrow_copy_assignable_v<T>;

    template<typename T>
    concept noexcept_move_assignable = move_assignable<T> and std::is_nothrow_move_assignable_v<T>;

    template<typename T, typename U>
    concept noexcept_assignable_from = assignable_from<T&, U> and std::is_nothrow_assignable_v<T, U>;

    template<typename T>
    concept noexcept_destructible = destructible<T> and std::is_nothrow_destructible_v<T>;

    template<typename T>
    concept prefer_pass_by_value = sizeof(T) <= (sizeof(void*) * 2) and trivially_copyable<T>;

    template<typename T>
    concept prefer_pass_by_ref = not prefer_pass_by_value<T>;

    template<typename T>
    concept hash_type = is_any_of<T, hash32, hash64>;
}}} // namespace stormkit::core::meta

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core { namespace meta::details {
    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class...> typename T, typename... Ts>
    constexpr auto is_specialization_of_helper(const T<Ts...>&) noexcept -> std::true_type {
        return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<typename, auto...> typename T, typename T2, auto... Ts>
    constexpr auto is_specialization_of_with_nttp_helper(const T<T2, Ts...>&) noexcept -> std::true_type {
        return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<typename, auto...> typename T, typename T1, auto... Ts>
    constexpr auto is_specialization_of_helper_nttp_tv(const T<T1, Ts...>&) noexcept -> std::true_type {
        return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class, class, auto...> typename T, typename T1, typename T2, auto... Ts>
    constexpr auto is_specialization_of_helper_nttp_ttv(const T<T1, T2, Ts...>&) noexcept -> std::true_type {
        return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class, class, auto, class...> typename T, typename T1, typename T2, auto Arg, typename... Ts>
    constexpr auto is_specialization_of_helper_nttp_ttvts(const T<T1, T2, Arg, Ts...>&) noexcept -> std::true_type {
        return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<template<class, class, auto, template<class> class...> typename T,
             typename T1,
             typename T2,
             auto Arg,
             template<class> typename... Ts>
    constexpr auto is_specialization_of_helper_nttp_ttvtcs(const T<T1, T2, Arg, Ts...>&) noexcept -> std::true_type {
        return {};
    }
}}} // namespace stormkit::core::meta::details
