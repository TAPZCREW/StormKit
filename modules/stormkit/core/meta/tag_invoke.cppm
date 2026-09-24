// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.meta.tag_invoke;

import std;

import stormkit.core.meta.concepts;
import stormkit.core.meta.type_manipulation;

namespace stormkit { inline namespace core { namespace meta::details {
    auto tag_invoke() = delete ("No default implementation for this CPO!");
}}} // namespace stormkit::core::meta::details

export namespace stormkit { inline namespace core { namespace meta {
    struct tag_invoke_fn final {
        template<typename Tag, typename... Ts>
        static constexpr auto operator()(Tag tag,
                                         Ts&&... args) noexcept(noexcept(tag_invoke(std::move(tag), std::forward<Ts>(args)...)))
          -> decltype(tag_invoke(std::move(tag), std::forward<Ts>(args)...))
            requires requires(Tag tag, Ts&&... args) { tag_invoke(std::move(tag), std::forward<Ts>(args)...); }
        {
            using details::tag_invoke;

            return tag_invoke(std::move(tag), std::forward<Ts>(args)...);
        }
    };

    inline constexpr auto tag_invoke = tag_invoke_fn {};

    template<typename Tag, typename... Ts>
    concept tag_invocable = requires(Tag tag, Ts&&... args) {
        stormkit::core::meta::tag_invoke(std::move(tag), std::forward<Ts>(args)...);
    };

    template<typename Tag, typename... Ts>
    concept noexcept_tag_invocable = tag_invocable<Tag, Ts...> and requires(Tag tag, Ts&&... args) {
        { stormkit::core::meta::tag_invoke(std::move(tag), std::forward<Ts>(args)...) } noexcept;
    };

    template<typename Tag, typename... Ts>
    using tag_invoke_result = std::invoke_result_t<tag_invoke_fn, Tag, Ts...>;

    template<auto& cpo>
    using tag = meta::to_plain_type<decltype(cpo)>;
}}} // namespace stormkit::core::meta
