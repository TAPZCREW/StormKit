// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.console.style;

import std;

import stormkit.core.types;
import stormkit.core.string.format;
import stormkit.core.string.static_string;
import stormkit.core.meta.type_manipulation;
import stormkit.core.meta.concepts;
import stormkit.core.meta.type_query;
import stormkit.core.typesafe.flags;
import stormkit.core.containers.hash_map;

namespace stdr = std::ranges;
namespace stdv = std::views;

using namespace std::literals;

export {
    namespace stormkit { inline namespace core {
        enum class console_color {
            RED,
            GREEN,
            YELLOW,
            BLUE,
            MAGENTA,
            CYAN,
            WHITE,
            BLACK,
            BRIGHT_RED,
            BRIGHT_GREEN,
            BRIGHT_YELLOW,
            BRIGHT_BLUE,
            BRIGHT_MAGENTA,
            BRIGHT_CYAN,
            BRIGHT_WHITE,
            BRIGHT_BLACK,
        };

        enum class style_modifier {
            NONE      = 0x0,
            BOLD      = 0x1,
            FAINT     = 0x2,
            ITALIC    = 0x4,
            INVERSE   = 0x8,
            UNDERLINE = 0xA,
        };

        template<typename T>
        struct stylized {
            constexpr auto render() const noexcept -> string;
            constexpr auto render_into(stdr::output_range<char> auto& out) const noexcept -> void;

            T                            value;
            std::optional<console_color> fg;
            std::optional<console_color> bg;
            style_modifier               modifiers = style_modifier::NONE;
        };

        struct console_style {
            template<typename T>
            [[nodiscard]]
            constexpr auto               operator|(T&& value) const noexcept -> stylized<T>;
            std::optional<console_color> fg        = std::nullopt;
            std::optional<console_color> bg        = std::nullopt;
            style_modifier               modifiers = style_modifier::NONE;
        };

        inline constexpr auto RED_TEXT_STYLE     = console_style { .fg = console_color::RED };
        inline constexpr auto GREEN_TEXT_STYLE   = console_style { .fg = console_color::GREEN };
        inline constexpr auto YELLOW_TEXT_STYLE  = console_style { .fg = console_color::YELLOW };
        inline constexpr auto BLUE_TEXT_STYLE    = console_style { .fg = console_color::BLUE };
        inline constexpr auto MAGENTA_TEXT_STYLE = console_style { .fg = console_color::MAGENTA };
        inline constexpr auto CYAN_TEXT_STYLE    = console_style { .fg = console_color::CYAN };
        inline constexpr auto WHITE_TEXT_STYLE   = console_style { .fg = console_color::WHITE };
        inline constexpr auto BLACK_TEXT_STYLE   = console_style { .fg = console_color::BLACK };

        namespace ecma48 {
            inline constexpr auto FOREGROUND = make_static_hash_map<console_color, string_view>({
              { console_color::BLACK,          "\x1B[30m" },
              { console_color::RED,            "\x1B[31m" },
              { console_color::GREEN,          "\x1B[32m" },
              { console_color::YELLOW,         "\x1B[33m" },
              { console_color::BLUE,           "\x1B[34m" },
              { console_color::MAGENTA,        "\x1B[35m" },
              { console_color::CYAN,           "\x1B[36m" },
              { console_color::WHITE,          "\x1B[37m" },
              { console_color::BRIGHT_BLACK,   "\x1B[90m" },
              { console_color::BRIGHT_RED,     "\x1B[91m" },
              { console_color::BRIGHT_GREEN,   "\x1B[92m" },
              { console_color::BRIGHT_YELLOW,  "\x1B[93m" },
              { console_color::BRIGHT_BLUE,    "\x1B[94m" },
              { console_color::BRIGHT_MAGENTA, "\x1B[95m" },
              { console_color::BRIGHT_CYAN,    "\x1B[96m" },
              { console_color::BRIGHT_WHITE,   "\x1B[97m" },
            });

            inline constexpr auto BACKGROUND = make_static_hash_map<console_color, string_view>({
              { console_color::BLACK,          "\x1B[40m"  },
              { console_color::RED,            "\x1B[41m"  },
              { console_color::GREEN,          "\x1B[42m"  },
              { console_color::YELLOW,         "\x1B[43m"  },
              { console_color::BLUE,           "\x1B[44m"  },
              { console_color::MAGENTA,        "\x1B[45m"  },
              { console_color::CYAN,           "\x1B[46m"  },
              { console_color::WHITE,          "\x1B[47m"  },
              { console_color::BRIGHT_BLACK,   "\x1B[100m" },
              { console_color::BRIGHT_RED,     "\x1B[101m" },
              { console_color::BRIGHT_GREEN,   "\x1B[102m" },
              { console_color::BRIGHT_YELLOW,  "\x1B[103m" },
              { console_color::BRIGHT_BLUE,    "\x1B[104m" },
              { console_color::BRIGHT_MAGENTA, "\x1B[105m" },
              { console_color::BRIGHT_CYAN,    "\x1B[106m" },
              { console_color::BRIGHT_WHITE,   "\x1B[107m" },
            });

            inline constexpr auto RESET     = "\x1B[0m"sv;
            inline constexpr auto BOLD      = "\x1B[1m"sv;
            inline constexpr auto FAINT     = "\x1B[2m"sv;
            inline constexpr auto ITALIC    = "\x1B[3m"sv;
            inline constexpr auto UNDERLINE = "\x1B[4m"sv;
            inline constexpr auto INVERSE   = "\x1B[7m"sv;
        } // namespace ecma48

        template<typename CharT, typename FormatContext>
        constexpr auto tag_invoke(format_as_fn<CharT>, meta::in<console_style> value, FormatContext& ctx) -> decltype(ctx.out());

        template<typename CharT, typename FormatContext, typename T>
        constexpr auto tag_invoke(format_as_fn<CharT>, const stylized<T>& value, FormatContext& ctx) -> decltype(ctx.out());
    }} // namespace stormkit::core

    template<>
    inline constexpr auto stormkit::core::meta::FLAG_TRAIT<stormkit::style_modifier> = true;
}

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

using namespace std::literals;

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    constexpr auto console_style::operator|(T&& value) const noexcept -> stylized<T> {
        return { std::forward<T>(value), fg, bg, modifiers };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    constexpr auto stylized<T>::render() const noexcept -> string {
        auto       out  = string {};
        const auto size = [this] noexcept {
            if constexpr (requires { stdr::size(value); }) return stdr::size(value);
            else if constexpr (requires { std::char_traits<meta::remove_indirections_of<T>>::length(value); })
                return std::char_traits<meta::remove_indirections_of<T>>::length(value);
            else {
                (void)this;
                return 1uz;
            }
        }();
        out.reserve(2uz + size + 1uz);
        render_into(out);
        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    constexpr auto stylized<T>::render_into(stdr::output_range<char> auto& out) const noexcept -> void {
        if (fg) out.append_range(ecma48::FOREGROUND.at(*fg));
        if (bg) out.append_range(ecma48::BACKGROUND.at(*bg));
        if (has_flag_bit(modifiers, style_modifier::BOLD)) out.append(ecma48::BOLD);
        if (has_flag_bit(modifiers, style_modifier::FAINT)) out.append(ecma48::FAINT);
        if (has_flag_bit(modifiers, style_modifier::ITALIC)) out.append(ecma48::ITALIC);
        if (has_flag_bit(modifiers, style_modifier::INVERSE)) out.append(ecma48::INVERSE);
        if (has_flag_bit(modifiers, style_modifier::UNDERLINE)) out.append(ecma48::UNDERLINE);
        if constexpr (meta::convertible_to<T, string_view>) out.append_range(string_view { value });
        else
            out.append_range(std::format("{}", value));

        out.append(ecma48::RESET);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename CharT, typename FormatContext>
    constexpr auto tag_invoke(format_as_fn<CharT>, meta::in<console_style> style, FormatContext& ctx) -> decltype(ctx.out()) {
        thread_local auto buff = static_string<32> {};
        auto              end  = stdr::begin(buff);

        if (style.fg) {
            auto [_, end_] = stdr::copy(ecma48::FOREGROUND.at(*style.fg), end);
            end            = end_;
        }
        if (style.bg) {
            auto [_, end_] = stdr::copy(ecma48::BACKGROUND.at(*style.bg), end);
            end            = end_;
        }
        if (has_flag_bit(style.modifiers, style_modifier::BOLD)) {
            auto [_, end_] = stdr::copy(ecma48::BOLD, end);
            end            = end_;
        }
        if (has_flag_bit(style.modifiers, style_modifier::FAINT)) {
            auto [_, end_] = stdr::copy(ecma48::FAINT, end);
            end            = end_;
        }
        if (has_flag_bit(style.modifiers, style_modifier::ITALIC)) {
            auto [_, end_] = stdr::copy(ecma48::ITALIC, end);
            end            = end_;
        }
        if (has_flag_bit(style.modifiers, style_modifier::INVERSE)) {
            auto [_, end_] = stdr::copy(ecma48::INVERSE, end);
            end            = end_;
        }
        if (has_flag_bit(style.modifiers, style_modifier::UNDERLINE)) {
            auto [_, end_] = stdr::copy(ecma48::UNDERLINE, end);
            end            = end_;
        }

        return std::format_to(ctx.out(), "{}", string_view { stdr::begin(buff), end });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename CharT, typename FormatContext, typename T>
    constexpr auto tag_invoke(format_as_fn<CharT>, const stylized<T>& value, FormatContext& ctx) -> decltype(ctx.out()) {
        return std::format_to(ctx.out(), "{}", value.render());
    }
}} // namespace stormkit::core
