// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

// TODO find a proper way to get these
#define CONCEPTS                                                                                           \
    template<typename First, typename... Ts>                                                               \
    concept same_as = apply_to<First, meta::same_as, Ts...>;                                               \
    template<typename First, typename... Ts>                                                               \
    concept is = apply_to<First, meta::is, Ts...>;                                                         \
    template<typename First, typename... Ts>                                                               \
    concept is_any_of = apply_to<First, meta::is_any_of, Ts...>;                                           \
    template<typename First, typename... Ts>                                                               \
    concept same_as_any_of = apply_to<First, meta::same_as_any_of, Ts...>;                                 \
    template<typename First, typename... Ts>                                                               \
    concept same_types_as = apply_to<First, meta::same_types_as, Ts...>;                                   \
    template<typename First, typename... Ts>                                                               \
    concept convertible_to = apply_to<First, meta::convertible_to, Ts...>;                                 \
    template<typename First, typename... Ts>                                                               \
    concept explicitly_convertible_to = apply_to<First, meta::explicitly_convertible_to, Ts...>;           \
    template<typename First, typename... Ts>                                                               \
    concept boolean_testable = apply_to<First, meta::boolean_testable, Ts...>;                             \
    template<typename First, typename... Ts>                                                               \
    concept std_optional = apply_to<First, meta::std_optional, Ts...>;                                     \
    template<typename First, typename... Ts>                                                               \
    concept std_expected = apply_to<First, meta::std_expected, Ts...>;                                     \
    template<typename First, typename... Ts>                                                               \
    concept std_variant = apply_to<First, meta::std_variant, Ts...>;                                       \
    template<typename First, typename... Ts>                                                               \
    concept std_span = apply_to<First, meta::std_span, Ts...>;                                             \
    template<typename First, typename... Ts>                                                               \
    concept std_mdspan = apply_to<First, meta::std_mdspan, Ts...>;                                         \
    template<typename First, typename... Ts>                                                               \
    concept std_array = apply_to<First, meta::std_array, Ts...>;                                           \
    template<typename First, typename... Ts>                                                               \
    concept std_reference_wrapper = apply_to<First, meta::std_reference_wrapper, Ts...>;                   \
    template<typename First, typename... Ts>                                                               \
    concept raw_pointer = apply_to<First, meta::raw_pointer, Ts...>;                                       \
    template<typename First, typename... Ts>                                                               \
    concept view_pointer = apply_to<First, meta::view_pointer, Ts...>;                                     \
    template<typename First, typename... Ts>                                                               \
    concept owning_pointer = apply_to<First, meta::owning_pointer, Ts...>;                                 \
    template<typename First, typename... Ts>                                                               \
    concept pointer = apply_to<First, meta::pointer, Ts...>;                                               \
    template<typename First, typename... Ts>                                                               \
    concept pointer_to = apply_to<First, meta::pointer_to, Ts...>;                                         \
    template<typename First, typename... Ts>                                                               \
    concept has_element_type = apply_to<First, meta::has_element_type, Ts...>;                             \
    template<typename First, typename... Ts>                                                               \
    concept has_value_type = apply_to<First, meta::has_value_type, Ts...>;                                 \
    template<typename First, typename... Ts>                                                               \
    concept has_error_type = apply_to<First, meta::has_error_type, Ts...>;                                 \
    template<typename First, typename... Ts>                                                               \
    concept has_expected_type = apply_to<First, meta::has_expected_type, Ts...>;                           \
    template<typename First, typename... Ts>                                                               \
    concept wrapped_value = apply_to<First, meta::wrapped_value, Ts...>;                                   \
    template<typename First, typename... Ts>                                                               \
    concept wrapped_value_of = apply_to<First, meta::wrapped_value_of, Ts...>;                             \
    template<typename First, typename... Ts>                                                               \
    concept polymorphic_type = apply_to<First, meta::polymorphic_type, Ts...>;                             \
    template<typename First, typename... Ts>                                                               \
    concept polymorphic_pointer = apply_to<First, meta::polymorphic_pointer, Ts...>;                       \
    template<typename First, typename... Ts>                                                               \
    concept polymorphic_reference = apply_to<First, meta::polymorphic_reference, Ts...>;                   \
    template<typename First, typename... Ts>                                                               \
    concept polymorphic_indirection = apply_to<First, meta::polymorphic_indirection, Ts...>;               \
    template<typename First, typename... Ts>                                                               \
    concept scoped_enum = apply_to<First, meta::scoped_enum, Ts...>;                                       \
    template<typename First, typename... Ts>                                                               \
    concept c_enum = apply_to<First, meta::c_enum, Ts...>;                                                 \
    template<typename First, typename... Ts>                                                               \
    concept enumeration = apply_to<First, meta::enumeration, Ts...>;                                       \
    template<typename First, typename... Ts>                                                               \
    concept integral = apply_to<First, meta::integral, Ts...>;                                             \
    template<typename First, typename... Ts>                                                               \
    concept floating_point = apply_to<First, meta::floating_point, Ts...>;                                 \
    template<typename First, typename... Ts>                                                               \
    concept arithmetic = apply_to<First, meta::arithmetic, Ts...>;                                         \
    template<typename First, typename... Ts>                                                               \
    concept scalar = apply_to<First, meta::scalar, Ts...>;                                                 \
    template<typename First, typename... Ts>                                                               \
    concept predicate = apply_to<First, meta::predicate, Ts...>;                                           \
    template<typename First, typename... Ts>                                                               \
    concept unary_predicate = apply_to<First, meta::unary_predicate, Ts...>;                               \
    template<typename First, typename... Ts>                                                               \
    concept binary_predicate = apply_to<First, meta::binary_predicate, Ts...>;                             \
    template<typename First, typename... Ts>                                                               \
    concept char_type = apply_to<First, meta::char_type, Ts...>;                                           \
    template<typename First, typename... Ts>                                                               \
    concept const_type = apply_to<First, meta::const_type, Ts...>;                                         \
    template<typename First, typename... Ts>                                                               \
    concept volatile_type = apply_to<First, meta::volatile_type, Ts...>;                                   \
    template<typename First, typename... Ts>                                                               \
    concept brace_initializable_to = apply_to<First, meta::brace_initializable_to, Ts...>;                 \
    template<typename First, typename... Ts>                                                               \
    concept unsigned_type = apply_to<First, meta::unsigned_type, Ts...>;                                   \
    template<typename First, typename... Ts>                                                               \
    concept signed_type = apply_to<First, meta::signed_type, Ts...>;                                       \
    template<typename First, typename... Ts>                                                               \
    concept has_same_signeness = apply_to<First, meta::has_same_signeness, Ts...>;                         \
    template<typename First, typename... Ts>                                                               \
    concept is_sign_narrowing = apply_to<First, meta::is_sign_narrowing, Ts...>;                           \
    template<typename First, typename... Ts>                                                               \
    concept is_byte_narrowing = apply_to<First, meta::is_byte_narrowing, Ts...>;                           \
    template<typename First, typename... Ts>                                                               \
    concept is_narrowing = apply_to<First, meta::is_narrowing, Ts...>;                                     \
    template<typename First, typename... Ts>                                                               \
    concept has_equality_operator = apply_to<First, meta::has_equality_operator, Ts...>;                   \
    template<typename First, typename... Ts>                                                               \
    concept default_constructible = apply_to<First, meta::default_constructible, Ts...>;                   \
    template<typename First, typename... Ts>                                                               \
    concept assignable_from = apply_to<First, meta::assignable_from, Ts...>;                               \
    template<typename First, typename... Ts>                                                               \
    concept constructible_from = apply_to<First, meta::constructible_from, Ts...>;                         \
    template<typename First, typename... Ts>                                                               \
    concept destructible = apply_to<First, meta::destructible, Ts...>;                                     \
    template<typename First, typename... Ts>                                                               \
    concept copyable = apply_to<First, meta::copyable, Ts...>;                                             \
    template<typename First, typename... Ts>                                                               \
    concept movable = apply_to<First, meta::movable, Ts...>;                                               \
    template<typename First, typename... Ts>                                                               \
    concept copy_assignable = apply_to<First, meta::copy_assignable, Ts...>;                               \
    template<typename First, typename... Ts>                                                               \
    concept move_assignable = apply_to<First, meta::move_assignable, Ts...>;                               \
    template<typename First, typename... Ts>                                                               \
    concept trivially_copyable = apply_to<First, meta::trivially_copyable, Ts...>;                         \
    template<typename First, typename... Ts>                                                               \
    concept trivially_movable = apply_to<First, meta::trivially_movable, Ts...>;                           \
    template<typename First, typename... Ts>                                                               \
    concept trivially_assignable_from = apply_to<First, meta::trivially_assignable_from, Ts...>;           \
    template<typename First, typename... Ts>                                                               \
    concept trivially_copy_assignable = apply_to<First, meta::trivially_copy_assignable, Ts...>;           \
    template<typename First, typename... Ts>                                                               \
    concept trivially_move_assignable = apply_to<First, meta::trivially_move_assignable, Ts...>;           \
    template<typename First, typename... Ts>                                                               \
    concept noexcept_default_constructible = apply_to<First, meta::noexcept_default_constructible, Ts...>; \
    template<typename First, typename... Ts>                                                               \
    concept noexcept_constructible_from = apply_to<First, meta::noexcept_constructible_from, Ts...>;       \
    template<typename First, typename... Ts>                                                               \
    concept noexcept_copyable = apply_to<First, meta::noexcept_copyable, Ts...>;                           \
    template<typename First, typename... Ts>                                                               \
    concept noexcept_movable = apply_to<First, meta::noexcept_movable, Ts...>;                             \
    template<typename First, typename... Ts>                                                               \
    concept noexcept_copy_assignable = apply_to<First, meta::noexcept_copy_assignable, Ts...>;             \
    template<typename First, typename... Ts>                                                               \
    concept noexcept_move_assignable = apply_to<First, meta::noexcept_move_assignable, Ts...>;             \
    template<typename First, typename... Ts>                                                               \
    concept noexcept_assignable_from = apply_to<First, meta::noexcept_assignable_from, Ts...>;             \
    template<typename First, typename... Ts>                                                               \
    concept noexcept_destructible = apply_to<First, meta::noexcept_destructible, Ts...>;                   \
    template<typename First, typename... Ts>                                                               \
    concept prefer_pass_by_value = apply_to<First, meta::prefer_pass_by_value, Ts...>;                     \
    template<typename First, typename... Ts>                                                               \
    concept prefer_pass_by_ref = apply_to<First, meta::prefer_pass_by_ref, Ts...>;                         \
    template<typename First, typename... Ts>                                                               \
    concept hash_type = apply_to<First, meta::hash_type, Ts...>;

export module stormkit.core.meta.type_manipulation;

import std;

import stormkit.core.meta.concepts;
import stormkit.core.meta.algorithms;

namespace stormkit { inline namespace core { namespace meta::details {
    template<typename T, typename U>
    struct forward_const_to;

    template<typename T, typename U>
    struct forward_ref_to;
}}} // namespace stormkit::core::meta::details

export namespace stormkit { inline namespace core { namespace meta {
    template<typename T>
    using to_plain_type = std::remove_cvref_t<T>;

    template<typename T>
    using to_decayed_type = std::decay_t<T>;

    template<typename T>
    using add_const_to = std::add_const_t<T>;

    template<typename T>
    using add_volatile_to = std::add_volatile_t<T>;

    template<typename T>
    using add_rvalue_ref_to = std::add_rvalue_reference_t<T>;

    template<typename T>
    using add_lvalue_ref_to = std::add_lvalue_reference_t<T>;

    template<typename T>
    using remove_const_of = std::remove_const_t<T>;

    template<typename T>
    using remove_refs_of = std::remove_reference_t<T>;

    template<typename T>
    using remove_pointer_of = std::remove_pointer_t<T>;

    template<typename T>
    using remove_indirections_of = remove_pointer_of<remove_refs_of<T>>;

    template<typename T, typename U>
    using forward_const_to = details::forward_const_to<T, U>::type;

    template<typename T, typename U>
    using forward_ref_to = details::forward_ref_to<T, U>::type;

    template<typename T, typename U>
    using forward_like = forward_ref_to<T, forward_const_to<T, U>>;

    template<template<typename> typename TypeModifier, template<typename...> concept C, typename... Ts>
    concept apply = C<TypeModifier<Ts>...>;

    namespace decayed {
        template<typename First, template<typename...> concept C, typename... Ts>
        concept apply_to = apply<meta::to_decayed_type, C, Ts..., First>;

        template<typename S, template<typename...> typename T>
        concept specialization_of = meta::specialization_of<meta::to_plain_type<S>, T>;

        template<typename S, template<class, auto...> typename T>
        concept specialization_of_nttp_tv = meta::specialization_of_nttp_tv<meta::to_plain_type<S>, T>;

        template<typename S, template<class, class, auto> typename T>
        concept specialization_of_nttp_ttv = meta::specialization_of_nttp_ttv<meta::to_plain_type<S>, T>;

        template<typename S, template<class, class, auto, class...> typename T>
        concept specialization_of_nttp_ttvts = meta::specialization_of_nttp_ttvts<meta::to_plain_type<S>, T>;

        CONCEPTS
    } // namespace decayed

    namespace plain {
        template<typename First, template<typename...> concept C, typename... Ts>
        concept apply_to = apply<meta::to_plain_type, C, First, Ts...>;

        template<typename S, template<typename...> typename T>
        concept specialization_of = meta::specialization_of<meta::to_plain_type<S>, T>;

        template<typename S, template<class, auto...> typename T>
        concept specialization_of_nttp_tv = meta::specialization_of_nttp_tv<meta::to_plain_type<S>, T>;

        template<typename S, template<class, class, auto> typename T>
        concept specialization_of_nttp_ttv = meta::specialization_of_nttp_ttv<meta::to_plain_type<S>, T>;

        template<typename S, template<class, class, auto, class...> typename T>
        concept specialization_of_nttp_ttvts = meta::specialization_of_nttp_ttvts<meta::to_plain_type<S>, T>;

        CONCEPTS
    } // namespace plain
}}} // namespace stormkit::core::meta

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core { namespace meta::details {
    template<typename T, typename U>
    struct forward_const_to {
      private:
        using T1 = remove_indirections_of<T>;
        using U1 = conditional<const_type<T1>, add_const_to<U>, U>;
        using U2 = conditional<volatile_type<T1>, add_volatile_to<U1>, U1>;

      public:
        using type = U2;
    };

    template<typename T, typename U>
    struct forward_ref_to {
      private:
        using U2 = conditional<lvalue_ref<T>, add_lvalue_ref_to<U>, U>;
        using U3 = conditional<rvalue_ref<T>, add_rvalue_ref_to<U2>, U2>;

      public:
        using type = U2;
    };
}}} // namespace stormkit::core::meta::details
