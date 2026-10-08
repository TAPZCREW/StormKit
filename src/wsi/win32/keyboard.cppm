// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform/windows.hpp>

#undef DELETE

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.wsi:win32.keyboard;

import std;

import stormkit.core;
import stormkit.wsi;

export namespace stormkit::wsi::win32 {
    constexpr auto extract_key(WPARAM k, LPARAM flags) noexcept -> key;
    constexpr auto convert_key(key k) noexcept -> int;
    constexpr auto extract_key_to_char(WPARAM k, LPARAM flags) noexcept -> char;
} // namespace stormkit::wsi::win32

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stdr = std::ranges;

namespace stormkit::wsi::win32 {
    namespace {
        constexpr auto scancode_as_key = make_static_hash_map<WPARAM, std::pair<key, char>>({
          { 'A',           { key::a, 'A' }               },
          { 'B',           { key::b, 'B' }               },
          { 'C',           { key::c, 'C' }               },
          { 'D',           { key::d, 'D' }               },
          { 'E',           { key::e, 'E' }               },
          { 'F',           { key::f, 'F' }               },
          { 'G',           { key::g, 'G' }               },
          { 'H',           { key::h, 'H' }               },
          { 'I',           { key::i, 'I' }               },
          { 'J',           { key::j, 'J' }               },
          { 'K',           { key::k, 'K' }               },
          { 'L',           { key::l, 'L' }               },
          { 'M',           { key::m, 'M' }               },
          { 'N',           { key::n, 'N' }               },
          { 'O',           { key::o, 'O' }               },
          { 'P',           { key::p, 'P' }               },
          { 'Q',           { key::q, 'Q' }               },
          { 'R',           { key::r, 'R' }               },
          { 'S',           { key::s, 'S' }               },
          { 'T',           { key::t, 'T' }               },
          { 'U',           { key::u, 'U' }               },
          { 'V',           { key::v, 'V' }               },
          { 'W',           { key::w, 'W' }               },
          { 'X',           { key::x, 'X' }               },
          { 'Y',           { key::y, 'Y' }               },
          { 'Z',           { key::z, 'Z' }               },

          { '0',           { key::num_0, '0' }           },
          { '1',           { key::num_1, '1' }           },
          { '2',           { key::num_2, '2' }           },
          { '3',           { key::num_3, '3' }           },
          { '4',           { key::num_4, '4' }           },
          { '5',           { key::num_5, '5' }           },
          { '6',           { key::num_6, '6' }           },
          { '7',           { key::num_7, '7' }           },
          { '8',           { key::num_8, '8' }           },
          { '9',           { key::num_9, '9' }           },

          { VK_LEFT,       { key::left, '?' }            },
          { VK_RIGHT,      { key::right, '?' }           },
          { VK_UP,         { key::up, '?' }              },
          { VK_DOWN,       { key::down, '?' }            },

          { VK_LCONTROL,   { key::l_control, '?' }       },
          { VK_LSHIFT,     { key::l_shift, '?' }         },
          { VK_LMENU,      { key::l_alt, '?' }           },
          { VK_LWIN,       { key::l_meta, '?' }          },
          { VK_RCONTROL,   { key::r_control, '?' }       },
          { VK_RSHIFT,     { key::r_shift, '?' }         },
          { VK_RMENU,      { key::r_alt, '?' }           },
          { VK_RWIN,       { key::r_meta, '?' }          },

          { VK_ESCAPE,     { key::escape, '?' }          },
          { VK_TAB,        { key::tab, '\t' }            },
          { 0x15D,         { key::menu, '?' }            },

          { VK_OEM_7,      { key::quote, '"' }           },
          { VK_OEM_5,      { key::back_slash, '\\' }     },
          { VK_OEM_COMMA,  { key::comma, ',' }           },

          { VK_OEM_3,      { key::grave_accent, '`' }    },
          { VK_OEM_4,      { key::l_bracket, '{' }       },
          { VK_OEM_MINUS,  { key::minus, '-' }           },
          { VK_OEM_PERIOD, { key::period, '.' }          },
          { VK_OEM_6,      { key::r_bracket, '}' }       },
          { VK_OEM_1,      { key::semi_colon, ';' }      },
          { VK_OEM_2,      { key::slash, '/' }           },

          { VK_OEM_102,    { key::iso, '?' }             },

          { VK_BACK,       { key::back_space, '?' }      },
          { VK_CAPITAL,    { key::caps_lock, '?' }       },
          { VK_RETURN,     { key::enter, '\n' }          },
          { VK_SPACE,      { key::space, ' ' }           },

          { VK_F1,         { key::f1, '?' }              },
          { VK_F2,         { key::f2, '?' }              },
          { VK_F3,         { key::f3, '?' }              },
          { VK_F4,         { key::f4, '?' }              },
          { VK_F5,         { key::f5, '?' }              },
          { VK_F6,         { key::f6, '?' }              },
          { VK_F7,         { key::f7, '?' }              },
          { VK_F8,         { key::f8, '?' }              },
          { VK_F9,         { key::f9, '?' }              },
          { VK_F10,        { key::f10, '?' }             },
          { VK_F11,        { key::f11, '?' }             },
          { VK_F12,        { key::f12, '?' }             },
          { VK_F14,        { key::f14, '?' }             },
          { VK_F15,        { key::f15, '?' }             },
          { VK_F16,        { key::f16, '?' }             },
          { VK_F17,        { key::f17, '?' }             },
          { VK_F18,        { key::f18, '?' }             },
          { VK_F19,        { key::f19, '?' }             },
          { VK_F20,        { key::f20, '?' }             },

          { VK_PRINT,      { key::print_screen, '?' }    },

          { VK_INSERT,     { key::insert, '?' }          },
          { VK_DELETE,     { key::del, '?' }             },
          { VK_HOME,       { key::home, '?' }            },
          { VK_END,        { key::end, '?' }             },
          { VK_NEXT,       { key::page_down, '?' }       },
          { VK_PRIOR,      { key::page_up, '?' }         },

          { VK_NUMLOCK,    { key::numpad_lock, '?' }     },
          { VK_ADD,        { key::numpad_add, '+' }      },
          { VK_DIVIDE,     { key::numpad_divide, '/' }   },
          { VK_MULTIPLY,   { key::numpad_multiply, '*' } },
          { VK_SUBTRACT,   { key::numpad_subtract, '-' } },
          { VK_NUMPAD0,    { key::numpad_0, '0' }        },
          { VK_NUMPAD1,    { key::numpad_1, '1' }        },
          { VK_NUMPAD2,    { key::numpad_2, '2' }        },
          { VK_NUMPAD3,    { key::numpad_3, '3' }        },
          { VK_NUMPAD4,    { key::numpad_4, '4' }        },
          { VK_NUMPAD5,    { key::numpad_5, '5' }        },
          { VK_NUMPAD6,    { key::numpad_6, '6' }        },
          { VK_NUMPAD7,    { key::numpad_7, '7' }        },
          { VK_NUMPAD8,    { key::numpad_8, '8' }        },
          { VK_NUMPAD9,    { key::numpad_9, '9' }        },
        });

        constexpr auto key_as_scancode = make_static_hash_map([] static noexcept -> decltype(auto) {
            auto out = array<std::pair<key, WPARAM>, scancode_as_key.size()> {};
            auto i   = 0_usize;
            for (const auto& [k, value] : scancode_as_key) out[i++] = std::make_pair(value.first, k);

            return out;
        }());
    } // namespace

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_CONST
    constexpr auto extract_key(WPARAM k, LPARAM) noexcept -> key {
        const auto it = scancode_as_key.find(k);
        if (it == stdr::cend(scancode_as_key)) return key::unknown;
        return it->second.first;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_CONST
    constexpr auto convert_key(key k) noexcept -> int {
        ENSURES(k != key::unknown);
        const auto it = key_as_scancode.find(k);
        return as<int>(it->second);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_CONST
    constexpr auto extract_key_to_char(WPARAM k, LPARAM) noexcept -> char {
        const auto it = scancode_as_key.find(k);
        if (it == stdr::cend(scancode_as_key)) return '?';
        return it->second.second;
    }
} // namespace stormkit::wsi::win32
