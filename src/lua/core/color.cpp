// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/lua/lua.hpp>

module stormkit.lua;

import std;

import stormkit.core;

namespace stormkit::lua::core {
    namespace {
        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T, typename... Constructors>
        auto _bind_color(string_view name, auto& parent, auto... values) {
            auto metatable = parent.template new_usertype<T>(name, sol::constructors<T(), Constructors...> {});
            metatable[sol::meta_function::as<string>] = &stormkit::as<string><T::LAYOUT, typename T::Storage>;
            metatable[sol::meta_function::is]  = &T::operator==;

            metatable["component_count"] = sol::var(std::cref(T::COMPONENTS_COUNT));
            metatable["layout"]          = sol::var(std::cref(T::LAYOUT));

            (
              [metatable, &values] mutable {
                  auto&& [k, v] = std::forward<decltype(values)>(values);
                  metatable[k]  = v;
              }(),
              ...);

            metatable["BLACK"]       = sol::var(stormkit::to_layout<T::LAYOUT>(stormkit::colors::BLACK<typename T::Storage>.c));
            metatable["GRAY"]        = sol::var(stormkit::to_layout<T::LAYOUT>(stormkit::colors::GRAY<typename T::Storage>.c));
            metatable["SILVER"]      = sol::var(stormkit::to_layout<T::LAYOUT>(stormkit::colors::SILVER<typename T::Storage>.c));
            metatable["WHITE"]       = sol::var(stormkit::to_layout<T::LAYOUT>(stormkit::colors::WHITE<typename T::Storage>.c));
            metatable["MAROON"]      = sol::var(stormkit::to_layout<T::LAYOUT>(stormkit::colors::MAROON<typename T::Storage>.c));
            metatable["RED"]         = sol::var(stormkit::to_layout<T::LAYOUT>(stormkit::colors::RED<typename T::Storage>.c));
            metatable["OLIVE"]       = sol::var(stormkit::to_layout<T::LAYOUT>(stormkit::colors::OLIVE<typename T::Storage>.c));
            metatable["YELLOW"]      = sol::var(stormkit::to_layout<T::LAYOUT>(stormkit::colors::YELLOW<typename T::Storage>.c));
            metatable["GREEN"]       = sol::var(stormkit::to_layout<T::LAYOUT>(stormkit::colors::GREEN<typename T::Storage>.c));
            metatable["LIME"]        = sol::var(stormkit::to_layout<T::LAYOUT>(stormkit::colors::LIME<typename T::Storage>.c));
            metatable["TEAL"]        = sol::var(stormkit::to_layout<T::LAYOUT>(stormkit::colors::TEAL<typename T::Storage>.c));
            metatable["AQUA"]        = sol::var(stormkit::to_layout<T::LAYOUT>(stormkit::colors::AQUA<typename T::Storage>.c));
            metatable["NAVY"]        = sol::var(stormkit::to_layout<T::LAYOUT>(stormkit::colors::NAVY<typename T::Storage>.c));
            metatable["BLUE"]        = sol::var(stormkit::to_layout<T::LAYOUT>(stormkit::colors::PURPLE<typename T::Storage>.c));
            metatable["PURPLE"]      = sol::var(stormkit::to_layout<T::LAYOUT>(stormkit::colors::PURPLE<typename T::Storage>.c));
            metatable["FUSCHIA"]     = sol::var(stormkit::to_layout<T::LAYOUT>(stormkit::colors::FUSCHIA<typename T::Storage>.c));
            metatable["TRANSPARENT"] = sol::var(stormkit::to_layout<
                                                T::LAYOUT>(stormkit::colors::TRANSPARENT<typename T::Storage>.c));
        }
    } // namespace

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto bind_color(sol::state& global_state) noexcept -> void {
        global_state["color_layout"] = global_state.create_table_with(
          sol::meta_function::as<string>,
          +[](ColorLayout layout) static noexcept { return as<string>(layout); },
          "R",
          ColorLayout::R,
          "RG",
          ColorLayout::RG,
          "RGB",
          ColorLayout::RGB,
          "BGR",
          ColorLayout::BGR,
          "RGBA",
          ColorLayout::RGBA,
          "ARGB",
          ColorLayout::ARGB,
          "BGRA",
          ColorLayout::BGRA,
          "ABGR",
          ColorLayout::ABGR);

        auto f32_metatable = global_state["fcolor"].get_or_create<sol::table>();
        _bind_color<fcolor_rgb, fcolor_rgb(f32, f32, f32)>("rgb",
                                                           f32_metatable,
                                                           std::pair { "r", &fcolor_rgb::r },
                                                           std::pair { "g", &fcolor_rgb::g },
                                                           std::pair { "b", &fcolor_rgb::b });
        _bind_color<fcolor_rgba, fcolor_rgba(f32, f32, f32, f32)>("rgba",
                                                                  f32_metatable,
                                                                  std::pair { "r", &fcolor_rgba::r },
                                                                  std::pair { "g", &fcolor_rgba::g },
                                                                  std::pair { "b", &fcolor_rgba::b },
                                                                  std::pair { "a", &fcolor_rgba::a });

        auto u8_metatable = global_state["ucolor"].get_or_create<sol::table>();
        _bind_color<ucolor_rgb, ucolor_rgb(u8, u8, u8)>("rgb",
                                                        u8_metatable,
                                                        std::pair { "r", &ucolor_rgb::r },
                                                        std::pair { "g", &ucolor_rgb::g },
                                                        std::pair { "b", &ucolor_rgb::b });
        _bind_color<ucolor_rgba, ucolor_rgba(u8, u8, u8, u8)>("rgba",
                                                              u8_metatable,
                                                              std::pair { "r", &ucolor_rgba::r },
                                                              std::pair { "g", &ucolor_rgba::g },
                                                              std::pair { "b", &ucolor_rgba::b },
                                                              std::pair { "a", &ucolor_rgba::a });
    }
} // namespace stormkit::lua::core
