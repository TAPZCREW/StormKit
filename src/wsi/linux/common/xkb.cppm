// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <xkbcommon/xkbcommon.h>

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.wsi:linux.common.xkb;

import std;

import stormkit.core;
import stormkit.log;
import stormkit.wsi;

export namespace stormkit::wsi::linux::common {
    namespace xkb {
        using Keymap  = raii_capsule<xkb_keymap*, xkb_keymap_new_from_string, xkb_keymap_unref, struct KeymapTag, nullptr>;
        using State   = raii_capsule<xkb_state*, xkb_state_new, xkb_state_unref, struct StateTag, nullptr>;
        using Context = raii_capsule<xkb_context*, xkb_context_new, xkb_context_unref, struct ContextTag, nullptr>;

        struct Mods {
            xkb_mod_index_t shift;
            xkb_mod_index_t lock;
            xkb_mod_index_t control;
            xkb_mod_index_t mod1;
            xkb_mod_index_t mod2;
            xkb_mod_index_t mod3;
            xkb_mod_index_t mod4;
            xkb_mod_index_t mod5;
        };
    } // namespace xkb

    auto stormkit_key_to_xkb(key k) noexcept -> xkb_keysym_t;
    auto xkb_key_to_stormkit(xkb_keysym_t key) noexcept -> Key;
} // namespace stormkit::wsi::linux::common

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stdr = std::ranges;
namespace stdv = std::views;

namespace stormkit::wsi::linux::common {
    namespace {
        constexpr auto scancode_as_key = make_static_hash_map<xkb_keysym_t, Key>({
          { XKB_KEY_a,            key::a               },
          { XKB_KEY_b,            key::b               },
          { XKB_KEY_c,            key::c               },
          { XKB_KEY_d,            key::d               },
          { XKB_KEY_e,            key::e               },
          { XKB_KEY_f,            key::f               },
          { XKB_KEY_g,            key::g               },
          { XKB_KEY_h,            key::h               },
          { XKB_KEY_i,            key::i               },
          { XKB_KEY_j,            key::j               },
          { XKB_KEY_k,            key::k               },
          { XKB_KEY_l,            key::l               },
          { XKB_KEY_m,            key::m               },
          { XKB_KEY_n,            key::n               },
          { XKB_KEY_o,            key::o               },
          { XKB_KEY_p,            key::p               },
          { XKB_KEY_q,            key::q               },
          { XKB_KEY_r,            key::r               },
          { XKB_KEY_s,            key::s               },
          { XKB_KEY_t,            key::t               },
          { XKB_KEY_u,            key::u               },
          { XKB_KEY_v,            key::v               },
          { XKB_KEY_w,            key::w               },
          { XKB_KEY_x,            key::x               },
          { XKB_KEY_y,            key::y               },
          { XKB_KEY_z,            key::z               },

          { XKB_KEY_0,            key::num_0           },
          { XKB_KEY_1,            key::num_1           },
          { XKB_KEY_2,            key::num_2           },
          { XKB_KEY_3,            key::num_3           },
          { XKB_KEY_4,            key::num_4           },
          { XKB_KEY_5,            key::num_5           },
          { XKB_KEY_6,            key::num_6           },
          { XKB_KEY_7,            key::num_7           },
          { XKB_KEY_8,            key::num_8           },
          { XKB_KEY_9,            key::num_9           },

          { XKB_KEY_Left,         key::left            },
          { XKB_KEY_Right,        key::right           },
          { XKB_KEY_Up,           key::up              },
          { XKB_KEY_Down,         key::down            },

          { XKB_KEY_Control_L,    key::l_control       },
          { XKB_KEY_Shift_L,      key::l_shift         },
          { XKB_KEY_Alt_L,        key::l_alt           },
          { XKB_KEY_Super_L,      key::l_meta          },
          { XKB_KEY_Control_R,    key::r_control       },
          { XKB_KEY_Shift_R,      key::r_shift         },
          { XKB_KEY_Alt_R,        key::r_alt           },
          { XKB_KEY_Super_R,      key::r_meta          },

          { XKB_KEY_Escape,       key::escape          },
          { XKB_KEY_Tab,          key::tab             },
          { XKB_KEY_Menu,         key::menu            },

          { XKB_KEY_apostrophe,   key::quote           },
          { XKB_KEY_backslash,    key::back_slash      },
          { XKB_KEY_comma,        key::comma           },
          { XKB_KEY_equal,        key::equal           },

          { XKB_KEY_grave,        key::grave_accent    },
          { XKB_KEY_bracketleft,  key::l_bracket       },
          { XKB_KEY_minus,        key::minus           },
          { XKB_KEY_period,       key::period          },
          { XKB_KEY_bracketright, key::r_bracket       },
          { XKB_KEY_semicolon,    key::semi_colon      },
          { XKB_KEY_slash,        key::slash           },

          { XKB_KEY_less,         key::iso             },

          { XKB_KEY_BackSpace,    key::back_space      },
          { XKB_KEY_Caps_Lock,    key::caps_lock       },
          { XKB_KEY_Return,       key::enter           },
          { XKB_KEY_space,        key::space           },

          { XKB_KEY_F1,           key::f1              },
          { XKB_KEY_F2,           key::f2              },
          { XKB_KEY_F3,           key::f3              },
          { XKB_KEY_F4,           key::f4              },
          { XKB_KEY_F5,           key::f5              },
          { XKB_KEY_F6,           key::f6              },
          { XKB_KEY_F7,           key::f7              },
          { XKB_KEY_F8,           key::f8              },
          { XKB_KEY_F9,           key::f9              },
          { XKB_KEY_F10,          key::f10             },
          { XKB_KEY_F11,          key::f11             },
          { XKB_KEY_F12,          key::f12             },
          { XKB_KEY_F14,          key::f14             },
          { XKB_KEY_F15,          key::f15             },
          { XKB_KEY_F16,          key::f16             },
          { XKB_KEY_F17,          key::f17             },
          { XKB_KEY_F18,          key::f18             },
          { XKB_KEY_F19,          key::f19             },
          { XKB_KEY_F20,          key::f20             },

          { XKB_KEY_Print,        key::print_screen    },

          { XKB_KEY_Insert,       key::insert          },
          { XKB_KEY_Delete,       key::del             },
          { XKB_KEY_Home,         key::home            },
          { XKB_KEY_End,          key::end             },
          { XKB_KEY_Page_Down,    key::page_down       },
          { XKB_KEY_Page_Up,      key::page_up         },

          { XKB_KEY_Num_Lock,     key::numpad_lock     },
          { XKB_KEY_KP_Add,       key::numpad_add      },
          { XKB_KEY_KP_Decimal,   key::numpad_decimal  },
          { XKB_KEY_KP_Divide,    key::numpad_divide   },
          { XKB_KEY_KP_Enter,     key::numpad_enter    },
          { XKB_KEY_KP_Equal,     key::numpad_equal    },
          { XKB_KEY_KP_Multiply,  key::numpad_multiply },
          { XKB_KEY_KP_Subtract,  key::numpad_subtract },
          { XKB_KEY_KP_0,         key::numpad_0        },
          { XKB_KEY_KP_1,         key::numpad_1        },
          { XKB_KEY_KP_2,         key::numpad_2        },
          { XKB_KEY_KP_3,         key::numpad_3        },
          { XKB_KEY_KP_4,         key::numpad_4        },
          { XKB_KEY_KP_5,         key::numpad_5        },
          { XKB_KEY_KP_6,         key::numpad_6        },
          { XKB_KEY_KP_7,         key::numpad_7        },
          { XKB_KEY_KP_8,         key::numpad_8        },
          { XKB_KEY_KP_9,         key::numpad_9        },
        });

        constexpr auto key_as_scancode = [] static noexcept -> decltype(auto) {
            auto out = array<std::pair<Key, xkb_keysym_t>, 111> {};
            auto i   = 0_usize;
            for (const auto& [key, value] : scancode_as_key) out[i++] = std::make_pair(value, key);

            return make_static_hash_map(out);
        }();
    } // namespace

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_CONST
    inline auto xkb_key_to_stormkit(xkb_keysym_t scancode) noexcept -> Key {
        const auto it = scancode_as_key.find(scancode);
        if (it == stdr::cend(scancode_as_key)) return key::unknown;
        return it->second;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_CONST
    inline auto stormkit_key_to_xkb(key k) noexcept -> xkb_keysym_t {
        ENSURES(k != key::unknown);
        const auto it = key_as_scancode.find(k);
        return it->second;
    }
} // namespace stormkit::wsi::linux::common

#undef STORMKIT_XKB_SCOPED
