// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
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
            read_only,
            read_write,
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
            using conditional_reference = meta::conditional<meta::const_qualified<Self>, const_reference, reference>;
            template<lock_access_mode Mode>
            using mode_conditional_reference = meta::conditional<Mode == lock_access_mode::read_only, const_reference, reference>;
            using pointer                    = ref_ptr<value_type>;
            using const_pointer              = ref_ptr<const value_type>;
            template<lock_access_mode Mode>
            using mode_conditional_pointer = meta::conditional<Mode == lock_access_mode::read_only, const_pointer, pointer>;
            using mutex_type               = Mutex;

            using copy_param_type = meta::in<stored_type>;
            using move_param_type = meta::take<stored_type>;

          private:
            template<template<class> typename Lock, lock_access_mode Mode>
            class accessor;

          public:
            template<template<class> typename Lock>
            using read_access = accessor<Lock, lock_access_mode::read_only>;
            template<template<class> typename Lock>
            using write_access = accessor<Lock, lock_access_mode::read_write>;

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

            template<lock_access_mode Mode, template<class> typename Lock, typename... LockTs, typename Self>
            auto access(this Self& self, LockTs&&... lock_args) noexcept -> accessor<Lock, Mode>;

            template<lock_access_mode                                 Mode,
                     std::invocable<mode_conditional_reference<Mode>> Closure,
                     template<class> typename Lock,
                     typename... LockTs,
                     typename Self>
            auto access(this Self& self, Closure&& closure, LockTs&&... lock_args) noexcept
              -> std::invoke_result_t<Closure, mode_conditional_reference<Mode>>;

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

            auto mutex() const noexcept -> mutex_type&;

          private:
            template<template<class> typename Lock, lock_access_mode Mode>
            class accessor {
              public:
                using pointer       = mode_conditional_pointer<Mode>;
                using const_pointer = mode_conditional_pointer<lock_access_mode::read_only>;
                template<typename U>
                using conditional_pointer = meta::conditional<meta::const_qualified<U>, const_pointer, pointer>;
                using reference           = mode_conditional_reference<Mode>;
                using const_reference     = mode_conditional_reference<lock_access_mode::read_only>;
                template<typename U>
                using conditional_reference = meta::conditional<meta::const_qualified<U>, const_reference, reference>;
                using locked_reference      = meta::conditional<Mode == lock_access_mode::read_only, const locked&, locked&>;

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
                pointer value_;
            };

            mutable mutex_type mutex_;
            T value_           STORMKIT_GUARDED_BY(mutex_);
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
        : value_ { value } {
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    STORMKIT_FORCE_INLINE
    inline locked<T, Mutex>::locked(move_param_type value) noexcept(meta::noexcept_movable<stored_type>)
        requires(meta::movable<stored_type>)
        : value_ { std::move(value) } {
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<typename... Ts>
    STORMKIT_FORCE_INLINE
    inline locked<T, Mutex>::locked(std::in_place_t, Ts&&... args) noexcept(meta::noexcept_constructible_from<stored_type, Ts...>)
        requires(meta::constructible_from<stored_type, Ts...>)
        : value_ { std::forward<Ts>(args)... } {
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

        value_ = std::move(other.value_);
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
    template<lock_access_mode Mode, template<class> typename Lock, typename... LockTs, typename Self>
    STORMKIT_FORCE_INLINE
    inline auto locked<T, Mutex>::access(this Self& self, LockTs&&... lock_args) noexcept -> accessor<Lock, Mode> {
        static_assert(not(Mode == lock_access_mode::read_write and meta::const_qualified<Self>),
                      "can't get read access on const locked<T>");

        if constexpr (meta::wrapped_value<stored_type> and meta::boolean_testable<stored_type>) expects(self.value_ != false);
        else if constexpr (meta::pointer<stored_type>)
            expects(self.value_ != nullptr);

        return accessor<Lock, Mode> { std::forward<Self&>(self), std::forward<LockTs>(lock_args)... };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<lock_access_mode                                                                     Mode,
             std::invocable<typename locked<T, Mutex>::template mode_conditional_reference<Mode>> Closure,
             template<class> typename Lock,
             typename... LockTs,
             typename Self>
    STORMKIT_FORCE_INLINE
    inline auto locked<T, Mutex>::access(this Self& self, Closure&& closure, LockTs&&... lock_args) noexcept
      -> std::invoke_result_t<Closure, mode_conditional_reference<Mode>> {
        auto access_ = self.template access<Mode, Lock>(std::forward<LockTs>(lock_args)...);
        return std::invoke(std::forward<Closure>(closure), *access_);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<template<class> typename Lock, typename... LockTs>
    STORMKIT_FORCE_INLINE
    inline auto locked<T, Mutex>::read(LockTs&&... lock_args) const noexcept -> read_access<Lock> {
        return access<lock_access_mode::read_only, Lock>(std::forward<LockTs>(lock_args)...);
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
        return access<lock_access_mode::read_only, Closure, Lock>(std::forward<Closure>(closure),
                                                                  std::forward<LockTs>(lock_args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<template<class> typename Lock, typename... LockTs>
    STORMKIT_FORCE_INLINE
    inline auto locked<T, Mutex>::write(LockTs&&... lock_args) noexcept -> write_access<Lock> {
        return access<lock_access_mode::read_write, Lock>(std::forward<LockTs>(lock_args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<std::invocable<typename locked<T, Mutex>::reference> Closure, template<class> typename Lock, typename... LockTs>
    STORMKIT_FORCE_INLINE
    inline auto locked<T, Mutex>::write(Closure&& closure, LockTs&&... lock_args) noexcept
      -> std::invoke_result_t<Closure, reference> {
        return access<lock_access_mode::read_write, Closure, Lock>(std::forward<Closure>(closure),
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
        auto value_ = write(std::forward<LockTs>(lock_args)...);
        *value_     = value;
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
        auto value_ = write(std::forward<LockTs>(lock_args)...);
        *value_     = value;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<typename Self>
    STORMKIT_FORCE_INLINE
    inline auto locked<T, Mutex>::unsafe(this Self& self) noexcept -> conditional_reference<Self> {
        return std::forward_like<Self&>(self.value_);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    STORMKIT_FORCE_INLINE
    inline auto locked<T, Mutex>::mutex() const noexcept -> mutex_type& {
        return mutex_;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<template<class> typename Lock, lock_access_mode Mode>
    template<typename... LockTs>
    STORMKIT_FORCE_INLINE
    inline locked<T, Mutex>::accessor<Lock, Mode>::accessor(reference value, mutex_type& mutex, LockTs&&... lock_args) noexcept
        : lock { mutex, std::forward<LockTs>(lock_args)... }, value_ { value } {
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<template<class> typename Lock, lock_access_mode Mode>
    template<typename... LockTs>
    STORMKIT_FORCE_INLINE
    inline locked<T, Mutex>::accessor<Lock, Mode>::accessor(locked_reference locked, LockTs&&... lock_args) noexcept
        requires(not meta::wrapped_value<T> and not meta::pointer<T>)
        : accessor { locked.value_, locked.mutex_, std::forward<LockTs>(lock_args)... } {
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<template<class> typename Lock, lock_access_mode Mode>
    template<typename... LockTs>
    STORMKIT_FORCE_INLINE
    inline locked<T, Mutex>::accessor<Lock, Mode>::accessor(locked_reference locked, LockTs&&... lock_args) noexcept
        requires(meta::wrapped_value<T>)
        : accessor { locked.value_.value(), locked.mutex_, std::forward<LockTs>(lock_args)... } {
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<template<class> typename Lock, lock_access_mode Mode>
    template<typename... LockTs>
    STORMKIT_FORCE_INLINE
    inline locked<T, Mutex>::accessor<Lock, Mode>::accessor(locked_reference locked, LockTs&&... lock_args) noexcept
        requires(meta::pointer<T>)
        : accessor { *(locked.value_), locked.mutex_, std::forward<LockTs>(lock_args)... } {
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<template<class> typename Lock, lock_access_mode Mode>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    inline auto locked<T, Mutex>::accessor<Lock, Mode>::operator->(this Self& self) noexcept -> conditional_pointer<Self> {
        return self.value_;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::is_decayed T, class Mutex>
        requires(meta::destructible<T>)
    template<template<class> typename Lock, lock_access_mode Mode>
    template<typename Self>
    STORMKIT_FORCE_INLINE
    inline auto locked<T, Mutex>::accessor<Lock, Mode>::operator*(this Self& self) noexcept -> conditional_reference<Self> {
        return *self.value_;
    }
}} // namespace stormkit::core
