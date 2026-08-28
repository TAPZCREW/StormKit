// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.string.format;

import std;

import stormkit.core.types;

import stormkit.core.meta.tag_invoke;
import stormkit.core.meta.concepts;
import stormkit.core.meta.algorithms;
import stormkit.core.meta.type_manipulation;
import stormkit.core.meta.type_query;
import stormkit.core.typesafe.safecasts;
import stormkit.core.typesafe.ref_ptr;
import stormkit.core.errors;
import stormkit.core.string.safecasts;
import stormkit.core.string.static_string;

namespace stdr = std::ranges;

export {
    namespace stormkit { inline namespace core {
        namespace meta {
            template<meta::enumeration T>
            inline constexpr auto DISABLE_DEFAULT_FORMATTER_FOR_ENUM = false;

            template<typename T>
            inline constexpr auto ENABLE_AS_STRING_AS_FORMATTER = false;

            template<typename T>
            concept as_string_formattable = ENABLE_AS_STRING_AS_FORMATTER<T> and has_as_string<T>;
        } // namespace meta

        template<typename CharT>
        struct format_as_fn {
          private:
            template<typename T, typename FormatContext>
            static constexpr auto IS_TAG_INVOKABLE = meta::tag_invocable<format_as_fn<CharT>, const T&, FormatContext&>;

          public:
            template<typename T, typename FormatContext>
            [[nodiscard]]
            static constexpr auto operator()(const T& value, FormatContext& ctx) noexcept -> decltype(ctx.out());
        };

        namespace meta {
            template<typename T, typename CharT>
            concept has_format_as = requires(const T& value) {
                {
                    format_as_fn<CharT> {}(value, std::declval<std::format_context&>())
                } -> meta::is<decltype(std::declval<std::format_context&>().out())>;
            };

            namespace plain {
                template<typename T, typename CharT>
                concept has_format_as = apply_to<T, meta::has_format_as, CharT>;
            }
        } // namespace meta

        template<typename CharT, typename FormatContext>
        constexpr auto tag_invoke(format_as_fn<CharT>, byte value, FormatContext& ctx) -> decltype(ctx.out());

        template<typename CharT, typename FormatContext, meta::negate<meta::raw_indirection> T>
        constexpr auto tag_invoke(format_as_fn<CharT>, ref_ptr<T> value, FormatContext& ctx) -> decltype(ctx.out());

        template<typename CharT, typename FormatContext>
        constexpr auto tag_invoke(format_as_fn<CharT>, meta::in<system_code> value, FormatContext& ctx) -> decltype(ctx.out());
    }} // namespace stormkit::core

    template<typename T, typename CharT>
        requires(stormkit::core::meta::plain::has_format_as<T, CharT>)
    struct std::formatter<T, CharT>: std::formatter<basic_string_view<CharT>, CharT> {
        using std::formatter<basic_string_view<CharT>, CharT>::parse;
        // [[nodiscard]]
        // constexpr auto parse(auto& ctx) noexcept -> decltype(ctx.begin());

        [[nodiscard]]
        constexpr auto format(const T&, auto& ctx) const noexcept -> decltype(ctx.out());
    };

    template<stormkit::core::meta::as_string_formattable T>
    constexpr auto std::enable_nonlocking_formatter_optimization<T> = true;
}

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename CharT>
    template<typename T, typename FormatContext>
    STORMKIT_FORCE_INLINE
    constexpr auto format_as_fn<CharT>::operator()(const T& value, FormatContext& ctx) noexcept -> decltype(ctx.out()) {
        if constexpr (IS_TAG_INVOKABLE<T, FormatContext>) return tag_invoke(format_as_fn<CharT> {}, value, ctx);
        else {
            auto out = ctx.out();
            stdr::copy("(No formatter defined for this type!)", out);
            ctx.advance_to(out);
            return ctx.out();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename CharT, typename FormatContext>
    constexpr auto tag_invoke(format_as_fn<CharT>, byte value, FormatContext& ctx) -> decltype(ctx.out()) {
        return std::format_to(ctx.out(), "{:0x}", value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename CharT, typename FormatContext, meta::negate<meta::raw_indirection> T>
    constexpr auto tag_invoke(format_as_fn<CharT>, ref_ptr<T> value, FormatContext& ctx) -> decltype(ctx.out()) {
        return std::format_to(ctx.out(), "{:0x}", value.get());
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename CharT, typename FormatContext>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(format_as_fn<CharT>, meta::in<system_code> error, FormatContext& ctx) -> decltype(ctx.out()) {
        return std::format_to(ctx.out(),
                              "{:#x} ({})",
                              static_cast<unsigned long long>(error.value()),
                              std::string_view { error.message() });
    }
}} // namespace stormkit::core

/////////////////////////////////////
/////////////////////////////////////
// template<typename T, typename CharT>
//     requires(stormkit::core::meta::plain::has_format_as<T, CharT>)
// constexpr auto std::formatter<T, CharT>::parse(auto& ctx) noexcept -> decltype(ctx.begin()) {
//     auto it = ctx.begin();
//     // auto end = ctx.end();

//    // if (it == end) return it;

//    // if (*it == '<') { ++it; }

//    return it;
// }

/////////////////////////////////////
/////////////////////////////////////
template<typename T, typename CharT>
    requires(stormkit::core::meta::plain::has_format_as<T, CharT>)
constexpr auto std::formatter<T, CharT>::format(const T& value, auto& ctx) const noexcept -> decltype(ctx.out()) {
    if constexpr (stormkit::core::meta::as_string_formattable<T>)
        return std::formatter<basic_string_view<CharT>, CharT>::format(stormkit::core::as<basic_string<CharT>>(value), ctx);
    else
        return stormkit::core::format_as_fn<CharT> {}(value, ctx);
}
