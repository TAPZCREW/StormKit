// Copyright (C) 2023 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#ifndef STORMKIT_TRY_EXPECTED_HPP
#define STORMKIT_TRY_EXPECTED_HPP

#include <stormkit/core/macro_utils.hpp>

#define Try_impl(expected_var_name, try_expression)                              \
    auto expected_var_name = (try_expression);                                   \
    do {                                                                         \
        if (not expected_var_name.has_value()) [[unlikely]] {                    \
            return { std::unexpected { std::move(expected_var_name.error()) } }; \
        }                                                                        \
    } while (false)

#define TryOr_impl(expected_var_name, try_expression, or_closure)                \
    auto expected_var_name = (try_expression);                                   \
    do                                                                           \
        if (not expected_var_name.has_value()) [[unlikely]] {                    \
            return std::invoke(or_closure, std::move(expected_var_name.error())) \
        }                                                                        \
    }                                                                            \
    while (false)

#define TryTransform_impl(expected_var_name, try_expression, transform_closure)                              \
    auto expected_var_name = (try_expression);                                                               \
    do {                                                                                                     \
        if (not expected_var_name.has_value()) [[unlikely]] {                                                \
            return std::unexpected { std::invoke(transform_closure, std::move(expected_var_name.error())) }; \
        }                                                                                                    \
    } while (false)

#define TryAssert_impl(expected_var_name, try_expression, msg) \
    auto expected_var_name = (try_expression);                 \
    stormkit::core::ensures(expected_var_name.has_value(), msg)

#define TryTo_impl(name, expected_var_name, try_expression) \
    Try_impl(expected_var_name, try_expression);            \
    auto name = *std::move(expected_var_name)
#define TryToOr_impl(name, expected_var_name, try_expression, or_closure) \
    TryOr_impl(expected_var_name, try_expression, or_closure);            \
    auto name = *std::move(expected_var_name)
#define TryToTransform_impl(name, expected_var_name, try_expression, transform_closure) \
    TryTransform_impl(expected_var_name, try_expression, transform_closure);            \
    auto name = *std::move(expected_var_name)
#define TryToAssert_impl(name, expected_var_name, try_expression, msg) \
    TryAssert_impl(expected_var_name, try_expression, msg);            \
    auto name = *std::move(expected_var_name)

#define Try(try_expression)               Try_impl(STORMKIT_UNIQUE_NAME(temp), try_expression)
#define TryOr(try_expression, or_closure) TryOr_impl(STORMKIT_UNIQUE_NAME(temp), try_expression, or_closure)
#define TryTransform(try_expression, transform_closure) \
    TryTransform_impl(STORMKIT_UNIQUE_NAME(temp), try_expression, transform_closure)
#define TryAssert(try_expression, msg) TryAssert_impl(STORMKIT_UNIQUE_NAME(temp), try_expression, msg)

#define TryTo(name, try_expression)               TryTo_impl(name, STORMKIT_UNIQUE_NAME(name), try_expression)
#define TryToOr(name, try_expression, or_closure) TryToOr_impl(name, STORMKIT_UNIQUE_NAME(name), try_expression, or_closure)
#define TryToTransform(name, try_expression, transform_closure) \
    TryToTransform_impl(name, STORMKIT_UNIQUE_NAME(name), try_expression, transform_closure)
#define TryToAssert(name, try_expression, msg) TryToAssert_impl(name, STORMKIT_UNIQUE_NAME(name), try_expression, msg)

#define CustomLoggedTryTo(name, try_expression, logger, msg)                                    \
    TryToOr_impl(name, STORMKIT_UNIQUE_NAME(name), try_expression, [&](auto&& error) noexcept { \
        logger("{}\n    > reason: {}", msg, error);                                             \
        return error;                                                                           \
    })

#define LoggedTryTo(name, try_expression, logger, msg) CustomLoggerTryTo(name, try_expression, elog, msg)

#define CustomLoggedTry(name, try_expression, logger, msg)                                    \
    TryOr_impl(name, STORMKIT_UNIQUE_NAME(name), try_expression, [&](auto&& error) noexcept { \
        logger("{}\n    > reason: {}", msg, error);                                           \
        return error;                                                                         \
    })
#define LoggedTry(name, try_expression, logger, msg) CustomLoggerTry(name, try_expression, elog, msg)

#endif
