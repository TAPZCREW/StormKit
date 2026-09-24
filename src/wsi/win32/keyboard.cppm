// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform/windows.hpp>

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.wsi:win32.keyboard;

import std;

import stormkit.core;
import stormkit.wsi;

export namespace stormkit::wsi::win32 {
    constexpr auto extract_key(WPARAM key, LPARAM flags) noexcept -> Key;
    constexpr auto convert_key(Key key) noexcept -> int;
    constexpr auto extract_key_to_char(WPARAM key, LPARAM flags) noexcept -> char;
} // namespace stormkit::wsi::win32

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stdr = std::ranges;

namespace stormkit::wsi::win32 {
    namespace {
        constexpr auto SCANCODE_AS_KEY = make_static_hash_map<WPARAM, std::pair<Key, char>>({
          { 'A',           { Key::A, 'A' }               },
          { 'B',           { Key::B, 'B' }               },
          { 'C',           { Key::C, 'C' }               },
          { 'D',           { Key::D, 'D' }               },
          { 'E',           { Key::E, 'E' }               },
          { 'F',           { Key::F, 'F' }               },
          { 'G',           { Key::G, 'G' }               },
          { 'H',           { Key::H, 'H' }               },
          { 'I',           { Key::I, 'I' }               },
          { 'J',           { Key::J, 'J' }               },
          { 'K',           { Key::K, 'K' }               },
          { 'L',           { Key::L, 'L' }               },
          { 'M',           { Key::M, 'M' }               },
          { 'N',           { Key::N, 'N' }               },
          { 'O',           { Key::O, 'O' }               },
          { 'P',           { Key::P, 'P' }               },
          { 'Q',           { Key::Q, 'Q' }               },
          { 'R',           { Key::R, 'R' }               },
          { 'S',           { Key::S, 'S' }               },
          { 'T',           { Key::T, 'T' }               },
          { 'U',           { Key::U, 'U' }               },
          { 'V',           { Key::V, 'V' }               },
          { 'W',           { Key::W, 'W' }               },
          { 'X',           { Key::X, 'X' }               },
          { 'Y',           { Key::Y, 'Y' }               },
          { 'Z',           { Key::Z, 'Z' }               },

          { '0',           { Key::NUM_0, '0' }           },
          { '1',           { Key::NUM_1, '1' }           },
          { '2',           { Key::NUM_2, '2' }           },
          { '3',           { Key::NUM_3, '3' }           },
          { '4',           { Key::NUM_4, '4' }           },
          { '5',           { Key::NUM_5, '5' }           },
          { '6',           { Key::NUM_6, '6' }           },
          { '7',           { Key::NUM_7, '7' }           },
          { '8',           { Key::NUM_8, '8' }           },
          { '9',           { Key::NUM_9, '9' }           },

          { VK_LEFT,       { Key::LEFT, '?' }            },
          { VK_RIGHT,      { Key::RIGHT, '?' }           },
          { VK_UP,         { Key::UP, '?' }              },
          { VK_DOWN,       { Key::DOWN, '?' }            },

          { VK_LCONTROL,   { Key::L_CONTROL, '?' }       },
          { VK_LSHIFT,     { Key::L_SHIFT, '?' }         },
          { VK_LMENU,      { Key::L_ALT, '?' }           },
          { VK_LWIN,       { Key::L_META, '?' }          },
          { VK_RCONTROL,   { Key::R_CONTROL, '?' }       },
          { VK_RSHIFT,     { Key::R_SHIFT, '?' }         },
          { VK_RMENU,      { Key::R_ALT, '?' }           },
          { VK_RWIN,       { Key::R_META, '?' }          },

          { VK_ESCAPE,     { Key::ESCAPE, '?' }          },
          { VK_TAB,        { Key::TAB, '\t' }            },
          { 0x15D,         { Key::MENU, '?' }            },

          { VK_OEM_7,      { Key::QUOTE, '"' }           },
          { VK_OEM_5,      { Key::BACK_SLASH, '\\' }     },
          { VK_OEM_COMMA,  { Key::COMMA, ',' }           },

          { VK_OEM_3,      { Key::GRAVE_ACCENT, '`' }    },
          { VK_OEM_4,      { Key::L_BRACKET, '{' }       },
          { VK_OEM_MINUS,  { Key::MINUS, '-' }           },
          { VK_OEM_PERIOD, { Key::PERIOD, '.' }          },
          { VK_OEM_6,      { Key::R_BRACKET, '}' }       },
          { VK_OEM_1,      { Key::SEMI_COLON, ';' }      },
          { VK_OEM_2,      { Key::SLASH, '/' }           },

          { VK_OEM_102,    { Key::ISO, '?' }             },

          { VK_BACK,       { Key::BACK_SPACE, '?' }      },
          { VK_CAPITAL,    { Key::CAPS_LOCK, '?' }       },
          { VK_RETURN,     { Key::ENTER, '\n' }          },
          { VK_SPACE,      { Key::SPACE, ' ' }           },

          { VK_F1,         { Key::F1, '?' }              },
          { VK_F2,         { Key::F2, '?' }              },
          { VK_F3,         { Key::F3, '?' }              },
          { VK_F4,         { Key::F4, '?' }              },
          { VK_F5,         { Key::F5, '?' }              },
          { VK_F6,         { Key::F6, '?' }              },
          { VK_F7,         { Key::F7, '?' }              },
          { VK_F8,         { Key::F8, '?' }              },
          { VK_F9,         { Key::F9, '?' }              },
          { VK_F10,        { Key::F10, '?' }             },
          { VK_F11,        { Key::F11, '?' }             },
          { VK_F12,        { Key::F12, '?' }             },
          { VK_F14,        { Key::F14, '?' }             },
          { VK_F15,        { Key::F15, '?' }             },
          { VK_F16,        { Key::F16, '?' }             },
          { VK_F17,        { Key::F17, '?' }             },
          { VK_F18,        { Key::F18, '?' }             },
          { VK_F19,        { Key::F19, '?' }             },
          { VK_F20,        { Key::F20, '?' }             },

          { VK_PRINT,      { Key::PRINT_SCREEN, '?' }    },

          { VK_INSERT,     { Key::INSERT, '?' }          },
          { VK_DELETE,     { Key::DELETE, '?' }          },
          { VK_HOME,       { Key::HOME, '?' }            },
          { VK_END,        { Key::END, '?' }             },
          { VK_NEXT,       { Key::PAGE_DOWN, '?' }       },
          { VK_PRIOR,      { Key::PAGE_UP, '?' }         },

          { VK_NUMLOCK,    { Key::NUMPAD_LOCK, '?' }     },
          { VK_ADD,        { Key::NUMPAD_ADD, '+' }      },
          { VK_DIVIDE,     { Key::NUMPAD_DIVIDE, '/' }   },
          { VK_MULTIPLY,   { Key::NUMPAD_MULTIPLY, '*' } },
          { VK_SUBTRACT,   { Key::NUMPAD_SUBTRACT, '-' } },
          { VK_NUMPAD0,    { Key::NUMPAD_0, '0' }        },
          { VK_NUMPAD1,    { Key::NUMPAD_1, '1' }        },
          { VK_NUMPAD2,    { Key::NUMPAD_2, '2' }        },
          { VK_NUMPAD3,    { Key::NUMPAD_3, '3' }        },
          { VK_NUMPAD4,    { Key::NUMPAD_4, '4' }        },
          { VK_NUMPAD5,    { Key::NUMPAD_5, '5' }        },
          { VK_NUMPAD6,    { Key::NUMPAD_6, '6' }        },
          { VK_NUMPAD7,    { Key::NUMPAD_7, '7' }        },
          { VK_NUMPAD8,    { Key::NUMPAD_8, '8' }        },
          { VK_NUMPAD9,    { Key::NUMPAD_9, '9' }        },
        });

        constexpr auto KEY_AS_SCANCODE = [] static noexcept -> decltype(auto) {
            auto out = array<std::pair<Key, WPARAM>, 111> {};
            auto i   = 0_usize;
            for (const auto& [key, value] : SCANCODE_AS_KEY) out[i++] = std::make_pair(value.first, key);

            return make_static_hash_map(out);
        }();
    } // namespace

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_CONST
    constexpr auto extract_key(WPARAM key, LPARAM) noexcept -> Key {
        const auto it = SCANCODE_AS_KEY.find(key);
        if (it == stdr::cend(SCANCODE_AS_KEY)) return Key::UNKNOWN;
        return it->second.first;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_CONST
    constexpr auto convert_key(Key key) noexcept -> int {
        ENSURES(key != Key::UNKNOWN);
        const auto it = KEY_AS_SCANCODE.find(key);
        return as<int>(it->second);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_CONST
    constexpr auto extract_key_to_char(WPARAM key, LPARAM) noexcept -> char {
        const auto it = SCANCODE_AS_KEY.find(key);
        if (it == stdr::cend(SCANCODE_AS_KEY)) return '?';
        return it->second.second;
    }
} // namespace stormkit::wsi::win32
