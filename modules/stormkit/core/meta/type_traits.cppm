// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.meta.type_traits;

import std;

// export {
//     namespace stormkit { inline namespace core { namespace meta {
//         template<typename Trait>
//         consteval auto validate_interface() noexcept -> bool;

//    template<typename Trait, typename Interface>
//    concept Implements = std::derived_from<Trait, Interface>;

//    template<typename T>
//    struct ArithmeticTraitInterface {
//        template<typename... Ts>
//        static auto add(Ts...) noexcept -> decltype(auto) {
//            static_assert(false, string { "add not implemented for " } + name_of<T>());
//        }

//    template<typename... Ts>
//    static auto add_eq(Ts...) noexcept -> decltype(auto) {
//        static_assert(false, string { "add_eq not implemented for " } + name_of<T>());
//    }

//    template<typename... Ts>
//    static auto sub(Ts...) noexcept -> decltype(auto) {
//        static_assert(false, string { "sub not implemented for " } + name_of<T>());
//    }

//    template<typename... Ts>
//    static auto sub_eq(Ts...) noexcept -> decltype(auto) {
//        static_assert(false, string { "sub_eq not implemented for " } + name_of<T>());
//    }

//    template<typename... Ts>
//    static auto div(Ts...) noexcept -> decltype(auto) {
//        static_assert(false, string { "div not implemented for " } + name_of<T>());
//    }

//    template<typename... Ts>
//    static auto div_eq(Ts...) noexcept -> decltype(auto) {
//        static_assert(false, string { "div_eq not implemented for " } + name_of<T>());
//    }

//    template<typename... Ts>
//    static auto mul(Ts...) noexcept -> decltype(auto) {
//        static_assert(false, string { "mul not implemented for " } + name_of<T>());
//    }

//    template<typename... Ts>
//    static auto mul_eq(Ts...) noexcept -> decltype(auto) {
//        static_assert(false, string { "mul_eq not implemented for " } + name_of<T>());
//    }
// };

//    template<typename T>
//    struct ArithmeticTrait {};

//    template<typename T>
//    concept ImplementsArithmetic = Implements<ArithmeticTrait<T>, ArithmeticTraitInterface<T>>;
// }}} // namespace stormkit::core::meta

//    template<typename T, typename U>
//        requires(stormkit::meta::plain::is<T, U>
//                 and stormkit::meta::ImplementsArithmetic<stormkit::meta::to_plain_type<T>>)
//    auto operator+(T&& first, U&& second) noexcept -> decltype(auto);

//    template<typename T, typename U>
//        requires(stormkit::meta::plain::is<T, U>
//                 and stormkit::meta::ImplementsArithmetic<stormkit::meta::to_plain_type<T>>)
//    auto operator+=(T&& first, U&& second) noexcept -> decltype(auto);

//    template<typename T, typename U>
//        requires(stormkit::meta::plain::is<T, U>
//                 and stormkit::meta::ImplementsArithmetic<stormkit::meta::to_plain_type<T>>)
//    auto operator-(T&& first, U&& second) noexcept -> decltype(auto);

//    template<typename T, typename U>
//        requires(stormkit::meta::plain::is<T, U>
//                 and stormkit::meta::ImplementsArithmetic<stormkit::meta::to_plain_type<T>>)
//    auto operator-=(T&& first, U&& second) noexcept -> decltype(auto);

//    template<typename T, typename U>
//        requires(stormkit::meta::plain::is<T, U>
//                 and stormkit::meta::ImplementsArithmetic<stormkit::meta::to_plain_type<T>>)
//    auto operator*(T&& first, U&& second) noexcept -> decltype(auto);

//    template<typename T, typename U>
//        requires(stormkit::meta::plain::is<T, U>
//                 and stormkit::meta::ImplementsArithmetic<stormkit::meta::to_plain_type<T>>)
//    auto operator*=(T&& first, U&& second) noexcept -> decltype(auto);

//    template<typename T, typename U>
//        requires(stormkit::meta::plain::is<T, U>
//                 and stormkit::meta::ImplementsArithmetic<stormkit::meta::to_plain_type<T>>)
//    auto operator/(T&& first, U&& second) noexcept -> decltype(auto);

//    template<typename T, typename U>
//        requires(stormkit::meta::plain::is<T, U>
//                 and stormkit::meta::ImplementsArithmetic<stormkit::meta::to_plain_type<T>>)
//    auto operator/=(T&& first, U&& second) noexcept -> decltype(auto);
// }

// namespace stormkit { inline namespace core { namespace meta {
//     ////////////////////////////////////////
//     ////////////////////////////////////////
//     template<typename Trait>
//     consteval auto validate_interface() noexcept -> bool {
//         auto _ = Trait {};

//    return true;
// }
// }}} // namespace stormkit::core::meta

// using namespace stormkit::meta;

// ////////////////////////////////////////
// ////////////////////////////////////////
// template<typename T, typename U>
//     requires(PlainIs<T, U> and ImplementsArithmetic<to_plain_type<T>>)
// STORMKIT_FORCE_INLINE
// auto operator+(T&& first, U&& second) noexcept -> decltype(auto) {
//     return ArithmeticTrait<to_plain_type<T>>::add(std::forward<T>(first), std::forward<U>(second));
// }

// ////////////////////////////////////////
// ////////////////////////////////////////
// template<typename T, typename U>
//     requires(PlainIs<T, U> and ImplementsArithmetic<to_plain_type<T>>)
// STORMKIT_FORCE_INLINE
// auto operator+=(T&& first, U&& second) noexcept -> decltype(auto) {
//     return ArithmeticTrait<to_plain_type<T>>::add_eq(std::forward<T>(first),
//     std::forward<U>(second));
// }

// ////////////////////////////////////////
// ////////////////////////////////////////
// template<typename T, typename U>
//     requires(PlainIs<T, U> and ImplementsArithmetic<to_plain_type<T>>)
// STORMKIT_FORCE_INLINE
// auto operator-(T&& first, U&& second) noexcept -> decltype(auto) {
//     return ArithmeticTrait<to_plain_type<T>>::sub(std::forward<T>(first), std::forward<U>(second));
// }

// ////////////////////////////////////////
// ////////////////////////////////////////
// template<typename T, typename U>
//     requires(PlainIs<T, U> and ImplementsArithmetic<to_plain_type<T>>)
// STORMKIT_FORCE_INLINE
// auto operator-=(T&& first, U&& second) noexcept -> decltype(auto) {
//     return ArithmeticTrait<to_plain_type<T>>::sub_eq(std::forward<T>(first),
//     std::forward<U>(second));
// }

// ////////////////////////////////////////
// ////////////////////////////////////////
// template<typename T, typename U>
//     requires(PlainIs<T, U> and ImplementsArithmetic<to_plain_type<T>>)
// STORMKIT_FORCE_INLINE
// auto operator*(T&& first, U&& second) noexcept -> decltype(auto) {
//     return ArithmeticTrait<to_plain_type<T>>::mul(std::forward<T>(first), std::forward<U>(second));
// }

// ////////////////////////////////////////
// ////////////////////////////////////////
// template<typename T, typename U>
//     requires(PlainIs<T, U> and ImplementsArithmetic<to_plain_type<T>>)
// STORMKIT_FORCE_INLINE
// auto operator*=(T&& first, U&& second) noexcept -> decltype(auto) {
//     return ArithmeticTrait<to_plain_type<T>>::mul_eq(std::forward<T>(first),
//     std::forward<U>(second));
// }

// ////////////////////////////////////////
// ////////////////////////////////////////
// template<typename T, typename U>
//     requires(PlainIs<T, U> and ImplementsArithmetic<to_plain_type<T>>)
// STORMKIT_FORCE_INLINE
// auto operator/(T&& first, U&& second) noexcept -> decltype(auto) {
//     return ArithmeticTrait<to_plain_type<T>>::div(std::forward<T>(first), std::forward<U>(second));
// }

// ////////////////////////////////////////
// ////////////////////////////////////////
// template<typename T, typename U>
//     requires(PlainIs<T, U> and ImplementsArithmetic<to_plain_type<T>>)
// STORMKIT_FORCE_INLINE
// auto operator/=(T&& first, U&& second) noexcept -> decltype(auto) {
//     return ArithmeticTrait<to_plain_type<T>>::div_eq(std::forward<T>(first),
//     std::forward<U>(second));
// }
