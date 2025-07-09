// Copryright (C) 2022 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#ifndef STORMKIT_FLAGS_MACRO_HPP
#define STORMKIT_FLAGS_MACRO_HPP

#define FLAG_ENUM(e)                                      \
    template<>                                            \
    struct stormkit::details::EnableBitmaskOperators<e> { \
        constexpr EnableBitmaskOperators() = default;     \
        static constexpr bool enable       = true;        \
    };

#define DISABLE_DEFAULT_FORMATER_FOR_ENUM(e) \
    template<>                               \
    inline constexpr auto stormkit::meta::DISABLE_DEFAULT_FORMATTER_FOR_ENUM<e> = true;

#endif
