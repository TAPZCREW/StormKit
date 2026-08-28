// Copryright (C) 2022 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#ifndef STORMKIT_LUA_API_HPP
#define STORMKIT_LUA_API_HPP

#include <stormkit/core/platform_macro.hpp>

#ifdef STORMKIT_LUA_BUILD
    #define STORMKIT_LUA_API STORMKIT_EXPORT
#else
    #define STORMKIT_LUA_API STORMKIT_IMPORT
#endif

#endif
