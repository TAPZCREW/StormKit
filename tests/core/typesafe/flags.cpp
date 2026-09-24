// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.core;
import stormkit.test;

#include <stormkit/test/test_macro.hpp>

using namespace stormkit::core;

enum class flag {
    A = 1,
    B = 2,
    C = 4,
};

enum flag2 {
    A = 1,
    B = 2,
    C = 4,
};

template<>
inline constexpr auto stormkit::core::meta::FLAG_TRAIT<flag> = true;

template<>
inline constexpr auto stormkit::core::meta::FLAG_TRAIT<flag2> = true;

namespace {
    auto _ = test::test_suite {
        "core.typesafe",
        {
          {
            "flags.enum_class.operators",
            [] static noexcept {
                {
                    auto foo = flag::A | flag::B;
                    EXPECTS(foo == (flag::A | flag::B));
                }

                {
                    auto foo = (flag::A ^ flag::A | flag::B);
                    EXPECTS(foo == flag::B);
                }

                {
                    auto foo = (flag::A | flag::B);
                    foo ^= flag::A;
                    EXPECTS(foo == flag::B);
                }

                {
                    auto foo = (flag::A | flag::B);
                    foo      = foo & flag::B;
                    EXPECTS(foo == flag::B);
                }

                {
                    auto foo = (flag::A | flag::B);
                    foo &= flag::B;
                    EXPECTS(foo == flag::B);
                }

                {
                    auto foo = ~(flag::B);
                    EXPECTS(foo == ~(flag::B));
                }
            },
          }, {
            "flags.enum_class.has_flag_bit",
            [] static noexcept {
                auto foo = flag::A | flag::B;
                EXPECTS(has_flag_bit(foo, flag::A));
            },
          }, {
            "flags.enum.operators",
            [] static noexcept {
                {
                    auto foo = flag2::A | flag2::B;
                    EXPECTS(foo == (flag2::A | flag2::B));
                }

                {
                    auto foo = (flag2::A ^ flag2::A | flag2::B);
                    EXPECTS(foo == flag2::B);
                }

                {
                    auto foo = (flag2::A | flag2::B);
                    foo ^= flag2::A;
                    EXPECTS(foo == flag2::B);
                }

                {
                    auto foo = (flag2::A | flag2::B);
                    foo      = foo & flag2::B;
                    EXPECTS(foo == flag2::B);
                }

                {
                    auto foo = (flag2::A | flag2::B);
                    foo &= flag2::B;
                    EXPECTS(foo == flag2::B);
                }

                {
                    auto foo = (~flag2::B);
                    EXPECTS(foo == static_cast<int>(~flag2::B));
                }
            },
          }, {
            "flags.enum.has_flag_bit",
            [] static noexcept {
                auto foo = flag2::A | flag2::B;
                EXPECTS(has_flag_bit(foo, flag2::A));
            },
          }, }
    };
} // namespace
