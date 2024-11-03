// Copryright (C) 2022 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#ifndef STORMKIT_FORMAT_MACRO_HPP
#define STORMKIT_FORMAT_MACRO_HPP

#include <stormkit/Core/PlatformMacro.hpp>

#define FORMATTER_DECLARE(_From)                                                         \
    template<_From T, typename CharT>                                                    \
    struct std::formatter<T, CharT> {                                                    \
        template<class ParseContext>                                                     \
        constexpr auto parse(ParseContext& ctx) noexcept -> decltype(ctx.begin());       \
                                                                                         \
        template<typename FormatContext, typename U>                                     \
            requires(stormkit::meta::IsStrict<T, stormkit::meta::CanonicalType<U>>)      \
        auto format(U&& data, FormatContext& ctx) const noexcept -> decltype(ctx.out()); \
    };

#define FORMATTER_DEFINE_PARSE(_From)                                                          \
    template<_From T, typename CharT>                                                          \
    template<class ParseContext>                                                               \
    STORMKIT_INLINE constexpr auto std::formatter<T, CharT>::parse(ParseContext& ctx) noexcept \
        -> decltype(ctx.begin())

#define FORMATTER_DEFINE_FORMAT(_From)                                                  \
    template<_From T, typename CharT>                                                   \
    template<typename FormatContext, typename U>                                        \
        requires(stormkit::meta::IsStrict<T, stormkit::meta::CanonicalType<U>>)         \
    STORMKIT_INLINE auto std::formatter<T, CharT>::format(U&& data, FormatContext& ctx) \
        const noexcept -> decltype(ctx.out())

#define FORMATTER_INHERIT_DECLARE(_Parent, _From)                                        \
    template<_From T, typename CharT>                                                    \
    struct std::formatter<T, CharT>: std::formatter<_Parent, CharT> {                    \
        template<typename FormatContext, typename U>                                     \
            requires(stormkit::meta::IsStrict<T, stormkit::meta::CanonicalType<U>>)      \
        auto format(U&& data, FormatContext& ctx) const noexcept -> decltype(ctx.out()); \
    };

#define FORMATTER_INHERIT_DEFINE_FORMAT(_From)                                          \
    template<_From T, typename CharT>                                                   \
    template<typename FormatContext, typename U>                                        \
        requires(stormkit::meta::IsStrict<T, stormkit::meta::CanonicalType<U>>)         \
    STORMKIT_INLINE auto std::formatter<T, CharT>::format(U&& data, FormatContext& ctx) \
        const noexcept -> decltype(ctx.out())

#define FORMATTER_INHERIT_DEFINE_FORMAT_AS_STRING(_Parent, _From)                                  \
    FORMATTER_INHERIT_DEFINE_FORMAT(_From) {                                                       \
        return formatter<_Parent, CharT>::format(stormkit::as<std::string>(std::forward<U>(data)), \
                                                 ctx);                                             \
    }

#define FORMATTER_DEFINE_FORMAT_AS_STRING(_From)                                                         \
    FORMATTER_DEFINE_PARSE(_From) {                                                               \
        return ctx.begin();                                                                       \
    }                                                                                             \
    FORMATTER_DEFINE_FORMAT(_From) {                                                              \
        return std::format_to(ctx.out(), "{}", stormkit::as<std::string>(std::forward<U>(data))); \
    }

#define FORMATTER(_From)            \
    FORMATTER_DECLARE(_From)        \
    FORMATTER_DEFINE_PARSE(_From) { \
        return ctx.begin();         \
    }                               \
    FORMATTER_DEFINE_FORMAT(_From)

#define FORMATTER_INHERIT(_Parse, _From)     \
    FORMATTER_INHERIT_DECLARE(_Parse, _From) \
    FORMATTER_INHERIT_DEFINE_FORMAT(_From)

#define FORMATTER_AS_STRING(_From) \
    FORMATTER_DECLARE(_From)       \
    FORMATTER_DEFINE_FORMAT_AS_STRING(_From)

#endif
