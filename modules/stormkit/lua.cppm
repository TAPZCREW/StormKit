// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>
#include <stormkit/core/try_expected.hpp>

#include <stormkit/lua/api.hpp>
#include <stormkit/lua/lua.hpp>

export module stormkit.lua;

import std;

import stormkit.core;

namespace stdfs = std::filesystem;

export namespace stormkit::lua {
    struct Modules {
        bool log      = false;
        bool image    = false;
        bool entities = false;
        bool wsi      = false;
        bool gpu      = false;
    };

    class STORMKIT_LUA_API Engine {
      public:
        using InitUserLibrariesClosure = std::function<void(sol::state&)>;

        ~Engine() noexcept;

        Engine(const Engine&)                    = delete;
        auto operator=(const Engine&) -> Engine& = delete;

        Engine(Engine&& other) noexcept;
        auto operator=(Engine&& other) noexcept -> Engine&;

        static auto load_from_file(stdfs::path              path,
                                   Modules                  modules             = {},
                                   InitUserLibrariesClosure init_user_libraries = monadic::noop()) noexcept -> Engine;
        auto        run() noexcept -> sol::state;

        static auto run(stdfs::path              file,
                        Modules                  modules             = {},
                        InitUserLibrariesClosure init_user_libraries = monadic::noop()) noexcept -> void;

      private:
        Engine(Modules&&, InitUserLibrariesClosure&&) noexcept;

        auto load(stdfs::path&&) noexcept -> void;

        auto init_libraries(sol::state&) noexcept -> void;

        Modules                  m_modules;
        InitUserLibrariesClosure m_init_user_libraries;

        dynarray<char> m_script;
    };

    template<typename... Ts>
    auto luacall(const sol::protected_function& function, Ts&&... args) noexcept -> decltype(function());
} // namespace stormkit::lua

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::lua {
    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline Engine::Engine(Engine&& other) noexcept = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Engine::operator=(Engine&& other) noexcept -> Engine& = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline Engine::~Engine() noexcept = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Engine::load_from_file(stdfs::path file, Modules modules, InitUserLibrariesClosure init_user_libraries) noexcept
      -> Engine {
        auto engine = Engine { std::move(modules), std::move(init_user_libraries) };
        engine.load(std::move(file));
        return engine;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Engine::run(stdfs::path file, Modules modules, InitUserLibrariesClosure init_user_libraries) noexcept -> void {
        Engine::load_from_file(std::move(file), std::move(modules), std::move(init_user_libraries)).run();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename... Ts>
    STORMKIT_FORCE_INLINE
    inline auto luacall(const sol::protected_function& function, Ts&&... args) noexcept -> decltype(function()) {
        auto result = function(std::forward<Ts>(args)...);
        if (not result.valid())
            ensures(false,
                    std::format("lua runtime error!\n-----------------------------------------\n{}",
                                sol::error { result }.what()));
        return result;
    }
} // namespace stormkit::lua
