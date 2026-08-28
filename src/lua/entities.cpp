// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/lua/lua.hpp>

module stormkit.lua;

import std;

import stormkit.core;
import stormkit.entities;

namespace stdr = std::ranges;
namespace stdv = std::views;

namespace stormkit::lua::entities {
    using stormkit::entities::ComponentType;
    using stormkit::entities::Entity;
    using stormkit::entities::EntityManager;
    using stormkit::entities::System;

    using stormkit::entities::lua::LuaComponent;

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto bind_manager(sol::table& entities) noexcept -> void {
        auto no_constructor = false;

        auto manager = [&entities, no_constructor]() {
            if (no_constructor) return entities.new_usertype<EntityManager>("manager", sol::no_constructor);
            else
                return entities.new_usertype<EntityManager>("manager");
        }(/*config.engine*/);

        manager["make_entity"]          = &EntityManager::make_entity;
        manager["destroy_entity"]       = &EntityManager::destroy_entity;
        manager["destroy_all_entities"] = &EntityManager::destroy_all_entities;
        manager["has_entity"]           = &EntityManager::has_entity;
        manager["add_component"]        = +[](EntityManager* manager, Entity entity, sol::table component) static noexcept {
            const auto type_closure = component.get<std::optional<sol::protected_function>>("type");
            ensures(type_closure.has_value(), "Missing type() function on lua component");

            const auto type   = sol::protected_function { *type_closure };
            const auto result = luacall(type, component);
            const auto value  = sol::object { result };

            ensures(value.is<string>(), "Component type() must return a string or a component type");
            const auto _type = hash(value.as<string>());
            manager->add_component<LuaComponent>(entity, LuaComponent { .data = std::move(component), ._type = _type });
        };
        manager["get_component"] = +[](EntityManager* manager, Entity entity, string_view name) static noexcept {
            return manager->get_component<LuaComponent>(entity, name).data;
        };
        manager["has_component"] = +[](EntityManager* manager, Entity entity, string_view name) static noexcept {
            return manager->has_component(entity, name);
        };
        manager["entities"]            = &EntityManager::entities;
        manager["entity_count"]        = &EntityManager::entity_count;
        manager["components_types_of"] = &EntityManager::components_types_of;
        manager["add_system"]          = +[](EntityManager*    manager,
                                             string            name,
                                             dynarray<string> types,
                                             sol::table        opt) static noexcept {
            auto update = opt.get<std::optional<sol::protected_function>>("update");
            expects(update.has_value(), std::format("No update closure supplied for system {}", name));

            auto _closures = System::Closures {
                .update =
                  [update = *std::move(update)](auto& manager, auto delta, const auto& entities) {
                      luacall(update, manager, delta.count(), sol::as_table(entities));
                  },
            };

            auto pre_update = opt.get<std::optional<sol::protected_function>>("pre_update");
            if (pre_update.has_value())
                _closures.pre_update = [pre_update = *std::move(pre_update)](auto& manager, const auto& entities) {
                    luacall(pre_update, manager, sol::as_table(entities));
                };
            auto post_update = opt.get<std::optional<sol::protected_function>>("post_update");
            if (post_update.has_value())
                _closures.post_update = [post_update = *std::move(post_update)](auto& manager, const auto& entities) {
                    luacall(post_update, manager, sol::as_table(entities));
                };

            manager->add_system(std::move(name),
                                types | stdv::transform([](const auto& type) static noexcept {
                                    return hash(type);
                                }) | stdr::to<dynarray<hash32>>(),
                                std::move(_closures));
        };
        manager["has_system"]    = &EntityManager::has_system;
        manager["remove_system"] = &EntityManager::remove_system;

        if (not no_constructor) {
            manager["step"] = +[](EntityManager* manager, float delta) static noexcept { manager->step(fsecond { delta }); };
        }
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto init_lua(sol::state& global_state) noexcept -> void {
        auto entities = global_state["entities"].get_or_create<sol::table>();
        bind_manager(entities);
    }
} // namespace stormkit::lua::entities
