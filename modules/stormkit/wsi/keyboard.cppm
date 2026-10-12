// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.wsi:keyboard;

import std;

import stormkit.core;

using namespace stormkit::literals;

export namespace stormkit::wsi {
    inline constexpr auto global_keyboard_id = 0_u8;

    enum class key : u8 {
        a = 0,
        b,
        c,
        d,
        e,
        f,
        g,
        h,
        i,
        j,
        k,
        l,
        m,
        n,
        o,
        p,
        q,
        r,
        s,
        t,
        u,
        v,
        w,
        x,
        y,
        z,
        num_0,
        num_1,
        num_2,
        num_3,
        num_4,
        num_5,
        num_6,
        num_7,
        num_8,
        num_9,

        left,
        right,
        up,
        down,

        l_control,
        l_shift,
        l_alt,
        l_meta,
        r_control,
        r_shift,
        r_alt,
        r_meta,

        escape,
        tab,
        menu,

        quote,
        back_slash,
        comma,
        equal,

        grave_accent,
        l_bracket,
        minus,
        period,
        r_bracket,
        semi_colon,
        slash,

        iso,

        back_space,
        caps_lock,
        enter,
        space,

        f1,
        f2,
        f3,
        f4,
        f5,
        f6,
        f7,
        f8,
        f9,
        f10,
        f11,
        f12,
        f13,
        f14,
        f15,
        f16,
        f17,
        f18,
        f19,
        f20,

        print_screen,

        insert,
        del,
        home,
        end,
        page_up,
        page_down,

        numpad_lock,
        numpad_add,
        numpad_decimal,
        numpad_divide,
        numpad_enter,
        numpad_equal,
        numpad_multiply,
        numpad_subtract,
        numpad_0,
        numpad_1,
        numpad_2,
        numpad_3,
        numpad_4,
        numpad_5,
        numpad_6,
        numpad_7,
        numpad_8,
        numpad_9,

        unknown = std::numeric_limits<u8>::max(),
    };

    [[nodiscard]]
    constexpr auto tag_invoke(as_fn<string_view>, key key, const std::source_location&) noexcept -> string_view;
} // namespace stormkit::wsi

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::wsi {
    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto tag_invoke(as_fn<string_view>, key key, const std::source_location&) noexcept -> string_view {
        switch (key) {
            case key::a: return "key::a";
            case key::b: return "key::b";
            case key::c: return "key::c";
            case key::d: return "key::d";
            case key::e: return "key::e";
            case key::f: return "key::f";
            case key::g: return "key::g";
            case key::h: return "key::h";
            case key::i: return "key::i";
            case key::j: return "key::j";
            case key::k: return "key::k";
            case key::l: return "key::l";
            case key::m: return "key::m";
            case key::n: return "key::n";
            case key::o: return "key::o";
            case key::p: return "key::p";
            case key::q: return "key::q";
            case key::r: return "key::r";
            case key::s: return "key::s";
            case key::t: return "key::t";
            case key::u: return "key::u";
            case key::v: return "key::v";
            case key::w: return "key::w";
            case key::x: return "key::x";
            case key::y: return "key::y";
            case key::z: return "key::z";
            case key::num_0: return "key::num_0";
            case key::num_1: return "key::num_1";
            case key::num_2: return "key::num_2";
            case key::num_3: return "key::num_3";
            case key::num_4: return "key::num_4";
            case key::num_5: return "key::num_5";
            case key::num_6: return "key::num_6";
            case key::num_7: return "key::num_7";
            case key::num_8: return "key::num_8";
            case key::num_9: return "key::num_9";

            case key::left: return "key::left";
            case key::right: return "key::right";
            case key::up: return "key::up";
            case key::down: return "key::down";

            case key::l_control: return "key::l_control";
            case key::l_shift: return "key::l_shift";
            case key::l_alt: return "key::l_alt";
            case key::l_meta: return "key::l_meta";
            case key::r_control: return "key::r_control";
            case key::r_shift: return "key::r_shift";
            case key::r_alt: return "key::r_alt";
            case key::r_meta: return "key::r_meta";

            case key::escape: return "key::escape";
            case key::tab: return "key::tab";
            case key::menu: return "key::menu";

            case key::quote: return "key::quote";
            case key::back_slash: return "key::back_slash";
            case key::comma: return "key::comma";
            case key::equal: return "key::equal";

            case key::grave_accent: return "key::grave_accent";
            case key::l_bracket: return "key::l_bracket";
            case key::minus: return "key::minus";
            case key::period: return "key::period";
            case key::r_bracket: return "key::r_bracket";
            case key::semi_colon: return "key::semi_colon";
            case key::slash: return "key::slash";

            case key::iso: return "key::iso";

            case key::back_space: return "key::back_space";
            case key::caps_lock: return "key::caps_lock";
            case key::enter: return "key::enter";
            case key::space: return "key::space";

            case key::f1: return "key::f1";
            case key::f2: return "key::f2";
            case key::f3: return "key::f3";
            case key::f4: return "key::f4";
            case key::f5: return "key::f5";
            case key::f6: return "key::f6";
            case key::f7: return "key::f7";
            case key::f8: return "key::f8";
            case key::f9: return "key::f9";
            case key::f10: return "key::f10";
            case key::f11: return "key::f11";
            case key::f12: return "key::f12";
            case key::f13: return "key::f13";
            case key::f14: return "key::f14";
            case key::f15: return "key::f15";
            case key::f16: return "key::f16";
            case key::f17: return "key::f17";
            case key::f18: return "key::f18";
            case key::f19: return "key::f19";
            case key::f20: return "key::f20";

            case key::print_screen: return "key::print_screen";

            case key::insert: return "key::insert";
            case key::del: return "key::del";
            case key::home: return "key::home";
            case key::end: return "key::end";
            case key::page_up: return "key::page_up";
            case key::page_down: return "key::page_down";

            case key::numpad_lock: return "key::numpad_lock";
            case key::numpad_add: return "key::numpad_add";
            case key::numpad_decimal: return "key::numpad_decimal";
            case key::numpad_divide: return "key::numpad_divide";
            case key::numpad_enter: return "key::numpad_enter";
            case key::numpad_equal: return "key::numpad_equal";
            case key::numpad_multiply: return "key::numpad_multiply";
            case key::numpad_subtract: return "key::numpad_subtract";
            case key::numpad_0: return "key::numpad_0";
            case key::numpad_1: return "key::numpad_1";
            case key::numpad_2: return "key::numpad_2";
            case key::numpad_3: return "key::numpad_3";
            case key::numpad_4: return "key::numpad_4";
            case key::numpad_5: return "key::numpad_5";
            case key::numpad_6: return "key::numpad_6";
            case key::numpad_7: return "key::numpad_7";
            case key::numpad_8: return "key::numpad_8";
            case key::numpad_9: return "key::numpad_9";
            case key::unknown: return "key::unknown";
            default: break;
        }
        std::unreachable();
    }
} // namespace stormkit::wsi
