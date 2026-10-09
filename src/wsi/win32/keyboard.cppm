// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.wsi:win32.keyboard;

import std;

import stormkit.core.win32;
import stormkit.core;
import stormkit.wsi;

export namespace stormkit::wsi::win32 {
    constexpr auto extract_key(::win32::WPARAM k, ::win32::LPARAM flags) noexcept -> key;
    constexpr auto convert_key(key k) noexcept -> int;
    constexpr auto extract_key_to_char(::win32::WPARAM k, ::win32::LPARAM flags) noexcept -> char;
} // namespace stormkit::wsi::win32

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stdr = std::ranges;

namespace stormkit::wsi::win32 {
    namespace {
        constexpr auto scancode_as_key = make_static_hash_map<::win32::WPARAM, std::pair<key, char>>({
          { 'A',                    { key::a, 'A' }               },
          { 'B',                    { key::b, 'B' }               },
          { 'C',                    { key::c, 'C' }               },
          { 'D',                    { key::d, 'D' }               },
          { 'E',                    { key::e, 'E' }               },
          { 'F',                    { key::f, 'F' }               },
          { 'G',                    { key::g, 'G' }               },
          { 'H',                    { key::h, 'H' }               },
          { 'I',                    { key::i, 'I' }               },
          { 'J',                    { key::j, 'J' }               },
          { 'K',                    { key::k, 'K' }               },
          { 'L',                    { key::l, 'L' }               },
          { 'M',                    { key::m, 'M' }               },
          { 'N',                    { key::n, 'N' }               },
          { 'O',                    { key::o, 'O' }               },
          { 'P',                    { key::p, 'P' }               },
          { 'Q',                    { key::q, 'Q' }               },
          { 'R',                    { key::r, 'R' }               },
          { 'S',                    { key::s, 'S' }               },
          { 'T',                    { key::t, 'T' }               },
          { 'U',                    { key::u, 'U' }               },
          { 'V',                    { key::v, 'V' }               },
          { 'W',                    { key::w, 'W' }               },
          { 'X',                    { key::x, 'X' }               },
          { 'Y',                    { key::y, 'Y' }               },
          { 'Z',                    { key::z, 'Z' }               },

          { '0',                    { key::num_0, '0' }           },
          { '1',                    { key::num_1, '1' }           },
          { '2',                    { key::num_2, '2' }           },
          { '3',                    { key::num_3, '3' }           },
          { '4',                    { key::num_4, '4' }           },
          { '5',                    { key::num_5, '5' }           },
          { '6',                    { key::num_6, '6' }           },
          { '7',                    { key::num_7, '7' }           },
          { '8',                    { key::num_8, '8' }           },
          { '9',                    { key::num_9, '9' }           },

          { ::win32::VK_LEFT,       { key::left, '?' }            },
          { ::win32::VK_RIGHT,      { key::right, '?' }           },
          { ::win32::VK_UP,         { key::up, '?' }              },
          { ::win32::VK_DOWN,       { key::down, '?' }            },

          { ::win32::VK_LCONTROL,   { key::l_control, '?' }       },
          { ::win32::VK_LSHIFT,     { key::l_shift, '?' }         },
          { ::win32::VK_LMENU,      { key::l_alt, '?' }           },
          { ::win32::VK_LWIN,       { key::l_meta, '?' }          },
          { ::win32::VK_RCONTROL,   { key::r_control, '?' }       },
          { ::win32::VK_RSHIFT,     { key::r_shift, '?' }         },
          { ::win32::VK_RMENU,      { key::r_alt, '?' }           },
          { ::win32::VK_RWIN,       { key::r_meta, '?' }          },

          { ::win32::VK_ESCAPE,     { key::escape, '?' }          },
          { ::win32::VK_TAB,        { key::tab, '\t' }            },
          { 0x15D,                  { key::menu, '?' }            },

          { ::win32::VK_OEM_7,      { key::quote, '"' }           },
          { ::win32::VK_OEM_5,      { key::back_slash, '\\' }     },
          { ::win32::VK_OEM_COMMA,  { key::comma, ',' }           },

          { ::win32::VK_OEM_3,      { key::grave_accent, '`' }    },
          { ::win32::VK_OEM_4,      { key::l_bracket, '{' }       },
          { ::win32::VK_OEM_MINUS,  { key::minus, '-' }           },
          { ::win32::VK_OEM_PERIOD, { key::period, '.' }          },
          { ::win32::VK_OEM_6,      { key::r_bracket, '}' }       },
          { ::win32::VK_OEM_1,      { key::semi_colon, ';' }      },
          { ::win32::VK_OEM_2,      { key::slash, '/' }           },

          { ::win32::VK_OEM_102,    { key::iso, '?' }             },

          { ::win32::VK_BACK,       { key::back_space, '?' }      },
          { ::win32::VK_CAPITAL,    { key::caps_lock, '?' }       },
          { ::win32::VK_RETURN,     { key::enter, '\n' }          },
          { ::win32::VK_SPACE,      { key::space, ' ' }           },

          { ::win32::VK_F1,         { key::f1, '?' }              },
          { ::win32::VK_F2,         { key::f2, '?' }              },
          { ::win32::VK_F3,         { key::f3, '?' }              },
          { ::win32::VK_F4,         { key::f4, '?' }              },
          { ::win32::VK_F5,         { key::f5, '?' }              },
          { ::win32::VK_F6,         { key::f6, '?' }              },
          { ::win32::VK_F7,         { key::f7, '?' }              },
          { ::win32::VK_F8,         { key::f8, '?' }              },
          { ::win32::VK_F9,         { key::f9, '?' }              },
          { ::win32::VK_F10,        { key::f10, '?' }             },
          { ::win32::VK_F11,        { key::f11, '?' }             },
          { ::win32::VK_F12,        { key::f12, '?' }             },
          { ::win32::VK_F14,        { key::f14, '?' }             },
          { ::win32::VK_F15,        { key::f15, '?' }             },
          { ::win32::VK_F16,        { key::f16, '?' }             },
          { ::win32::VK_F17,        { key::f17, '?' }             },
          { ::win32::VK_F18,        { key::f18, '?' }             },
          { ::win32::VK_F19,        { key::f19, '?' }             },
          { ::win32::VK_F20,        { key::f20, '?' }             },

          { ::win32::VK_PRINT,      { key::print_screen, '?' }    },

          { ::win32::VK_INSERT,     { key::insert, '?' }          },
          { ::win32::VK_DELETE,     { key::del, '?' }             },
          { ::win32::VK_HOME,       { key::home, '?' }            },
          { ::win32::VK_END,        { key::end, '?' }             },
          { ::win32::VK_NEXT,       { key::page_down, '?' }       },
          { ::win32::VK_PRIOR,      { key::page_up, '?' }         },

          { ::win32::VK_NUMLOCK,    { key::numpad_lock, '?' }     },
          { ::win32::VK_ADD,        { key::numpad_add, '+' }      },
          { ::win32::VK_DIVIDE,     { key::numpad_divide, '/' }   },
          { ::win32::VK_MULTIPLY,   { key::numpad_multiply, '*' } },
          { ::win32::VK_SUBTRACT,   { key::numpad_subtract, '-' } },
          { ::win32::VK_NUMPAD0,    { key::numpad_0, '0' }        },
          { ::win32::VK_NUMPAD1,    { key::numpad_1, '1' }        },
          { ::win32::VK_NUMPAD2,    { key::numpad_2, '2' }        },
          { ::win32::VK_NUMPAD3,    { key::numpad_3, '3' }        },
          { ::win32::VK_NUMPAD4,    { key::numpad_4, '4' }        },
          { ::win32::VK_NUMPAD5,    { key::numpad_5, '5' }        },
          { ::win32::VK_NUMPAD6,    { key::numpad_6, '6' }        },
          { ::win32::VK_NUMPAD7,    { key::numpad_7, '7' }        },
          { ::win32::VK_NUMPAD8,    { key::numpad_8, '8' }        },
          { ::win32::VK_NUMPAD9,    { key::numpad_9, '9' }        },
        });

        constexpr auto key_as_scancode = make_static_hash_map([] static noexcept -> decltype(auto) {
            auto out = array<std::pair<key, ::win32::WPARAM>, scancode_as_key.size()> {};
            auto i   = 0_usize;
            for (const auto& [k, value] : scancode_as_key) out[i++] = std::make_pair(value.first, k);

            return out;
        }());
    } // namespace

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_CONST
    constexpr auto extract_key(::win32::WPARAM k, ::win32::LPARAM) noexcept -> key {
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
    constexpr auto extract_key_to_char(::win32::WPARAM k, ::win32::LPARAM) noexcept -> char {
        const auto it = scancode_as_key.find(k);
        if (it == stdr::cend(scancode_as_key)) return '?';
        return it->second.second;
    }
} // namespace stormkit::wsi::win32
