module;

#include <stormkit/core/contract_macro.hpp>

#include "CppBridge.hpp"

#include <Carbon/Carbon.h>

module stormkit.wsi;

import std;
import stormkit.core;

import :macos.window;

namespace stormkit::wsi::macos {
    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_CONST
    constexpr auto mouse_button(i32 button) noexcept -> mouse_button {
        switch (button) {
            case 0: return mouse_button::left;
            case 1: return mouse_button::right;
            case 2: return mouse_button::middle;
            case 3: return mouse_button::button_1;
            case 4: return mouse_button::button_2;
            default: std::unreachable();
        }
    }

    consteval auto generate_key_array() -> decltype(auto) {
        auto out = array<key, 256> {};
        stdr::fill(out, key::unknown);

        out[0x00] = key::a;
        out[0x0B] = key::b;
        out[0x08] = key::c;
        out[0x02] = key::d;
        out[0x0E] = key::e;
        out[0x03] = key::f;
        out[0x05] = key::g;
        out[0x04] = key::h;
        out[0x22] = key::i;
        out[0x26] = key::j;
        out[0x28] = key::k;
        out[0x25] = key::l;
        out[0x2E] = key::m;
        out[0x2D] = key::n;
        out[0x1F] = key::o;
        out[0x23] = key::p;
        out[0x0C] = key::q;
        out[0x0F] = key::r;
        out[0x01] = key::s;
        out[0x11] = key::t;
        out[0x20] = key::u;
        out[0x09] = key::v;
        out[0x0D] = key::w;
        out[0x07] = key::x;
        out[0x10] = key::y;
        out[0x06] = key::z;

        out[0x1D] = key::num_0;
        out[0x12] = key::num_1;
        out[0x13] = key::num_2;
        out[0x14] = key::num_3;
        out[0x15] = key::num_4;
        out[0x17] = key::num_5;
        out[0x16] = key::num_6;
        out[0x1A] = key::num_7;
        out[0x1C] = key::num_8;
        out[0x19] = key::num_9;

        out[0x7B] = key::left;
        out[0x7C] = key::right;
        out[0x7E] = key::up;
        out[0x7D] = key::down;

        out[0x3B] = key::l_control;
        out[0x38] = key::l_shift;
        out[0x3A] = key::l_alt;
        out[0x37] = key::l_meta;
        out[0x3E] = key::r_control;
        out[0x3C] = key::r_shift;
        out[0x3D] = key::r_alt;
        out[0x36] = key::r_meta;

        out[0x35] = key::escape;
        out[0x30] = key::tab;
        out[0x6E] = key::menu;

        out[0x27] = key::quote;
        out[0x2A] = key::back_slash;
        out[0x2B] = key::comma;
        out[0x18] = key::equal;

        out[0x32] = key::grave_accent;
        out[0x21] = key::l_bracket;
        out[0x1B] = key::minus;
        out[0x2F] = key::period;
        out[0x1E] = key::r_bracket;
        out[0x29] = key::semi_colon;
        out[0x2C] = key::slash;

        out[0x0A] = key::iso;

        out[0x33] = key::back_space;
        out[0x39] = key::caps_lock;
        out[0x24] = key::enter;
        out[0x31] = key::space;

        out[0x7A] = key::f1;
        out[0x78] = key::f2;
        out[0x63] = key::f3;
        out[0x76] = key::f4;
        out[0x60] = key::f5;
        out[0x61] = key::f6;
        out[0x62] = key::f7;
        out[0x64] = key::f8;
        out[0x65] = key::f9;
        out[0x6D] = key::f10;
        out[0x67] = key::f11;
        out[0x6F] = key::f12;
        out[0x6B] = key::f14;
        out[0x71] = key::f15;
        out[0x6A] = key::f16;
        out[0x40] = key::f17;
        out[0x4F] = key::f18;
        out[0x50] = key::f19;
        out[0x5A] = key::f20;

        out[0x69] = key::print_screen;

        out[0x72] = key::insert;
        out[0x75] = key::del;
        out[0x73] = key::home;
        out[0x77] = key::end;
        out[0x79] = key::page_down;
        out[0x74] = key::page_up;

        out[0x47] = key::numpad_lock;
        out[0x45] = key::numpad_add;
        out[0x41] = key::numpad_decimal;
        out[0x4B] = key::numpad_divide;
        out[0x4C] = key::numpad_enter;
        out[0x51] = key::numpad_equal;
        out[0x43] = key::numpad_multiply;
        out[0x4E] = key::numpad_subtract;
        out[0x52] = key::numpad_0;
        out[0x53] = key::numpad_1;
        out[0x54] = key::numpad_2;
        out[0x55] = key::numpad_3;
        out[0x56] = key::numpad_4;
        out[0x57] = key::numpad_5;
        out[0x58] = key::numpad_6;
        out[0x59] = key::numpad_7;
        out[0x5B] = key::numpad_8;
        out[0x5C] = key::numpad_9;

        return out;
    }

    consteval auto generate_scancode_array(array_view<const key, 256> keys) -> decltype(auto) {
        auto out = array<u8, 256> {};
        stdr::fill(out, 0);

        for (auto i : range(256_u8)) {
            const auto key  = keys[i];
            const auto _key = unchecked_narrow<usize>(key);
            if (key != key::unknown) out[_key] = i;
        }

        return out;
    }

    namespace {
        constexpr auto scancode_as_key = generate_key_array();
        [[maybe_unused]]
        constexpr auto key_as_scancode = generate_scancode_array(scancode_as_key);
    } // namespace
} // namespace stormkit::wsi::macos

using namespace stormkit;
using namespace stormkit::wsi;
using namespace stormkit::wsi::macos;

extern "C" {
    /////////////////////////////////////
    /////////////////////////////////////
    auto swiftClosedEvent(id ptr) noexcept -> bool {
        EXPECTS(ptr != 0);

        auto& window = *reinterpret_cast<stormkit::wsi::macos::window*>(ptr);

        const auto closed = window.closed_event();
        if (closed) window.close();

        return closed;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto swiftResizedEvent(id ptr, float width, float height) noexcept -> void {
        EXPECTS(ptr != 0);

        auto& window = *reinterpret_cast<stormkit::wsi::macos::window*>(ptr);
        window.window_base::set_extent(math::extent2 { width, height }.to<u32>());

        window.resized_event(window.extent());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto swiftRestoredEvent(id ptr) noexcept -> void {
        EXPECTS(ptr != 0);

        auto& window = *reinterpret_cast<stormkit::wsi::macos::window*>(ptr);

        window.restored_event();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto swiftMinimizedEvent(id ptr) noexcept -> void {
        EXPECTS(ptr != 0);

        auto& window = *reinterpret_cast<stormkit::wsi::macos::window*>(ptr);

        window.minimized_event();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto swiftActivatedEvent(id ptr) noexcept -> void {
        EXPECTS(ptr != 0);

        auto& window = *reinterpret_cast<stormkit::wsi::macos::window*>(ptr);

        window.activate_event();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto swiftDeactivatedEvent(id ptr) noexcept -> void {
        EXPECTS(ptr != 0);

        auto& window = *reinterpret_cast<stormkit::wsi::macos::window*>(ptr);

        window.deactivate_event();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto swiftMouseDownEvent(id ptr, i32 button, i32 x, i32 y) noexcept -> void {
        EXPECTS(ptr != 0);

        auto&      window                        = *reinterpret_cast<stormkit::wsi::macos::window*>(ptr);
        auto&      state                         = window.mouse_state(global_mouse_id);
        const auto _button                       = mouse_button(button);
        state.buttons[common::as_index(_button)] = common::button_state::down;

        window.mouse_button_down_event(global_mouse_id, _button, math::vec2 { x, y }.to<i32>());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto swiftMouseUpEvent(id ptr, i32 button, i32 x, i32 y) noexcept -> void {
        EXPECTS(ptr != 0);

        auto&      window                        = *reinterpret_cast<stormkit::wsi::macos::window*>(ptr);
        auto&      state                         = window.mouse_state(global_mouse_id);
        const auto _button                       = mouse_button(button);
        state.buttons[common::as_index(_button)] = common::button_state::up;
        state.last_position                      = math::vec2 { x, y }.to<u32>();

        window.mouse_button_up_event(global_mouse_id, _button, math::vec2 { x, y }.to<i32>());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto swiftMouseMovedEvent(id ptr, i32 x, i32 y) noexcept -> void {
        EXPECTS(ptr != 0);

        auto& window        = *reinterpret_cast<stormkit::wsi::macos::window*>(ptr);
        auto& state         = window.mouse_state(global_mouse_id);
        state.last_position = math::vec2 { x, y }.to<u32>();

        window.mouse_moved_event(global_mouse_id, state.last_position.to<i32>());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto swiftkeyDownEvent(id ptr, u16 scancode, char c) noexcept -> void {
        EXPECTS(ptr != 0);

        const auto key = scancode_as_key[scancode];
        if (key == key::unknown) [[unlikely]]
            return;
        auto& window                      = *reinterpret_cast<stormkit::wsi::macos::window*>(ptr);
        auto& state                       = window.keyboard_state(global_keyboard_id);
        state.keys[common::as_index(key)] = common::key_state::down;

        window.key_down_event(global_keyboard_id, key, c);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto swiftkeyUpEvent(id ptr, u16 scancode, char c) noexcept -> void {
        EXPECTS(ptr != 0);

        const auto key = scancode_as_key[scancode];
        if (key == key::unknown) [[unlikely]]
            return;

        auto& window                      = *reinterpret_cast<stormkit::wsi::macos::window*>(ptr);
        auto& state                       = window.keyboard_state(global_keyboard_id);
        state.keys[common::as_index(key)] = common::key_state::up;

        window.key_up_event(global_keyboard_id, key, c);
    }
}
