// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.meta.tag_invoke;

import std;

import stormkit.core.meta.concepts;
import stormkit.core.meta.type_manipulation;

export namespace stormkit { inline namespace core { namespace meta {
    template<typename Customisation_point, typename... Ts>
    concept tag_invocable = requires(const Customisation_point& cpo, Ts&&... args) {
        tag_invoke(cpo, std::forward<Ts>(args)...);
    };

    template<typename Tag, typename... Ts>
    using tag_invoke_result = decltype(tag_invoke(std::declval<Tag>(), std::declval<Ts>()...));

    inline constexpr struct tag_invoke_fn final {
        template<typename Tag, typename... Ts>
        static constexpr auto operator()(Tag tag, Ts&&... args) noexcept -> tag_invoke_result<Tag, Ts...>;
    } tag_invoke_cpo;
}}} // namespace stormkit::core::meta

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core { namespace meta {
    namespace details {
        auto tag_invoke(auto&&...) = delete ("No default implementation for this CPO!");
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Tag, typename... Ts>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke_fn::operator()(Tag tag, Ts&&... args) noexcept -> tag_invoke_result<Tag, Ts...> {
        using details::tag_invoke;

        return tag_invoke(tag, std::forward<Ts>(args)...);
    }
}}} // namespace stormkit::core::meta
