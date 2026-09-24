// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.string.format;

import std;

import stormkit.core.types;

import stormkit.core.heap;
import stormkit.core.meta.tag_invoke;
import stormkit.core.meta.concepts;
import stormkit.core.meta.algorithms;
import stormkit.core.meta.type_manipulation;
import stormkit.core.meta.type_query;
import stormkit.core.typesafe.safecasts;
import stormkit.core.typesafe.ref_ptr;
import stormkit.core.errors;
import stormkit.core.string.static_string;

namespace stdr = std::ranges;

export namespace stormkit { inline namespace core {
    namespace meta {
        template<typename T>
        inline constexpr auto DISABLE_AS_STRING_VIEW_FORMATTER = false;

        template<typename T>
        concept format_as_string_view_enabled = has_as_string_view<T> and not DISABLE_AS_STRING_VIEW_FORMATTER<T>;

        template<typename T>
        concept format_context = requires(meta::remove_const_of<T>& ctx) {
            typename T::char_type;
            ctx.arg(0);
            { ctx.locale() } -> is<std::locale>;
            { ctx.out() } -> std::output_iterator<const typename T::char_type&>;
            ctx.advance_to(ctx.out());
        };

        static_assert(format_context<std::format_context>);
    } // namespace meta

    template<typename CharT>
    struct format_as_fn {
        template<typename T, meta::format_context FormatContext>
        static constexpr auto TAG_INVOKABLE = meta::tag_invocable<format_as_fn<CharT>, const T&, FormatContext&>;

        template<typename T, meta::format_context FormatContext>
        [[nodiscard]]
        static constexpr auto operator()(const T& value, FormatContext& ctx) noexcept -> decltype(ctx.out())
            requires(TAG_INVOKABLE<T, FormatContext>);
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

    template<typename CharT>
    constexpr auto tag_invoke(format_as_fn<CharT>, byte value, meta::format_context auto& ctx) -> decltype(ctx.out());

    template<typename CharT, meta::negate<meta::raw_indirection> T>
    constexpr auto tag_invoke(format_as_fn<CharT>, ref_ptr<T> value, meta::format_context auto& ctx) -> decltype(ctx.out());

    template<typename CharT>
    constexpr auto tag_invoke(format_as_fn<CharT>, meta::in<system_code> value, meta::format_context auto& ctx)
      -> decltype(ctx.out());

    template<typename CharT, typename T>
        requires(std::formattable<T, CharT> and not meta::has_as_string_view<T>)
    constexpr auto tag_invoke(as_fn<basic_string<CharT>>, const T& value, source_location_arg) -> basic_string<CharT>;

    template<typename CharT>
    constexpr auto tag_invoke(format_as_fn<CharT>, allocation_error value, meta::format_context auto& ctx) -> decltype(ctx.out());

    template<typename... Args>
    constexpr auto format_to(meta::format_context auto& ctx, std::format_string<Args...> fmt, Args&&... args) -> void;
}} // namespace stormkit::core

export {
    template<typename T, typename CharT>
        requires(stormkit::core::meta::has_format_as<T, CharT> or stormkit::core::meta::format_as_string_view_enabled<T>)
    struct std::formatter<T, CharT>: formatter<basic_string_view<CharT>, CharT> {
        using formatter<basic_string_view<CharT>, CharT>::parse;

        [[nodiscard]]
        constexpr auto format(const T& value, stormkit::meta::format_context auto& ctx) const noexcept -> decltype(ctx.out());
    };

    template<stormkit::core::meta::format_as_string_view_enabled T>
    constexpr auto std::enable_nonlocking_formatter_optimization<T> = true;
}

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename CharT>
    template<typename T, meta::format_context FormatContext>
    STORMKIT_FORCE_INLINE
    constexpr auto format_as_fn<CharT>::operator()(const T& value, FormatContext& ctx) noexcept -> decltype(ctx.out())
        requires(TAG_INVOKABLE<T, FormatContext>)
    {
        return tag_invoke(format_as_fn<CharT> {}, value, ctx);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(format_as_fn<CharT>, byte value, meta::format_context auto& ctx) -> decltype(ctx.out()) {
        return std::format_to(ctx.out(), "{:0x}", as<u16>(value));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename CharT, meta::negate<meta::raw_indirection> T>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(format_as_fn<CharT>, ref_ptr<T> value, meta::format_context auto& ctx) -> decltype(ctx.out()) {
        return std::format_to(ctx.out(), "{:0x}", value.get());
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename CharT>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(format_as_fn<CharT>, meta::in<system_code> error, meta::format_context auto& ctx)
      -> decltype(ctx.out()) {
        return std::format_to(ctx.out(),
                              "{:#x} ({})",
                              static_cast<unsigned long long>(error.value()),
                              std::string_view { error.message() });
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename CharT, typename T>
        requires(std::formattable<T, CharT> and not meta::has_as_string_view<T>)
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(as_fn<basic_string<CharT>>, const T& value, source_location_arg) -> basic_string<CharT> {
        return std::format("{}", value);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename CharT>
    constexpr auto tag_invoke(format_as_fn<CharT>, allocation_error value, meta::format_context auto& ctx)
      -> decltype(ctx.out()) {
        return std::format_to(ctx.out(), "Failed to allocate {}, reason: {}", value.size, value.type);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename... Args>
    constexpr auto format_to(meta::format_context auto& ctx, std::format_string<Args...> fmt, Args&&... args) -> void {
        ctx.advance_to(std::format_to(ctx.out(), std::move(fmt), std::forward<Args>(args)...));
    }

}} // namespace stormkit::core

///////////////////////////////////
///////////////////////////////////
template<typename T, typename CharT>
    requires(stormkit::core::meta::has_format_as<T, CharT> or stormkit::core::meta::format_as_string_view_enabled<T>)
 STORMKIT_FORCE_INLINE
constexpr auto std::formatter<T, CharT>::format(const T& value, stormkit::meta::format_context auto& ctx) const noexcept
  -> decltype(ctx.out()) {
    if constexpr (stormkit::core::meta::format_as_string_view_enabled<T>)
        return formatter<basic_string_view<CharT>>::format(stormkit::core::as<string_view>(value), ctx);
    else
        return stormkit::core::format_as_fn<CharT> {}(value, ctx);
}

auto foo = std::format("{}", std::byte {});
