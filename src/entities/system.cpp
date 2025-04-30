// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module stormkit.entities;

import std;

import stormkit.core;

namespace stormkit::entities {
    /////////////////////////////////////
    /////////////////////////////////////
    System::System(EntityManager& manager, UInt32 priority, ComponentTypes types)
        : m_manager { as_ref(manager) }, m_priority { priority }, m_types { std::move(types) } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    System::System(System&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto System::operator=(System&&) noexcept -> System& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    System::~System() = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto System::pre_update() -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto System::post_update() -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto System::add_entity(Entity e) -> void {
        expects(e != INVALID_ENTITY);

        m_entities.insert(e);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto System::remove_entity(Entity e) -> void {
        expects(e != INVALID_ENTITY);

        m_entities.erase(e);
    }
} // namespace stormkit::entities
