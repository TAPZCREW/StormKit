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
    constexpr auto mouse_button(i32 button) noexcept -> MouseButton {
        switch (button) {
            case 0: return MouseButton::LEFT;
            case 1: return MouseButton::RIGHT;
            case 2: return MouseButton::MIDDLE;
            case 3: return MouseButton::BUTTON_1;
            case 4: return MouseButton::BUTTON_2;
            default: std::unreachable();
        }
    }

    consteval auto generate_key_array() -> decltype(auto) {
        auto out = array<Key, 256> {};
        stdr::fill(out, Key::UNKNOWN);

        out[0x00] = Key::A;
        out[0x0B] = Key::B;
        out[0x08] = Key::C;
        out[0x02] = Key::D;
        out[0x0E] = Key::E;
        out[0x03] = Key::F;
        out[0x05] = Key::G;
        out[0x04] = Key::H;
        out[0x22] = Key::I;
        out[0x26] = Key::J;
        out[0x28] = Key::K;
        out[0x25] = Key::L;
        out[0x2E] = Key::M;
        out[0x2D] = Key::N;
        out[0x1F] = Key::O;
        out[0x23] = Key::P;
        out[0x0C] = Key::Q;
        out[0x0F] = Key::R;
        out[0x01] = Key::S;
        out[0x11] = Key::T;
        out[0x20] = Key::U;
        out[0x09] = Key::V;
        out[0x0D] = Key::W;
        out[0x07] = Key::X;
        out[0x10] = Key::Y;
        out[0x06] = Key::Z;

        out[0x1D] = Key::NUM_0;
        out[0x12] = Key::NUM_1;
        out[0x13] = Key::NUM_2;
        out[0x14] = Key::NUM_3;
        out[0x15] = Key::NUM_4;
        out[0x17] = Key::NUM_5;
        out[0x16] = Key::NUM_6;
        out[0x1A] = Key::NUM_7;
        out[0x1C] = Key::NUM_8;
        out[0x19] = Key::NUM_9;

        out[0x7B] = Key::LEFT;
        out[0x7C] = Key::RIGHT;
        out[0x7E] = Key::UP;
        out[0x7D] = Key::DOWN;

        out[0x3B] = Key::L_CONTROL;
        out[0x38] = Key::L_SHIFT;
        out[0x3A] = Key::L_ALT;
        out[0x37] = Key::L_META;
        out[0x3E] = Key::R_CONTROL;
        out[0x3C] = Key::R_SHIFT;
        out[0x3D] = Key::R_ALT;
        out[0x36] = Key::R_META;

        out[0x35] = Key::ESCAPE;
        out[0x30] = Key::TAB;
        out[0x6E] = Key::MENU;

        out[0x27] = Key::QUOTE;
        out[0x2A] = Key::BACK_SLASH;
        out[0x2B] = Key::COMMA;
        out[0x18] = Key::EQUAL;

        out[0x32] = Key::GRAVE_ACCENT;
        out[0x21] = Key::L_BRACKET;
        out[0x1B] = Key::MINUS;
        out[0x2F] = Key::PERIOD;
        out[0x1E] = Key::R_BRACKET;
        out[0x29] = Key::SEMI_COLON;
        out[0x2C] = Key::SLASH;

        out[0x0A] = Key::ISO;

        out[0x33] = Key::BACK_SPACE;
        out[0x39] = Key::CAPS_LOCK;
        out[0x24] = Key::ENTER;
        out[0x31] = Key::SPACE;

        out[0x7A] = Key::F1;
        out[0x78] = Key::F2;
        out[0x63] = Key::F3;
        out[0x76] = Key::F4;
        out[0x60] = Key::F5;
        out[0x61] = Key::F6;
        out[0x62] = Key::F7;
        out[0x64] = Key::F8;
        out[0x65] = Key::F9;
        out[0x6D] = Key::F10;
        out[0x67] = Key::F11;
        out[0x6F] = Key::F12;
        out[0x6B] = Key::F14;
        out[0x71] = Key::F15;
        out[0x6A] = Key::F16;
        out[0x40] = Key::F17;
        out[0x4F] = Key::F18;
        out[0x50] = Key::F19;
        out[0x5A] = Key::F20;

        out[0x69] = Key::PRINT_SCREEN;

        out[0x72] = Key::INSERT;
        out[0x75] = Key::DELETE;
        out[0x73] = Key::HOME;
        out[0x77] = Key::END;
        out[0x79] = Key::PAGE_DOWN;
        out[0x74] = Key::PAGE_UP;

        out[0x47] = Key::NUMPAD_LOCK;
        out[0x45] = Key::NUMPAD_ADD;
        out[0x41] = Key::NUMPAD_DECIMAL;
        out[0x4B] = Key::NUMPAD_DIVIDE;
        out[0x4C] = Key::NUMPAD_ENTER;
        out[0x51] = Key::NUMPAD_EQUAL;
        out[0x43] = Key::NUMPAD_MULTIPLY;
        out[0x4E] = Key::NUMPAD_SUBTRACT;
        out[0x52] = Key::NUMPAD_0;
        out[0x53] = Key::NUMPAD_1;
        out[0x54] = Key::NUMPAD_2;
        out[0x55] = Key::NUMPAD_3;
        out[0x56] = Key::NUMPAD_4;
        out[0x57] = Key::NUMPAD_5;
        out[0x58] = Key::NUMPAD_6;
        out[0x59] = Key::NUMPAD_7;
        out[0x5B] = Key::NUMPAD_8;
        out[0x5C] = Key::NUMPAD_9;

        return out;
    }

    consteval auto generate_scancode_array(array_view<const Key, 256> keys) -> decltype(auto) {
        auto out = array<u8, 256> {};
        stdr::fill(out, 0);

        for (auto i : range(256_u8)) {
            const auto key  = keys[i];
            const auto _key = unchecked_narrow<usize>(key);
            if (key != Key::UNKNOWN) out[_key] = i;
        }

        return out;
    }

    namespace {
        constexpr auto SCANCODE_AS_KEY = generate_key_array();
        [[maybe_unused]]
        constexpr auto KEY_AS_SCANCODE = generate_scancode_array(SCANCODE_AS_KEY);
    } // namespace
} // namespace stormkit::wsi::macos

using namespace stormkit;
using namespace stormkit::wsi;
using namespace stormkit::wsi::macos;
extern "C" {
    /////////////////////////////////////
    /////////////////////////////////////
    auto swiftClosedEvent(ID ptr) noexcept -> bool {
        EXPECTS(ptr != 0);

        auto& window = *std::bit_cast<stormkit::wsi::macos::Window*>(ptr);

        const auto closed = window.closed_event();
        if (closed) window.close();

        return closed;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto swiftResizedEvent(ID ptr, float width, float height) noexcept -> void {
        EXPECTS(ptr != 0);

        auto& window = *std::bit_cast<stormkit::wsi::macos::Window*>(ptr);
        window.WindowBase::set_extent(math::extent2 { width, height }.to<u32>());

        window.resized_event(window.extent());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto swiftRestoredEvent(ID ptr) noexcept -> void {
        EXPECTS(ptr != 0);

        auto& window = *std::bit_cast<stormkit::wsi::macos::Window*>(ptr);

        window.restored_event();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto swiftMinimizedEvent(ID ptr) noexcept -> void {
        EXPECTS(ptr != 0);

        auto& window = *std::bit_cast<stormkit::wsi::macos::Window*>(ptr);

        window.minimized_event();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto swiftActivatedEvent(ID ptr) noexcept -> void {
        EXPECTS(ptr != 0);

        auto& window = *std::bit_cast<stormkit::wsi::macos::Window*>(ptr);

        window.activate_event();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto swiftDeactivatedEvent(ID ptr) noexcept -> void {
        EXPECTS(ptr != 0);

        auto& window = *std::bit_cast<stormkit::wsi::macos::Window*>(ptr);

        window.deactivate_event();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto swiftMouseDownEvent(ID ptr, i32 button, i32 x, i32 y) noexcept -> void {
        EXPECTS(ptr != 0);

        auto&      window  = *std::bit_cast<stormkit::wsi::macos::Window*>(ptr);
        auto&      state   = window.mouse_state(GLOBAL_MOUSE_ID);
        const auto _button = mouse_button(button);
        state.buttons[common::as_index(_button)] = common::ButtonState::DOWN;

        window.mouse_button_down_event(GLOBAL_MOUSE_ID, _button, math::vec2 { x, y }.to<i32>());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto swiftMouseUpEvent(ID ptr, i32 button, i32 x, i32 y) noexcept -> void {
        EXPECTS(ptr != 0);

        auto&      window  = *std::bit_cast<stormkit::wsi::macos::Window*>(ptr);
        auto&      state   = window.mouse_state(GLOBAL_MOUSE_ID);
        const auto _button = mouse_button(button);
        state.buttons[common::as_index(_button)] = common::ButtonState::UP;
        state.last_position                      = math::vec2 { x, y }.to<u32>();

        window.mouse_button_up_event(GLOBAL_MOUSE_ID, _button, math::vec2 { x, y }.to<i32>());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto swiftMouseMovedEvent(ID ptr, i32 x, i32 y) noexcept -> void {
        EXPECTS(ptr != 0);

        auto& window        = *std::bit_cast<stormkit::wsi::macos::Window*>(ptr);
        auto& state         = window.mouse_state(GLOBAL_MOUSE_ID);
        state.last_position = math::vec2 { x, y }.to<u32>();

        window.mouse_moved_event(GLOBAL_MOUSE_ID, state.last_position.to<i32>());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto swiftKeyDownEvent(ID ptr, u16 scancode, char c) noexcept -> void {
        EXPECTS(ptr != 0);

        const auto key = SCANCODE_AS_KEY[scancode];
        if (key == Key::UNKNOWN) [[unlikely]]
            return;
        auto& window                      = *std::bit_cast<stormkit::wsi::macos::Window*>(ptr);
        auto& state                       = window.keyboard_state(GLOBAL_KEYBOARD_ID);
        state.keys[common::as_index(key)] = common::KeyState::DOWN;

        window.key_down_event(GLOBAL_KEYBOARD_ID, key, c);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto swiftKeyUpEvent(ID ptr, u16 scancode, char c) noexcept -> void {
        EXPECTS(ptr != 0);

        const auto key = SCANCODE_AS_KEY[scancode];
        if (key == Key::UNKNOWN) [[unlikely]]
            return;

        auto& window                      = *std::bit_cast<stormkit::wsi::macos::Window*>(ptr);
        auto& state                       = window.keyboard_state(GLOBAL_KEYBOARD_ID);
        state.keys[common::as_index(key)] = common::KeyState::UP;

        window.key_up_event(GLOBAL_KEYBOARD_ID, key, c);
    }
}
