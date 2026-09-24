// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>

#include <stormkit/entities/api.hpp>

export module stormkit.entities;

import std;

import stormkit.core;

namespace stdr = std::ranges;
namespace stdv = std::views;

namespace cmeta = stormkit::core::meta;

export namespace stormkit::ecs {
    using component_id = u32;

    using entity = u32;

    inline constexpr auto INVALID_ENTITY = entity { 0 };
    class system;

    namespace meta {
        template<typename T>
        concept is_component = requires(T&& component) {
            { component.type() } -> cmeta::is<component_id>;
        };

        template<typename T>
        concept component_with_static_type = is_component<T> and requires(T) {
            { T::type() } -> cmeta::same_as<component_id>;
        };

    } // namespace meta

    struct message {
        u32              id;
        dynarray<entity> entities;
    };

    class STORMKIT_ENTITIES_API message_bus {
      public:
        message_bus();
        ~message_bus();

        message_bus(const message_bus&)                    = delete;
        auto operator=(const message_bus&) -> message_bus& = delete;

        message_bus(message_bus&&);
        auto operator=(message_bus&&) -> message_bus&;

        auto push(message&& message) -> void;
        [[nodiscard]]
        auto top() const -> const message&;
        auto pop() -> void;

        [[nodiscard]]
        auto empty() const noexcept -> bool;

      private:
        std::queue<message> m_messages;
    };

    class entity_manager;

    class STORMKIT_ENTITIES_API system {
      public:
        using component_ids = dynarray<component_id>;

        using pre_update_cb          = std::function<void(entity_manager&, array_view<const entity>)>;
        using update_cb              = std::function<void(entity_manager&, fsecond, array_view<const entity>)>;
        using post_update_cb         = std::function<void(entity_manager&, array_view<const entity>)>;
        using on_message_received_cb = std::function<void(entity_manager&, const message&, array_view<const entity>)>;

        struct Closures {
            pre_update_cb          pre_update = monadic::noop();
            update_cb              update;
            post_update_cb         post_update         = monadic::noop();
            on_message_received_cb on_message_received = monadic::noop();
        };

        system(string name, component_ids components, Closures&& closures) noexcept;

        system(const system&)                    = delete;
        auto operator=(const system&) -> system& = delete;

        system(system&&) noexcept;
        auto operator=(system&&) noexcept -> system&;

        ~system() noexcept;

        [[nodiscard]]
        auto name() const noexcept -> const string&;
        [[nodiscard]]
        auto components_used() const noexcept -> const component_ids&;

      private:
        auto add_entity(entity e) noexcept -> void;
        auto remove_entity(entity e) noexcept -> void;

        auto pre_update(entity_manager&) noexcept -> void;
        auto update(entity_manager&, fsecond) noexcept -> void;
        auto post_update(entity_manager&) noexcept -> void;

        auto on_message_received(entity_manager&, const message&) noexcept -> void;

        string m_name;

        component_ids m_types;

        Closures m_closures;

        dynarray<entity> m_entities;

        friend class entity_manager;
    };

    namespace meta {
        template<typename T>
        concept system_type = requires(T& value) {
            value.update(std::declval<entity_manager&>(), std::declval<fsecond>(), std::declval<dynarray<entity>>());
        };
    } // namespace meta

    struct component_store {};

    class STORMKIT_ENTITIES_API entity_manager {
      public:
        using delete_cb                                 = std::function<void(byte*)>;
        static constexpr auto ADDED_ENTITY_MESSAGE_ID   = 1;
        static constexpr auto REMOVED_ENTITY_MESSAGE_ID = 2;

        entity_manager() noexcept;
        ~entity_manager() noexcept;

        entity_manager(const entity_manager&)                    = delete;
        auto operator=(const entity_manager&) -> entity_manager& = delete;

        entity_manager(entity_manager&&) noexcept;
        auto operator=(entity_manager&&) noexcept -> entity_manager&;

        auto make_entity() noexcept -> entity;
        auto destroy_entity(entity entity) noexcept -> void;
        auto destroy_all_entities() noexcept -> void;
        auto has_entity(entity entity) const noexcept -> bool;

        template<meta::is_component T>
        auto add_component(entity entity, T&& component) noexcept -> cmeta::to_plain_type<T>&;

        auto destroy_component(entity entity, string_view name) noexcept -> void;
        auto destroy_component(entity entity, component_id type) noexcept -> void;

        template<meta::component_with_static_type T>
        auto has_component(entity entity) const noexcept -> bool;
        auto has_component(entity entity, string_view name) const noexcept -> bool;
        auto has_component(entity entity, component_id type) const noexcept -> bool;

        auto entities() const noexcept -> const dynarray<entity>&;

        auto entities_with_component(component_id type) const noexcept -> dynarray<entity>;
        auto entities_with_component(string_view name) const noexcept -> dynarray<entity>;

        template<meta::component_with_static_type T, class Self>
        auto get_component(this Self& self, entity entity) noexcept -> cmeta::forward_const_to<Self, T>&;
        template<meta::is_component T, class Self>
        auto get_component(this Self& self, entity entity, component_id) noexcept -> cmeta::forward_const_to<Self, T>&;
        template<meta::is_component T, class Self>
        auto get_component(this Self& self, entity entity, string_view) noexcept -> cmeta::forward_const_to<Self, T>&;

        template<meta::component_with_static_type T, class Self>
        auto components_of_type(this Self& self) noexcept -> dynarray<ref<cmeta::forward_const_to<Self, T>>>;
        template<meta::is_component T, class Self>
        auto components_of_type(this Self& self, component_id type) noexcept -> dynarray<ref<cmeta::forward_const_to<Self, T>>>;
        template<meta::is_component T, class Self>
        auto components_of_type(this Self& self, string_view name) noexcept -> dynarray<ref<cmeta::forward_const_to<Self, T>>>;

        auto components_types_of(entity entity) const noexcept -> dynarray<component_id>;

        template<meta::system_type T>
        auto add_system(string name, system::component_ids types, T& system) noexcept -> system&;
        auto add_system(string name, system::component_ids types, system::Closures&& closures) noexcept -> system&;
        auto has_system(string_view name) const noexcept -> bool;
        auto remove_system(string_view name) noexcept -> void;

        template<class Self>
        auto systems(this Self& self) noexcept -> cmeta::forward_const_to<Self, dynarray<system>&>;

        template<class Self>
        auto get_system(this Self& self, string_view name) noexcept -> cmeta::forward_const_to<Self, system&>;

        auto flush() noexcept -> void;
        auto step(fsecond delta) noexcept -> void;

        auto entity_count() const noexcept -> usize;

        auto add_raw_component(entity entity, component_id type, array_view<const byte> component, delete_cb delete_func) noexcept
          -> array_view<byte>;

        template<class Self>
        auto get_raw_component(this Self& self, entity entity, component_id type) noexcept
          -> array_view<cmeta::forward_const_to<Self, byte>>;

      private:
        using component_key = u64;

        struct store {
            component_id     type;
            usize            size;
            dynarray<entity> entities;
            dynarray<byte>   data;
            delete_cb        delete_func;
        };

        using component_store = dynarray<store>;

        auto purpose_to_systems(entity e) noexcept -> void;
        auto remove_from_systems(entity e) noexcept -> void;
        auto get_needed_entities(system& system) noexcept -> void;

        entity m_next_valid_entity = 1;

        dynarray<entity> m_entities;

        dynarray<entity> m_free_entities;

        hash_set<entity> m_added_entities;
        hash_set<entity> m_updated_entities;
        hash_set<entity> m_removed_entities;

        dynarray<system> m_systems;

        component_store m_components;

        message_bus m_message_bus;
    };
} // namespace stormkit::ecs

namespace stormkit::ecs {
    /////////////////////////////////////
    /////////////////////////////////////
    inline auto message_bus::empty() const noexcept -> bool {
        return std::empty(m_messages);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto system::name() const noexcept -> const string& {
        return m_name;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto system::components_used() const noexcept -> const component_ids& {
        return m_types;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_component T>
    auto entity_manager::add_component(entity entity, T&& component) noexcept -> cmeta::to_plain_type<T>& {
        using PureT = cmeta::to_plain_type<T>;

        auto _component = add_raw_component(entity,
                                            component.type(),
                                            std::bit_cast<array<byte, sizeof(T)>>(std::forward<T>(component)),
                                            [](auto ptr) static noexcept {
                                                std::launder(reinterpret_cast<PureT*>(ptr))->~PureT();
                                            });

        return *std::launder(reinterpret_cast<PureT*>(stdr::data(_component));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto entity_manager::destroy_component(entity entity, string_view name) noexcept -> void {
        destroy_component(entity, hash(name));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::component_with_static_type T>
    inline auto entity_manager::has_component(entity entity) const noexcept -> bool {
        return has_component(entity, T::type());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto entity_manager::has_component(entity entity, string_view name) const noexcept -> bool {
        return has_component(entity, hash(name));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto entity_manager::entities() const noexcept -> const dynarray<entity>& {
        return m_entities;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto entity_manager::entities_with_component(component_id type) const noexcept -> dynarray<entity> {
        // clang-format off
        return entities() 
               | stdv::filter([this, type](auto entity) noexcept { return has_component(entity, type); })
               | stdr::to<dynarray>();
        // clang-format on
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto entity_manager::entities_with_component(string_view name) const noexcept -> dynarray<entity> {
        return entities_with_component(hash(name));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::component_with_static_type T, class Self>
    auto entity_manager::get_component(this Self& self, entity entity) noexcept -> cmeta::forward_const_to<Self, T>& {
        return self.template get_component<T>(entity, T::type());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_component T, class Self>
    auto entity_manager::get_component(this Self& self, entity entity, component_id type) noexcept
      -> cmeta::forward_const_to<Self, T>& {
        return *std::launder(reinterpret_cast<cmeta::forward_const_to<Self, T>*>(self.get_raw_component(entity, type)));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_component T, class Self>
    auto entity_manager::get_component(this Self& self, entity entity, string_view name) noexcept
      -> cmeta::forward_const_to<Self, T>& {
        return self.template get_component<T>(entity, hash(name));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::component_with_static_type T, class Self>
    auto entity_manager::components_of_type(this Self& self) noexcept -> dynarray<ref<cmeta::forward_const_to<Self, T>>> {
        return self.template components_of_type<T>(T::type());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_component T, class Self>
    auto entity_manager::components_of_type(this Self& self, component_id type) noexcept
      -> dynarray<ref<cmeta::forward_const_to<Self, T>>> {
        // clang-format off
        return self.m_entities 
               | stdv::filter([&self, type](auto entity) noexcept { return self.has_component(entity, type); })
               | stdv::transform([&self, type](auto entity) noexcept { return self.template get_component<T>(entity, type); })
               // | stdv::transform(monadic::forward_like<Self&>())
               | stdv::transform(monadic::as_ref())
               | stdr::to<dynarray>();
        // clang-format on
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_component T, class Self>
    auto entity_manager::components_of_type(this Self& self, string_view name) noexcept
      -> dynarray<ref<cmeta::forward_const_to<Self, T>>> {
        return self.template components_of_type<T>(hash(name));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto entity_manager::components_types_of(entity entity) const noexcept -> dynarray<component_id> {
        EXPECTS(has_entity(entity));

        auto out = dynarray<component_id> {};
        for (const auto& [type, _, entities, _, _] : m_components) {
            for (auto e : entities)
                if (e == entity) {
                    out.emplace_back(type);
                    break;
                }
        }
        return out;
    }

    namespace meta {
        template<typename T>
        concept HasPreUpdate = requires(T& value) { value.pre_update(std::declval<entity_manager&>()); };

        template<typename T>
        concept HasPostUpdate = requires(T& value) { value.post_update(std::declval<entity_manager&>()); };

        template<typename T>
        concept Hason_message_received_cb = requires(T& value) {
            value.on_message_received(std::declval<entity_manager&>(),
                                      std::declval<const message&>(),
                                      std::declval<array_view<const entity>>());
        };
    } // namespace meta

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::system_type T>
    inline auto entity_manager::add_system(string name, system::component_ids types, T& system) noexcept -> system& {
        auto closures = system::Closures {
            .update = bind_front(&T::update, &system),
        };

        if constexpr (meta::HasPreUpdate<T>) closures.pre_update = bind_front(&T::pre_update, &system);
        if constexpr (meta::HasPostUpdate<T>) closures.post_update = bind_front(&T::post_update, &system);
        if constexpr (meta::Hason_message_received_cb<T>)
            closures.on_message_received = bind_front(&T::on_message_received, &system);

        return add_system(std::move(name), std::move(types), std::move(closures));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto entity_manager::add_system(string name, system::component_ids types, system::Closures&& closures) noexcept
      -> system& {
        auto& system = m_systems.emplace_back(std::move(name), std::move(types), std::move(closures));

        get_needed_entities(system);

        return system;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto entity_manager::has_system(string_view name) const noexcept -> bool {
        return stdr::any_of(m_systems, [name](const auto& system) noexcept { return system.name() == name; });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto entity_manager::remove_system(string_view name) noexcept -> void {
        auto&& [begin, end] = stdr::remove_if(m_systems, [&name](const auto& system) { return name == system.name(); });
        m_systems.erase(begin, end);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<class Self>
    auto entity_manager::systems(this Self& self) noexcept -> dynarray<ref<cmeta::forward_const_to<Self, system>>> {
        constexpr auto as_refer = [] {
            if constexpr (cmeta::const_type<Self>) return monadic::as_ref();
            else
                return monadic::as_ref_mut();
        }();

        return self.m_systems | stdv::transform(as_refer) | stdr::to<dynarray<ref<cmeta::forward_const_to<Self, system>>>>();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<class Self>
    auto entity_manager::get_system(this Self& self, string_view name) noexcept -> cmeta::forward_const_to<Self, system&> {
        EXPECTS(self.has_system(name));

        const auto it = stdr::find_if(self.m_systems, [name](const auto& system) noexcept { return system.name() == name; });
        return std::forward_like<Self&>(*it->get());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto entity_manager::entity_count() const noexcept -> usize {
        return std::size(m_entities);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto entity_manager::add_raw_component(entity entity, component_id type, array_view<const byte> component, delete_cb delete_func) noexcept
      -> array_view<byte> {
        EXPECTS(has_entity(entity));
        EXPECTS(not has_component(entity, type));

        const auto _size = stdr::size(component);

        auto it = stdr::find_if(m_components, [type = type](const auto& pair) noexcept { return pair.type == type; });
        if (it == stdr::cend(m_components))
            it = m_components.emplace(stdr::cend(m_components),
                                      store { type, stdr::size(component), {}, {}, std::move(delete_func) });

        ENSURES(it != stdr::cend(m_components));

        auto& [_, size, entities, components, _] = *it;
        ENSURES(size == _size);

        const auto old_size = stdr::size(components);
        components.resize(old_size + sizeof(entity) + size);

        new (stdr::data(components) + old_size) entity { entity };
        auto _component = array_view { stdr::data(components) + old_size + sizeof(entity), _size };
        stdr::copy(component, stdr::begin(_component));

        entities.emplace_back(entity);

        m_updated_entities.emplace(entity);

        return _component;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<class Self>
    auto entity_manager::get_raw_component(this Self& self, entity entity, component_id type) noexcept
      -> array_view<cmeta::forward_const_to<Self, byte>> {
        EXPECTS(self.has_entity(entity));
        EXPECTS(self.has_component(entity, type));

        auto it = stdr::find_if(self.m_components, [&type](const auto& pair) noexcept { return pair.type == type; });
        ENSURES(it != stdr::cend(self.m_components));

        auto& [_, size, _, components, _] = *it;

        auto component_it = stdr::data(components);
        for (;;) {
            auto e = *std::launder(std::bit_cast<entity*>(component_it));
            if (e != entity) {
                component_it += sizeof(entity) + size;
                continue;
            }

            component_it += sizeof(entity);

            break;
        }

        return { component_it, size };
    }
} // namespace stormkit::ecs
