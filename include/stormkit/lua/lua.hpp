#ifndef STORMKIT_LUA_HPP
#define STORMKIT_LUA_HPP

#include <stormkit/core/platform_macro.hpp>

#include <stormkit/lua/api.hpp>

STORMKIT_PUSH_WARNINGS

#define LUA_API        extern "C" STORMKIT_LUA_API
#define LUACODE_API    extern "C" STORMKIT_LUA_API
#define LUACODEGEN_API extern "C" STORMKIT_LUA_API

extern "C" {
#include <lua.h>

#include <luacode.h>
#include <lualib.h>
}

// #pragma clang diagnostic ignored "-Wdeprecated-declarations"
#define SOL_USING_CXX_LUA    0
#define SOL_NO_LUA_HPP       1
#define SOL_USE_LUAU         1
#define SOL_SAFE_STACK_CHECK 1
#define SOL_LUA_BIT32_LIB    1
#define LUA_VERSION_NUM      501
#include <sol/sol.hpp>
#undef assert

STORMKIT_POP_WARNINGS

#endif
