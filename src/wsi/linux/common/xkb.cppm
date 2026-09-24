// Copyright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
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
        using Keymap  = RAIICapsule<xkb_keymap*, xkb_keymap_new_from_string, xkb_keymap_unref, struct KeymapTag, nullptr>;
        using State   = RAIICapsule<xkb_state*, xkb_state_new, xkb_state_unref, struct StateTag, nullptr>;
        using Context = RAIICapsule<xkb_context*, xkb_context_new, xkb_context_unref, struct ContextTag, nullptr>;

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

    auto stormkit_key_to_xkb(Key key) noexcept -> xkb_keysym_t;
    auto xkb_key_to_stormkit(xkb_keysym_t key) noexcept -> Key;
} // namespace stormkit::wsi::linux::common

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stdr = std::ranges;
namespace stdv = std::views;

namespace stormkit::wsi::linux::common {
    namespace {
        constexpr auto SCANCODE_AS_KEY = make_static_hash_map<xkb_keysym_t, Key>({
          { XKB_KEY_a,            Key::A               },
          { XKB_KEY_b,            Key::B               },
          { XKB_KEY_c,            Key::C               },
          { XKB_KEY_d,            Key::D               },
          { XKB_KEY_e,            Key::E               },
          { XKB_KEY_f,            Key::F               },
          { XKB_KEY_g,            Key::G               },
          { XKB_KEY_h,            Key::H               },
          { XKB_KEY_i,            Key::I               },
          { XKB_KEY_j,            Key::J               },
          { XKB_KEY_k,            Key::K               },
          { XKB_KEY_l,            Key::L               },
          { XKB_KEY_m,            Key::M               },
          { XKB_KEY_n,            Key::N               },
          { XKB_KEY_o,            Key::O               },
          { XKB_KEY_p,            Key::P               },
          { XKB_KEY_q,            Key::Q               },
          { XKB_KEY_r,            Key::R               },
          { XKB_KEY_s,            Key::S               },
          { XKB_KEY_t,            Key::T               },
          { XKB_KEY_u,            Key::U               },
          { XKB_KEY_v,            Key::V               },
          { XKB_KEY_w,            Key::W               },
          { XKB_KEY_x,            Key::X               },
          { XKB_KEY_y,            Key::Y               },
          { XKB_KEY_z,            Key::Z               },

          { XKB_KEY_0,            Key::NUM_0           },
          { XKB_KEY_1,            Key::NUM_1           },
          { XKB_KEY_2,            Key::NUM_2           },
          { XKB_KEY_3,            Key::NUM_3           },
          { XKB_KEY_4,            Key::NUM_4           },
          { XKB_KEY_5,            Key::NUM_5           },
          { XKB_KEY_6,            Key::NUM_6           },
          { XKB_KEY_7,            Key::NUM_7           },
          { XKB_KEY_8,            Key::NUM_8           },
          { XKB_KEY_9,            Key::NUM_9           },

          { XKB_KEY_Left,         Key::LEFT            },
          { XKB_KEY_Right,        Key::RIGHT           },
          { XKB_KEY_Up,           Key::UP              },
          { XKB_KEY_Down,         Key::DOWN            },

          { XKB_KEY_Control_L,    Key::L_CONTROL       },
          { XKB_KEY_Shift_L,      Key::L_SHIFT         },
          { XKB_KEY_Alt_L,        Key::L_ALT           },
          { XKB_KEY_Super_L,      Key::L_META          },
          { XKB_KEY_Control_R,    Key::R_CONTROL       },
          { XKB_KEY_Shift_R,      Key::R_SHIFT         },
          { XKB_KEY_Alt_R,        Key::R_ALT           },
          { XKB_KEY_Super_R,      Key::R_META          },

          { XKB_KEY_Escape,       Key::ESCAPE          },
          { XKB_KEY_Tab,          Key::TAB             },
          { XKB_KEY_Menu,         Key::MENU            },

          { XKB_KEY_apostrophe,   Key::QUOTE           },
          { XKB_KEY_backslash,    Key::BACK_SLASH      },
          { XKB_KEY_comma,        Key::COMMA           },
          { XKB_KEY_equal,        Key::EQUAL           },

          { XKB_KEY_grave,        Key::GRAVE_ACCENT    },
          { XKB_KEY_bracketleft,  Key::L_BRACKET       },
          { XKB_KEY_minus,        Key::MINUS           },
          { XKB_KEY_period,       Key::PERIOD          },
          { XKB_KEY_bracketright, Key::R_BRACKET       },
          { XKB_KEY_semicolon,    Key::SEMI_COLON      },
          { XKB_KEY_slash,        Key::SLASH           },

          { XKB_KEY_less,         Key::ISO             },

          { XKB_KEY_BackSpace,    Key::BACK_SPACE      },
          { XKB_KEY_Caps_Lock,    Key::CAPS_LOCK       },
          { XKB_KEY_Return,       Key::ENTER           },
          { XKB_KEY_space,        Key::SPACE           },

          { XKB_KEY_F1,           Key::F1              },
          { XKB_KEY_F2,           Key::F2              },
          { XKB_KEY_F3,           Key::F3              },
          { XKB_KEY_F4,           Key::F4              },
          { XKB_KEY_F5,           Key::F5              },
          { XKB_KEY_F6,           Key::F6              },
          { XKB_KEY_F7,           Key::F7              },
          { XKB_KEY_F8,           Key::F8              },
          { XKB_KEY_F9,           Key::F9              },
          { XKB_KEY_F10,          Key::F10             },
          { XKB_KEY_F11,          Key::F11             },
          { XKB_KEY_F12,          Key::F12             },
          { XKB_KEY_F14,          Key::F14             },
          { XKB_KEY_F15,          Key::F15             },
          { XKB_KEY_F16,          Key::F16             },
          { XKB_KEY_F17,          Key::F17             },
          { XKB_KEY_F18,          Key::F18             },
          { XKB_KEY_F19,          Key::F19             },
          { XKB_KEY_F20,          Key::F20             },

          { XKB_KEY_Print,        Key::PRINT_SCREEN    },

          { XKB_KEY_Insert,       Key::INSERT          },
          { XKB_KEY_Delete,       Key::DELETE          },
          { XKB_KEY_Home,         Key::HOME            },
          { XKB_KEY_End,          Key::END             },
          { XKB_KEY_Page_Down,    Key::PAGE_DOWN       },
          { XKB_KEY_Page_Up,      Key::PAGE_UP         },

          { XKB_KEY_Num_Lock,     Key::NUMPAD_LOCK     },
          { XKB_KEY_KP_Add,       Key::NUMPAD_ADD      },
          { XKB_KEY_KP_Decimal,   Key::NUMPAD_DECIMAL  },
          { XKB_KEY_KP_Divide,    Key::NUMPAD_DIVIDE   },
          { XKB_KEY_KP_Enter,     Key::NUMPAD_ENTER    },
          { XKB_KEY_KP_Equal,     Key::NUMPAD_EQUAL    },
          { XKB_KEY_KP_Multiply,  Key::NUMPAD_MULTIPLY },
          { XKB_KEY_KP_Subtract,  Key::NUMPAD_SUBTRACT },
          { XKB_KEY_KP_0,         Key::NUMPAD_0        },
          { XKB_KEY_KP_1,         Key::NUMPAD_1        },
          { XKB_KEY_KP_2,         Key::NUMPAD_2        },
          { XKB_KEY_KP_3,         Key::NUMPAD_3        },
          { XKB_KEY_KP_4,         Key::NUMPAD_4        },
          { XKB_KEY_KP_5,         Key::NUMPAD_5        },
          { XKB_KEY_KP_6,         Key::NUMPAD_6        },
          { XKB_KEY_KP_7,         Key::NUMPAD_7        },
          { XKB_KEY_KP_8,         Key::NUMPAD_8        },
          { XKB_KEY_KP_9,         Key::NUMPAD_9        },
        });

        constexpr auto KEY_AS_SCANCODE = [] static noexcept -> decltype(auto) {
            auto out = array<std::pair<Key, xkb_keysym_t>, 111> {};
            auto i   = 0_usize;
            for (const auto& [key, value] : SCANCODE_AS_KEY) out[i++] = std::make_pair(value, key);

            return make_static_hash_map(out);
        }();
    } // namespace

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_CONST
    inline auto xkb_key_to_stormkit(xkb_keysym_t scancode) noexcept -> Key {
        const auto it = SCANCODE_AS_KEY.find(scancode);
        if (it == stdr::cend(SCANCODE_AS_KEY)) return Key::UNKNOWN;
        return it->second;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_CONST
    inline auto stormkit_key_to_xkb(Key key) noexcept -> xkb_keysym_t {
        ENSURES(key != Key::UNKNOWN);
        const auto it = KEY_AS_SCANCODE.find(key);
        return it->second;
    }
} // namespace stormkit::wsi::linux::common

#undef STORMKIT_XKB_SCOPED
