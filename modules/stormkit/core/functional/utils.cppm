// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.functional.utils;

import std;

import stormkit.core.meta;
import stormkit.core.types;
import stormkit.core.typesafe;

namespace stormkit { inline namespace core { namespace details {
    struct either_fn {
        [[nodiscard]]
        static constexpr auto operator()(bool condition, std::invocable auto&& true_, std::invocable auto&& false_) noexcept
          -> decltype(false_());

        template<typename T>
        using forward_wrapped_type = meta::forward_like<T, meta::value_type<meta::to_plain_type<T>>>;

        template<meta::pointer T>
        [[nodiscard]]
        static constexpr auto operator()(T                                             value,
                                         std::invocable<meta::pointed_type<T>&> auto&& true_,
                                         std::invocable auto&&                         false_) noexcept -> decltype(false_());

        template<meta::plain::wrapped_value T>
        [[nodiscard]]
        static constexpr auto operator()(T&&                                            value,
                                         std::invocable<forward_wrapped_type<T>> auto&& true_,
                                         std::invocable auto&&                          false_) noexcept -> decltype(false_())
            requires(meta::plain::boolean_testable<T>);
    };
}}} // namespace stormkit::core::details

export namespace stormkit { inline namespace core {
    inline constexpr auto either = details::either_fn {};

    using std::bind_back;
    using std::bind_front;

    template<typename T, typename Func, typename... Ts>
        requires(std::invocable<Func, T&, Ts...> and meta::is<std::invoke_result_t<Func, T&, Ts...>, void>)
    [[nodiscard]]
    constexpr auto init_by(Func&& func, Ts&&... args) noexcept -> T;
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    namespace details {
        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        constexpr auto either_fn::operator()(bool condition, std::invocable auto&& true_, std::invocable auto&& false_) noexcept
          -> decltype(false_()) {
            if (condition) return true_();
            return false_();
        }

        /////////////////////////////////////
        /////////////////////////////////////
        template<meta::pointer T>
        STORMKIT_FORCE_INLINE
        constexpr auto either_fn::operator()(T                                             value,
                                             std::invocable<meta::pointed_type<T>&> auto&& true_,
                                             std::invocable auto&& false_) noexcept -> decltype(false_()) {
            if (static_cast<bool>(value)) return true_(unref(value));
            return false_();
        }

        /////////////////////////////////////
        /////////////////////////////////////
        template<meta::plain::wrapped_value T>
        STORMKIT_FORCE_INLINE
        constexpr auto either_fn::operator()(T&&                                            value,
                                             std::invocable<forward_wrapped_type<T>> auto&& true_,
                                             std::invocable auto&&                          false_) noexcept -> decltype(false_())
            requires(meta::plain::boolean_testable<T>)
        {
            if (static_cast<bool>(value)) return true_(std::forward_like<T>(unref(value)));
            return false_();
        }
    } // namespace details

#if not(defined(__cpp_lib_bind_back) and __cpp_lib_bind_back >= 202306L)
    /////////////////////////////////////
    /////////////////////////////////////
    template<auto FUNC, typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto bind_back(Ts&&... args) noexcept -> decltype(auto) {
        using func_type = decltype(FUNC);
        if constexpr (meta::pointer<func_type> or std::is_member_pointer_v<func_type>) static_assert(FUNC != nullptr);
        return
          [... bound_args(std::forward<Ts>(args))]<typename Self, typename... CallTs>(
            this Self&&,
            CallTs&&... call_args) noexcept(std::is_nothrow_invocable_v<func_type,
                                                                        CallTs...,
                                                                        meta::forward_like<Self, meta::to_decayed_type<Ts>>...>)
            -> decltype(auto) {
              return std::invoke(FUNC, std::forward<CallTs>(call_args)..., std::forward_like<Self>(bound_args)...);
          };
    }
#endif

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T, typename Func, typename... Ts>
        requires(std::invocable<Func, T&, Ts...> and meta::is<std::invoke_result_t<Func, T&, Ts...>, void>)
    STORMKIT_CONST STORMKIT_FORCE_INLINE
    constexpr auto init_by(Func&& func, Ts&&... args) noexcept -> T {
        auto out = T {};
        std::invoke(std::forward<Func>(func), out, std::forward<Ts>(args)...);
        return out;
    }
}} // namespace stormkit::core
