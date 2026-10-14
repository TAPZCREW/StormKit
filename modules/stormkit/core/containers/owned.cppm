// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.containers.owned;

import std;

import stormkit.core.contract;
import stormkit.core.meta;
import stormkit.core.functional;

namespace stormkit { inline namespace core { namespace meta::details {
    template<typename Func, typename Handle_type>
    concept plain_constructor = is<return_type<Func>, Handle_type>;

    template<typename Func, typename Handle_type>
    concept expected_constructor = specialization_of<return_type<Func>, std::expected>
                                   and is<value_type<return_type<Func>>, Handle_type>;

    template<typename Func, typename Handle_type>
    concept valid_constructor = plain_constructor<Func, Handle_type> or expected_constructor<Func, Handle_type>;

    template<typename Func, typename Handle_type>
    concept valid_deleter = std::invocable<Func, Handle_type>;

    template<typename Trait>
    concept is_owned_trait = requires(const Trait& trait) {
        typename Trait::element_type;
        trait.constructor;
        trait.deleter;
    };
    // } and details::valid_constructor(decltype(Trait::constructor), typename Trait::element_type) and
    // details::valid_deleter(decltype(Trait::deleter), typename Trait::element_type);

    template<typename Trait>
    concept trait_has_null_handle = is_owned_trait<Trait> and requires(const Trait& trait) {
        { trait.null_handle } -> meta::is<typename Trait::element_type>;
    };

    template<is_owned_trait Trait>
    struct null_handle_selector {
        static constexpr auto value = typename Trait::element_type { nullptr };
    };

    template<is_owned_trait Trait>
        requires(trait_has_null_handle<Trait>)
    struct null_handle_selector<Trait> {
        static constexpr auto value = Trait::null_handle;
    };
}}} // namespace stormkit::core::meta::details

export namespace stormkit { inline namespace core {
    template<typename T>
    struct default_tag;

    namespace meta {
        template<typename Trait>
        concept is_owned_trait = details::is_owned_trait<Trait>;
    } // namespace meta

    template<typename Trait>
        requires(meta::is_owned_trait<Trait>)
    class owned {
        using owned_type = owned<Trait>;

      public:
        using trait_type       = Trait;
        using element_type     = trait_type::element_type;
        using constructor_type = decltype(trait_type::constructor);
        using deleter_type     = decltype(trait_type::deleter);

        static constexpr auto null_handle = meta::details::null_handle_selector<Trait>::value;

        constexpr ~owned() noexcept;

        constexpr owned(owned&)                    = delete;
        constexpr auto operator=(owned&) -> owned& = delete;

        constexpr owned(owned&&) noexcept;
        constexpr auto operator=(owned&&) noexcept -> owned&;

        template<typename... Ts>
        static constexpr auto create(Ts&&... args) noexcept -> owned_type
            requires(meta::details::plain_constructor<constructor_type, element_type> and std::invocable<constructor_type, Ts...>)
        ;

        template<typename... Ts>
        static constexpr auto create(Ts&&... args) noexcept -> decltype(auto)
            requires(meta::details::expected_constructor<constructor_type, element_type>
                     and std::invocable<constructor_type, Ts...>);

        static constexpr auto take(element_type&& value) noexcept -> owned_type;
        static constexpr auto empty() noexcept -> owned_type;

        constexpr auto managed() const noexcept -> element_type;
        constexpr auto release() noexcept -> element_type;
        constexpr auto reset(element_type handle = null_handle) noexcept -> void;

      private:
        constexpr owned() noexcept;
        constexpr owned(element_type handle) noexcept;

        constexpr auto destroy() noexcept -> void;

        element_type managed_ = null_handle;
    };

    template<typename T>
    class view {};
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////
namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
        requires(meta::is_owned_trait<Trait>)
    STORMKIT_FORCE_INLINE constexpr owned<Trait>::owned() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
        requires(meta::is_owned_trait<Trait>)
    STORMKIT_FORCE_INLINE
    constexpr owned<Trait>::owned(element_type handle) noexcept
        : managed_ { handle } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
        requires(meta::is_owned_trait<Trait>)
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::destroy() noexcept -> void {
        if (managed_ != null_handle) trait_type::deleter(managed_);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
        requires(meta::is_owned_trait<Trait>)
    STORMKIT_FORCE_INLINE constexpr owned<Trait>::~owned() noexcept {
        destroy();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
        requires(meta::is_owned_trait<Trait>)
    STORMKIT_FORCE_INLINE
    constexpr owned<Trait>::owned(owned&& other) noexcept
        : managed_ { std::exchange(other.managed_, null_handle) } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
        requires(meta::is_owned_trait<Trait>)
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::operator=(owned&& other) noexcept -> owned& {
        if (&other == this) [[unlikely]]
            return *this;

        managed_ = std::exchange(other.managed_, null_handle);

        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
        requires(meta::is_owned_trait<Trait>)
    template<typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::create(Ts&&... args) noexcept -> owned_type
        requires(meta::details::plain_constructor<constructor_type, element_type> and std::invocable<constructor_type, Ts...>)
    {
        return owned { trait_type::constructor(std::forward<Ts>(args)...) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
        requires(meta::is_owned_trait<Trait>)
    template<typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::create(Ts&&... args) noexcept -> decltype(auto)
      // -> meta::replace_first<owned_type, meta::return_type<constructor_type>>
        requires(meta::details::expected_constructor<constructor_type, element_type> and std::invocable<constructor_type, Ts...>)
    {
        using out_type = meta::replace_first<owned_type, meta::return_type<constructor_type>>;
        auto out       = out_type { std::in_place };

        auto result = trait_type::constructor(std::forward<Ts>(args)...);
        if (not result) out = std::unexpected { constructor_type::move(result.error()) };
        else
            out->reset(std::move(result.value()));

        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
        requires(meta::is_owned_trait<Trait>)
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::take(element_type&& value) noexcept -> owned_type {
        return owned_type { value };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
        requires(meta::is_owned_trait<Trait>)
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::empty() noexcept -> owned_type {
        return owned_type {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
        requires(meta::is_owned_trait<Trait>)
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::managed() const noexcept -> element_type {
        return managed_;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
        requires(meta::is_owned_trait<Trait>)
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::release() noexcept -> element_type {
        const auto managed = managed_;
        managed_           = null_handle;
        return managed;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
        requires(meta::is_owned_trait<Trait>)
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::reset(element_type handle) noexcept -> void {
        managed_ = handle;
    }
}} // namespace stormkit::core
