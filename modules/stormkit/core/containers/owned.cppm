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
    } and details::valid_constructor<decltype(Trait::constructor), typename Trait::element_type> and details::valid_deleter<decltype(Trait::deleter), typename Trait::element_type>;

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

    template<typename T, auto Constructor, auto Destructor, T Null_handle>
    struct owned_trait {
        using element_type = T;

        static constexpr auto null_handle = Null_handle;

        static constexpr auto constructor = Constructor;
        static constexpr auto deleter     = Destructor;
    };

    template<typename Trait>
    class STORMKIT_OWNER STORMKIT_TRIVIALLY_RELOCATABLE owned {
        static_assert(meta::is_owned_trait<Trait>, "Trait should follow is_owned_trait concept for its interface!");
        using owned_type = owned<Trait>;

      public:
        using trait_type       = Trait;
        using element_type     = trait_type::element_type;
        using constructor_type = decltype(trait_type::constructor);
        using deleter_type     = decltype(trait_type::deleter);

        static constexpr auto null_handle = meta::details::null_handle_selector<trait_type>::value;

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

        constexpr auto handle() const noexcept STORMKIT_LIFETIMEBOUND -> element_type;
        constexpr auto release() noexcept -> element_type;
        constexpr auto reset(element_type handle = null_handle) noexcept -> void;

        constexpr          operator element_type() const noexcept STORMKIT_LIFETIMEBOUND;
        constexpr explicit operator bool() const noexcept;

        [[nodiscard]]
        constexpr auto operator==(element_type) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto operator<(element_type) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto operator<=(element_type) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto operator>(element_type) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto operator>=(element_type) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto operator<=>(element_type) const noexcept -> std::compare_three_way_result_t<element_type, element_type>
            requires std::three_way_comparable<element_type, element_type>;

        [[nodiscard]]
        constexpr auto operator==(const owned<trait_type>&) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto operator<(const owned<trait_type>&) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto operator<=(const owned<trait_type>&) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto operator>(const owned<trait_type>&) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto operator>=(const owned<trait_type>&) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto operator<=>(const owned<trait_type>&) const noexcept
          -> std::compare_three_way_result_t<element_type, element_type>
            requires std::three_way_comparable<element_type, element_type>;

      private:
        constexpr owned() noexcept;
        constexpr owned(element_type handle) noexcept;

        constexpr auto destroy() noexcept -> void;

        element_type handle_ = null_handle;
    };

    template<typename Trait>
    class STORMKIT_VIEW STORMKIT_TRIVIALLY_RELOCATABLE view {
        static_assert(meta::is_owned_trait<Trait>, "Trait should follow is_owned_trait concept for its interface!");

      public:
        using trait_type   = Trait;
        using element_type = trait_type::element_type;

        static constexpr auto null_handle = meta::details::null_handle_selector<trait_type>::value;

        constexpr view(const owned<trait_type>& owned_handle STORMKIT_LIFETIMEBOUND) noexcept;
        constexpr view(element_type handle) noexcept;
        constexpr ~view() noexcept;

        constexpr view(const view&) noexcept;
        constexpr auto operator=(const view&) noexcept -> view&;

        constexpr view(view&&) noexcept;
        constexpr auto operator=(view&&) noexcept -> view&;

        constexpr auto handle() const noexcept STORMKIT_LIFETIMEBOUND -> element_type;

        constexpr          operator element_type() const noexcept STORMKIT_LIFETIMEBOUND;
        constexpr explicit operator bool() const noexcept;

        [[nodiscard]]
        constexpr auto operator==(element_type) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto operator<(element_type) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto operator<=(element_type) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto operator>(element_type) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto operator>=(element_type) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto operator<=>(element_type) const noexcept -> std::compare_three_way_result_t<element_type, element_type>
            requires std::three_way_comparable<element_type, element_type>;

        [[nodiscard]]
        constexpr auto operator==(const owned<trait_type>&) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto operator<(const owned<trait_type>&) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto operator<=(const owned<trait_type>&) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto operator>(const owned<trait_type>&) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto operator>=(const owned<trait_type>&) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto operator<=>(const owned<trait_type>&) const noexcept
          -> std::compare_three_way_result_t<element_type, element_type>
            requires std::three_way_comparable<element_type, element_type>;

        [[nodiscard]]
        constexpr auto operator==(view<trait_type>) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto operator<(view<trait_type>) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto operator<=(view<trait_type>) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto operator>(view<trait_type>) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto operator>=(view<trait_type>) const noexcept -> bool;
        [[nodiscard]]
        constexpr auto operator<=>(view<trait_type>) const noexcept -> std::compare_three_way_result_t<element_type, element_type>
            requires std::three_way_comparable<element_type, element_type>;

      private:
        element_type handle_;
    };
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////
namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr owned<Trait>::owned() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr owned<Trait>::owned(element_type handle) noexcept
        : handle_ { handle } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::destroy() noexcept -> void {
        if (handle_ != null_handle) trait_type::deleter(handle_);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr owned<Trait>::~owned() noexcept {
        destroy();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr owned<Trait>::owned(owned&& other) noexcept
        : handle_ { std::exchange(other.handle_, null_handle) } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::operator=(owned&& other) noexcept -> owned& {
        if (&other == this) [[unlikely]]
            return *this;

        handle_ = std::exchange(other.handle_, null_handle);

        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
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
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::take(element_type&& value) noexcept -> owned_type {
        return owned_type { value };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::empty() noexcept -> owned_type {
        return owned_type {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::handle() const noexcept -> element_type {
        return handle_;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::release() noexcept -> element_type {
        const auto handle = handle_;
        handle_           = null_handle;
        return handle;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::reset(element_type handle) noexcept -> void {
        handle_ = handle;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr owned<Trait>::operator element_type() const noexcept {
        return handle_;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr owned<Trait>::operator bool() const noexcept {
        return handle_ != null_handle;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::operator==(element_type other) const noexcept -> bool {
        return handle_ == other;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::operator<(element_type other) const noexcept -> bool {
        return std::less<element_type> {}(handle_, other);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::operator<=(element_type other) const noexcept -> bool {
        return !(other < *this);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::operator>(element_type other) const noexcept -> bool {
        return other < *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::operator>=(element_type other) const noexcept -> bool {
        return !(*this < other);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::operator<=>(element_type other) const noexcept
      -> std::compare_three_way_result_t<element_type, element_type>
        requires std::three_way_comparable<element_type, element_type>
    {
        return std::compare_three_way {}(handle_, other);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::operator==(const owned<trait_type>& other) const noexcept -> bool {
        return operator==(other.handle());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::operator<(const owned<trait_type>& other) const noexcept -> bool {
        return operator<(other.handle());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::operator<=(const owned<trait_type>& other) const noexcept -> bool {
        return operator<=(other.handle());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::operator>(const owned<trait_type>& other) const noexcept -> bool {
        return operator>(other.handle());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::operator>=(const owned<trait_type>& other) const noexcept -> bool {
        return operator>=(other.handle());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto owned<Trait>::operator<=>(const owned<trait_type>& other) const noexcept
      -> std::compare_three_way_result_t<element_type, element_type>
        requires std::three_way_comparable<element_type, element_type>
    {
        return operator<=>(other.handle());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr view<Trait>::view(const owned<trait_type>& owned_handle) noexcept
        : handle_ { owned_handle.handle() } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr view<Trait>::view(element_type handle) noexcept
        : handle_ { handle } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr view<Trait>::~view() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr view<Trait>::view(const view&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto view<Trait>::operator=(const view&) noexcept -> view& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr view<Trait>::view(view&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto view<Trait>::operator=(view&&) noexcept -> view& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto view<Trait>::handle() const noexcept -> element_type {
        return handle_;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr view<Trait>::operator element_type() const noexcept {
        return handle_;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr view<Trait>::operator bool() const noexcept {
        return handle_ != null_handle;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto view<Trait>::operator==(element_type other) const noexcept -> bool {
        return handle_ == other;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto view<Trait>::operator<(element_type other) const noexcept -> bool {
        return std::less<element_type> {}(handle_, other);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto view<Trait>::operator<=(element_type other) const noexcept -> bool {
        return !(other < *this);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto view<Trait>::operator>(element_type other) const noexcept -> bool {
        return other < *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto view<Trait>::operator>=(element_type other) const noexcept -> bool {
        return !(*this < other);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto view<Trait>::operator<=>(element_type other) const noexcept
      -> std::compare_three_way_result_t<element_type, element_type>
        requires std::three_way_comparable<element_type, element_type>
    {
        return std::compare_three_way {}(handle_, other);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto view<Trait>::operator==(const owned<trait_type>& other) const noexcept -> bool {
        return operator==(other.handle());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto view<Trait>::operator<(const owned<trait_type>& other) const noexcept -> bool {
        return operator<(other.handle());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto view<Trait>::operator<=(const owned<trait_type>& other) const noexcept -> bool {
        return operator<=(other.handle());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto view<Trait>::operator>(const owned<trait_type>& other) const noexcept -> bool {
        return operator>(other.handle());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto view<Trait>::operator>=(const owned<trait_type>& other) const noexcept -> bool {
        return operator>=(other.handle());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto view<Trait>::operator<=>(const owned<trait_type>& other) const noexcept
      -> std::compare_three_way_result_t<element_type, element_type>
        requires std::three_way_comparable<element_type, element_type>
    {
        return operator<=>(other.handle());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto view<Trait>::operator==(view<trait_type> other) const noexcept -> bool {
        return operator==(other.handle());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto view<Trait>::operator<(view<trait_type> other) const noexcept -> bool {
        return operator<(other.handle());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto view<Trait>::operator<=(view<trait_type> other) const noexcept -> bool {
        return operator<=(other.handle());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto view<Trait>::operator>(view<trait_type> other) const noexcept -> bool {
        return operator>(other.handle());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto view<Trait>::operator>=(view<trait_type> other) const noexcept -> bool {
        return operator>=(other.handle());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Trait>
    STORMKIT_FORCE_INLINE
    constexpr auto view<Trait>::operator<=>(view<trait_type> other) const noexcept
      -> std::compare_three_way_result_t<element_type, element_type>
        requires std::three_way_comparable<element_type, element_type>
    {
        return operator<=>(other.handle());
    }
}} // namespace stormkit::core
