// Copryright (C) 2022 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#ifndef STORMKIT_MACRO_UTILS_HPP
#define STORMKIT_MACRO_UTILS_HPP

#define STORMKIT_STRINGIFY_DETAILS(x)  #x
#define STORMKIT_STRINGIFY(x)          STORMKIT_STRINGIFY_DETAILS(x)
#define STORMKIT_PRAGMA_FROM_STRING(x) _Pragma(STORMKIT_STRINGIFY(x))

#define STORMKIT_CONCAT_DETAILS(a, b) a##b
#define STORMKIT_CONCAT(a, b)         STORMKIT_CONCAT_DETAILS(a, b)

#define STORMKIT_UNIQUE_NAME(base) STORMKIT_CONCAT(base, __COUNTER__)

#endif
