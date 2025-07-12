// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.core;
import stormkit.test;

#include <stormkit/core/flags_macro.hpp>
#include <stormkit/test/test_macro.hpp>

using namespace stormkit::core;

enum class Flag {
    A = 1,
    B = 2,
    C = 4,
};

enum Flag2 {
    A = 1,
    B = 2,
    C = 4,
};

FLAG_ENUM(Flag)
FLAG_ENUM(Flag2)

namespace {
    auto _ = test::TestSuite {
        "Core.typesafe",
        {
          {
            "Flags.enum_class.operators",
            [] static noexcept {
                {
                    auto foo = Flag::A | Flag::B;
                    EXPECTS(foo == (Flag::A | Flag::B));
                }

                {
                    auto foo = (Flag::A ^ Flag::A | Flag::B);
                    EXPECTS(foo == Flag::B);
                }

                {
                    auto foo = (Flag::A | Flag::B);
                    foo ^= Flag::A;
                    EXPECTS(foo == Flag::B);
                }

                {
                    auto foo = (Flag::A | Flag::B);
                    foo      = foo & Flag::B;
                    EXPECTS(foo == Flag::B);
                }

                {
                    auto foo = (Flag::A | Flag::B);
                    foo &= Flag::B;
                    EXPECTS(foo == Flag::B);
                }

                {
                    auto foo = ~(Flag::B);
                    EXPECTS(foo == ~(Flag::B));
                }
            },
          }, {
            "Flags.enum_class.check_flag_bit",
            [] static noexcept {
                auto foo = Flag::A | Flag::B;
                EXPECTS(check_flag_bit(foo, Flag::A));
            },
          }, {
            "Flags.enum_class.next_value",
            [] static noexcept {
                auto foo = Flag::A;
                EXPECTS(next_value(foo) == Flag::B);
            },
          }, {
            "Flags.enum.operators",
            [] static noexcept {
                {
                    auto foo = Flag2::A | Flag2::B;
                    EXPECTS(foo == (Flag2::A | Flag2::B));
                }

                {
                    auto foo = (Flag2::A ^ Flag2::A | Flag2::B);
                    EXPECTS(foo == Flag2::B);
                }

                {
                    auto foo = (Flag2::A | Flag2::B);
                    foo ^= Flag2::A;
                    EXPECTS(foo == Flag2::B);
                }

                {
                    auto foo = (Flag2::A | Flag2::B);
                    foo      = foo & Flag2::B;
                    EXPECTS(foo == Flag2::B);
                }

                {
                    auto foo = (Flag2::A | Flag2::B);
                    foo &= Flag2::B;
                    EXPECTS(foo == Flag2::B);
                }

                {
                    auto foo = (~Flag2::B);
                    EXPECTS(foo == (~Flag2::B));
                }
            },
          }, {
            "Flags.enum.check_flag_bit",
            [] static noexcept {
                auto foo = Flag2::A | Flag2::B;
                EXPECTS(check_flag_bit(foo, Flag2::A));
            },
          }, {
            "Flags.enum.next_value",
            [] static noexcept {
                auto foo = Flag2::A;
                EXPECTS(next_value(foo) == Flag2::B);
            },
          }, }
    };
} // namespace
