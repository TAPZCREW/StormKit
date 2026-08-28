// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.meta.type_query;

import std;

import stormkit.core.meta.concepts;
import stormkit.core.meta.type_manipulation;
import stormkit.core.meta.algorithms;

namespace stdr = std::ranges;

namespace stormkit { inline namespace core { namespace meta::details {
    template<typename T>
    struct pointer_type;
    template<typename T>
    struct pointed_type;
    template<typename T>
    struct indirection_target_type;
    template<typename T>
    struct callable_trait;
}}} // namespace stormkit::core::meta::details

export namespace stormkit { inline namespace core { namespace meta {
    template<typename T>
    using underlying_type = std::underlying_type_t<T>;

    template<typename T>
    using pointer_type = details::pointer_type<T>::type;

    template<typename T>
    using pointed_type = details::pointed_type<T>::type;

    template<typename T>
    using indirection_target_type = details::indirection_target_type<T>::type;

    template<typename T>
    using return_type = details::callable_trait<T>::return_type;

    template<has_expected_type T>
    using expected_type = typename T::expected_type;

    template<has_element_type T>
    using element_type = typename T::element_type;

    template<has_value_type T>
    using value_type = typename T::value_type;

    template<has_value_type T>
    using error_type = typename T::error_type;

    template<stdr::range Range>
    using iterator_type = stdr::iterator_t<Range>;

    template<stdr::range Range>
    using sentinel_type = stdr::sentinel_t<Range>;

    template<stdr::input_range Range>
    using range_value_type = stdr::range_value_t<Range>;

    template<stdr::input_range Range>
    using range_value_type_ref = stdr::range_reference_t<Range>;

    template<typename T>
    using in = lazy_conditional<prefer_pass_by_value<T>, T, const T&>;

    template<typename T>
    using take = lazy_conditional<prefer_pass_by_value<T>, T, T&&>;

    template<arithmetic T, arithmetic V>
    constexpr auto is_greater() noexcept;

    template<arithmetic T, arithmetic V>
    using safe_narrow_type = conditional<is_greater<T, V>(), T, V>;

    template<arithmetic T, arithmetic V>
    using safe_narrow_other_type = conditional<is_greater<T, V>(), V, T>;

    template<arithmetic T>
    using arithmetic_ordering_type = conditional<integral<T>, std::strong_ordering, std::partial_ordering>;

    template<enumeration>
    constexpr auto enumerate() noexcept -> decltype(auto) = delete;

    template<typename T>
    consteval auto name_of() noexcept -> std::string_view;
}}} // namespace stormkit::core::meta

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core { namespace meta {
    namespace details {
        template<pointer T>
        struct pointer_type<T> {
            using type = typename std::pointer_traits<T>::pointer;
        };

        template<typename T>
        struct pointer_type<std::reference_wrapper<T>> {
            using type = std::reference_wrapper<T>::type*;
        };

        template<pointer T>
        struct pointed_type<T> {
            using type = typename std::pointer_traits<T>::element_type;
        };

        template<typename T>
        struct pointed_type<std::reference_wrapper<T>> {
            using type = std::reference_wrapper<T>::type;
        };

        template<reference T>
            requires(not meta::pointer<meta::remove_refs_of<T>>)
        struct indirection_target_type<T> {
            using type = meta::remove_refs_of<T>;
        };

        template<plain::pointer T>
        struct indirection_target_type<T> {
            using type = typename std::pointer_traits<remove_refs_of<T>>::element_type;
        };

        template<typename T>
        struct indirection_target_type<std::reference_wrapper<T>> {
            using type = std::reference_wrapper<T>::type;
        };

        template<class Return, class... Ts>
        struct signature_trait {
            using return_type = Return;
        };

        template<class Return, class... Ts>
        struct callable_trait<Return(Ts...)>: signature_trait<Return, Ts...> {};

        template<class Return, class... Ts>
        struct callable_trait<Return(Ts...) noexcept>: signature_trait<Return, Ts...> {};

        template<class Return, class... Ts>
        struct callable_trait<Return (*)(Ts...)>: callable_trait<Return(Ts...)> {};

        template<class Return, class... Ts>
        struct callable_trait<Return (*)(Ts...) noexcept>: callable_trait<Return(Ts...) noexcept> {};

        template<class Object, class Return, class... Ts>
        struct callable_trait<Return (Object::*)(Ts...)>: callable_trait<Return(Ts...)> {};

        template<class Object, class Return, class... Ts>
        struct callable_trait<Return (Object::*)(Ts...) const>: callable_trait<Return(Ts...)> {};

        template<class Object, class Return, class... Ts>
        struct callable_trait<Return (Object::*)(Ts...) &>: callable_trait<Return(Ts...)> {};

        template<class Object, class Return, class... Ts>
        struct callable_trait<Return (Object::*)(Ts...) const &>: callable_trait<Return(Ts...)> {};

        template<class Object, class Return, class... Ts>
        struct callable_trait<Return (Object::*)(Ts...) &&>: callable_trait<Return(Ts...)> {};

        template<class Object, class Return, class... Ts>
        struct callable_trait<Return (Object::*)(Ts...) const &&>: callable_trait<Return(Ts...)> {};

        template<class Object, class Return, class... Ts>
        struct callable_trait<Return (Object::*)(Ts...) noexcept>: callable_trait<Return(Ts...) noexcept> {};

        template<class Object, class Return, class... Ts>
        struct callable_trait<Return (Object::*)(Ts...) const noexcept>: callable_trait<Return(Ts...) noexcept> {};

        template<class Object, class Return, class... Ts>
        struct callable_trait<Return (Object::*)(Ts...) & noexcept>: callable_trait<Return(Ts...) noexcept> {};

        template<class Object, class Return, class... Ts>
        struct callable_trait<Return (Object::*)(Ts...) const & noexcept>: callable_trait<Return(Ts...) noexcept> {};

        template<class Object, class Return, class... Ts>
        struct callable_trait<Return (Object::*)(Ts...) && noexcept>: callable_trait<Return(Ts...) noexcept> {};

        template<class Object, class Return, class... Ts>
        struct callable_trait<Return (Object::*)(Ts...) const && noexcept>: callable_trait<Return(Ts...) noexcept> {};

    } // namespace details

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<arithmetic T, arithmetic V>
    constexpr auto is_greater() noexcept {
        using type = decltype(T {} + V {});

        return static_cast<type>(std::numeric_limits<T>::max()) > static_cast<type>(std::numeric_limits<V>::max());
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    consteval auto name_of() noexcept -> std::string_view {
#if defined(STORMKIT_COMPILER_GCC)
        constexpr auto prefix   = std::string_view { " [with T = " };
        constexpr auto suffix   = std::string_view { "]" };
        constexpr auto function = std::string_view { __PRETTY_FUNCTION__ };
#elif defined(STORMKIT_COMPILER_CLANG)
        constexpr auto prefix   = std::string_view { " [T = " };
        constexpr auto suffix   = std::string_view { "]" };
        constexpr auto function = std::string_view { __PRETTY_FUNCTION__ };
#elif defined(STORMKIT_COMPILER_MSVC)
        constexpr auto prefix   = std::string_view { "<" };
        constexpr auto suffix   = std::string_view { ">(void)" };
        constexpr auto function = std::string_view { __FUNCSIG__ };
#else
    #error Unsupported compiler
#endif
        static_assert(not function.empty());

        constexpr auto start = function.find(prefix) + prefix.size();
        constexpr auto end   = function.rfind(suffix);

        static_assert(start < end);

        constexpr auto        name = function.substr(start, (end - start));
        static constexpr auto res  = []<std::size_t... Idxs>(std::string_view str, std::index_sequence<Idxs...>) {
            return std::array { str[Idxs]... };
        }(name, std::make_index_sequence<name.size()> {});

        return std::string_view { res.data(), res.size() };
    }
}}} // namespace stormkit::core::meta
