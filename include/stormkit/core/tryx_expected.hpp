// Copyright (C) 2023 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#ifndef STORMKIT_TRYX_EXPECTED_HPP
#define STORMKIT_TRYX_EXPECTED_HPP

#include <stormkit/core/platform_macro.hpp>

#if (defined(__clang__) or defined(__GNUC__))
    #define TryX(try_expression)                                       \
        __extension__({                                                \
            auto res = (try_expression);                               \
            if (not res.has_value()) [[unlikely]]                      \
                return { std::unexpected { std::move(res.error()) } }; \
            *std::move(res);                                           \
        })
    #define TryXOr(try_expression, or_closure)                                                                \
        __extension__({                                                                                       \
            auto res = (try_expression);                                                                      \
            if (not res.has_value()) [[unlikely]] { return std::invoke(or_closure, std::move(res.error())); } \
            *std::move(res);                                                                                  \
        })
    #define TryXTransform(try_expression, transform_closure)                                       \
        __extension__({                                                                            \
            auto res = (try_expression);                                                           \
            if (not res.has_value()) [[unlikely]] {                                                \
                return std::unexpected { std::invoke(transform_closure, std::move(res.error())) }; \
            }                                                                                      \
            *std::move(res);                                                                       \
        })
    #define TryXAssert(try_expression, msg)                \
        __extension__({                                    \
            auto res = (try_expression);                   \
            stormkit::core::ensures(res.has_value(), msg); \
            *std::move(res);                               \
        })

    #define Return return
#else
    #define TryX(try_expression)                             co_await (try_expression)
    #define TryXOr(try_expression, or_closure)               co_await (try_expression).or_else(or_closure)
    #define TryXTransform(try_expression, transform_closure) co_await (try_expression).transform_error(transform_closure)
    #define TryXAssert(try_expression, msg) co_await (try_expression).or_else(stormkit::core::monadic::assert(msg))

    #define Return co_return
#endif

#define CustomLoggedTryX(try_expression, logger, msg)          \
    TryXTransform(try_expression, [&](auto&& error) noexcept { \
        logger("{}\n    > reason: {}", msg, error);            \
        return error;                                          \
    })
#define CustomLoggedTryXOr(try_expression, or_closure, logger, msg)      \
    TryXOr(try_expression, [&]<typename Error>(Error&& error) noexcept { \
        logger("{}\n    > reason: {}", msg, error);                      \
        return std::invoke(or_closure, std::forward<Error>(error));      \
    })
#define CustomLoggedTryXTransform(m, transform_closure, logger, msg)       \
    TryXTransform(m, [&]<typename Error>(Error&& error) noexcept {         \
        logger("{}\n    > reason: {}", msg, error);                        \
        return std::invoke(transform_closure, std::forward<Error>(error)); \
    })
#define LoggedTryX(try_expression, msg)               CustomLoggedTryX(try_expression, elog, msg)
#define LoggedTryXOr(try_expression, or_closure, msg) CustomLoggedTryXOr(try_expression, or_closure, elog, msg)
#define LoggedTryXTransform(try_expression, transform_closure, msg) \
    CustomLoggedTryXTransform(try_expression, transform_closure, elog, msg)

#define TryXLift(try_expression) TryXOr(try_expression, stormkit::core::monadic::identity())

#define CustomLoggedTryXLift(try_expression, logger, msg) \
    TryXOr(try_expression, [&](auto&& error) noexcept {   \
        logger("{}\n    > reason: {}", msg, error);       \
        return error;                                     \
    })
#define LoggedTryXLift(try_expression) CustomLoggedTryXLift(try_expression, elog, msg)

#define DiscardTryX(try_expression) \
    [[maybe_unused]]                \
    auto _ = TryX(try_expression)
#define DiscardTryXOr(try_expression, or_closure) \
    [[maybe_unused]]                              \
    auto _ = TryXOr(try_expression, or_closure)
#define DiscardTryXTransform(try_expression, transform_closure) \
    [[maybe_unused]]                                            \
    auto _ = TryXTransform(try_expression, transform_closure)

#define CustomLoggedDiscardTryX(try_expression, logger, msg) \
    [[maybe_unused]]                                         \
    auto _ = CustomLoggedTryX(try_expression, logger, msg)
#define LoggedDiscardTryX(try_expression, msg) CustomLoggedDiscardTryX(try_expression, elog, msg)
#define CustomLoggedDiscardTryXOr(try_expression, or_closure, logger, msg) \
    [[maybe_unused]]                                                       \
    auto _ = CustomLoggedTryXOr(try_expression, or_closure, logger, msg)
#define LoggedDiscardTryXOr(try_expression, or_closure, msg) CustomLoggedDiscardTryXOr(try_expression, or_closure, elog, msg)
#define CustomLoggedDiscardTryXTransform(try_expression, transform_closure, logger, msg) \
    [[maybe_unused]]                                                                     \
    auto _ = CustomLoggedTryXTransform(try_expression, transform_closure, logger, msg)
#define LoggedDiscardTryXTransform(try_expression, transform_closure, msg) \
    CustomLoggedDiscardTryXTransform(try_expression, transform_closure, elog, msg)

#define DiscardTryXLift(try_expression) \
    [[maybe_unused]]                    \
    auto _ = TryXLift(try_expression)
#define CustomLoggedDiscardTryXLift(try_expression, logger, msg) \
    [[maybe_unused]]                                             \
    auto _ = CustomLoggedTryXLift(try_expression, logger, msg)
#define LoggedDiscardTryXLift(try_expression) \
    [[maybe_unused]]                          \
    auto _ = LoggedTryXLift(try_expression, msg)

#define DiscardTryXAssert(try_expression, msg) \
    [[maybe_unused]]                           \
    auto _ = TryXAssert(try_expression, msg)

#endif
