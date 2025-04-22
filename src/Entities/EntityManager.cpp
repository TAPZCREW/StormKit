// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module stormkit.Entities;

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
    auto EntityManager::makeEntity() -> Entity {
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
    auto EntityManager::destroyEntity(Entity entity) -> void {
        expects(entity != INVALID_ENTITY);

        if (hasEntity(entity)) {
            m_removed_entities.emplace(entity);
            m_message_bus.push(Message { REMOVED_ENTITY_MESSAGE_ID, { entity } });
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::destroyAllEntities() -> void {
        for (auto&& e : entities()) {
            m_removed_entities.emplace(e);
            m_message_bus.push(Message { REMOVED_ENTITY_MESSAGE_ID, { e } });
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::hasEntity(Entity entity) const -> bool {
        expects(entity != INVALID_ENTITY);

        return std::ranges::any_of(entities(), monadic::is(entity))
               or std::ranges::any_of(m_added_entities, monadic::is(entity));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::hasComponent(Entity entity, Component::Type type) const -> bool {
        expects(entity != INVALID_ENTITY and type != Component::INVALID_TYPE);

        return std::ranges::any_of(m_registered_components_for_entities.at(entity),
                                   monadic::is(type));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::step(Secondf delta) -> void {
        for (auto entity : m_removed_entities) {
            m_cached_entities_dirty = true;

            auto it = m_registered_components_for_entities.find(entity);
            // a this point, all entities should be valid
            ensures(it != std::ranges::cend(m_registered_components_for_entities));

            for (auto&& key : it->second | std::views::transform([entity](auto&& type) {
                                  return componentKeyFor(entity, type);
                              }))
                m_components.erase(key);

            m_entities.erase(entity);

            removeFromSystems(entity);

            if (not std::ranges::any_of(m_added_entities, monadic::is(entity)))
                m_free_entities.push(entity);
        }
        m_removed_entities.clear();

        if (not std::empty(m_added_entities)) m_cached_entities_dirty = true;
        std::ranges::for_each(m_added_entities,
                              [this](auto&& entity) { m_entities.emplace(entity); });
        m_added_entities.clear();

        std::ranges::for_each(m_updated_entities,
                              [this](auto&& entity) { purposeToSystems(entity); });
        m_updated_entities.clear();

        while (!m_message_bus.empty()) {
            for (auto& system : m_systems) system->onMessageReceived(m_message_bus.top());
            m_message_bus.pop();
        }

        for (auto& system : m_systems) system->preUpdate();
        for (auto& system : m_systems) system->update(delta);
        for (auto& system : m_systems) system->postUpdate();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::purposeToSystems(Entity e) -> void {
        expects(e != INVALID_ENTITY);

        const auto reliable_system_filter = [e, this](auto&& system) {
            for (auto component_type : system->componentsUsed())
                if (not hasComponent(e, component_type)) return false;

            return true;
        };

        std::ranges::for_each(systems() | std::views::filter(reliable_system_filter),
                              [e](auto&& system) { system->addEntity(e); });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::removeFromSystems(Entity e) -> void {
        expects(e != INVALID_ENTITY);

        for (auto& s : m_systems) { s->removeEntity(e); }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::getNeededEntities(System& system) -> void {
        const auto reliable_entity_filter = [&system, this](auto&& entity) {
            for (auto component_type : system.componentsUsed())
                if (not hasComponent(entity, component_type)) return false;

            return true;
        };

        std::ranges::for_each(entities() | std::views::filter(reliable_entity_filter),
                              [&system](auto&& e) { system.addEntity(e); });
    }
} // namespace stormkit::entities
