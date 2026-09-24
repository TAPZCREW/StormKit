// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/api.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.parallelism.locked;

import std;

import stormkit.core.meta;
import stormkit.core.types;
import stormkit.core.typesafe;

namespace stormkit { inline namespace core {
    export {
        enum class lock_access_mode : u8 {
            READ_ONLY,
            READ_WRITE,
        };
    }

    namespace details {
        using default_mutex = std::mutex;
        template<typename Mutex>
        using default_read_only_lock = std::unique_lock<Mutex>;
        template<typename Mutex>
        using default_read_write_lock = std::unique_lock<Mutex>;

        namespace meta {
            template<typename T>
            struct locked_value_type_impl {
                using type = T;
            };

            template<core::meta::wrapped_value T>
            struct locked_value_type_impl<T> {
                using type = core::meta::value_type<T>;
            };

            template<core::meta::pointer T>
            struct locked_value_type_impl<T> {
                using type = core::meta::pointed_type<T>;
            };

            template<typename T>
            using locked_value_type = locked_value_type_impl<T>::type;
        } // namespace meta
    } // namespace details

    export {
        template<meta::is_decayed T, class Mutex = details::default_mutex>
            requires(meta::destructible<T>)
        class STORMKIT_CORE_API locked {
          public:
            using stored_type     = T;
            using value_type      = details::meta::locked_value_type<stored_type>;
            using reference       = value_type&;
            using const_reference = const value_type&;
            template<typename Self>
            using conditional_reference = meta::conditional<meta::const_type<Self>, const_reference, reference>;
            template<lock_access_mode MODE>
            using mode_conditional_reference = meta::conditional<MODE == lock_access_mode::READ_ONLY, const_reference, reference>;
            using pointer                    = ref_ptr<value_type>;
            using const_pointer              = ref_ptr<const value_type>;
            template<lock_access_mode MODE>
            using mode_conditional_pointer = meta::conditional<MODE == lock_access_mode::READ_ONLY, const_pointer, pointer>;
            using mutex_type               = Mutex;

            using copy_param_type = meta::in<stored_type>;
            using move_param_type = meta::take<stored_type>;

          private:
            template<template<class> typename Lock, lock_access_mode MODE>
            class accessor;

          public:
            template<template<class> typename Lock>
            using read_access = accessor<Lock, lock_access_mode::READ_ONLY>;
            template<template<class> typename Lock>
            using write_access = accessor<Lock, lock_access_mode::READ_WRITE>;

            locked() noexcept(meta::noexcept_default_constructible<stored_type>);

            locked(copy_param_type value) noexcept(meta::noexcept_copyable<stored_type>)
                requires(meta::copyable<stored_type>);

            locked(move_param_type value) noexcept(meta::noexcept_movable<stored_type>)
                requires(meta::movable<stored_type>);

            template<typename... Ts>
            locked(std::in_place_t, Ts&&... args) noexcept(meta::noexcept_constructible_from<stored_type, Ts...>)
                requires(meta::constructible_from<stored_type, Ts...>);

            locked(const locked&)                    = delete;
            auto operator=(const locked&) -> locked& = delete;

            locked(locked&&) noexcept
                requires(meta::movable<stored_type>);
            auto operator=(locked&&) noexcept -> locked&
                requires(meta::move_assignable<stored_type>);

            ~locked() noexcept;

            template<lock_access_mode MODE, template<class> typename Lock, typename... LockTs, typename Self>
            auto access(this Self& self, LockTs&&... lock_args) noexcept -> accessor<Lock, MODE>;

            template<lock_access_mode                                 MODE,
                     std::invocable<mode_conditional_reference<MODE>> Closure,
                     template<class> typename Lock,
                     typename... LockTs,
                     typename Self>
            auto access(this Self& self, Closure&& closure, LockTs&&... lock_args) noexcept
              -> std::invoke_result_t<Closure, mode_conditional_reference<MODE>>;

            template<template<class> typename Lock = details::default_read_only_lock, typename... LockTs>
            auto read(LockTs&&... lock_args) const noexcept -> read_access<Lock>;

            template<std::invocable<const_reference> Closure,
                     template<class> typename Lock = details::default_read_only_lock,
                     typename... LockTs>
            auto read(Closure&& closure, LockTs&&... lock_args) const noexcept -> std::invoke_result_t<Closure, const_reference>;

            template<template<class> typename Lock = details::default_read_write_lock, typename... LockTs>
            auto write(LockTs&&... lock_args) noexcept -> write_access<Lock>;

            template<std::invocable<reference> Closure,
                     template<class> typename Lock = details::default_read_write_lock,
                     typename... LockTs>
            auto write(Closure&& closure, LockTs&&... lock_args) noexcept -> std::invoke_result_t<Closure, reference>;

            template<template<class> typename Lock = details::default_read_only_lock, typename... LockTs>
            auto copy(LockTs&&... lock_args) const noexcept -> value_type;

            template<template<class> typename Lock = details::default_read_write_lock, typename... LockTs>
            auto assign(copy_param_type value, LockTs&&... lock_args) noexcept(meta::noexcept_copy_assignable<value_type>) -> void
                requires(meta::copy_assignable<value_type>);

            template<template<class> typename Lock = details::default_read_write_lock, typename... LockTs>
            auto assign(move_param_type value, LockTs&&... lock_args) noexcept(meta::noexcept_move_assignable<value_type>) -> void
                requires(meta::move_assignable<value_type>);

            template<typename Self>
            auto unsafe(this Self& self) noexcept -> conditional_reference<Self>;

            auto mutex() const noexcept -> const mutex_type&;

          private:
            template<template<class> typename Lock, lock_access_mode MODE>
            class accessor {
              public:
                using pointer       = mode_conditional_pointer<MODE>;
                using const_pointer = mode_conditional_pointer<lock_access_mode::READ_ONLY>;
                template<typename U>
                using conditional_pointer = meta::conditional<meta::const_type<U>, const_pointer, pointer>;
                using reference           = mode_conditional_reference<MODE>;
                using const_reference     = mode_conditional_reference<lock_access_mode::READ_ONLY>;
                template<typename U>
                using conditional_reference = meta::conditional<meta::const_type<U>, const_reference, reference>;
                using locked_reference      = meta::conditional<MODE == lock_access_mode::READ_ONLY, const locked&, locked&>;

                template<typename... LockTs>
                accessor(reference value, mutex_type& mutex, LockTs&&... args) noexcept;

                template<typename... LockTs>
                explicit(sizeof...(LockTs) == 0) accessor(locked_reference locked, LockTs&&... args) noexcept
                    requires(not meta::wrapped_value<T> and not meta::pointer<T>);

                template<typename... LockTs>
                explicit(sizeof...(LockTs) == 0) accessor(locked_reference locked, LockTs&&... args) noexcept
                    requires(meta::wrapped_value<T>);

                template<typename... LockTs>
                explicit(sizeof...(LockTs) == 0) accessor(locked_reference locked, LockTs&&... args) noexcept
                    requires(meta::pointer<T>);

                template<typename Self>
                auto operator->(this Self& self) noexcept -> conditional_pointer<Self>;

                template<typename Self>
                auto operator*(this Self& self) noexcept -> conditional_reference<Self>;

                mutable Lock<mutex_type> lock;

              private:
                pointer m_value;
            };

            mutable mutex_type m_mutex;
            T m_value          STORMKIT_GUARDED_BY(m_mutex);
        };

        template<typename T>
        locked(T) -> locked<T>;
    }
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    STORMKIT_FORCE_INLINE
    inline locked<T, Mutex>::locked() noexcept(meta::noexcept_default_constructible<stored_type>) = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    STORMKIT_FORCE_INLINE
    inline locked<T, Mutex>::locked(copy_param_type value) noexcept(meta::noexcept_copyable<stored_type>)
        requires(meta::copyable<stored_type>)
        : m_value { value } {
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    STORMKIT_FORCE_INLINE
    inline locked<T, Mutex>::locked(move_param_type value) noexcept(meta::noexcept_movable<stored_type>)
        requires(meta::movable<stored_type>)
        : m_value { std::move(value) } {
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<typename... Ts>
    STORMKIT_FORCE_INLINE
    inline locked<T, Mutex>::locked(std::in_place_t, Ts&&... args) noexcept(meta::noexcept_constructible_from<stored_type, Ts...>)
        requires(meta::constructible_from<stored_type, Ts...>)
        : m_value { std::forward<Ts>(args)... } {
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    STORMKIT_FORCE_INLINE
    inline locked<T, Mutex>::locked(locked&& other) noexcept
        requires(meta::movable<stored_type>)
    {
        auto _ = other.write();

        m_value = std::move(other.m_value);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    STORMKIT_FORCE_INLINE
    inline auto locked<T, Mutex>::operator=(locked&& other) noexcept -> locked&
        requires(meta::move_assignable<stored_type>)
    {
        if (&other == this) [[unlikely]]
            return *this;

        auto from = other.write();
        auto to   = write();

        *to = std::move(*from);

        return *this;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    STORMKIT_FORCE_INLINE
    inline locked<T, Mutex>::~locked() noexcept = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<lock_access_mode MODE, template<class> typename Lock, typename... LockTs, typename Self>
    STORMKIT_FORCE_INLINE
    inline auto locked<T, Mutex>::access(this Self& self, LockTs&&... lock_args) noexcept -> accessor<Lock, MODE> {
        static_assert(not(MODE == lock_access_mode::READ_WRITE and meta::const_type<Self>),
                      "can't get read access on const locked<T>");

        if constexpr (meta::wrapped_value<stored_type> and meta::boolean_testable<stored_type>) expects(self.m_value != false);
        else if constexpr (meta::pointer<stored_type>)
            expects(self.m_value != nullptr);

        return accessor<Lock, MODE> { std::forward<Self&>(self), std::forward<LockTs>(lock_args)... };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<lock_access_mode                                                                     MODE,
             std::invocable<typename locked<T, Mutex>::template mode_conditional_reference<MODE>> Closure,
             template<class> typename Lock,
             typename... LockTs,
             typename Self>
    STORMKIT_FORCE_INLINE
    inline auto locked<T, Mutex>::access(this Self& self, Closure&& closure, LockTs&&... lock_args) noexcept
      -> std::invoke_result_t<Closure, mode_conditional_reference<MODE>> {
        auto access_ = self.template access<MODE, Lock>(std::forward<LockTs>(lock_args)...);
        return std::invoke(std::forward<Closure>(closure), *access_);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<template<class> typename Lock, typename... LockTs>
    STORMKIT_FORCE_INLINE
    inline auto locked<T, Mutex>::read(LockTs&&... lock_args) const noexcept -> read_access<Lock> {
        return access<lock_access_mode::READ_ONLY, Lock>(std::forward<LockTs>(lock_args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<std::invocable<typename locked<T, Mutex>::const_reference> Closure,
             template<class> typename Lock,
             typename... LockTs>
    STORMKIT_FORCE_INLINE
    inline auto locked<T, Mutex>::read(Closure&& closure, LockTs&&... lock_args) const noexcept
      -> std::invoke_result_t<Closure, const_reference> {
        return access<lock_access_mode::READ_ONLY, Closure, Lock>(std::forward<Closure>(closure),
                                                                  std::forward<LockTs>(lock_args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<template<class> typename Lock, typename... LockTs>
    STORMKIT_FORCE_INLINE
    inline auto locked<T, Mutex>::write(LockTs&&... lock_args) noexcept -> write_access<Lock> {
        return access<lock_access_mode::READ_WRITE, Lock>(std::forward<LockTs>(lock_args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<std::invocable<typename locked<T, Mutex>::reference> Closure, template<class> typename Lock, typename... LockTs>
    STORMKIT_FORCE_INLINE
    inline auto locked<T, Mutex>::write(Closure&& closure, LockTs&&... lock_args) noexcept
      -> std::invoke_result_t<Closure, reference> {
        return access<lock_access_mode::READ_WRITE, Closure, Lock>(std::forward<Closure>(closure),
                                                                   std::forward<LockTs>(lock_args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<template<class> typename Lock, typename... LockTs>
    STORMKIT_FORCE_INLINE
    inline auto locked<T, Mutex>::copy(LockTs&&... lock_args) const noexcept -> value_type {
        return auto(*read(std::forward<LockTs>(lock_args)...));
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<template<class> typename Lock, typename... LockTs>
    STORMKIT_FORCE_INLINE
    inline auto locked<T, Mutex>::assign(copy_param_type value,
                                         LockTs&&... lock_args) noexcept(meta::noexcept_copy_assignable<value_type>) -> void
        requires(meta::copy_assignable<value_type>)
    {
        *write(std::forward<LockTs>(lock_args)...) = value;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<template<class> typename Lock, typename... LockTs>
    STORMKIT_FORCE_INLINE
    inline auto locked<T, Mutex>::assign(move_param_type value,
                                         LockTs&&... lock_args) noexcept(meta::noexcept_move_assignable<value_type>) -> void
        requires(meta::move_assignable<value_type>)
    {
        *write(std::forward<LockTs>(lock_args)...) = std::move(value);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<typename Self>
    STORMKIT_FORCE_INLINE
    inline auto locked<T, Mutex>::unsafe(this Self& self) noexcept -> conditional_reference<Self> {
        return std::forward_like<Self&>(self.m_value);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    STORMKIT_FORCE_INLINE
    inline auto locked<T, Mutex>::mutex() const noexcept -> const mutex_type& {
        return m_mutex;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<template<class> typename Lock, lock_access_mode MODE>
    template<typename... LockTs>
    STORMKIT_FORCE_INLINE
    inline locked<T, Mutex>::accessor<Lock, MODE>::accessor(reference value, mutex_type& mutex, LockTs&&... lock_args) noexcept
        : lock { mutex, std::forward<LockTs>(lock_args)... }, m_value { value } {
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<template<class> typename Lock, lock_access_mode MODE>
    template<typename... LockTs>
    STORMKIT_FORCE_INLINE
    inline locked<T, Mutex>::accessor<Lock, MODE>::accessor(locked_reference locked, LockTs&&... lock_args) noexcept
        requires(not meta::wrapped_value<T> and not meta::pointer<T>)
        : accessor { locked.m_value, locked.m_mutex, std::forward<LockTs>(lock_args)... } {
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<template<class> typename Lock, lock_access_mode MODE>
    template<typename... LockTs>
    STORMKIT_FORCE_INLINE
    inline locked<T, Mutex>::accessor<Lock, MODE>::accessor(locked_reference locked, LockTs&&... lock_args) noexcept
        requires(meta::wrapped_value<T>)
        : accessor { locked.m_value.value(), locked.m_mutex, std::forward<LockTs>(lock_args)... } {
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<template<class> typename Lock, lock_access_mode MODE>
    template<typename... LockTs>
    STORMKIT_FORCE_INLINE
    inline locked<T, Mutex>::accessor<Lock, MODE>::accessor(locked_reference locked, LockTs&&... lock_args) noexcept
        requires(meta::pointer<T>)
        : accessor { *(locked.m_value), locked.m_mutex, std::forward<LockTs>(lock_args)... } {
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<template<class> typename Lock, lock_access_mode MODE>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    inline auto locked<T, Mutex>::accessor<Lock, MODE>::operator->(this Self& self) noexcept -> conditional_pointer<Self> {
        return self.m_value;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<template<class> typename Lock, lock_access_mode MODE>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    inline auto locked<T, Mutex>::accessor<Lock, MODE>::operator*(this Self& self) noexcept -> conditional_reference<Self> {
        return *self.m_value;
    }
}} // namespace stormkit::core
