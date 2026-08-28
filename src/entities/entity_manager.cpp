// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>

module stormkit.entities;

import std;

import stormkit.core;

namespace stdr = std::ranges;
namespace stdv = std::views;

namespace stormkit::entities {
    /////////////////////////////////////
    /////////////////////////////////////
    EntityManager::EntityManager() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    EntityManager::EntityManager(EntityManager&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::operator=(EntityManager&&) noexcept -> EntityManager& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    EntityManager::~EntityManager() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::make_entity() noexcept -> Entity {
        const auto entity = [this]() {
            if (stdr::empty(m_free_entities)) return m_next_valid_entity++;
            else {
                auto entity = m_free_entities.back();
                m_free_entities.pop_back();
                return entity;
            }
        }();

        m_added_entities.emplace(entity);
        m_updated_entities.emplace(entity);
        m_message_bus.push(Message { ADDED_ENTITY_MESSAGE_ID, { entity } });

        return entity;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::destroy_entity(Entity entity) noexcept -> void {
        EXPECTS(entity != INVALID_ENTITY);

        if (has_entity(entity)) {
            m_removed_entities.emplace(entity);
            m_message_bus.push(Message { REMOVED_ENTITY_MESSAGE_ID, { entity } });
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::destroy_all_entities() noexcept -> void {
        for (const auto& e : entities()) {
            m_removed_entities.emplace(e);
            m_message_bus.push(Message { REMOVED_ENTITY_MESSAGE_ID, { e } });
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::has_entity(Entity entity) const noexcept -> bool {
        EXPECTS(entity != INVALID_ENTITY);

        return stdr::any_of(entities(), monadic::is(entity)) or stdr::any_of(m_added_entities, monadic::is(entity));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::destroy_component(Entity entity, ComponentType type) noexcept -> void {
        EXPECTS(has_entity(entity));
        EXPECTS(has_component(entity, type));

        auto it = stdr::find_if(m_components, [type](const auto& pair) noexcept { return pair.type == type; });

        auto& [_, size, entities, components, delete_func] = *it;
        auto component_it                                  = stdr::begin(components);

        for (;;) {
            auto e = *std::launder(std::bit_cast<Entity*>(&*component_it));

            if (e == entity) break;

            component_it += as<ioffset>(sizeof(Entity) + size);
        }

        delete_func(&*component_it + sizeof(Entity));

        components.erase(component_it, component_it + as<ioffset>(size));

        auto&& [begin, end] = stdr::remove(entities, entity);
        entities.erase(begin, end);

        m_updated_entities.emplace(entity);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::has_component(Entity entity, ComponentType type) const noexcept -> bool {
        EXPECTS(entity != INVALID_ENTITY);

        auto it = stdr::find_if(m_components, [&type](const auto& pair) noexcept { return pair.type == type; });
        if (it == stdr::cend(m_components)) return false;

        auto& [_, size, entities, _, _] = *it;

        return stdr::any_of(entities, monadic::is(entity));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::flush() noexcept -> void {
        if (not stdr::empty(m_removed_entities)) {
            if (stdr::size(m_entities) == stdr::size(m_removed_entities)) {
                for (auto& [_, size, entities, data, delete_func] : m_components) {
                    for (auto component_it = stdr::data(data); component_it != stdr::data(data) + stdr::size(data);
                         component_it += as<ioffset>(sizeof(Entity) + size))
                        delete_func(component_it + sizeof(Entity));

                    entities.clear();
                    data.clear();
                }

                merge(m_free_entities, m_entities);
                m_entities.clear();
                m_removed_entities.clear();
            } else {
                for (auto entity : m_removed_entities) {
                    const auto components_types = components_types_of(entity);

                    for (auto t : components_types) destroy_component(entity, t);

                    auto&& [begin, end] = stdr::remove(m_entities, entity);
                    m_entities.erase(begin, end);

                    remove_from_systems(entity);

                    if (not stdr::any_of(m_added_entities, monadic::is(entity))) m_free_entities.emplace_back(entity);
                }
                m_removed_entities.clear();
            }
        }

        stdr::for_each(m_added_entities, [this](auto&& entity) noexcept { m_entities.emplace_back(entity); });
        m_added_entities.clear();

        stdr::for_each(m_updated_entities, [this](auto&& entity) noexcept { purpose_to_systems(entity); });
        m_updated_entities.clear();

        while (!m_message_bus.empty()) {
            for (auto& system : m_systems) system.on_message_received(*this, m_message_bus.top());
            m_message_bus.pop();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::step(fsecond delta) noexcept -> void {
        flush();
        for (auto& system : m_systems) system.pre_update(*this);
        for (auto& system : m_systems) system.update(*this, delta);
        for (auto& system : m_systems) system.post_update(*this);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::purpose_to_systems(Entity e) noexcept -> void {
        EXPECTS(e != INVALID_ENTITY);

        const auto reliable_system_filter = [e, this](auto&& system) {
            for (auto component_type : system->components_used())
                if (not has_component(e, component_type)) return false;

            return true;
        };

        stdr::for_each(systems() | stdv::filter(reliable_system_filter), [e](auto&& system) noexcept { system->add_entity(e); });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::remove_from_systems(Entity e) noexcept -> void {
        EXPECTS(e != INVALID_ENTITY);

        for (auto& s : m_systems) { s.remove_entity(e); }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::get_needed_entities(System& system) noexcept -> void {
        const auto reliable_entity_filter = [&system, this](auto&& entity) noexcept {
            for (auto component_type : system.components_used())
                if (not has_component(entity, component_type)) return false;

            return true;
        };

        stdr::for_each(entities() | stdv::filter(reliable_entity_filter), [&system](auto&& e) noexcept { system.add_entity(e); });
    }

} // namespace stormkit::entities
