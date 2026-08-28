// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

import std;

#define NO_CONSTANTS
#include <stormkit/core/config.hpp>

export module stormkit;

export import stormkit.core;

#if STORMKIT_LIB_LOG_ENABLED and defined(STORMKIT_IMPORT_LOG)
export import stormkit.log;
#endif
#if STORMKIT_LIB_ENTITIES_ENABLED and defined(STORMKIT_IMPORT_ENTITIES)
export import stormkit.entities;
#endif
#if STORMKIT_LIB_IMAGE_ENABLED and defined(STORMKIT_IMPORT_IMAGE)
export import stormkit.image;
#endif
#if STORMKIT_LIB_WSI_ENABLED and defined(STORMKIT_IMPORT_WSI)
export import stormkit.wsi;
#endif
#if STORMKIT_LIB_GPU_ENABLED and defined(STORMKIT_IMPORT_GPU)
export import stormkit.gpu;
#endif
#if STORMKIT_LIB_LUA_ENABLED and defined(STORMKIT_IMPORT_LUA)
export import stormkit.lua;
#endif
