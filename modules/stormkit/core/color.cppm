// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.color;

import std;

import stormkit.core.types;

import stormkit.core.typesafe.safecasts;
import stormkit.core.meta.concepts;
import stormkit.core.meta.type_query;
import stormkit.core.string.safecasts;
import stormkit.core.string.format;

using namespace stormkit::literals;

export namespace stormkit { inline namespace core {
    namespace meta {
        template<typename T>
        concept color_component_type = is_any_of<T, u8, f32, f64>;
    } // namespace meta

    enum class color_layout {
        R,
        RG,
        RGB,
        BGR,
        RGBA,
        ARGB,
        BGRA,
        ABGR,
    };

    template<meta::color_component_type T>
    inline constexpr auto COLOR_COMPONENT_MAX = T { 1 };

    template<>
    inline constexpr auto COLOR_COMPONENT_MAX<u8> = 0xFF;

    template<color_layout LAYOUT, meta::color_component_type T>
    struct color;

    template<meta::color_component_type T>
    struct color<color_layout::R, T> {
        using component_type = T;

        static constexpr auto LAYOUT           = color_layout::R;
        static constexpr auto COMPONENTS_COUNT = 1;

        component_type r;

        constexpr auto operator==(meta::in<color> other) const noexcept -> bool;
    };

    template<meta::color_component_type T>
    struct color<color_layout::RG, T> {
        using component_type = T;

        static constexpr auto LAYOUT           = color_layout::RG;
        static constexpr auto COMPONENTS_COUNT = 2;

        component_type r;
        component_type g;

        constexpr auto operator==(meta::in<color> other) const noexcept -> bool;
    };

    template<meta::color_component_type T>
    struct color<color_layout::RGB, T> {
        using component_type = T;

        static constexpr auto LAYOUT           = color_layout::RGB;
        static constexpr auto COMPONENTS_COUNT = 3;

        component_type r;
        component_type g;
        component_type b;

        constexpr auto operator==(meta::in<color> other) const noexcept -> bool;
    };

    template<meta::color_component_type T>
    struct color<color_layout::RGBA, T> {
        using component_type = T;

        static constexpr auto LAYOUT           = color_layout::RGBA;
        static constexpr auto COMPONENTS_COUNT = 4;

        component_type r;
        component_type g;
        component_type b;
        component_type a;

        constexpr auto operator==(meta::in<color> other) const noexcept -> bool;
    };

    template<meta::color_component_type T>
    struct color<color_layout::ARGB, T> {
        using component_type = T;

        static constexpr auto LAYOUT           = color_layout::ARGB;
        static constexpr auto COMPONENTS_COUNT = 4;

        component_type a;
        component_type r;
        component_type g;
        component_type b;

        constexpr auto operator==(meta::in<color> other) const noexcept -> bool;
    };

    template<meta::color_component_type T>
    struct color<color_layout::BGR, T> {
        using component_type = T;

        static constexpr auto LAYOUT           = color_layout::BGR;
        static constexpr auto COMPONENTS_COUNT = 3;

        component_type b;
        component_type g;
        component_type r;

        constexpr auto operator==(meta::in<color> other) const noexcept -> bool;
    };

    template<meta::color_component_type T>
    struct color<color_layout::BGRA, T> {
        using component_type = T;

        static constexpr auto LAYOUT           = color_layout::BGRA;
        static constexpr auto COMPONENTS_COUNT = 4;

        component_type b;
        component_type g;
        component_type r;
        component_type a;

        constexpr auto operator==(meta::in<color> other) const noexcept -> bool;
    };

    template<meta::color_component_type T>
    struct color<color_layout::ABGR, T> {
        using component_type = T;

        static constexpr auto LAYOUT           = color_layout::ABGR;
        static constexpr auto COMPONENTS_COUNT = 4;

        component_type a;
        component_type b;
        component_type g;
        component_type r;

        constexpr auto operator==(meta::in<color> other) const noexcept -> bool;
    };

    template<meta::color_component_type T>
    using color_r = color<color_layout::R, T>;
    template<meta::color_component_type T>
    using color_rg = color<color_layout::RG, T>;
    template<meta::color_component_type T>
    using color_rgb = color<color_layout::RGB, T>;
    template<meta::color_component_type T>
    using color_rgba = color<color_layout::RGBA, T>;
    template<meta::color_component_type T>
    using color_argb = color<color_layout::ARGB, T>;
    template<meta::color_component_type T>
    using color_bgr = color<color_layout::BGR, T>;
    template<meta::color_component_type T>
    using color_bgra = color<color_layout::BGRA, T>;
    template<meta::color_component_type T>
    using color_abgr = color<color_layout::ABGR, T>;

    using fcolor_r    = color_r<f32>;
    using fcolor_rg   = color_rg<f32>;
    using fcolor_rgb  = color_rgb<f32>;
    using fcolor_rgba = color_rgba<f32>;
    using fcolor_argb = color_argb<f32>;
    using fcolor_bgr  = color_bgr<f32>;
    using fcolor_bgra = color_bgra<f32>;
    using fcolor_abgr = color_abgr<f32>;

    using ucolor_r    = color_r<u8>;
    using ucolor_rg   = color_rg<u8>;
    using ucolor_rgb  = color_rgb<u8>;
    using ucolor_rgba = color_rgba<u8>;
    using ucolor_argb = color_argb<u8>;
    using ucolor_bgr  = color_bgr<u8>;
    using ucolor_bgra = color_bgra<u8>;
    using ucolor_abgr = color_abgr<u8>;

    template<color_layout TO_LAYOUT, meta::color_component_type T, color_layout FROM_LAYOUT, meta::color_component_type U>
    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<color<TO_LAYOUT, T>>,
                              const color<FROM_LAYOUT, U>& value,
                              source_location_arg = std::source_location::current()) noexcept -> string_view;

    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<string_view>,
                              color_layout value,
                              source_location_arg = std::source_location::current()) noexcept -> string_view;

    template<color_layout LAYOUT, meta::color_component_type T>
    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<string>,
                              const color<LAYOUT, T>& value,
                              source_location_arg = std::source_location::current()) noexcept -> string;

    template<typename CharT, typename FormatContext, color_layout LAYOUT, meta::color_component_type T>
    constexpr auto tag_invoke(format_as_fn<CharT>, const color<LAYOUT, T>& value, FormatContext& ctx) -> decltype(ctx.out());
}} // namespace stormkit::core

namespace stormkit { inline namespace core { namespace details {
    template<color_layout LAYOUT, meta::color_component_type T>
    struct implicit_color_converter {
        template<color_layout TO_LAYOUT, meta::color_component_type U>
        constexpr operator color<TO_LAYOUT, U>() const noexcept;

        color<LAYOUT, T> c;
    };
}}} // namespace stormkit::core::details

export namespace stormkit { inline namespace core {
    /// This namespace contain preset colors, these colors are defined from [HTML 4.01
    /// colors](https://en.wikipedia.org/wiki/Web_colors#HTML_color_names).
    ///```
    /// | name        | red |green|blue |alpha|
    /// |-------------|-----|-----|-----|-----|
    /// | BLACK       | 0   | 0   | 0   | 255 |
    /// | Gray        | 127 | 127 | 127 | 255 |
    /// | SILVER      | 190 | 190 | 190 | 255 |
    /// | WHITE       | 255 | 255 | 255 | 255 |
    /// | Maroon      | 127 | 0   | 0   | 255 |
    /// | RED         | 255 | 0   | 0   | 255 |
    /// | Olive       | 127 | 127 | 0   | 255 |
    /// | YELLOW      | 255 | 255 | 0   | 255 |
    /// | GREEN       | 0   | 127 | 0   | 255 |
    /// | LIME        | 0   | 255 | 0   | 255 |
    /// | TEAL        | 0   | 127 | 127 | 255 |
    /// | AQUA        | 0   | 255 | 255 | 255 |
    /// | NAVY        | 0   | 0   | 127 | 255 |
    /// | BLUE        | 0   | 0   | 255 | 255 |
    /// | PURPLE      | 127 | 0   | 127 | 255 |
    /// | FUSCHIA     | 255 | 0   | 255 | 255 |
    /// | TRANSPARENT | 0   | 0   | 0   | 0   |
    ///```

    namespace colors {
        template<meta::color_component_type T>
        inline constexpr auto BLACK = details::implicit_color_converter<color_layout::RGBA, T> {
            .c = { .r = 0, .g = 0, .b = 0, .a = COLOR_COMPONENT_MAX<T> }
        };

        template<meta::color_component_type T>
        inline constexpr auto GRAY = details::implicit_color_converter<color_layout::RGBA, T> {
            .c = { .r = COLOR_COMPONENT_MAX<T> / T { 2 },
                  .g = COLOR_COMPONENT_MAX<T> / T { 2 },
                  .b = COLOR_COMPONENT_MAX<T> / T { 2 },
                  .a = COLOR_COMPONENT_MAX<T> }
        };

        template<meta::color_component_type T>
        inline constexpr auto SILVER = details::implicit_color_converter<color_layout::RGBA, T> {
            .c = { .r = COLOR_COMPONENT_MAX<T> / T { 2 } + COLOR_COMPONENT_MAX<T> / T { 4 },
                  .g = COLOR_COMPONENT_MAX<T> / T { 2 } + COLOR_COMPONENT_MAX<T> / T { 4 },
                  .b = COLOR_COMPONENT_MAX<T> / T { 2 } + COLOR_COMPONENT_MAX<T> / T { 4 },
                  .a = COLOR_COMPONENT_MAX<T> }
        };

        template<meta::color_component_type T>
        inline constexpr auto WHITE = details::implicit_color_converter<color_layout::RGBA, T> {
            .c = { .r = COLOR_COMPONENT_MAX<T>,
                  .g = COLOR_COMPONENT_MAX<T>,
                  .b = COLOR_COMPONENT_MAX<T>,
                  .a = COLOR_COMPONENT_MAX<T> }
        };

        template<meta::color_component_type T>
        inline constexpr auto MAROON = details::implicit_color_converter<color_layout::RGBA, T> {
            .c = { .r = COLOR_COMPONENT_MAX<T> / T { 2 }, .g = T { 0 }, .b = T { 0 }, .a = COLOR_COMPONENT_MAX<T> }
        };

        template<meta::color_component_type T>
        inline constexpr auto RED = details::implicit_color_converter<color_layout::RGBA, T> {
            .c = { .r = COLOR_COMPONENT_MAX<T>, .g = 0, .b = 0, .a = COLOR_COMPONENT_MAX<T> }
        };

        template<meta::color_component_type T>
        inline constexpr auto OLIVE = details::implicit_color_converter<color_layout::RGBA, T> {
            .c = { .r = COLOR_COMPONENT_MAX<T> / T { 2 },
                  .g = COLOR_COMPONENT_MAX<T> / T { 2 },
                  .b = T { 0 },
                  .a = COLOR_COMPONENT_MAX<T> }
        };

        template<meta::color_component_type T>
        inline constexpr auto YELLOW = details::implicit_color_converter<color_layout::RGBA, T> {
            .c = { .r = COLOR_COMPONENT_MAX<T>, .g = COLOR_COMPONENT_MAX<T>, .b = T { 0 }, .a = COLOR_COMPONENT_MAX<T> }
        };

        template<meta::color_component_type T>
        inline constexpr auto GREEN = details::implicit_color_converter<color_layout::RGBA, T> {
            .c = { .r = 0, .g = COLOR_COMPONENT_MAX<T> / T { 2 }, .b = 0, .a = COLOR_COMPONENT_MAX<T> }
        };

        template<meta::color_component_type T>
        inline constexpr auto LIME = details::implicit_color_converter<color_layout::RGBA, T> {
            .c = { .r = 0, .g = COLOR_COMPONENT_MAX<T>, .b = 0, .a = COLOR_COMPONENT_MAX<T> }
        };

        template<meta::color_component_type T>
        inline constexpr auto TEAL = details::implicit_color_converter<color_layout::RGBA, T> {
            .c = { .r = 0,
                  .g = COLOR_COMPONENT_MAX<T> / T { 2 },
                  .b = COLOR_COMPONENT_MAX<T> / T { 2 },
                  .a = COLOR_COMPONENT_MAX<T> }
        };

        template<meta::color_component_type T>
        inline constexpr auto AQUA = details::implicit_color_converter<color_layout::RGBA, T> {
            .c = { .r = 0, .g = COLOR_COMPONENT_MAX<T>, .b = COLOR_COMPONENT_MAX<T>, .a = COLOR_COMPONENT_MAX<T> }
        };

        template<meta::color_component_type T>
        inline constexpr auto NAVY = details::implicit_color_converter<color_layout::RGBA, T> {
            .c = { .r = 0, .g = 0, .b = COLOR_COMPONENT_MAX<T> / T { 2 }, .a = COLOR_COMPONENT_MAX<T> }
        };

        template<meta::color_component_type T>
        inline constexpr auto BLUE = details::implicit_color_converter<color_layout::RGBA, T> {
            .c = { .r = 0, .g = 0, .b = COLOR_COMPONENT_MAX<T>, .a = COLOR_COMPONENT_MAX<T> }
        };

        template<meta::color_component_type T>
        inline constexpr auto PURPLE = details::implicit_color_converter<color_layout::RGBA, T> {
            .c = { .r = COLOR_COMPONENT_MAX<T> / T { 2 },
                  .g = 0,
                  .b = COLOR_COMPONENT_MAX<T> / T { 2 },
                  .a = COLOR_COMPONENT_MAX<T> }
        };

        template<meta::color_component_type T>
        inline constexpr auto FUSCHIA = details::implicit_color_converter<color_layout::RGBA, T> {
            .c = { .r = COLOR_COMPONENT_MAX<T>, .g = 0, .b = COLOR_COMPONENT_MAX<T>, .a = COLOR_COMPONENT_MAX<T> }
        };

        template<meta::color_component_type T>
        inline constexpr auto TRANSPARENT = details::implicit_color_converter<color_layout::RGBA, T> {
            .c = { .r = 0, .g = 0, .b = 0, .a = 0 }
        };
    } // namespace colors
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    template<color_layout LAYOUT, meta::color_component_type T>
    template<color_layout TO_LAYOUT, meta::color_component_type U>
    constexpr details::implicit_color_converter<LAYOUT, T>::operator color<TO_LAYOUT, U>() const noexcept {
        return as<color<TO_LAYOUT, U>>(c);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::color_component_type T>
    STORMKIT_FORCE_INLINE
    constexpr auto color<color_layout::R, T>::operator==(meta::in<color> other) const noexcept -> bool {
        return r == other.r;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::color_component_type T>
    STORMKIT_FORCE_INLINE
    constexpr auto color<color_layout::RG, T>::operator==(meta::in<color> other) const noexcept -> bool {
        return r == other.r and g == other.g;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::color_component_type T>
    STORMKIT_FORCE_INLINE
    constexpr auto color<color_layout::RGB, T>::operator==(meta::in<color> other) const noexcept -> bool {
        return r == other.r and g == other.g and b == other.b;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::color_component_type T>
    STORMKIT_FORCE_INLINE
    constexpr auto color<color_layout::RGBA, T>::operator==(meta::in<color> other) const noexcept -> bool {
        return r == other.r and g == other.g and b == other.b and a == other.a;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::color_component_type T>
    STORMKIT_FORCE_INLINE
    constexpr auto color<color_layout::ARGB, T>::operator==(meta::in<color> other) const noexcept -> bool {
        return r == other.r and g == other.g and b == other.b and a == other.a;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::color_component_type T>
    STORMKIT_FORCE_INLINE
    constexpr auto color<color_layout::BGR, T>::operator==(meta::in<color> other) const noexcept -> bool {
        return r == other.r and g == other.g and b == other.b;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::color_component_type T>
    STORMKIT_FORCE_INLINE
    constexpr auto color<color_layout::BGRA, T>::operator==(meta::in<color> other) const noexcept -> bool {
        return r == other.r and g == other.g and b == other.b and a == other.a;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::color_component_type T>
    STORMKIT_FORCE_INLINE
    constexpr auto color<color_layout::ABGR, T>::operator==(meta::in<color> other) const noexcept -> bool {
        return r == other.r and g == other.g and b == other.b and a == other.a;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<color_layout TO_LAYOUT, meta::color_component_type T, color_layout FROM_LAYOUT, meta::color_component_type U>
    constexpr auto tag_invoke(as_fn<color<TO_LAYOUT, T>>,
                              const color<FROM_LAYOUT, U>& value,
                              source_location_arg = std::source_location::current()) noexcept -> color<TO_LAYOUT, T> {
        static constexpr auto FROM_COMPONENT_COUNT = color<FROM_LAYOUT, U>::COMPONENT_COUNT;
        static constexpr auto TO_COMPONENT_COUNT   = color<TO_LAYOUT, T>::COMPONENT_COUNT;

        static constexpr auto AS_COMPONENT_TYPE = [](U component) static noexcept {
            if constexpr (meta::is<T, U>) return component;
            else if constexpr (meta::is<T, u8>)
                return as<u8>(component * 255);
            else
                return as<T>(component) / T { 255 };
        };

        auto out = color<TO_LAYOUT, T> {};

        if constexpr (TO_COMPONENT_COUNT >= 1) out.r = AS_COMPONENT_TYPE(value.r);
        if constexpr (TO_COMPONENT_COUNT >= 2 and FROM_COMPONENT_COUNT >= 2) out.g = AS_COMPONENT_TYPE(value.g);
        if constexpr (TO_COMPONENT_COUNT >= 3 and FROM_COMPONENT_COUNT >= 3) out.b = AS_COMPONENT_TYPE(value.b);
        if constexpr (TO_COMPONENT_COUNT == 4) {
            if constexpr (FROM_COMPONENT_COUNT == 4) out.a = AS_COMPONENT_TYPE(value.a);
            else
                out.a = COLOR_COMPONENT_MAX<T>;
        }

        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_CONST
    constexpr auto tag_invoke(as_fn<string_view>, color_layout layout, source_location_arg) noexcept -> string_view {
        switch (layout) {
            case color_layout::R: return "R";
            case color_layout::RG: return "RG";
            case color_layout::RGB: return "RGB";
            case color_layout::BGR: return "BGR";
            case color_layout::RGBA: return "RGBA";
            case color_layout::ARGB: return "ARGB";
            case color_layout::BGRA: return "BGRA";
            case color_layout::ABGR: return "ABGR";
            default: break;
        }
        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<color_layout LAYOUT, meta::color_component_type T>
    constexpr auto tag_invoke(as_fn<string>, const color<LAYOUT, T>& value, source_location_arg) noexcept -> string {
        static constexpr auto COMPONENT_COUNT = color<LAYOUT, T>::COMPONENT_COUNT;

        auto&& color_u8 = [&value] noexcept {
            if constexpr (meta::is<T, u8>) return value;
            else
                return as<color<LAYOUT, u8>>(value);
        }();

        auto color_as_int = 0xFF_u32;
        if constexpr (COMPONENT_COUNT >= 1) color_as_int |= (as<u32>(color_u8.r) << 24);
        if constexpr (COMPONENT_COUNT >= 2) color_as_int |= (as<u32>(color_u8.g) << 16);
        if constexpr (COMPONENT_COUNT >= 3) color_as_int |= (as<u32>(color_u8.b) << 8);
        if constexpr (COMPONENT_COUNT == 4) color_as_int |= (as<u32>(color_u8.a));

        auto out = std::string {};
        out.reserve(10);
        out.push_back('0');
        out.push_back('x');
        out.append(to_upper(as<string>(color_as_int, 16)));
        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename CharT, typename FormatContext, color_layout LAYOUT, meta::color_component_type T>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(format_as_fn<CharT>, const color<LAYOUT, T>& value, FormatContext& ctx) -> decltype(ctx.out()) {
        static constexpr auto COMPONENT_COUNT = color<LAYOUT, T>::COMPONENT_COUNT;

        ctx.advance_to(std::format_to(ctx.out(), "[color layout: {}, ", LAYOUT));

        if constexpr (COMPONENT_COUNT == 1) ctx.advance_to(std::format_to(ctx.out(), "red: {}", value.r));
        else if constexpr (COMPONENT_COUNT == 2)
            ctx.advance_to(std::format_to(ctx.out(), "red: {}, green: {}", value.r, value.g));
        else if constexpr (COMPONENT_COUNT == 3)
            ctx.advance_to(std::format_to(ctx.out(), "red: {}, green: {}, blue: {}", value.r, value.g, value.b));
        else if constexpr (COMPONENT_COUNT == 4)
            ctx.advance_to(std::
                             format_to(ctx.out(), "red: {}, green: {}, blue: {}, alpha: {}", value.r, value.g, value.b, value.a));

        return std::format_to(ctx.out(), ", hex: {}]", as<string>(value));
    }
}} // namespace stormkit::core
