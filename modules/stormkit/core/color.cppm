// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
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
import stormkit.core.meta.algorithms;
import stormkit.core.meta.type_query;
import stormkit.core.string.format;
import stormkit.core.string.algorithms;

using namespace stormkit::literals;

export namespace stormkit { inline namespace core {
    namespace meta {
        template<typename T>
        concept color_component = is_any_of<T, u8, f32, f64>;
    } // namespace meta

    enum class color_layout {
        r,
        rg,
        rgb,
        bgr,
        rgba,
        argb,
        bgra,
        abgr,
    };

    template<meta::color_component T>
    inline constexpr auto color_component_max = T { 1 };

    template<>
    inline constexpr auto color_component_max<u8> = 0xFF;

    template<color_layout Layout, meta::color_component T>
    struct color;

    template<meta::color_component T>
    struct color<color_layout::r, T> {
        using component_type = T;
        using param_type     = meta::conditional<(sizeof(component_type) * 1) <= (sizeof(void*) * 2), color, const color&>;

        static constexpr auto layout          = color_layout::r;
        static constexpr auto component_count = 1;

        component_type r;

        constexpr auto operator==(param_type other) const noexcept -> bool;
    };

    template<meta::color_component T>
    struct color<color_layout::rg, T> {
        using component_type = T;
        using param_type     = meta::conditional<(sizeof(component_type) * 2) <= (sizeof(void*) * 2), color, const color&>;

        static constexpr auto layout          = color_layout::rg;
        static constexpr auto component_count = 2;

        component_type r;
        component_type g;

        constexpr auto operator==(param_type other) const noexcept -> bool;
    };

    template<meta::color_component T>
    struct color<color_layout::rgb, T> {
        using component_type = T;
        using param_type     = meta::conditional<(sizeof(component_type) * 3) <= (sizeof(void*) * 2), color, const color&>;

        static constexpr auto layout          = color_layout::rgb;
        static constexpr auto component_count = 3;

        component_type r;
        component_type g;
        component_type b;

        constexpr auto operator==(param_type other) const noexcept -> bool;
    };

    template<meta::color_component T>
    struct color<color_layout::rgba, T> {
        using component_type = T;
        using param_type     = meta::conditional<(sizeof(component_type) * 4) <= (sizeof(void*) * 2), color, const color&>;

        static constexpr auto layout          = color_layout::rgba;
        static constexpr auto component_count = 4;

        component_type r;
        component_type g;
        component_type b;
        component_type a;

        constexpr auto operator==(param_type other) const noexcept -> bool;
    };

    template<meta::color_component T>
    struct color<color_layout::argb, T> {
        using component_type = T;
        using param_type     = meta::conditional<(sizeof(component_type) * 4) <= (sizeof(void*) * 2), color, const color&>;

        static constexpr auto layout          = color_layout::argb;
        static constexpr auto component_count = 4;

        component_type a;
        component_type r;
        component_type g;
        component_type b;

        constexpr auto operator==(param_type other) const noexcept -> bool;
    };

    template<meta::color_component T>
    struct color<color_layout::bgr, T> {
        using component_type = T;
        using param_type     = meta::conditional<(sizeof(component_type) * 3) <= (sizeof(void*) * 2), color, const color&>;

        static constexpr auto layout          = color_layout::bgr;
        static constexpr auto component_count = 3;

        component_type b;
        component_type g;
        component_type r;

        constexpr auto operator==(param_type other) const noexcept -> bool;
    };

    template<meta::color_component T>
    struct color<color_layout::bgra, T> {
        using component_type = T;
        using param_type     = meta::conditional<(sizeof(component_type) * 4) <= (sizeof(void*) * 2), color, const color&>;

        static constexpr auto layout          = color_layout::bgra;
        static constexpr auto component_count = 4;

        component_type b;
        component_type g;
        component_type r;
        component_type a;

        constexpr auto operator==(param_type other) const noexcept -> bool;
    };

    template<meta::color_component T>
    struct color<color_layout::abgr, T> {
        using component_type = T;
        using param_type     = meta::conditional<(sizeof(component_type) * 4) <= (sizeof(void*) * 2), color, const color&>;

        static constexpr auto layout          = color_layout::abgr;
        static constexpr auto component_count = 4;

        component_type a;
        component_type b;
        component_type g;
        component_type r;

        constexpr auto operator==(param_type other) const noexcept -> bool;
    };

    template<meta::color_component T>
    using color_r = color<color_layout::r, T>;
    template<meta::color_component T>
    using color_rg = color<color_layout::rg, T>;
    template<meta::color_component T>
    using color_rgb = color<color_layout::rgb, T>;
    template<meta::color_component T>
    using color_rgba = color<color_layout::rgba, T>;
    template<meta::color_component T>
    using color_argb = color<color_layout::argb, T>;
    template<meta::color_component T>
    using color_bgr = color<color_layout::bgr, T>;
    template<meta::color_component T>
    using color_bgra = color<color_layout::bgra, T>;
    template<meta::color_component T>
    using color_abgr = color<color_layout::abgr, T>;

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

    template<color_layout To_layout, meta::color_component T, color_layout From_layout, meta::color_component U>
    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<color<To_layout, T>>,
                              const color<From_layout, U>& value,
                              source_location_arg = std::source_location::current()) noexcept -> color<To_layout, T>;

    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<string_view>,
                              color_layout value,
                              source_location_arg = std::source_location::current()) noexcept -> string_view;

    template<color_layout Layout, meta::color_component T>
    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<string>,
                              const color<Layout, T>& value,
                              source_location_arg = std::source_location::current()) noexcept -> string;

    template<typename CharT, typename FormatContext, color_layout Layout, meta::color_component T>
    constexpr auto tag_invoke(format_as_fn<CharT>, const color<Layout, T>& value, FormatContext& ctx) -> decltype(ctx.out());
}} // namespace stormkit::core

namespace stormkit { inline namespace core { namespace details {
    template<color_layout Layout, meta::color_component T>
    struct implicit_color_converter {
        template<color_layout To_layout, meta::color_component U>
        constexpr operator color<To_layout, U>() const noexcept;

        color<Layout, T> c;
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
        template<meta::color_component T = u8>
        inline constexpr auto black = details::implicit_color_converter<color_layout::rgba, T> {
            .c = { .r = 0, .g = 0, .b = 0, .a = color_component_max<T> }
        };

        template<meta::color_component T = u8>
        inline constexpr auto gray = details::implicit_color_converter<color_layout::rgba, T> {
            .c = { .r = color_component_max<T> / T { 2 },
                  .g = color_component_max<T> / T { 2 },
                  .b = color_component_max<T> / T { 2 },
                  .a = color_component_max<T> }
        };

        template<meta::color_component T = u8>
        inline constexpr auto silver = details::implicit_color_converter<color_layout::rgba, T> {
            .c = { .r = color_component_max<T> / T { 2 } + color_component_max<T> / T { 4 },
                  .g = color_component_max<T> / T { 2 } + color_component_max<T> / T { 4 },
                  .b = color_component_max<T> / T { 2 } + color_component_max<T> / T { 4 },
                  .a = color_component_max<T> }
        };

        template<meta::color_component T = u8>
        inline constexpr auto white = details::implicit_color_converter<color_layout::rgba, T> {
            .c = { .r = color_component_max<T>,
                  .g = color_component_max<T>,
                  .b = color_component_max<T>,
                  .a = color_component_max<T> }
        };

        template<meta::color_component T = u8>
        inline constexpr auto maroon = details::implicit_color_converter<color_layout::rgba, T> {
            .c = { .r = color_component_max<T> / T { 2 }, .g = T { 0 }, .b = T { 0 }, .a = color_component_max<T> }
        };

        template<meta::color_component T = u8>
        inline constexpr auto red = details::implicit_color_converter<color_layout::rgba, T> {
            .c = { .r = color_component_max<T>, .g = 0, .b = 0, .a = color_component_max<T> }
        };

        template<meta::color_component T = u8>
        inline constexpr auto olive = details::implicit_color_converter<color_layout::rgba, T> {
            .c = { .r = color_component_max<T> / T { 2 },
                  .g = color_component_max<T> / T { 2 },
                  .b = T { 0 },
                  .a = color_component_max<T> }
        };

        template<meta::color_component T = u8>
        inline constexpr auto yellow = details::implicit_color_converter<color_layout::rgba, T> {
            .c = { .r = color_component_max<T>, .g = color_component_max<T>, .b = T { 0 }, .a = color_component_max<T> }
        };

        template<meta::color_component T = u8>
        inline constexpr auto green = details::implicit_color_converter<color_layout::rgba, T> {
            .c = { .r = 0, .g = color_component_max<T> / T { 2 }, .b = 0, .a = color_component_max<T> }
        };

        template<meta::color_component T = u8>
        inline constexpr auto lime = details::implicit_color_converter<color_layout::rgba, T> {
            .c = { .r = 0, .g = color_component_max<T>, .b = 0, .a = color_component_max<T> }
        };

        template<meta::color_component T = u8>
        inline constexpr auto teal = details::implicit_color_converter<color_layout::rgba, T> {
            .c = { .r = 0,
                  .g = color_component_max<T> / T { 2 },
                  .b = color_component_max<T> / T { 2 },
                  .a = color_component_max<T> }
        };

        template<meta::color_component T = u8>
        inline constexpr auto aqua = details::implicit_color_converter<color_layout::rgba, T> {
            .c = { .r = 0, .g = color_component_max<T>, .b = color_component_max<T>, .a = color_component_max<T> }
        };

        template<meta::color_component T = u8>
        inline constexpr auto navy = details::implicit_color_converter<color_layout::rgba, T> {
            .c = { .r = 0, .g = 0, .b = color_component_max<T> / T { 2 }, .a = color_component_max<T> }
        };

        template<meta::color_component T = u8>
        inline constexpr auto blue = details::implicit_color_converter<color_layout::rgba, T> {
            .c = { .r = 0, .g = 0, .b = color_component_max<T>, .a = color_component_max<T> }
        };

        template<meta::color_component T = u8>
        inline constexpr auto purple = details::implicit_color_converter<color_layout::rgba, T> {
            .c = { .r = color_component_max<T> / T { 2 },
                  .g = 0,
                  .b = color_component_max<T> / T { 2 },
                  .a = color_component_max<T> }
        };

        template<meta::color_component T = u8>
        inline constexpr auto fuschia = details::implicit_color_converter<color_layout::rgba, T> {
            .c = { .r = color_component_max<T>, .g = 0, .b = color_component_max<T>, .a = color_component_max<T> }
        };

        template<meta::color_component T = u8>
        inline constexpr auto transparent = details::implicit_color_converter<color_layout::rgba, T> {
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
    template<color_layout Layout, meta::color_component T>
    template<color_layout To_layout, meta::color_component U>
    constexpr details::implicit_color_converter<Layout, T>::operator color<To_layout, U>() const noexcept {
        if constexpr (Layout == To_layout and meta::same_as<T, U>) return c;
        else
            return as<color<To_layout, U>>(c);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::color_component T>
    STORMKIT_FORCE_INLINE
    constexpr auto color<color_layout::r, T>::operator==(param_type other) const noexcept -> bool {
        return r == other.r;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::color_component T>
    STORMKIT_FORCE_INLINE
    constexpr auto color<color_layout::rg, T>::operator==(param_type other) const noexcept -> bool {
        return r == other.r and g == other.g;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::color_component T>
    STORMKIT_FORCE_INLINE
    constexpr auto color<color_layout::rgb, T>::operator==(param_type other) const noexcept -> bool {
        return r == other.r and g == other.g and b == other.b;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::color_component T>
    STORMKIT_FORCE_INLINE
    constexpr auto color<color_layout::rgba, T>::operator==(param_type other) const noexcept -> bool {
        return r == other.r and g == other.g and b == other.b and a == other.a;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::color_component T>
    STORMKIT_FORCE_INLINE
    constexpr auto color<color_layout::argb, T>::operator==(param_type other) const noexcept -> bool {
        return r == other.r and g == other.g and b == other.b and a == other.a;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::color_component T>
    STORMKIT_FORCE_INLINE
    constexpr auto color<color_layout::bgr, T>::operator==(param_type other) const noexcept -> bool {
        return r == other.r and g == other.g and b == other.b;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::color_component T>
    STORMKIT_FORCE_INLINE
    constexpr auto color<color_layout::bgra, T>::operator==(param_type other) const noexcept -> bool {
        return r == other.r and g == other.g and b == other.b and a == other.a;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::color_component T>
    STORMKIT_FORCE_INLINE
    constexpr auto color<color_layout::abgr, T>::operator==(param_type other) const noexcept -> bool {
        return r == other.r and g == other.g and b == other.b and a == other.a;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<color_layout To_layout, meta::color_component T, color_layout From_layout, meta::color_component U>
    constexpr auto tag_invoke(as_fn<color<To_layout, T>>, const color<From_layout, U>& value, source_location_arg) noexcept
      -> color<To_layout, T> {
        static constexpr auto from_component_count = color<From_layout, U>::component_count;
        static constexpr auto to_component_count   = color<To_layout, T>::component_count;

        static constexpr auto as_component_count = [](U component) static noexcept {
            if constexpr (meta::is<T, U>) return component;
            else if constexpr (meta::is<T, u8>)
                return as<u8>(component * 255);
            else
                return as<T>(component) / T { 255 };
        };

        auto out = color<To_layout, T> {};

        if constexpr (to_component_count >= 1) out.r = as_component_count(value.r);
        if constexpr (to_component_count >= 2 and from_component_count >= 2) out.g = as_component_count(value.g);
        if constexpr (to_component_count >= 3 and from_component_count >= 3) out.b = as_component_count(value.b);
        if constexpr (to_component_count == 4) {
            if constexpr (from_component_count == 4) out.a = as_component_count(value.a);
            else
                out.a = color_component_max<T>;
        }

        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_CONST
    constexpr auto tag_invoke(as_fn<string_view>, color_layout layout, source_location_arg) noexcept -> string_view {
        switch (layout) {
            case color_layout::r: return "color_layout::r";
            case color_layout::rg: return "color_layout::rg";
            case color_layout::rgb: return "color_layout::rgb";
            case color_layout::bgr: return "color_layout::bgr";
            case color_layout::rgba: return "color_layout::rgba";
            case color_layout::argb: return "color_layout::argb";
            case color_layout::bgra: return "color_layout::bgra";
            case color_layout::abgr: return "color_layout::abgr";
            default: break;
        }
        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<color_layout Layout, meta::color_component T>
    constexpr auto tag_invoke(as_fn<string>, const color<Layout, T>& value, source_location_arg) noexcept -> string {
        static constexpr auto component_count = color<Layout, T>::component_count;

        auto&& color_u8 = [&value] noexcept {
            if constexpr (meta::is<T, u8>) return value;
            else
                return as<color<Layout, u8>>(value);
        }();

        auto color_as_int = 0xFF_u32;
        if constexpr (component_count >= 1) color_as_int |= (as<u32>(color_u8.r) << 24);
        if constexpr (component_count >= 2) color_as_int |= (as<u32>(color_u8.g) << 16);
        if constexpr (component_count >= 3) color_as_int |= (as<u32>(color_u8.b) << 8);
        if constexpr (component_count == 4) color_as_int |= (as<u32>(color_u8.a));

        auto out = std::string {};
        out.reserve(10);
        out.push_back('0');
        out.push_back('x');
        out.append(to_upper(as<string>(color_as_int, 16)));
        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename CharT, typename FormatContext, color_layout Layout, meta::color_component T>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(format_as_fn<CharT>, const color<Layout, T>& value, FormatContext& ctx) -> decltype(ctx.out()) {
        static constexpr auto component_count = color<Layout, T>::component_count;

        ctx.advance_to(std::format_to(ctx.out(), "[color layout: {}, ", Layout));

        if constexpr (component_count == 1) ctx.advance_to(std::format_to(ctx.out(), "red: {}", value.r));
        else if constexpr (component_count == 2)
            ctx.advance_to(std::format_to(ctx.out(), "red: {}, green: {}", value.r, value.g));
        else if constexpr (component_count == 3)
            ctx.advance_to(std::format_to(ctx.out(), "red: {}, green: {}, blue: {}", value.r, value.g, value.b));
        else if constexpr (component_count == 4)
            ctx.advance_to(std::
                             format_to(ctx.out(), "red: {}, green: {}, blue: {}, alpha: {}", value.r, value.g, value.b, value.a));

        return std::format_to(ctx.out(), ", hex: {}]", as<string>(value));
    }
}} // namespace stormkit::core
