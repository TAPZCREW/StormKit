// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.functional.error_handling;

import std;

import stormkit.core.types;

import stormkit.core.console.io;
import stormkit.core.contract;
import stormkit.core.meta.type_manipulation;

export namespace stormkit { inline namespace core { namespace monadic {
    [[nodiscard]]
    constexpr auto assert(std::optional<string> message  = std::nullopt,
                          std::source_location  location = std::source_location::current()) noexcept -> decltype(auto);

    template<typename T>
    [[nodiscard]]
    constexpr auto assert(std::optional<string> message  = std::nullopt,
                          std::source_location  location = std::source_location::current()) noexcept -> decltype(auto);

    [[nodiscard]]
    constexpr auto log(std::invocable auto&& logger, string&& message) noexcept -> decltype(auto);

    [[nodiscard]]
    constexpr auto throw_as_exception() noexcept -> decltype(auto);
}}} // namespace stormkit::core::monadic

#ifdef STORMKIT_COMPILER_CLANG
    #define NORETURN_LAMBDA [[noreturn]]
#else
    #define NORETURN_LAMBDA
#endif

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core { namespace monadic {
    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    constexpr auto assert(std::optional<string> message, std::source_location location) noexcept -> decltype(auto) {
        return [message = std::move(message), location = std::move(location)]<typename E> NORETURN_LAMBDA(const E& error) -> T {
            if (message.has_value()) core::assert(false, std::format("{} ({})", *message, error), std::move(location));
            else
                core::assert(false, std::move(location));

            std::unreachable();
        };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto assert(std::optional<string> message, std::source_location location) noexcept -> decltype(auto) {
        return [message = std::move(message), location = std::move(location)]<typename E> NORETURN_LAMBDA(const E& error) -> E {
            if (message.has_value()) core::assert(false, std::format("{} ({})", *message, error), std::move(location));
            else
                core::assert(false, std::format("{}", error), std::move(location));

            std::unreachable();
        };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto log(std::invocable auto&& logger, string&& message) noexcept -> decltype(auto) {
        return [logger  = std::forward<decltype(logger)>,
                message = std::move(message)](auto&& error) -> std::expected<void, meta::to_plain_type<decltype(error)>> {
            logger(message, error);

            return {};
        };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto throw_as_exception() noexcept -> decltype(auto) {
        return [] NORETURN_LAMBDA(auto&& error) static -> std::expected<void, meta::to_plain_type<decltype(error)>> {
            throw std::forward<decltype(error)>(error);
        };
    }
}}} // namespace stormkit::core::monadic
