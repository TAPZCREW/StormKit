// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.core;

import Test;

#include <stormkit/core/flags_macro.hpp>

using namespace stormkit::core;

#define expects(x) test::expects(x, #x)

enum class Flag {
    A = 1,
    B = 2,
    C = 4
};

enum Flag2 {
    A = 1,
    B = 2,
    C = 4
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
                        expects(foo == (Flag::A | Flag::B));
                    }

                    {
                        auto foo = (Flag::A ^ Flag::A | Flag::B);
                        expects(foo == Flag::B);
                    }

                    {
                        auto foo = (Flag::A | Flag::B);
                        foo ^= Flag::A;
                        expects(foo == Flag::B);
                    }

                    {
                        auto foo = (Flag::A | Flag::B);
                        foo      = foo & Flag::B;
                        expects(foo == Flag::B);
                    }

                    {
                        auto foo = (Flag::A | Flag::B);
                        foo &= Flag::B;
                        expects(foo == Flag::B);
                    }

                    {
                        auto foo = ~(Flag::B);
                        expects(foo == ~Flag::B);
                    }
                },
            }, {
                "Flags.enum_class.checkFlag",
                [] static noexcept {
                    auto foo = Flag::A | Flag::B;
                    expects(checkFlag(foo, Flag::A));
                },
            }, {
                "Flags.enum_class.next_value",
                [] static noexcept {
                    auto foo = Flag::A;
                    expects(next_value(foo) == Flag::B);
                },
            }, {
                "Flags.enum.operators",
                [] static noexcept {
                    {
                        auto foo = Flag2::A | Flag2::B;
                        expects(foo == (Flag2::A | Flag2::B));
                    }

                    {
                        auto foo = (Flag2::A ^ Flag2::A | Flag2::B);
                        expects(foo == Flag2::B);
                    }

                    {
                        auto foo = (Flag2::A | Flag2::B);
                        foo ^= Flag2::A;
                        expects(foo == Flag2::B);
                    }

                    {
                        auto foo = (Flag2::A | Flag2::B);
                        foo      = foo & Flag2::B;
                        expects(foo == Flag2::B);
                    }

                    {
                        auto foo = (Flag2::A | Flag2::B);
                        foo &= Flag2::B;
                        expects(foo == Flag2::B);
                    }

                    {
                        auto foo = ~(Flag2::B);
                        expects(foo == ~Flag2::B);
                    }
                },
            }, {
                "Flags.enum.checkFlag",
                [] static noexcept {
                    auto foo = Flag2::A | Flag2::B;
                    expects(checkFlag(foo, Flag2::A));
                },
            }, {
                "Flags.enum.next_value",
                [] static noexcept {
                    auto foo = Flag2::A;
                    expects(next_value(foo) == Flag2::B);
                },
            }, }
    };
} // namespace
