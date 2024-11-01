// Copyright (C) 2022 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#ifndef STORMKIT_HASH_MACRO_HPP
#define STORMKIT_HASH_MACRO_HPP

#include <stormkit/Core/PlatformMacro.hpp>

/// \brief `HASH_FUNC` declare hash func and define an operator== for a given `x` type
#define HASH_FUNC(x, ...)                                                                  \
    template<>                                                                             \
    struct STORMKIT_API std::hash<x> {                                                     \
        [[nodiscard]] auto operator()(const x& value) const noexcept -> stormkit::Hash64 { \
            auto hash = stormkit::Hash64 { 0 };                                            \
            stormkit::hashCombine(hash, __VA_ARGS__);                                      \
            return hash;                                                                   \
        }                                                                                  \
    };

#define CONSTRAINED_HASH_FUNC(x, ...)                                                      \
    template<x T>                                                                          \
    struct STORMKIT_API std::hash<T> {                                                     \
        [[nodiscard]] auto operator()(const T& value) const noexcept -> stormkit::Hash64 { \
            auto hash = stormkit::Hash64 { 0 };                                            \
            stormkit::hashCombine(hash, __VA_ARGS__);                                      \
            return hash;                                                                   \
        }                                                                                  \
    };

#define TEMPLATED_HASH_FUNC(x, z, y, ...)                                                     \
    template<z y>                                                                             \
    struct STORMKIT_API std::hash<x<y>> {                                                     \
        [[nodiscard]] auto operator()(const x<y>& value) const noexcept -> stormkit::Hash64 { \
            auto hash = stormkit::Hash64 { 0 };                                               \
            stormkit::hashCombine(hash, __VA_ARGS__);                                         \
            return hash;                                                                      \
        }                                                                                     \
    };

#define EQUAL_FUNC(x)                                                                           \
    template<>                                                                                  \
    struct std::equal_to<x> {                                                                   \
        [[nodiscard]] constexpr auto operator()(const x& first, const x& second) const noexcept \
            -> bool {                                                                           \
            const auto hasher = std::hash<x> {};                                                \
            return hasher(first) == hasher(second);                                             \
        }                                                                                       \
    };

#define CONSTRAINED_EQUAL_FUNC(x)                                                               \
    template<x T>                                                                               \
    struct std::equal_to<T> {                                                                   \
        [[nodiscard]] constexpr auto operator()(const T& first, const T& second) const noexcept \
            -> bool {                                                                           \
            const auto hasher = std::hash<T> {};                                                \
            return hasher(first) == hasher(second);                                             \
        }                                                                                       \
    };

#define TEMPLATED_EQUAL_FUNC(x, z, y)                                                        \
    template<z y>                                                                            \
    struct std::equal_to<x<y>> {                                                             \
        [[nodiscard]] constexpr auto operator()(const x<y>& first,                           \
                                                const x<y>& second) const noexcept -> bool { \
            const auto hasher = std::hash<x<y>> {};                                          \
            return hasher(first) == hasher(second);                                          \
        }                                                                                    \
    };

#define HASH_EQUAL_FUNC(x, ...) \
    HASH_FUNC(x, __VA_ARGS__)   \
    EQUAL_FUNC(x)

#define CONSTRAINED_HASH_EQUAL_FUNC(x, ...) \
    CONSTRAINED_HASH_FUNC(x, __VA_ARGS__)   \
    CONSTRAINED_EQUAL_FUNC(x)

#define TEMPLATED_HASH_EQUAL_FUNC(x, z, y, ...) \
    TEMPLATED_HASH_FUNC(x, z, y, __VA_ARGS__)   \
    TEMPLATED_EQUAL_FUNC(x, z, y)
#endif
