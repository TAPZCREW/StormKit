// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.functional.monadic;

import std;

import stormkit.core.meta;
import stormkit.core.types;
import stormkit.core.typesafe;
import stormkit.core.functional.utils;
import stormkit.core.containers.utils;
import stormkit.core.overload_set;

namespace stdr = std::ranges;

export namespace stormkit { inline namespace core { namespace monadic {
    struct anything {
        template<typename T>
        operator T&(this auto& self) noexcept;

        template<typename T>
        operator T&&(this auto&& self) noexcept;
    };

    [[nodiscard]]
    constexpr auto identity(auto&& value) noexcept -> decltype(auto);
    [[nodiscard]]
    constexpr auto consume(auto&& value) noexcept -> decltype(auto);
    template<typename T>
    [[nodiscard]]
    constexpr auto value() noexcept -> decltype(auto);
    template<typename T>
    [[nodiscard]]
    constexpr auto as(const std::source_location& location = std::source_location::current()) noexcept -> decltype(auto);
    template<typename T>
    [[nodiscard]]
    constexpr auto unchecked_narrow() noexcept -> decltype(auto);
    [[nodiscard]]
    constexpr auto discard() noexcept -> decltype(auto);
    [[nodiscard]]
    constexpr auto set(auto& output) noexcept -> decltype(auto);
    [[nodiscard]]
    constexpr auto emplace_to(stdr::range auto& container) noexcept -> decltype(auto);
    [[nodiscard]]
    constexpr auto append_to(stdr::range auto& range) noexcept -> decltype(auto);
    [[nodiscard]]
    constexpr auto wrap(auto&& func) noexcept -> decltype(auto);
    template<auto Func>
    [[nodiscard]]
    constexpr auto wrap() noexcept -> decltype(auto);
    [[nodiscard]]
    constexpr auto unwrap() noexcept -> decltype(auto);
    [[nodiscard]]
    constexpr auto unwrap_error() noexcept -> decltype(auto);
    [[nodiscard]]
    constexpr auto unpack_tuple_to(auto&& func) noexcept -> decltype(auto);

    [[nodiscard]]
    constexpr auto noop() noexcept -> decltype(auto);

    template<typename T, std::invocable<T> First, std::invocable<std::invoke_result_t<First, T>> Second>
    [[nodiscard]]
    constexpr auto map(First&& first, Second&& second) noexcept -> decltype(auto);

    [[nodiscard]]
    constexpr auto map(auto&& first, auto&& second) noexcept -> decltype(auto);

    [[nodiscard]]
    constexpr auto either(std::regular_invocable<anything> auto&&... visitors) noexcept -> decltype(auto);

    template<typename T>
    [[nodiscard]]
    constexpr auto either(meta::unary_predicate<T> auto&& predicate,
                          std::invocable<T> auto&&        true_,
                          std::invocable<T> auto&&        false_) noexcept -> decltype(auto);

    template<auto Index>
    [[nodiscard]]
    constexpr auto get() noexcept -> decltype(auto);

    [[nodiscard]]
    constexpr auto is() noexcept -> decltype(auto);

    template<typename T>
    [[nodiscard]]
    constexpr auto is() noexcept -> decltype(auto);

    [[nodiscard]]
    constexpr auto clone() noexcept -> decltype(auto);

    template<typename T>
    [[nodiscard]]
    constexpr auto init() noexcept -> decltype(auto);

    template<typename T>
    [[nodiscard]]
    constexpr auto init(auto&&... args) noexcept -> decltype(auto);

    template<typename T, auto Initializer>
    [[nodiscard]]
    constexpr auto init_by() noexcept -> decltype(auto);

    [[nodiscard]]
    constexpr auto unref() noexcept -> decltype(auto);

    template<typename T>
    [[nodiscard]]
    constexpr auto forward_like() noexcept -> decltype(auto);
}}} // namespace stormkit::core::monadic

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core { namespace monadic {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto identity(T&& value) noexcept -> decltype(auto) {
        return [value = std::forward<T>(value)] mutable noexcept -> decltype(auto) { return std::forward_like<T>(value); };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto consume(auto&& value) noexcept -> decltype(auto) {
        return [value = std::move(value)](auto&&...) mutable noexcept -> meta::to_plain_type<decltype(value)> {
            return std::move(value);
        };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto value() noexcept -> decltype(auto) {
        return
          []<meta::owning_pointer T>(T&& value) static noexcept -> decltype(auto) { return std::forward_like<T>(value.get()); };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    STORMKIT_PURE
    constexpr auto as(const std::source_location& location) noexcept -> decltype(auto) {
        return [location]<typename U>(U&& value) noexcept -> T { return core::as<T>(std::forward<U>(value), location); };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    STORMKIT_CONST
    constexpr auto unchecked_narrow() noexcept -> decltype(auto) {
        return []<typename U>(U&& value) static noexcept -> decltype(auto) {
            return core::unchecked_narrow<T>(std::forward<U>(value));
        };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    STORMKIT_PURE
    constexpr auto is(T&& value) noexcept -> decltype(auto) {
        return [value = std::forward<T>(value)]<typename U>(U&& other) { return core::is<equal>(value, std::forward<U>(other)); };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto append_to(stdr::range auto& range) noexcept -> decltype(auto) {
        return [&range]<typename T>(T&& val) noexcept { range.emplace(stdr::cend(range), std::forward<T>(val)); };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    STORMKIT_PURE
    constexpr auto wrap(T&& func) noexcept {
        return [func = std::forward<T>(func)]<typename... Ts>(Ts&&... args) noexcept -> decltype(auto) {
            return std::invoke(func, std::forward<Ts>(args)...);
        };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto unwrap() noexcept -> decltype(auto) {
        return []<typename T>(T&& value) static noexcept -> decltype(auto) { return std::forward_like<T>(value.value()); };
    };

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto unwrap_error() noexcept -> decltype(auto) {
        return []<typename T>(T&& value) static noexcept -> decltype(auto) { return std::forward_like<T>(value.error()); };
    };

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<auto Func>
    STORMKIT_FORCE_INLINE
    STORMKIT_CONST
    constexpr auto wrap() noexcept {
        return []<typename... Ts>(Ts&&... args) static noexcept -> decltype(auto) {
            return std::invoke(Func, std::forward<Ts>(args)...);
        };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    STORMKIT_PURE
    constexpr auto unpack_tuple_to(T&& func) noexcept -> decltype(auto) {
        return [func = std::forward<T>(func)]<typename U>(U&& arg) noexcept { return std::apply(func, std::forward<U>(arg)); };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto noop() noexcept -> decltype(auto) {
        return [](auto&&...) static noexcept -> void {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto discard() noexcept -> decltype(auto) {
        return [](auto&&...) static noexcept -> void {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto set(auto& output) noexcept -> decltype(auto) {
        return [&output]<typename T>(T&& value) mutable noexcept -> void { output = std::forward<T>(value); };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto emplace_to(stdr::range auto& container) noexcept -> decltype(auto) {
        return [&container]<typename T>(T&& value) noexcept -> void { container.emplace_back(std::forward<T>(value)); };
    }

    template<typename First, typename Second, typename... Ts>
    constexpr auto is_noexcept = noexcept(std::declval<Second>()(std::declval<First>()(std::declval<Ts>()...)));

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, std::invocable<T> First, std::invocable<std::invoke_result_t<First, T>> Second>
    STORMKIT_FORCE_INLINE
    STORMKIT_PURE
    constexpr auto map(First&& first, Second&& second) noexcept -> decltype(auto) {
        return map(std::forward<First>(first), std::forward<Second>(second));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename First, typename Second>
    STORMKIT_FORCE_INLINE
    STORMKIT_PURE
    constexpr auto map(First&& first, Second&& second) noexcept -> decltype(auto) {
        using FirstP  = meta::to_plain_type<First>;
        using SecondP = meta::to_plain_type<Second>;

        return [first  = std::forward<First>(first),
                second = std::forward<Second>(second)]<typename... Ts>(Ts&&... args) noexcept(is_noexcept<FirstP, SecondP, Ts...>)
                 -> decltype(auto) { return second(first(std::forward<Ts>(args)...)); };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    constexpr auto either(meta::unary_predicate<T> auto&& predicate,
                          std::invocable<T> auto&&        true_,
                          std::invocable<T> auto&&        false_) noexcept -> decltype(auto) {
        [predicate = std::move(predicate),
         true_     = std::move(true_),
         false_    = std::move(false_)](auto&& elem) noexcept -> decltype(auto) {
            if (predicate(elem)) return first(std::forward<decltype(elem)>(elem));
            return second(elem);
        };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<auto Index>
    STORMKIT_FORCE_INLINE
    STORMKIT_CONST
    constexpr auto get() noexcept -> decltype(auto) {
        return []<typename U>(U&& value) static noexcept -> decltype(auto) { return std::get<Index>(std::forward<U>(value)); };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto is() noexcept -> decltype(auto) {
        return []<typename T, typename U>(T&& first, U&& second) static noexcept -> decltype(auto) {
            return core::is<equal>(std::forward<T>(first), std::forward<U>(second));
        };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    STORMKIT_CONST
    constexpr auto is() noexcept -> decltype(auto) {
        return []<typename U>(U&& value) static noexcept { return core::is<T>(std::forward<U>(value)); };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<std::regular_invocable<anything>... Ts>
    STORMKIT_FORCE_INLINE
    STORMKIT_PURE
    constexpr auto either(Ts&&... visitors) noexcept -> decltype(auto) {
        return [... visitors = std::forward<Ts>(visitors)]<typename T>(T&& variant) mutable noexcept -> decltype(auto) {
            return std::visit(core::overload_set { std::forward<Ts>(visitors)... }, std::forward<T>(variant));
        };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto clone() noexcept -> decltype(auto) {
        return []<typename T>(T&& value) static noexcept(noexcept(std::is_nothrow_copy_constructible_v<meta::to_plain_type<T>>))
                 -> decltype(auto) { return auto(std::forward<T>(value)); };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    STORMKIT_CONST
    constexpr auto init() noexcept -> decltype(auto) {
        return []<typename... Ts>(Ts&&... args) static noexcept(noexcept(std::is_nothrow_constructible_v<T, Ts...>))
                 -> decltype(auto) { return T { std::forward<Ts>(args)... }; };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, typename... Ts>
    STORMKIT_FORCE_INLINE
    STORMKIT_PURE
    constexpr auto init(Ts&&... args) noexcept -> decltype(auto) {
        return [... args = std::forward<Ts>(args)]() mutable noexcept(noexcept(std::is_nothrow_constructible_v<T, Ts...>))
                 -> decltype(auto) { return T { std::forward<Ts>(args)... }; };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, auto Initializer>
    STORMKIT_FORCE_INLINE
    STORMKIT_CONST
    constexpr auto init_by() noexcept -> decltype(auto) {
        return []<typename... Ts>(Ts&&... args) static noexcept -> decltype(auto) {
            return stormkit::init_by<T>(Initializer, std::forward<Ts>(args)...);
        };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto unref() noexcept -> decltype(auto) {
        return [](const auto& value) static noexcept -> decltype(auto) { return *value; };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    STORMKIT_CONST
    constexpr auto forward_like() noexcept -> decltype(auto) {
        return [](auto&& value) static noexcept -> decltype(auto) { return std::forward_like<T>(value); };
    }
}}} // namespace stormkit::core::monadic
