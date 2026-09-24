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
    System::System(string name, ComponentTypes types, Closures&& closures) noexcept
        : m_name { std::move(name) }, m_types { std::move(types) }, m_closures { std::move(closures) } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    System::System(System&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto System::operator=(System&&) noexcept -> System& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    System::~System() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto System::add_entity(Entity e) noexcept -> void {
        EXPECTS(e != INVALID_ENTITY);

        m_entities.emplace_back(e);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto System::remove_entity(Entity e) noexcept -> void {
        EXPECTS(e != INVALID_ENTITY);

        const auto [begin, end] = stdr::remove(m_entities, e);
        m_entities.erase(begin, end);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto System::pre_update(EntityManager& manager) noexcept -> void {
        m_closures.pre_update(manager, m_entities);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto System::update(EntityManager& manager, fsecond delta) noexcept -> void {
        m_closures.update(manager, delta, m_entities);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto System::post_update(EntityManager& manager) noexcept -> void {
        m_closures.post_update(manager, m_entities);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto System::on_message_received(EntityManager& manager, const Message& message) noexcept -> void {
        m_closures.on_message_received(manager, message, m_entities);
    }
} // namespace stormkit::entities
