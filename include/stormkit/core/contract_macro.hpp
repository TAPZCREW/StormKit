// Copryright (C) 2022 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#ifndef STORMKIT_CONTRACT_MACRO_HPP
#define STORMKIT_CONTRACT_MACRO_HPP

#include <stormkit/core/platform_macro.hpp>

#define EXPECTS(x, ...) stormkit::expects(x, #x __VA_OPT__(, ) __VA_ARGS__)
#define ENSURES(x, ...) stormkit::ensures(x, #x __VA_OPT__(, ) __VA_ARGS__)
#define ASSERT(x, ...)  stormkit::assert(x, #x __VA_OPT__(, ) __VA_ARGS__)

#endif
