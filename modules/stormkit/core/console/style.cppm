// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
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
            red,
            green,
            yellow,
            blue,
            magenta,
            cyan,
            white,
            black,
            bright_red,
            bright_green,
            bright_yellow,
            bright_blue,
            bright_magenta,
            bright_cyan,
            bright_white,
            bright_black,
        };

        enum class style_modifier {
            none      = 0x0,
            bold      = 0x1,
            faint     = 0x2,
            italic    = 0x4,
            inverse   = 0x8,
            underline = 0xA,
        };

        template<typename T>
        struct stylized {
            constexpr auto render() const noexcept -> string;
            constexpr auto render_into(stdr::output_range<char> auto& out) const noexcept -> void;

            T                            value;
            std::optional<console_color> fg;
            std::optional<console_color> bg;
            style_modifier               modifiers = style_modifier::none;
        };

        struct console_style {
            template<typename T>
            [[nodiscard]]
            constexpr auto               operator|(T&& value) const noexcept -> stylized<T>;
            std::optional<console_color> fg        = std::nullopt;
            std::optional<console_color> bg        = std::nullopt;
            style_modifier               modifiers = style_modifier::none;
        };

        inline constexpr auto red_text_style     = console_style { .fg = console_color::red };
        inline constexpr auto green_text_style   = console_style { .fg = console_color::green };
        inline constexpr auto yellow_text_style  = console_style { .fg = console_color::yellow };
        inline constexpr auto blue_text_style    = console_style { .fg = console_color::blue };
        inline constexpr auto magenta_text_style = console_style { .fg = console_color::magenta };
        inline constexpr auto cyan_text_style    = console_style { .fg = console_color::cyan };
        inline constexpr auto white_text_style   = console_style { .fg = console_color::white };
        inline constexpr auto black_text_style   = console_style { .fg = console_color::black };

        namespace ecma48 {
            inline constexpr auto foreground = make_static_hash_map<console_color, string_view>({
              { console_color::black,          "\x1B[30m" },
              { console_color::red,            "\x1B[31m" },
              { console_color::green,          "\x1B[32m" },
              { console_color::yellow,         "\x1B[33m" },
              { console_color::blue,           "\x1B[34m" },
              { console_color::magenta,        "\x1B[35m" },
              { console_color::cyan,           "\x1B[36m" },
              { console_color::white,          "\x1B[37m" },
              { console_color::bright_black,   "\x1B[90m" },
              { console_color::bright_red,     "\x1B[91m" },
              { console_color::bright_green,   "\x1B[92m" },
              { console_color::bright_yellow,  "\x1B[93m" },
              { console_color::bright_blue,    "\x1B[94m" },
              { console_color::bright_magenta, "\x1B[95m" },
              { console_color::bright_cyan,    "\x1B[96m" },
              { console_color::bright_white,   "\x1B[97m" },
            });

            inline constexpr auto background = make_static_hash_map<console_color, string_view>({
              { console_color::black,          "\x1B[40m"  },
              { console_color::red,            "\x1B[41m"  },
              { console_color::green,          "\x1B[42m"  },
              { console_color::yellow,         "\x1B[43m"  },
              { console_color::blue,           "\x1B[44m"  },
              { console_color::magenta,        "\x1B[45m"  },
              { console_color::cyan,           "\x1B[46m"  },
              { console_color::white,          "\x1B[47m"  },
              { console_color::bright_black,   "\x1B[100m" },
              { console_color::bright_red,     "\x1B[101m" },
              { console_color::bright_green,   "\x1B[102m" },
              { console_color::bright_yellow,  "\x1B[103m" },
              { console_color::bright_blue,    "\x1B[104m" },
              { console_color::bright_magenta, "\x1B[105m" },
              { console_color::bright_cyan,    "\x1B[106m" },
              { console_color::bright_white,   "\x1B[107m" },
            });

            inline constexpr auto reset     = "\x1B[0m"sv;
            inline constexpr auto bold      = "\x1B[1m"sv;
            inline constexpr auto faint     = "\x1B[2m"sv;
            inline constexpr auto italic    = "\x1B[3m"sv;
            inline constexpr auto underline = "\x1B[4m"sv;
            inline constexpr auto inverse   = "\x1B[7m"sv;
        } // namespace ecma48

        template<typename CharT, typename FormatContext>
        constexpr auto tag_invoke(format_as_fn<CharT>, meta::in<console_style> value, FormatContext& ctx) -> decltype(ctx.out());

        template<typename CharT, typename FormatContext, typename T>
        constexpr auto tag_invoke(format_as_fn<CharT>, const stylized<T>& value, FormatContext& ctx) -> decltype(ctx.out());
    }} // namespace stormkit::core

    template<>
    inline constexpr auto stormkit::core::meta::flag_trait<stormkit::style_modifier> = true;
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
        if (fg) out.append_range(ecma48::foreground.at(*fg));
        if (bg) out.append_range(ecma48::background.at(*bg));
        if (has_flag_bit(modifiers, style_modifier::bold)) out.append(ecma48::bold);
        if (has_flag_bit(modifiers, style_modifier::faint)) out.append(ecma48::faint);
        if (has_flag_bit(modifiers, style_modifier::italic)) out.append(ecma48::italic);
        if (has_flag_bit(modifiers, style_modifier::inverse)) out.append(ecma48::inverse);
        if (has_flag_bit(modifiers, style_modifier::underline)) out.append(ecma48::underline);
        if constexpr (meta::convertible_to<T, string_view>) out.append_range(string_view { value });
        else
            out.append_range(std::format("{}", value));

        out.append(ecma48::reset);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename CharT, typename FormatContext>
    constexpr auto tag_invoke(format_as_fn<CharT>, meta::in<console_style> style, FormatContext& ctx) -> decltype(ctx.out()) {
        thread_local auto buff = static_string<32> {};
        auto              end  = stdr::begin(buff);

        if (style.fg) {
            auto [_, end_] = stdr::copy(ecma48::foreground.at(*style.fg), end);
            end            = end_;
        }
        if (style.bg) {
            auto [_, end_] = stdr::copy(ecma48::background.at(*style.bg), end);
            end            = end_;
        }
        if (has_flag_bit(style.modifiers, style_modifier::bold)) {
            auto [_, end_] = stdr::copy(ecma48::bold, end);
            end            = end_;
        }
        if (has_flag_bit(style.modifiers, style_modifier::faint)) {
            auto [_, end_] = stdr::copy(ecma48::faint, end);
            end            = end_;
        }
        if (has_flag_bit(style.modifiers, style_modifier::italic)) {
            auto [_, end_] = stdr::copy(ecma48::italic, end);
            end            = end_;
        }
        if (has_flag_bit(style.modifiers, style_modifier::inverse)) {
            auto [_, end_] = stdr::copy(ecma48::inverse, end);
            end            = end_;
        }
        if (has_flag_bit(style.modifiers, style_modifier::underline)) {
            auto [_, end_] = stdr::copy(ecma48::underline, end);
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
