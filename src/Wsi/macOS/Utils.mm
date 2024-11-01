// Copyright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#include "Utils.hpp"

#include <cstdint>
#include <utility>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#import <AppKit/NSEvent.h>

#import <Carbon/Carbon.h>

#import <IOKit/hid/IOHIDDevice.h>
#import <IOKit/hid/IOHIDManager.h>

namespace stormkit::wsi::macos {
    enum class Key {
        A = 0,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        Num0,
        Num1,
        Num2,
        Num3,
        Num4,
        Num5,
        Num6,
        Num7,
        Num8,
        Num9,
        Escape,
        L_Control,
        L_Shift,
        L_Alt,
        L_System,
        R_Control,
        R_Shift,
        R_Alt,
        R_System,
        Menu,
        L_Bracket,
        R_Bracket,
        Semi_Colon,
        Comma,
        Period,
        Quote,
        Slash,
        Back_Slash,
        Tilde,
        Equal,
        Hyphen,
        Space,
        Enter,
        Back_Space,
        Tab,
        Page_Up,
        Page_Down,
        Begin,
        End,
        Home,
        Insert,
        Delete,
        Add,
        Substract,
        Multiply,
        Divide,
        Left,
        Right,
        Up,
        Down,
        Numpad0,
        Numpad1,
        Numpad2,
        Numpad3,
        Numpad4,
        Numpad5,
        Numpad6,
        Numpad7,
        Numpad8,
        Numpad9,
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        F13,
        F14,
        F15,
        Pause,
        KeyCount,
        Unknow = std::numeric_limits<std::uint8_t>::max(),
    };

    enum class MouseButton {
        Left = 0,
        Right,
        Middle,
        Button1,
        Button2,
        ButtonCount,
        Unknow = std::numeric_limits<std::uint8_t>::max(),
    };
    /////////////////////////////////////
    /////////////////////////////////////
    auto mouseButton(int button) noexcept -> int {
        switch (button) {
            case 0: return static_cast<int>(MouseButton::Left);
            case 1: return static_cast<int>(MouseButton::Right);
            case 2: return static_cast<int>(MouseButton::Middle);
            case 3: return static_cast<int>(MouseButton::Button1);
            case 4: return static_cast<int>(MouseButton::Button2);
            default: return static_cast<int>(MouseButton::Unknow);
        }

        std::unreachable();

        return static_cast<int>(MouseButton::Unknow);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto localizedKey(char code) noexcept -> int {
        switch (code) {
            case 'a': [[fallthrough]];
            case 'A': return static_cast<int>(Key::A);
            case 'b': [[fallthrough]];
            case 'B': return static_cast<int>(Key::B);
            case 'c': [[fallthrough]];
            case 'C': return static_cast<int>(Key::C);
            case 'd': [[fallthrough]];
            case 'D': return static_cast<int>(Key::D);
            case 'e': [[fallthrough]];
            case 'E': return static_cast<int>(Key::E);
            case 'f': [[fallthrough]];
            case 'F': return static_cast<int>(Key::F);
            case 'g': [[fallthrough]];
            case 'G': return static_cast<int>(Key::G);
            case 'h': [[fallthrough]];
            case 'H': return static_cast<int>(Key::H);
            case 'i': [[fallthrough]];
            case 'I': return static_cast<int>(Key::I);
            case 'j': [[fallthrough]];
            case 'J': return static_cast<int>(Key::J);
            case 'k': [[fallthrough]];
            case 'K': return static_cast<int>(Key::K);
            case 'l': [[fallthrough]];
            case 'L': return static_cast<int>(Key::L);
            case 'm': [[fallthrough]];
            case 'M': return static_cast<int>(Key::M);
            case 'n': [[fallthrough]];
            case 'N': return static_cast<int>(Key::N);
            case 'o': [[fallthrough]];
            case 'O': return static_cast<int>(Key::O);
            case 'p': [[fallthrough]];
            case 'P': return static_cast<int>(Key::P);
            case 'q': [[fallthrough]];
            case 'Q': return static_cast<int>(Key::Q);
            case 'r': [[fallthrough]];
            case 'R': return static_cast<int>(Key::R);
            case 's': [[fallthrough]];
            case 'S': return static_cast<int>(Key::S);
            case 't': [[fallthrough]];
            case 'T': return static_cast<int>(Key::T);
            case 'u': [[fallthrough]];
            case 'U': return static_cast<int>(Key::U);
            case 'v': [[fallthrough]];
            case 'V': return static_cast<int>(Key::V);
            case 'w': [[fallthrough]];
            case 'W': return static_cast<int>(Key::W);
            case 'x': [[fallthrough]];
            case 'X': return static_cast<int>(Key::X);
            case 'y': [[fallthrough]];
            case 'Y': return static_cast<int>(Key::Y);
            case 'z': [[fallthrough]];
            case 'Z': return static_cast<int>(Key::Z);
        }

        return static_cast<int>(Key::Unknow);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto nonLocalizedKey(unsigned short code) noexcept -> int {
        switch (code) {
            case kVK_ANSI_A: return static_cast<int>(Key::A);
            case kVK_ANSI_B: return static_cast<int>(Key::B);
            case kVK_ANSI_C: return static_cast<int>(Key::C);
            case kVK_ANSI_D: return static_cast<int>(Key::D);
            case kVK_ANSI_E: return static_cast<int>(Key::E);
            case kVK_ANSI_F: return static_cast<int>(Key::F);
            case kVK_ANSI_G: return static_cast<int>(Key::G);
            case kVK_ANSI_H: return static_cast<int>(Key::H);
            case kVK_ANSI_I: return static_cast<int>(Key::I);
            case kVK_ANSI_J: return static_cast<int>(Key::J);
            case kVK_ANSI_K: return static_cast<int>(Key::K);
            case kVK_ANSI_L: return static_cast<int>(Key::L);
            case kVK_ANSI_M: return static_cast<int>(Key::M);
            case kVK_ANSI_N: return static_cast<int>(Key::N);
            case kVK_ANSI_O: return static_cast<int>(Key::O);
            case kVK_ANSI_P: return static_cast<int>(Key::P);
            case kVK_ANSI_Q: return static_cast<int>(Key::Q);
            case kVK_ANSI_R: return static_cast<int>(Key::R);
            case kVK_ANSI_S: return static_cast<int>(Key::S);
            case kVK_ANSI_T: return static_cast<int>(Key::T);
            case kVK_ANSI_U: return static_cast<int>(Key::U);
            case kVK_ANSI_V: return static_cast<int>(Key::V);
            case kVK_ANSI_W: return static_cast<int>(Key::W);
            case kVK_ANSI_X: return static_cast<int>(Key::X);
            case kVK_ANSI_Y: return static_cast<int>(Key::Y);
            case kVK_ANSI_Z: return static_cast<int>(Key::Z);

            case kVK_ANSI_0: return static_cast<int>(Key::Num0);
            case kVK_ANSI_1: return static_cast<int>(Key::Num1);
            case kVK_ANSI_2: return static_cast<int>(Key::Num2);
            case kVK_ANSI_3: return static_cast<int>(Key::Num3);
            case kVK_ANSI_4: return static_cast<int>(Key::Num4);
            case kVK_ANSI_5: return static_cast<int>(Key::Num5);
            case kVK_ANSI_6: return static_cast<int>(Key::Num6);
            case kVK_ANSI_7: return static_cast<int>(Key::Num7);
            case kVK_ANSI_8: return static_cast<int>(Key::Num8);
            case kVK_ANSI_9: return static_cast<int>(Key::Num9);

            case kVK_ANSI_Keypad0: return static_cast<int>(Key::Numpad0);
            case kVK_ANSI_Keypad1: return static_cast<int>(Key::Numpad1);
            case kVK_ANSI_Keypad2: return static_cast<int>(Key::Numpad2);
            case kVK_ANSI_Keypad3: return static_cast<int>(Key::Numpad3);
            case kVK_ANSI_Keypad4: return static_cast<int>(Key::Numpad4);
            case kVK_ANSI_Keypad5: return static_cast<int>(Key::Numpad5);
            case kVK_ANSI_Keypad6: return static_cast<int>(Key::Numpad6);
            case kVK_ANSI_Keypad7: return static_cast<int>(Key::Numpad7);
            case kVK_ANSI_Keypad8: return static_cast<int>(Key::Numpad8);
            case kVK_ANSI_Keypad9: return static_cast<int>(Key::Numpad9);

            case kVK_ANSI_KeypadPlus: return static_cast<int>(Key::Add);
            case kVK_ANSI_KeypadMinus: return static_cast<int>(Key::Substract);
            case kVK_ANSI_KeypadMultiply: return static_cast<int>(Key::Multiply);
            case kVK_ANSI_KeypadDivide: return static_cast<int>(Key::Divide);
            case kVK_ANSI_KeypadEquals:
            case kVK_ANSI_Equal: return static_cast<int>(Key::Equal);

            case kVK_ANSI_Slash: return static_cast<int>(Key::Slash);
            case kVK_ANSI_Backslash: return static_cast<int>(Key::Back_Slash);
            case kVK_ANSI_Minus: return static_cast<int>(Key::Hyphen);
            case kVK_ANSI_KeypadDecimal:
            case kVK_ANSI_Period: return static_cast<int>(Key::Period);
            case kVK_ANSI_Comma: return static_cast<int>(Key::Comma);
            case kVK_ANSI_Semicolon: return static_cast<int>(Key::Semi_Colon);
            case kVK_ANSI_Quote: return static_cast<int>(Key::Quote);
            case kVK_ANSI_LeftBracket: return static_cast<int>(Key::L_Bracket);
            case kVK_ANSI_RightBracket: return static_cast<int>(Key::R_Bracket);
            case kVK_Tab: return static_cast<int>(Key::Tab);
            case kVK_ANSI_KeypadEnter:
            case kVK_Return: return static_cast<int>(Key::Enter);
            case kVK_Space: return static_cast<int>(Key::Space);
            case kVK_ISO_Section: return static_cast<int>(Key::Tilde);

            case NSF1FunctionKey:
            case kVK_F1: return static_cast<int>(Key::F1);
            case NSF2FunctionKey:
            case kVK_F2: return static_cast<int>(Key::F2);
            case NSF3FunctionKey:
            case kVK_F3: return static_cast<int>(Key::F3);
            case NSF4FunctionKey:
            case kVK_F4: return static_cast<int>(Key::F4);
            case NSF5FunctionKey:
            case kVK_F5: return static_cast<int>(Key::F5);
            case NSF6FunctionKey:
            case kVK_F6: return static_cast<int>(Key::F6);
            case NSF7FunctionKey:
            case kVK_F7: return static_cast<int>(Key::F7);
            case NSF8FunctionKey:
            case kVK_F8: return static_cast<int>(Key::F8);
            case NSF9FunctionKey:
            case kVK_F9: return static_cast<int>(Key::F9);
            case NSF10FunctionKey:
            case kVK_F10: return static_cast<int>(Key::F10);
            case NSF11FunctionKey:
            case kVK_F11: return static_cast<int>(Key::F11);
            case NSF12FunctionKey:
            case kVK_F12: return static_cast<int>(Key::F12);
            case NSF13FunctionKey:
            case kVK_F13: return static_cast<int>(Key::F13);
            case NSF14FunctionKey:
            case kVK_F14: return static_cast<int>(Key::F14);
            case NSF15FunctionKey:
            case kVK_F15: return static_cast<int>(Key::F15);

            case NSUpArrowFunctionKey:
            case kVK_UpArrow: return static_cast<int>(Key::Up);
            case NSDownArrowFunctionKey:
            case kVK_DownArrow: return static_cast<int>(Key::Down);
            case NSLeftArrowFunctionKey:
            case kVK_LeftArrow: return static_cast<int>(Key::Left);
            case NSRightArrowFunctionKey:
            case kVK_RightArrow: return static_cast<int>(Key::Right);

            case kVK_Control: return static_cast<int>(Key::L_Control);
            case kVK_RightControl: return static_cast<int>(Key::R_Control);
            case kVK_Shift: return static_cast<int>(Key::L_Shift);
            case kVK_RightShift: return static_cast<int>(Key::R_Shift);
            case kVK_Option: return static_cast<int>(Key::L_Alt);
            case kVK_RightOption: return static_cast<int>(Key::R_Alt);
            case kVK_Command: return static_cast<int>(Key::L_System);
            case kVK_RightCommand: return static_cast<int>(Key::R_System);

            case kVK_Escape: return static_cast<int>(Key::Escape);

            case NSMenuFunctionKey:
            case 0x7f: return static_cast<int>(Key::Menu);
            case NSPageUpFunctionKey:
            case kVK_PageUp: return static_cast<int>(Key::Page_Up);
            case NSPageDownFunctionKey:
            case kVK_PageDown: return static_cast<int>(Key::Page_Down);
            case NSEndFunctionKey:
            case kVK_End: return static_cast<int>(Key::End);
            case NSHomeFunctionKey:
            case kVK_Home: return static_cast<int>(Key::Home);
            case NSInsertFunctionKey:
            case kVK_Help: return static_cast<int>(Key::Insert);
            case NSDeleteFunctionKey:
            case kVK_ForwardDelete: return static_cast<int>(Key::Delete);

            case NSBeginFunctionKey: return static_cast<int>(Key::Begin);
            case NSPauseFunctionKey: return static_cast<int>(Key::Pause);
            default: return static_cast<int>(Key::Unknow);
        }

        std::unreachable();

        return static_cast<int>(Key::Unknow);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto usageToVirtualCode(int usage) noexcept -> int {
        switch (usage) {
            case kHIDUsage_KeyboardErrorRollOver: return 0xff;
            case kHIDUsage_KeyboardPOSTFail: return 0xff;
            case kHIDUsage_KeyboardErrorUndefined: return 0xff;

            case kHIDUsage_KeyboardA: return 0x00;
            case kHIDUsage_KeyboardB: return 0x0b;
            case kHIDUsage_KeyboardC: return 0x08;
            case kHIDUsage_KeyboardD: return 0x02;
            case kHIDUsage_KeyboardE: return 0x0e;
            case kHIDUsage_KeyboardF: return 0x03;
            case kHIDUsage_KeyboardG: return 0x05;
            case kHIDUsage_KeyboardH: return 0x04;
            case kHIDUsage_KeyboardI: return 0x22;
            case kHIDUsage_KeyboardJ: return 0x26;
            case kHIDUsage_KeyboardK: return 0x28;
            case kHIDUsage_KeyboardL: return 0x25;
            case kHIDUsage_KeyboardM: return 0x2e;
            case kHIDUsage_KeyboardN: return 0x2d;
            case kHIDUsage_KeyboardO: return 0x1f;
            case kHIDUsage_KeyboardP: return 0x23;
            case kHIDUsage_KeyboardQ: return 0x0c;
            case kHIDUsage_KeyboardR: return 0x0f;
            case kHIDUsage_KeyboardS: return 0x01;
            case kHIDUsage_KeyboardT: return 0x11;
            case kHIDUsage_KeyboardU: return 0x20;
            case kHIDUsage_KeyboardV: return 0x09;
            case kHIDUsage_KeyboardW: return 0x0d;
            case kHIDUsage_KeyboardX: return 0x07;
            case kHIDUsage_KeyboardY: return 0x10;
            case kHIDUsage_KeyboardZ: return 0x06;

            case kHIDUsage_Keyboard1: return 0x12;
            case kHIDUsage_Keyboard2: return 0x13;
            case kHIDUsage_Keyboard3: return 0x14;
            case kHIDUsage_Keyboard4: return 0x15;
            case kHIDUsage_Keyboard5: return 0x17;
            case kHIDUsage_Keyboard6: return 0x16;
            case kHIDUsage_Keyboard7: return 0x1a;
            case kHIDUsage_Keyboard8: return 0x1c;
            case kHIDUsage_Keyboard9: return 0x19;
            case kHIDUsage_Keyboard0: return 0x1d;

            case kHIDUsage_KeyboardReturnOrEnter: return 0x24;
            case kHIDUsage_KeyboardEscape: return 0x35;
            case kHIDUsage_KeyboardDeleteOrBackspace: return 0x33;
            case kHIDUsage_KeyboardTab: return 0x30;
            case kHIDUsage_KeyboardSpacebar: return 0x31;
            case kHIDUsage_KeyboardHyphen: return 0x1b;
            case kHIDUsage_KeyboardEqualSign: return 0x18;
            case kHIDUsage_KeyboardOpenBracket: return 0x21;
            case kHIDUsage_KeyboardCloseBracket: return 0x1e;
            case kHIDUsage_KeyboardBackslash: return 0x2a;
            case kHIDUsage_KeyboardNonUSPound: return 0xff;
            case kHIDUsage_KeyboardSemicolon: return 0x29;
            case kHIDUsage_KeyboardQuote: return 0x27;
            case kHIDUsage_KeyboardGraveAccentAndTilde: return 0x32;
            case kHIDUsage_KeyboardComma: return 0x2b;
            case kHIDUsage_KeyboardPeriod: return 0x2F;
            case kHIDUsage_KeyboardSlash: return 0x2c;
            case kHIDUsage_KeyboardCapsLock: return 0x39;

            case kHIDUsage_KeyboardF1: return 0x7a;
            case kHIDUsage_KeyboardF2: return 0x78;
            case kHIDUsage_KeyboardF3: return 0x63;
            case kHIDUsage_KeyboardF4: return 0x76;
            case kHIDUsage_KeyboardF5: return 0x60;
            case kHIDUsage_KeyboardF6: return 0x61;
            case kHIDUsage_KeyboardF7: return 0x62;
            case kHIDUsage_KeyboardF8: return 0x64;
            case kHIDUsage_KeyboardF9: return 0x65;
            case kHIDUsage_KeyboardF10: return 0x6d;
            case kHIDUsage_KeyboardF11: return 0x67;
            case kHIDUsage_KeyboardF12: return 0x6f;

            case kHIDUsage_KeyboardPrintScreen: return 0xff;
            case kHIDUsage_KeyboardScrollLock: return 0xff;
            case kHIDUsage_KeyboardPause: return 0xff;
            case kHIDUsage_KeyboardInsert: return 0x72;
            case kHIDUsage_KeyboardHome: return 0x73;
            case kHIDUsage_KeyboardPageUp: return 0x74;
            case kHIDUsage_KeyboardDeleteForward: return 0x75;
            case kHIDUsage_KeyboardEnd: return 0x77;
            case kHIDUsage_KeyboardPageDown: return 0x79;

            case kHIDUsage_KeyboardRightArrow: return 0x7c;
            case kHIDUsage_KeyboardLeftArrow: return 0x7b;
            case kHIDUsage_KeyboardDownArrow: return 0x7d;
            case kHIDUsage_KeyboardUpArrow: return 0x7e;

            case kHIDUsage_KeypadNumLock: return 0x47;
            case kHIDUsage_KeypadSlash: return 0x4b;
            case kHIDUsage_KeypadAsterisk: return 0x43;
            case kHIDUsage_KeypadHyphen: return 0x4e;
            case kHIDUsage_KeypadPlus: return 0x45;
            case kHIDUsage_KeypadEnter: return 0x4c;

            case kHIDUsage_Keypad1: return 0x53;
            case kHIDUsage_Keypad2: return 0x54;
            case kHIDUsage_Keypad3: return 0x55;
            case kHIDUsage_Keypad4: return 0x56;
            case kHIDUsage_Keypad5: return 0x57;
            case kHIDUsage_Keypad6: return 0x58;
            case kHIDUsage_Keypad7: return 0x59;
            case kHIDUsage_Keypad8: return 0x5b;
            case kHIDUsage_Keypad9: return 0x5c;
            case kHIDUsage_Keypad0: return 0x52;

            case kHIDUsage_KeypadPeriod: return 0x41;
            case kHIDUsage_KeyboardNonUSBackslash: return 0xff;
            case kHIDUsage_KeyboardApplication: return 0x6e;
            case kHIDUsage_KeyboardPower: return 0xff;
            case kHIDUsage_KeypadEqualSign: return 0x51;

            case kHIDUsage_KeyboardF13: return 0x69;
            case kHIDUsage_KeyboardF14: return 0x6b;
            case kHIDUsage_KeyboardF15: return 0x71;
            case kHIDUsage_KeyboardF16: return 0xff;
            case kHIDUsage_KeyboardF17: return 0xff;
            case kHIDUsage_KeyboardF18: return 0xff;
            case kHIDUsage_KeyboardF19: return 0xff;
            case kHIDUsage_KeyboardF20: return 0xff;
            case kHIDUsage_KeyboardF21: return 0xff;
            case kHIDUsage_KeyboardF22: return 0xff;
            case kHIDUsage_KeyboardF23: return 0xff;
            case kHIDUsage_KeyboardF24: return 0xff;

            case kHIDUsage_KeyboardExecute: return 0xff;
            case kHIDUsage_KeyboardHelp: return 0xff;
            case kHIDUsage_KeyboardMenu: return 0x7F;
            case kHIDUsage_KeyboardSelect: return 0x4c;
            case kHIDUsage_KeyboardStop: return 0xff;
            case kHIDUsage_KeyboardAgain: return 0xff;
            case kHIDUsage_KeyboardUndo: return 0xff;
            case kHIDUsage_KeyboardCut: return 0xff;
            case kHIDUsage_KeyboardCopy: return 0xff;
            case kHIDUsage_KeyboardPaste: return 0xff;
            case kHIDUsage_KeyboardFind: return 0xff;

            case kHIDUsage_KeyboardMute: return 0xff;
            case kHIDUsage_KeyboardVolumeUp: return 0xff;
            case kHIDUsage_KeyboardVolumeDown: return 0xff;

            case kHIDUsage_KeyboardLockingCapsLock: return 0xff;
            case kHIDUsage_KeyboardLockingNumLock: return 0xff;
            case kHIDUsage_KeyboardLockingScrollLock: return 0xff;

            case kHIDUsage_KeypadComma: return 0xff;
            case kHIDUsage_KeypadEqualSignAS400: return 0xff;
            case kHIDUsage_KeyboardInternational1: return 0xff;
            case kHIDUsage_KeyboardInternational2: return 0xff;
            case kHIDUsage_KeyboardInternational3: return 0xff;
            case kHIDUsage_KeyboardInternational4: return 0xff;
            case kHIDUsage_KeyboardInternational5: return 0xff;
            case kHIDUsage_KeyboardInternational6: return 0xff;
            case kHIDUsage_KeyboardInternational7: return 0xff;
            case kHIDUsage_KeyboardInternational8: return 0xff;
            case kHIDUsage_KeyboardInternational9: return 0xff;

            case kHIDUsage_KeyboardLANG1: return 0xff;
            case kHIDUsage_KeyboardLANG2: return 0xff;
            case kHIDUsage_KeyboardLANG3: return 0xff;
            case kHIDUsage_KeyboardLANG4: return 0xff;
            case kHIDUsage_KeyboardLANG5: return 0xff;
            case kHIDUsage_KeyboardLANG6: return 0xff;
            case kHIDUsage_KeyboardLANG7: return 0xff;
            case kHIDUsage_KeyboardLANG8: return 0xff;
            case kHIDUsage_KeyboardLANG9: return 0xff;

            case kHIDUsage_KeyboardAlternateErase: return 0xff;
            case kHIDUsage_KeyboardSysReqOrAttention: return 0xff;
            case kHIDUsage_KeyboardCancel: return 0xff;
            case kHIDUsage_KeyboardClear: return 0xff;
            case kHIDUsage_KeyboardPrior: return 0xff;
            case kHIDUsage_KeyboardReturn: return 0xff;
            case kHIDUsage_KeyboardSeparator: return 0xff;
            case kHIDUsage_KeyboardOut: return 0xff;
            case kHIDUsage_KeyboardOper: return 0xff;
            case kHIDUsage_KeyboardClearOrAgain: return 0xff;
            case kHIDUsage_KeyboardCrSelOrProps: return 0xff;
            case kHIDUsage_KeyboardExSel: return 0xff;

            case kHIDUsage_KeyboardLeftControl: return 0x3b;
            case kHIDUsage_KeyboardLeftShift: return 0x38;
            case kHIDUsage_KeyboardLeftAlt: return 0x3a;
            case kHIDUsage_KeyboardLeftGUI: return 0x37;
            case kHIDUsage_KeyboardRightControl: return 0x3e;
            case kHIDUsage_KeyboardRightShift: return 0x3c;
            case kHIDUsage_KeyboardRightAlt: return 0x3d;
            case kHIDUsage_KeyboardRightGUI: return 0x36;
            case kHIDUsage_Keyboard_Reserved: return 0xff;
            default: return 0xff;
        }

        std::unreachable();

        return 0xff;
    }
} // namespace stormkit::wsi::macos
