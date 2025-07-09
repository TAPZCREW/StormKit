// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#ifndef STORMKIT_LOG_MACRO_HPP
#define STORMKIT_LOG_MACRO_HPP

#include <stormkit/core/platform_macro.hpp>

#define NAMED_LOGGER(NAME, module_chars)                              \
    namespace {                                                       \
        constexpr auto NAME = stormkit::log::Module { module_chars }; \
    }

#define LOGGER(module)                                                        \
    NAMED_LOGGER(LOG_MODULE, module)                                          \
    template<class... Args>                                                   \
    STORMKIT_FORCE_INLINE inline auto dlog(Args&&... args) noexcept -> void { \
        LOG_MODULE.dlog(std::forward<Args>(args)...);                         \
    }                                                                         \
    template<class... Args>                                                   \
    STORMKIT_FORCE_INLINE inline auto ilog(Args&&... args) noexcept -> void { \
        LOG_MODULE.ilog(std::forward<Args>(args)...);                         \
    }                                                                         \
    template<class... Args>                                                   \
    STORMKIT_FORCE_INLINE inline auto wlog(Args&&... args) noexcept -> void { \
        LOG_MODULE.wlog(std::forward<Args>(args)...);                         \
    }                                                                         \
    template<class... Args>                                                   \
    STORMKIT_FORCE_INLINE inline auto elog(Args&&... args) noexcept -> void { \
        LOG_MODULE.elog(std::forward<Args>(args)...);                         \
    }                                                                         \
    template<class... Args>                                                   \
    STORMKIT_FORCE_INLINE inline auto flog(Args&&... args) noexcept -> void { \
        LOG_MODULE.flog(std::forward<Args>(args)...);                         \
    }

#define IN_MODULE_NAMED_LOGGER(NAME, module_chars) \
    inline constexpr auto NAME = stormkit::log::Module { module_chars };

#define IN_MODULE_LOGGER(module)                                              \
    IN_MODULE_NAMED_LOGGER(LOG_MODULE, module)                                \
    template<class... Args>                                                   \
    STORMKIT_FORCE_INLINE inline auto dlog(Args&&... args) noexcept -> void { \
        LOG_MODULE.dlog(std::forward<Args>(args)...);                         \
    }                                                                         \
    template<class... Args>                                                   \
    STORMKIT_FORCE_INLINE inline auto ilog(Args&&... args) noexcept -> void { \
        LOG_MODULE.ilog(std::forward<Args>(args)...);                         \
    }                                                                         \
    template<class... Args>                                                   \
    STORMKIT_FORCE_INLINE inline auto wlog(Args&&... args) noexcept -> void { \
        LOG_MODULE.wlog(std::forward<Args>(args)...);                         \
    }                                                                         \
    template<class... Args>                                                   \
    STORMKIT_FORCE_INLINE inline auto elog(Args&&... args) noexcept -> void { \
        LOG_MODULE.elog(std::forward<Args>(args)...);                         \
    }                                                                         \
    template<class... Args>                                                   \
    STORMKIT_FORCE_INLINE inline auto flog(Args&&... args) noexcept -> void { \
        LOG_MODULE.flog(std::forward<Args>(args)...);                         \
    }

#endif
