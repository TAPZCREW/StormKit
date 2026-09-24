// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

export module stormkit.core.singleton;

import std;

import stormkit.core.types;

import stormkit.core.heap;
import stormkit.core.meta.type_query;
import stormkit.core.meta.concepts;

export namespace stormkit { inline namespace core {
    template<typename T>
    class singleton {
      public:
        template<typename... Ts>
        static auto instance(Ts&&... args) noexcept(meta::noexcept_constructible_from<T, Ts...>) -> T&;

        singleton(singleton&&)      = delete;
        singleton(const singleton&) = delete;

        auto operator=(singleton&&) -> singleton&      = delete;
        auto operator=(const singleton&) -> singleton& = delete;

      protected:
        singleton() noexcept;
        ~singleton() noexcept;

      private:
        static auto once_flag() noexcept -> std::once_flag&;

        static inline heap_ptr<T> m_instance = nullptr;
    };
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    singleton<T>::singleton() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    singleton<T>::~singleton() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    template<typename... Ts>
    auto singleton<T>::instance(Ts&&... args) noexcept(meta::noexcept_constructible_from<T, Ts...>) -> T& {
        const auto init = [](Ts&&... args) { m_instance = allocate_unsafe<T>(std::forward<Ts>(args)...); };

        std::call_once(once_flag(), init, std::forward<Ts>(args)...);

        return *m_instance;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    auto singleton<T>::once_flag() noexcept -> std::once_flag& {
        static auto once_flag = std::once_flag {};
        return once_flag;
    }
}} // namespace stormkit::core
