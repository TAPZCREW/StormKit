// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>

module stormkit.entities;

import std;

import stormkit.core;

namespace stormkit::entities {
    /////////////////////////////////////
    /////////////////////////////////////
    EntityManager::EntityManager() = default;

    /////////////////////////////////////
    /////////////////////////////////////
    EntityManager::EntityManager(EntityManager&&) = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::operator=(EntityManager&&) -> EntityManager& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    EntityManager::~EntityManager() = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::make_entity() -> Entity {
        const auto entity = [this]() {
            if (std::empty(m_free_entities)) return m_next_valid_entity++;
            else {
                auto entity = m_free_entities.front();
                m_free_entities.pop();
                return entity;
            }
        }();

        m_added_entities.emplace(entity);
        m_updated_entities.emplace(entity);
        m_registered_components_for_entities[entity] = {};
        m_message_bus.push(Message { ADDED_ENTITY_MESSAGE_ID, { entity } });

        return entity;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::destroy_entity(Entity entity) -> void {
        EXPECTS(entity != INVALID_ENTITY);

        if (has_entity(entity)) {
            m_removed_entities.emplace(entity);
            m_message_bus.push(Message { REMOVED_ENTITY_MESSAGE_ID, { entity } });
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::destroy_all_entities() -> void {
        for (auto&& e : entities()) {
            m_removed_entities.emplace(e);
            m_message_bus.push(Message { REMOVED_ENTITY_MESSAGE_ID, { e } });
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::has_entity(Entity entity) const -> bool {
        EXPECTS(entity != INVALID_ENTITY);

        return std::ranges::any_of(entities(), monadic::is(entity))
               or std::ranges::any_of(m_added_entities, monadic::is(entity));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::has_component(Entity entity, Component::Type type) const -> bool {
        EXPECTS(entity != INVALID_ENTITY and type != Component::INVALID_TYPE);

        return std::ranges::any_of(m_registered_components_for_entities.at(entity),
                                   monadic::is(type));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::step(Secondf delta) -> void {
        for (auto entity : m_removed_entities) {
            auto it = m_registered_components_for_entities.find(entity);
            // a this point, all entities should be valid
            ensures(it != std::ranges::cend(m_registered_components_for_entities));

            for (auto&& key : it->second | std::views::transform([entity](auto&& type) {
                                  return component_key_for(entity, type);
                              }))
                m_components.erase(key);

            m_entities.erase(entity);

            remove_from_systems(entity);

            if (not std::ranges::any_of(m_added_entities, monadic::is(entity)))
                m_free_entities.push(entity);
        }
        m_removed_entities.clear();

        std::ranges::for_each(m_added_entities, [this](auto&& entity) {
            m_entities.emplace(entity);
        });
        m_added_entities.clear();

        std::ranges::for_each(m_updated_entities, [this](auto&& entity) {
            purpose_to_systems(entity);
        });
        m_updated_entities.clear();

        while (!m_message_bus.empty()) {
            for (auto& system : m_systems) system->on_message_received(m_message_bus.top());
            m_message_bus.pop();
        }

        for (auto& system : m_systems) system->pre_update();
        for (auto& system : m_systems) system->update(delta);
        for (auto& system : m_systems) system->post_update();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::purpose_to_systems(Entity e) -> void {
        EXPECTS(e != INVALID_ENTITY);

        const auto reliable_system_filter = [e, this](auto&& system) {
            for (auto component_type : system->components_used())
                if (not has_component(e, component_type)) return false;

            return true;
        };

        std::ranges::for_each(systems() | std::views::filter(reliable_system_filter),
                              [e](auto&& system) { system->add_entity(e); });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::remove_from_systems(Entity e) -> void {
        EXPECTS(e != INVALID_ENTITY);

        for (auto& s : m_systems) { s->remove_entity(e); }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::get_needed_entities(System& system) -> void {
        const auto reliable_entity_filter = [&system, this](auto&& entity) {
            for (auto component_type : system.components_used())
                if (not has_component(entity, component_type)) return false;

            return true;
        };

        std::ranges::for_each(entities() | std::views::filter(reliable_entity_filter),
                              [&system](auto&& e) { system.add_entity(e); });
    }
} // namespace stormkit::entities
