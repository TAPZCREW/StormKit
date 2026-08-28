// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/memory_macro.hpp>
#include <stormkit/core/platform_macro.hpp>

#include <stormkit/entities/api.hpp>

#include <string_view>

#include <stormkit/core/config.hpp>
#ifdef STORMKIT_LIB_LUA_ENABLED
    #include <stormkit/lua/lua.hpp>
#endif

export module stormkit.entities;

import std;

import stormkit.core;

namespace stdr = std::ranges;
namespace stdv = std::views;

namespace cmeta = stormkit::core::meta;

export namespace stormkit::entities {
    using ComponentType = u32;

#ifdef STORMKIT_LIB_LUA_ENABLED
    namespace lua {
        struct LuaComponent {
            sol::table    data;
            ComponentType _type;

            STORMKIT_FORCE_INLINE
            inline auto   type() const noexcept -> ComponentType {
                return _type;
            }
        };
    } // namespace lua
#endif

    using Entity   = u32;
    using Entities = dynarray<Entity>;

    inline constexpr auto INVALID_ENTITY = Entity { 0 };
    class System;

    struct EntityHashFunc {
#ifdef STORMKIT_COMPILER_MSVC
        [[nodiscard]]
        auto operator()(Entity k) const noexcept -> hash64;
#else
        [[nodiscard]]
        static auto operator()(Entity k) noexcept -> hash64;
#endif
    };

    namespace meta {
        template<typename T>
        concept IsComponentType = requires(T&& component) {
            { component.type() } -> cmeta::is<ComponentType>;
        };

        template<typename T>
        concept ComponentWithStaticType = IsComponentType<T> and requires(T) {
            { T::type() } -> cmeta::same_as<ComponentType>;
        };

    } // namespace meta

    struct Message {
        u32      id;
        Entities entities;
    };

    class STORMKIT_ENTITIES_API MessageBus {
      public:
        MessageBus();
        ~MessageBus();

        MessageBus(const MessageBus&)                    = delete;
        auto operator=(const MessageBus&) -> MessageBus& = delete;

        MessageBus(MessageBus&&);
        auto operator=(MessageBus&&) -> MessageBus&;

        auto push(Message&& message) -> void;
        [[nodiscard]]
        auto top() const -> const Message&;
        auto pop() -> void;

        [[nodiscard]]
        auto empty() const noexcept -> bool;

      private:
        std::queue<Message> m_messages;
    };

    class EntityManager;

    class STORMKIT_ENTITIES_API System {
      public:
        using ComponentTypes = dynarray<ComponentType>;

        using PreUpdateClosure  = std::function<void(EntityManager&, const Entities&)>;
        using UpdateClosure     = std::function<void(EntityManager&, fsecond, const Entities&)>;
        using PostUpdateClosure = std::function<void(EntityManager&, const Entities&)>;
        using OnMessageReceived = std::function<void(EntityManager&, const Message&, const Entities&)>;

        struct Closures {
            PreUpdateClosure  pre_update = monadic::noop();
            UpdateClosure     update;
            PostUpdateClosure post_update         = monadic::noop();
            OnMessageReceived on_message_received = monadic::noop();
        };

        System(string name, ComponentTypes types, Closures&& closures) noexcept;

        System(const System&)                    = delete;
        auto operator=(const System&) -> System& = delete;

        System(System&&) noexcept;
        auto operator=(System&&) noexcept -> System&;

        ~System() noexcept;

        [[nodiscard]]
        auto name() const noexcept -> const string&;
        [[nodiscard]]
        auto components_used() const noexcept -> const ComponentTypes&;

      private:
        auto add_entity(Entity e) noexcept -> void;
        auto remove_entity(Entity e) noexcept -> void;

        auto pre_update(EntityManager&) noexcept -> void;
        auto update(EntityManager&, fsecond) noexcept -> void;
        auto post_update(EntityManager&) noexcept -> void;

        auto on_message_received(EntityManager&, const Message&) noexcept -> void;

        string m_name;

        ComponentTypes m_types;

        Closures m_closures;

        Entities m_entities;

        friend class EntityManager;
    };

    namespace meta {
        template<typename T>
        concept IsUsableAsSystem = requires(T& value) {
            value.update(std::declval<EntityManager&>(), std::declval<fsecond>(), std::declval<Entities>());
        };
    } // namespace meta

    struct ComponentStore {};

    class STORMKIT_ENTITIES_API EntityManager {
      public:
        using DeleteFunc                                = std::function<void(byte*)>;
        static constexpr auto ADDED_ENTITY_MESSAGE_ID   = 1;
        static constexpr auto REMOVED_ENTITY_MESSAGE_ID = 2;

        EntityManager() noexcept;
        ~EntityManager() noexcept;

        EntityManager(const EntityManager&)                    = delete;
        auto operator=(const EntityManager&) -> EntityManager& = delete;

        EntityManager(EntityManager&&) noexcept;
        auto operator=(EntityManager&&) noexcept -> EntityManager&;

        auto make_entity() noexcept -> Entity;
        auto destroy_entity(Entity entity) noexcept -> void;
        auto destroy_all_entities() noexcept -> void;
        auto has_entity(Entity entity) const noexcept -> bool;

        template<meta::IsComponentType T>
        auto add_component(Entity entity, T&& component) noexcept -> cmeta::to_plain_type<T>&;

        auto destroy_component(Entity entity, string_view name) noexcept -> void;
        auto destroy_component(Entity entity, ComponentType type) noexcept -> void;

        template<meta::ComponentWithStaticType T>
        auto has_component(Entity entity) const noexcept -> bool;
        auto has_component(Entity entity, string_view name) const noexcept -> bool;
        auto has_component(Entity entity, ComponentType type) const noexcept -> bool;

        auto entities() const noexcept -> const Entities&;

        auto entities_with_component(ComponentType type) const noexcept -> Entities;
        auto entities_with_component(string_view name) const noexcept -> Entities;

        template<meta::ComponentWithStaticType T, class Self>
        auto get_component(this Self& self, Entity entity) noexcept -> cmeta::forward_const_to<Self, T>&;
        template<meta::IsComponentType T, class Self>
        auto get_component(this Self& self, Entity entity, ComponentType) noexcept -> cmeta::forward_const_to<Self, T>&;
        template<meta::IsComponentType T, class Self>
        auto get_component(this Self& self, Entity entity, string_view) noexcept -> cmeta::forward_const_to<Self, T>&;

        template<meta::ComponentWithStaticType T, class Self>
        auto components_of_type(this Self& self) noexcept -> dynarray<ref<cmeta::forward_const_to<Self, T>>>;
        template<meta::IsComponentType T, class Self>
        auto components_of_type(this Self& self, ComponentType type) noexcept -> dynarray<ref<cmeta::forward_const_to<Self, T>>>;
        template<meta::IsComponentType T, class Self>
        auto components_of_type(this Self& self, string_view name) noexcept -> dynarray<ref<cmeta::forward_const_to<Self, T>>>;

        auto components_types_of(Entity entity) const noexcept -> dynarray<ComponentType>;

        template<meta::IsUsableAsSystem T>
        auto add_system(string name, System::ComponentTypes types, T& system) noexcept -> System&;
        auto add_system(string name, System::ComponentTypes types, System::Closures&& closures) noexcept -> System&;
        auto has_system(string_view name) const noexcept -> bool;
        auto remove_system(string_view name) noexcept -> void;

        template<class Self>
        auto systems(this Self& self) noexcept -> dynarray<ref<cmeta::forward_const_to<Self, System>>>;

        template<class Self>
        auto get_system(this Self& self, string_view name) noexcept -> cmeta::forward_const_to<Self, System&>;

        auto flush() noexcept -> void;
        auto step(fsecond delta) noexcept -> void;

        auto entity_count() const noexcept -> usize;

        auto add_raw_component(Entity entity, ComponentType type, byte_view component, DeleteFunc delete_func) noexcept
          -> byte_view_mut;

        template<class Self>
        auto get_raw_component(this Self& self, Entity entity, ComponentType type) noexcept
          -> array_view<cmeta::forward_const_to<Self, byte>>;

      private:
        using ComponentKey = u64;

        struct Store {
            ComponentType  type;
            usize          size;
            Entities       entities;
            byte_dynarray data;
            DeleteFunc     delete_func;
        };

        using ComponentStore = dynarray<Store>;

        auto purpose_to_systems(Entity e) noexcept -> void;
        auto remove_from_systems(Entity e) noexcept -> void;
        auto get_needed_entities(System& system) noexcept -> void;

        Entity m_next_valid_entity = 1;

        Entities m_entities;

        Entities m_free_entities;

        hash_set<Entity> m_added_entities;
        hash_set<Entity> m_updated_entities;
        hash_set<Entity> m_removed_entities;

        dynarray<System> m_systems;

        ComponentStore m_components;

        MessageBus m_message_bus;
    };
} // namespace stormkit::entities

namespace stormkit::entities {
    /////////////////////////////////////
    /////////////////////////////////////
#ifdef STORMKIT_COMPILER_MSVC
    inline auto EntityHashFunc::operator()(Entity k) const noexcept -> hash64 {
#else
    inline auto EntityHashFunc::operator()(Entity k) noexcept -> hash64 {
#endif
        return as<hash64>(k);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto MessageBus::empty() const noexcept -> bool {
        return std::empty(m_messages);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto System::name() const noexcept -> const string& {
        return m_name;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto System::components_used() const noexcept -> const ComponentTypes& {
        return m_types;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::IsComponentType T>
    auto EntityManager::add_component(Entity entity, T&& component) noexcept -> cmeta::to_plain_type<T>& {
        using PureT = cmeta::to_plain_type<T>;

        auto _component = add_raw_component(entity,
                                            component.type(),
                                            as<array_view>(as_bytes, std::forward<T>(component)),
                                            [](auto ptr) static noexcept { std::launder(std::bit_cast<PureT*>(ptr))->~PureT(); });

        return bytes_mut_as<PureT>(_component);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto EntityManager::destroy_component(Entity entity, string_view name) noexcept -> void {
        destroy_component(entity, hash(name));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::ComponentWithStaticType T>
    inline auto EntityManager::has_component(Entity entity) const noexcept -> bool {
        return has_component(entity, T::type());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto EntityManager::has_component(Entity entity, string_view name) const noexcept -> bool {
        return has_component(entity, hash(name));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto EntityManager::entities() const noexcept -> const Entities& {
        return m_entities;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto EntityManager::entities_with_component(ComponentType type) const noexcept -> Entities {
        // clang-format off
        return entities() 
               | stdv::filter([this, type](auto entity) noexcept { return has_component(entity, type); })
               | stdr::to<Entities>();
        // clang-format on
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto EntityManager::entities_with_component(string_view name) const noexcept -> Entities {
        return entities_with_component(hash(name));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::ComponentWithStaticType T, class Self>
    auto EntityManager::get_component(this Self& self, Entity entity) noexcept -> cmeta::forward_const_to<Self, T>& {
        return self.template get_component<T>(entity, T::type());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::IsComponentType T, class Self>
    auto EntityManager::get_component(this Self& self, Entity entity, ComponentType type) noexcept
      -> cmeta::forward_const_to<Self, T>& {
        if constexpr (cmeta::const_type<Self>) return bytes_as<T>(self.get_raw_component(entity, type));
        else
            return bytes_mut_as<T>(self.get_raw_component(entity, type));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::IsComponentType T, class Self>
    auto EntityManager::get_component(this Self& self, Entity entity, string_view name) noexcept
      -> cmeta::forward_const_to<Self, T>& {
        return self.template get_component<T>(entity, hash(name));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::ComponentWithStaticType T, class Self>
    auto EntityManager::components_of_type(this Self& self) noexcept -> dynarray<ref<cmeta::forward_const_to<Self, T>>> {
        return self.template components_of_type<T>(T::type());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::IsComponentType T, class Self>
    auto EntityManager::components_of_type(this Self& self, ComponentType type) noexcept
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
    template<meta::IsComponentType T, class Self>
    auto EntityManager::components_of_type(this Self& self, string_view name) noexcept
      -> dynarray<ref<cmeta::forward_const_to<Self, T>>> {
        return self.template components_of_type<T>(hash(name));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto EntityManager::components_types_of(Entity entity) const noexcept -> dynarray<ComponentType> {
        EXPECTS(has_entity(entity));

        auto out = dynarray<ComponentType> {};
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
        concept HasPreUpdate = requires(T& value) { value.pre_update(std::declval<EntityManager&>()); };

        template<typename T>
        concept HasPostUpdate = requires(T& value) { value.post_update(std::declval<EntityManager&>()); };

        template<typename T>
        concept HasOnMessageReceived = requires(T& value) {
            value.on_message_received(std::declval<EntityManager&>(),
                                      std::declval<const Message&>(),
                                      std::declval<const Entities&>());
        };
    } // namespace meta

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::IsUsableAsSystem T>
    inline auto EntityManager::add_system(string name, System::ComponentTypes types, T& system) noexcept -> System& {
        auto closures = System::Closures {
            .update = bind_front(&T::update, &system),
        };

        if constexpr (meta::HasPreUpdate<T>) closures.pre_update = bind_front(&T::pre_update, &system);
        if constexpr (meta::HasPostUpdate<T>) closures.post_update = bind_front(&T::post_update, &system);
        if constexpr (meta::HasOnMessageReceived<T>) closures.on_message_received = bind_front(&T::on_message_received, &system);

        return add_system(std::move(name), std::move(types), std::move(closures));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto EntityManager::add_system(string name, System::ComponentTypes types, System::Closures&& closures) noexcept
      -> System& {
        auto& system = m_systems.emplace_back(std::move(name), std::move(types), std::move(closures));

        get_needed_entities(system);

        return system;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto EntityManager::has_system(string_view name) const noexcept -> bool {
        return stdr::any_of(m_systems, [name](const auto& system) noexcept { return system.name() == name; });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto EntityManager::remove_system(string_view name) noexcept -> void {
        auto&& [begin, end] = stdr::remove_if(m_systems, [&name](const auto& system) { return name == system.name(); });
        m_systems.erase(begin, end);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<class Self>
    auto EntityManager::systems(this Self& self) noexcept -> dynarray<ref<cmeta::forward_const_to<Self, System>>> {
        constexpr auto as_refer = [] {
            if constexpr (cmeta::const_type<Self>) return monadic::as_ref();
            else
                return monadic::as_ref_mut();
        }();

        return self.m_systems | stdv::transform(as_refer) | stdr::to<dynarray<ref<cmeta::forward_const_to<Self, System>>>>();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<class Self>
    auto EntityManager::get_system(this Self& self, string_view name) noexcept -> cmeta::forward_const_to<Self, System&> {
        EXPECTS(self.has_system(name));

        const auto it = stdr::find_if(self.m_systems, [name](const auto& system) noexcept { return system.name() == name; });
        return std::forward_like<Self&>(*it->get());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto EntityManager::entity_count() const noexcept -> usize {
        return std::size(m_entities);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto EntityManager::add_raw_component(Entity        entity,
                                          ComponentType type,
                                          byte_view   component,
                                          DeleteFunc    delete_func) noexcept -> byte_view_mut {
        EXPECTS(has_entity(entity));
        EXPECTS(not has_component(entity, type));

        const auto _size = stdr::size(component);

        auto it = stdr::find_if(m_components, [type = type](const auto& pair) noexcept { return pair.type == type; });
        if (it == stdr::cend(m_components))
            it = m_components.emplace(stdr::cend(m_components),
                                      Store { type, stdr::size(component), {}, {}, std::move(delete_func) });

        ENSURES(it != stdr::cend(m_components));

        auto& [_, size, entities, components, _] = *it;
        ENSURES(size == _size);

        const auto old_size = stdr::size(components);
        components.resize(old_size + sizeof(Entity) + size);

        new (stdr::data(components) + old_size) Entity { entity };
        auto _component = array_view { stdr::data(components) + old_size + sizeof(Entity), _size };
        stdr::copy(component, stdr::begin(_component));

        entities.emplace_back(entity);

        m_updated_entities.emplace(entity);

        return _component;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<class Self>
    auto EntityManager::get_raw_component(this Self& self, Entity entity, ComponentType type) noexcept
      -> array_view<cmeta::forward_const_to<Self, byte>> {
        EXPECTS(self.has_entity(entity));
        EXPECTS(self.has_component(entity, type));

        auto it = stdr::find_if(self.m_components, [&type](const auto& pair) noexcept { return pair.type == type; });
        ENSURES(it != stdr::cend(self.m_components));

        auto& [_, size, _, components, _] = *it;

        auto component_it = stdr::data(components);
        for (;;) {
            auto e = *std::launder(std::bit_cast<Entity*>(component_it));
            if (e != entity) {
                component_it += sizeof(Entity) + size;
                continue;
            }

            component_it += sizeof(Entity);

            break;
        }

        return { component_it, size };
    }
} // namespace stormkit::entities
