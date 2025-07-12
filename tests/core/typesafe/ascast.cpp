// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.core;
import stormkit.test;

#include <stormkit/test/test_macro.hpp>

using namespace stormkit::core;

namespace {
    using schar        = signed char;
    using uchar        = unsigned char;
    using sshort       = signed short;
    using ushort       = unsigned short;
    using sint         = signed int;
    using uint         = unsigned int;
    using slongint     = signed long int;
    using ulongint     = unsigned long int;
    using slonglongint = signed long long int;
    using ulonglongint = unsigned long long int;

    const auto char1 = char { 1 };
    const auto char2 = char { 2 };

    const auto signed_char1 = schar { 1 };
    const auto signed_char2 = schar { 2 };

    const auto unsigned_char1 = uchar { 1 };
    const auto unsigned_char2 = uchar { 2 };

    const auto signed_short1 = sshort { 1 };
    const auto signed_short2 = sshort { 2 };

    const auto unsigned_short1 = ushort { 1 };
    const auto unsigned_short2 = ushort { 2 };

    const auto signed_int1 = sint { 1 };
    const auto signed_int2 = sint { 2 };

    const auto unsigned_int1 = uint { 1 };
    const auto unsigned_int2 = uint { 2 };

    const auto signed_longint1 = slongint { 1 };
    const auto signed_longint2 = slongint { 2 };

    const auto unsigned_longint1 = ulongint { 1 };
    const auto unsigned_longint2 = ulongint { 2 };

    const auto signed_longlongint1 = slonglongint { 1 };
    const auto signed_longlongint2 = slonglongint { 2 };

    const auto unsigned_longlongint1 = ulonglongint { 1 };
    const auto unsigned_longlongint2 = ulonglongint { 2 };

    auto _ = test::TestSuite {
        "Core.typesafe",
        {
          { "AsCast.same_type",
            [] static noexcept {
                auto foo = i32 { 9 };
                EXPECTS(as<i32>(i32 { 1 }) == 1);
                EXPECTS(&as<i32>(foo) == &foo);
            } },
          { "AsCast.is_variant",
            [] static noexcept {
                auto variant = std::variant<bool, int> { false };

                EXPECTS(is<bool>(variant));
                EXPECTS(not is<int>(variant));
            } },
          { "AsCast.as_variant",
            [] static noexcept {
                using Variant1 = std::variant<bool, int>;

                auto       variant  = Variant1 { false };
                const auto variant2 = Variant1 { 9 };

                EXPECTS(as<int>(variant2) == 9);
                EXPECTS(as<bool>(variant) == false);
                as<bool>(variant) = true;
                EXPECTS(as<bool>(variant) == true);
                EXPECTS(as<bool>(std::move(variant)) == true);
                EXPECTS(as<int>(std::move(variant2)) == 9);
            } },
          { "AsCast.is_runtime_polymorphic",
            [] static noexcept {
                struct Foo {
                    virtual ~Foo() {}
                    virtual auto foo() -> void = 0;
                };
                struct Bar: Foo {
                    auto foo() -> void override {}
                };
                struct BarFoo: Foo {
                    auto foo() -> void override {}
                };

                auto bar = std::unique_ptr<Foo> { std::make_unique<Bar>() };

                EXPECTS(is<Bar>(bar));
                EXPECTS(not is<BarFoo>(bar));
                EXPECTS(is<Bar>(bar.get()));
                EXPECTS(not is<BarFoo>(bar.get()));
                EXPECTS(is<Bar>(*bar));
                EXPECTS(not is<BarFoo>(*bar));
            } },
          { "AsCast.as_runtime_polymorphic",
            [] static noexcept {
                struct Foo {
                    virtual ~Foo() {}
                    virtual auto foo() const -> int = 0;
                };
                struct Bar: Foo {
                    auto foo() const -> int override { return 1; }
                };
                struct BarFoo: Foo {
                    auto foo() const -> int override { return 5; }
                };

                auto bar = std::unique_ptr<Foo> { std::make_unique<Bar>() };
                auto xxx = as<Ref<Bar>>(bar);

                EXPECTS(as<Bar*>(bar)->foo() == 1);
                EXPECTS(as<Ref<Bar>>(bar)->foo() == 1);
                EXPECTS(as<Ref<const Bar>>(std::as_const(bar))->foo() == 1);
                EXPECTS(as<Bar&>(*bar).foo() == 1);
                EXPECTS(as<Ref<Bar>>(bar.get())->foo() == 1);
                EXPECTS(as<Bar&>(*(bar.get())).foo() == 1);
            } },
          // { "AsCast.as_byte(s)",
          //     [] static noexcept {
          //         struct Foo {
          //             int  a;
          //             int  b;
          //             char c;
          //         };

          //    auto bar           = Foo { 9, 3, 'x' };
          //    auto bar_byte_view = as<ByteView>(bar);
          //    auto bar_ptr       = as<Foo*>(bar_byte_view);

          //    auto bar2           = std::make_unique<Foo>(9, 3, 'x');
          //    auto bar_byte_view2 = as<ByteView>(bar2);
          //    auto bar_ptr2       = as<Ref<Foo>>(bar_byte_view2);

          //    auto bar3           = std::make_shared<Foo>(9, 3, 'x');
          //    auto bar_byte_view3 = as<ByteView>(bar3);
          //    auto bar_ptr3       = as<Ref<Foo>>(bar_byte_view3);

          //    auto bar4           = as_ref(bar);
          //    auto bar_byte_view4 = as<ByteView>(bar4);

          //    auto bar_ptr4 = as<Ref<Foo>>(bar_byte_view4);

          //    auto bar5           = &bar;
          //    auto bar_byte_view5 = as<ByteView>(bar5);
          //    auto bar_ptr5       = as<Ref<Foo>>(bar_byte_view5);

          //    EXPECTS(std::data(bar_byte_view) != nullptr);
          //    EXPECTS(bar_ptr->a == 9);
          //    EXPECTS(bar_ptr->b == 3);
          //    EXPECTS(bar_ptr->c == 'x');
          //    EXPECTS(std::data(bar_byte_view2) != nullptr);
          //    EXPECTS(bar_ptr2->a == 9);
          //    EXPECTS(bar_ptr2->b == 3);
          //    EXPECTS(bar_ptr2->c == 'x');
          //    EXPECTS(std::data(bar_byte_view3) != nullptr);
          //    EXPECTS(bar_ptr3->a == 9);
          //    EXPECTS(bar_ptr3->b == 3);
          //    EXPECTS(bar_ptr3->c == 'x');
          //    EXPECTS(std::data(bar_byte_view4) != nullptr);
          //    EXPECTS(bar_ptr4->a == 9);
          //    EXPECTS(bar_ptr4->b == 3);
          //    EXPECTS(bar_ptr4->c == 'x');
          // } },
          { "AsCast.as_enumeration",
            [] static noexcept {
                enum Foo : i32 {
                    A = 5,
                };
                enum class Bar : u32 {
                    A = 5,
                };

                auto foo = Foo::A;
                EXPECTS(as<Underlying>(foo) == 5);
                EXPECTS(as<Underlying>(Bar::A) == 5);
                EXPECTS(as<i32>(foo) == 5);
                EXPECTS(as<u32>(Bar::A) == 5);
                EXPECTS(as<Foo>(as<i32>(foo)) == Foo::A);
                EXPECTS(as<Underlying>(auto(foo)) == 5);
                EXPECTS(as<i32>(auto(foo)) == 5);
                EXPECTS(as<Foo>(as<i32>(auto(foo))) == Foo::A);
            } },
          { "AsCast.is_arithmetic",
            [] static noexcept {
                EXPECTS(is(7, 7));
                EXPECTS(is(19.f, 19.f));
                EXPECTS(is(-5., -5.));
            } },
          { "AsCast.as_arithmetic",
            [] static noexcept {
                static_assert(meta::IsSignNarrowing<int, unsigned int>);
                static_assert(meta::IsSignNarrowing<unsigned int, int>);
                static_assert(not meta::IsSignNarrowing<char, unsigned int>);
                static_assert(meta::IsSignNarrowing<unsigned int, char>);
                static_assert(meta::IsNarrowing<short, int>);
                static_assert(not meta::IsNarrowing<int, short>);
                static_assert(meta::IsNarrowing<int, unsigned int>);
                static_assert(meta::IsNarrowing<unsigned int, int>);

                using schar = signed char;
                EXPECTS(as<signed char>(127) == schar { 127 });
                EXPECTS(as<signed char>(-80) != schar { -81 });

                EXPECTS(as<int>(8u) == 8);
                EXPECTS(as<int>(1820) != 7);

                EXPECTS(as<unsigned int>(14) == 14u);

                EXPECTS(is(as<float>(19), 19));
                EXPECTS(is(as<float>(-5), -5.));
            } },
          { "AsCast.as_byte(s)",
            [] static noexcept {
                using uchar = unsigned char;
                using schar = signed char;

                static_assert(meta::IsByteNarrowing<std::byte, int>);
                static_assert(not meta::IsByteNarrowing<int, std::byte>);

                static_assert(is_safe_narrowing<std::byte>(5));
                static_assert(not is_safe_narrowing<std::byte>(-5));
                static_assert(is_safe_narrowing<int>(std::byte { 5 }));
                static_assert(not is_safe_narrowing<std::byte>(1000));

                EXPECTS(as<std::byte>(uchar { 8 }) == std::byte { 8 });
                EXPECTS(as<std::byte>(schar { 8 }) == std::byte { 8 });
                EXPECTS(as<std::byte>(char { 8 }) == std::byte { 8 });
                EXPECTS(as<int>(std::byte { 8 }) == 8);
            } },
          { "AsCast.narrow_integers",
            [] static noexcept {
                EXPECTS(narrow<char>(int { 1 }));
                EXPECTS(narrow<char>(int { 1 }));
                EXPECTS(narrow<char>(int { 1 }));
                EXPECTS(narrow<char>(int { 1 }));
                EXPECTS(narrow<char>(int { 1 }));
                EXPECTS(narrow<char>(int { 1 }));
            } },
          { "AsCast.is_same_float",
            [] {
                auto float1 = 0.1f;
                auto float2 = 0.2f;

                auto double1 = 0.1;
                auto double2 = 0.2;

                auto longdouble1 = 0.1l;
                auto longdouble2 = 0.2l;

                EXPECTS((is(float1, float1)));
                EXPECTS((is(double1, double1)));
                EXPECTS((is(longdouble1, longdouble1)));

                EXPECTS((not is(float1, float2)));
                EXPECTS((not is(float1, double2)));
                EXPECTS((not is(longdouble1, longdouble2)));

                float1 = 100.1f;
                float2 = 100.2f;

                double1 = 1000.1;
                double2 = 1000.2;

                longdouble1 = 10000.1l;
                longdouble2 = 10000.2l;

                EXPECTS((is(float1, float1)));
                EXPECTS((is(double1, double1)));
                EXPECTS((is(longdouble1, longdouble1)));

                EXPECTS((not is(float1, float2)));
                EXPECTS((not is(double1, double2)));
                EXPECTS((not is(longdouble1, longdouble2)));
            } },
          { "AsCast.is_different_float",
            [] {
                auto float1 = 0.1f;
                auto float2 = 0.2f;

                auto double1 = 0.1;
                auto double2 = 0.2;

                auto longdouble1 = 0.1l;
                auto longdouble2 = 0.2l;

                EXPECTS((is(float1, double1)));
                EXPECTS((is(double1, float1)));
                EXPECTS((is(float1, longdouble1)));
                EXPECTS((is(longdouble1, float1)));
                EXPECTS((is(double1, longdouble1)));
                EXPECTS((is(longdouble1, double1)));

                EXPECTS((not is(float1, double2)));
                EXPECTS((not is(double2, float1)));
                EXPECTS((not is(float2, double1)));
                EXPECTS((not is(double1, float2)));
                EXPECTS((not is(float1, longdouble2)));
                EXPECTS((not is(longdouble2, float1)));
                EXPECTS((not is(float2, longdouble1)));
                EXPECTS((not is(longdouble1, float2)));
                EXPECTS((not is(double1, longdouble2)));
                EXPECTS((not is(longdouble2, double1)));
                EXPECTS((not is(double2, longdouble1)));
                EXPECTS((not is(longdouble1, double2)));

                float1 = 1000.1f;
                float2 = 1000.2f;

                double1 = 1000.1;
                double2 = 1000.2;

                longdouble1 = 1000.1l;
                longdouble2 = 1000.2l;

                EXPECTS((is(float1, double1)));
                EXPECTS((is(double1, float1)));
                EXPECTS((is(float1, longdouble1)));
                EXPECTS((is(longdouble1, float1)));
                EXPECTS((is(double1, longdouble1)));
                EXPECTS((is(longdouble1, double1)));

                EXPECTS((not is(float1, double2)));
                EXPECTS((not is(double2, float1)));
                EXPECTS((not is(float2, double1)));
                EXPECTS((not is(double1, float2)));
                EXPECTS((not is(float1, longdouble2)));
                EXPECTS((not is(longdouble2, float1)));
                EXPECTS((not is(float2, longdouble1)));
                EXPECTS((not is(longdouble1, float2)));
                EXPECTS((not is(double1, longdouble2)));
                EXPECTS((not is(longdouble2, double1)));
                EXPECTS((not is(double2, longdouble1)));
                EXPECTS((not is(longdouble1, double2)));
            } },
          { "AsCast.is_same_integer",
            [] {
                EXPECTS((is(char1, char1)));

                EXPECTS((is(signed_char1, signed_char1)));
                EXPECTS((is(unsigned_char1, unsigned_char1)));

                EXPECTS((is(signed_short1, signed_short1)));
                EXPECTS((is(unsigned_short1, unsigned_short1)));

                EXPECTS((is(signed_int1, signed_int1)));
                EXPECTS((is(unsigned_int1, unsigned_int1)));

                EXPECTS((is(signed_longint1, signed_longint1)));
                EXPECTS((is(unsigned_longint1, unsigned_longint1)));

                EXPECTS((is(signed_longlongint1, signed_longlongint1)));
                EXPECTS((is(unsigned_longlongint1, unsigned_longlongint1)));

                EXPECTS((not is(char1, char2)));

                EXPECTS((not is(signed_char1, signed_char2)));
                EXPECTS((not is(unsigned_char1, unsigned_char2)));

                EXPECTS((not is(signed_short1, signed_short2)));
                EXPECTS((not is(unsigned_short1, unsigned_short2)));

                EXPECTS((not is(signed_int1, signed_int2)));
                EXPECTS((not is(unsigned_int1, unsigned_int2)));

                EXPECTS((not is(signed_longint1, signed_longint2)));
                EXPECTS((not is(unsigned_longint1, unsigned_longint2)));

                EXPECTS((not is(signed_longlongint1, signed_longlongint2)));
                EXPECTS((not is(unsigned_longlongint1, unsigned_longlongint2)));
            } },
          { "AsCast.is_different_integer",
            [] {
                // char
                EXPECTS((is(char1, signed_char1)));
                EXPECTS((is(char1, unsigned_char1)));

                EXPECTS((is(char1, signed_short1)));
                EXPECTS((is(char1, unsigned_short1)));

                EXPECTS((is(char1, signed_int1)));
                EXPECTS((is(char1, unsigned_int1)));

                EXPECTS((is(char1, signed_longint1)));
                EXPECTS((is(char1, unsigned_longint1)));

                EXPECTS((is(char1, signed_longlongint1)));
                EXPECTS((is(char1, unsigned_longlongint1)));

                // signed char
                EXPECTS((is(signed_char1, char1)));
                EXPECTS((is(signed_char1, unsigned_char1)));

                EXPECTS((is(signed_char1, signed_short1)));
                EXPECTS((is(signed_char1, unsigned_short1)));

                EXPECTS((is(signed_char1, signed_int1)));
                EXPECTS((is(signed_char1, unsigned_int1)));

                EXPECTS((is(signed_char1, signed_longint1)));
                EXPECTS((is(signed_char1, unsigned_longint1)));

                EXPECTS((is(signed_char1, signed_longlongint1)));
                EXPECTS((is(signed_char1, unsigned_longlongint1)));

                // unsigned char
                EXPECTS((is(unsigned_char1, char1)));
                EXPECTS((is(unsigned_char1, signed_char1)));

                EXPECTS((is(unsigned_char1, signed_short1)));
                EXPECTS((is(unsigned_char1, unsigned_short1)));

                EXPECTS((is(unsigned_char1, signed_int1)));
                EXPECTS((is(unsigned_char1, unsigned_int1)));

                EXPECTS((is(unsigned_char1, signed_longint1)));
                EXPECTS((is(unsigned_char1, unsigned_longint1)));

                EXPECTS((is(unsigned_char1, signed_longlongint1)));
                EXPECTS((is(unsigned_char1, unsigned_longlongint1)));

                // signed short
                EXPECTS((is(signed_short1, char1)));

                EXPECTS((is(signed_short1, signed_char1)));
                EXPECTS((is(signed_short1, unsigned_char1)));

                EXPECTS((is(signed_short1, unsigned_short1)));

                EXPECTS((is(signed_short1, signed_int1)));
                EXPECTS((is(signed_short1, unsigned_int1)));

                EXPECTS((is(signed_short1, signed_longint1)));
                EXPECTS((is(signed_short1, unsigned_longint1)));

                EXPECTS((is(signed_short1, signed_longlongint1)));
                EXPECTS((is(signed_short1, unsigned_longlongint1)));

                // unsigned short
                EXPECTS((is(unsigned_short1, char1)));
                EXPECTS((is(unsigned_short1, signed_char1)));
                EXPECTS((is(unsigned_short1, unsigned_char1)));

                EXPECTS((is(unsigned_short1, signed_short1)));

                EXPECTS((is(unsigned_short1, signed_int1)));
                EXPECTS((is(unsigned_short1, unsigned_int1)));

                EXPECTS((is(unsigned_short1, signed_longint1)));
                EXPECTS((is(unsigned_short1, unsigned_longint1)));

                EXPECTS((is(unsigned_short1, signed_longlongint1)));
                EXPECTS((is(unsigned_short1, unsigned_longlongint1)));

                // signed int
                EXPECTS((is(signed_int1, char1)));

                EXPECTS((is(signed_int1, signed_char1)));
                EXPECTS((is(signed_int1, unsigned_char1)));

                EXPECTS((is(signed_int1, signed_short1)));
                EXPECTS((is(signed_int1, unsigned_short1)));

                EXPECTS((is(signed_int1, unsigned_int1)));

                EXPECTS((is(signed_int1, signed_longint1)));
                EXPECTS((is(signed_int1, unsigned_longint1)));

                EXPECTS((is(signed_int1, signed_longlongint1)));
                EXPECTS((is(signed_int1, unsigned_longlongint1)));

                // unsigned int
                EXPECTS((is(unsigned_int1, char1)));
                EXPECTS((is(unsigned_int1, signed_char1)));
                EXPECTS((is(unsigned_int1, unsigned_char1)));

                EXPECTS((is(unsigned_int1, signed_short1)));
                EXPECTS((is(unsigned_int1, unsigned_short1)));

                EXPECTS((is(unsigned_int1, signed_int1)));

                EXPECTS((is(unsigned_int1, signed_longint1)));
                EXPECTS((is(unsigned_int1, unsigned_longint1)));

                EXPECTS((is(unsigned_int1, signed_longlongint1)));
                EXPECTS((is(unsigned_int1, unsigned_longlongint1)));

                // signed longint
                EXPECTS((is(signed_longint1, char1)));

                EXPECTS((is(signed_longint1, signed_char1)));
                EXPECTS((is(signed_longint1, unsigned_char1)));

                EXPECTS((is(signed_longint1, signed_short1)));
                EXPECTS((is(signed_longint1, unsigned_short1)));

                EXPECTS((is(signed_longint1, unsigned_int1)));

                EXPECTS((is(signed_longint1, unsigned_longint1)));

                EXPECTS((is(signed_longint1, signed_longlongint1)));
                EXPECTS((is(signed_longint1, unsigned_longlongint1)));

                // unsigned longint
                EXPECTS((is(unsigned_longint1, char1)));
                EXPECTS((is(unsigned_longint1, signed_char1)));
                EXPECTS((is(unsigned_longint1, unsigned_char1)));

                EXPECTS((is(unsigned_longint1, signed_short1)));
                EXPECTS((is(unsigned_longint1, unsigned_short1)));

                EXPECTS((is(unsigned_longint1, signed_int1)));
                EXPECTS((is(unsigned_longint1, unsigned_int1)));

                EXPECTS((is(unsigned_longint1, signed_longint1)));

                EXPECTS((is(unsigned_longint1, signed_longlongint1)));
                EXPECTS((is(unsigned_longint1, unsigned_longlongint1)));

                // signed longlongint
                EXPECTS((is(signed_longlongint1, char1)));

                EXPECTS((is(signed_longlongint1, signed_char1)));
                EXPECTS((is(signed_longlongint1, unsigned_char1)));

                EXPECTS((is(signed_longlongint1, signed_short1)));
                EXPECTS((is(signed_longlongint1, unsigned_short1)));

                EXPECTS((is(signed_longlongint1, unsigned_int1)));

                EXPECTS((is(signed_longlongint1, signed_longint1)));
                EXPECTS((is(signed_longlongint1, unsigned_longint1)));

                EXPECTS((is(signed_longlongint1, unsigned_longlongint1)));

                // unsigned longlongint
                EXPECTS((is(unsigned_longlongint1, char1)));
                EXPECTS((is(unsigned_longlongint1, signed_char1)));
                EXPECTS((is(unsigned_longlongint1, unsigned_char1)));

                EXPECTS((is(unsigned_longlongint1, signed_short1)));
                EXPECTS((is(unsigned_longlongint1, unsigned_short1)));

                EXPECTS((is(unsigned_longlongint1, signed_int1)));
                EXPECTS((is(unsigned_longlongint1, unsigned_int1)));

                EXPECTS((is(unsigned_longlongint1, signed_longint1)));
                EXPECTS((is(unsigned_longlongint1, unsigned_longint1)));

                EXPECTS((is(unsigned_longlongint1, signed_longlongint1)));

                // char
                EXPECTS((not is(char1, signed_char2)));
                EXPECTS((not is(char1, unsigned_char2)));

                EXPECTS((not is(char1, signed_short2)));
                EXPECTS((not is(char1, unsigned_short2)));

                EXPECTS((not is(char1, signed_int2)));
                EXPECTS((not is(char1, unsigned_int2)));

                EXPECTS((not is(char1, signed_longint2)));
                EXPECTS((not is(char1, unsigned_longint2)));

                EXPECTS((not is(char1, signed_longlongint2)));
                EXPECTS((not is(char1, unsigned_longlongint2)));

                // signed char
                EXPECTS((not is(signed_char1, char2)));
                EXPECTS((not is(signed_char1, unsigned_char2)));

                EXPECTS((not is(signed_char1, signed_short2)));
                EXPECTS((not is(signed_char1, unsigned_short2)));

                EXPECTS((not is(signed_char1, signed_int2)));
                EXPECTS((not is(signed_char1, unsigned_int2)));

                EXPECTS((not is(signed_char1, signed_longint2)));
                EXPECTS((not is(signed_char1, unsigned_longint2)));

                EXPECTS((not is(signed_char1, signed_longlongint2)));
                EXPECTS((not is(signed_char1, unsigned_longlongint2)));

                // unsigned char
                EXPECTS((not is(unsigned_char1, char2)));
                EXPECTS((not is(unsigned_char1, signed_char2)));

                EXPECTS((not is(unsigned_char1, signed_short2)));
                EXPECTS((not is(unsigned_char1, unsigned_short2)));

                EXPECTS((not is(unsigned_char1, signed_int2)));
                EXPECTS((not is(unsigned_char1, unsigned_int2)));

                EXPECTS((not is(unsigned_char1, signed_longint2)));
                EXPECTS((not is(unsigned_char1, unsigned_longint2)));

                EXPECTS((not is(unsigned_char1, signed_longlongint2)));
                EXPECTS((not is(unsigned_char1, unsigned_longlongint2)));

                // signed short
                EXPECTS((not is(signed_short1, char2)));

                EXPECTS((not is(signed_short1, signed_char2)));
                EXPECTS((not is(signed_short1, unsigned_char2)));

                EXPECTS((not is(signed_short1, unsigned_short2)));

                EXPECTS((not is(signed_short1, signed_int2)));
                EXPECTS((not is(signed_short1, unsigned_int2)));

                EXPECTS((not is(signed_short1, signed_longint2)));
                EXPECTS((not is(signed_short1, unsigned_longint2)));

                EXPECTS((not is(signed_short1, signed_longlongint2)));
                EXPECTS((not is(signed_short1, unsigned_longlongint2)));

                // unsigned short
                EXPECTS((not is(unsigned_short1, char2)));
                EXPECTS((not is(unsigned_short1, signed_char2)));
                EXPECTS((not is(unsigned_short1, unsigned_char2)));

                EXPECTS((not is(unsigned_short1, signed_short2)));

                EXPECTS((not is(unsigned_short1, signed_int2)));
                EXPECTS((not is(unsigned_short1, unsigned_int2)));

                EXPECTS((not is(unsigned_short1, signed_longint2)));
                EXPECTS((not is(unsigned_short1, unsigned_longint2)));

                EXPECTS((not is(unsigned_short1, signed_longlongint2)));
                EXPECTS((not is(unsigned_short1, unsigned_longlongint2)));

                // signed int
                EXPECTS((not is(signed_int1, char2)));

                EXPECTS((not is(signed_int1, signed_char2)));
                EXPECTS((not is(signed_int1, unsigned_char2)));

                EXPECTS((not is(signed_int1, signed_short2)));
                EXPECTS((not is(signed_int1, unsigned_short2)));

                EXPECTS((not is(signed_int1, unsigned_int2)));

                EXPECTS((not is(signed_int1, signed_longint2)));
                EXPECTS((not is(signed_int1, unsigned_longint2)));

                EXPECTS((not is(signed_int1, signed_longlongint2)));
                EXPECTS((not is(signed_int1, unsigned_longlongint2)));

                // unsigned int
                EXPECTS((not is(unsigned_int1, char2)));
                EXPECTS((not is(unsigned_int1, signed_char2)));
                EXPECTS((not is(unsigned_int1, unsigned_char2)));

                EXPECTS((not is(unsigned_int1, signed_short2)));
                EXPECTS((not is(unsigned_int1, unsigned_short2)));

                EXPECTS((not is(unsigned_int1, signed_int2)));

                EXPECTS((not is(unsigned_int1, signed_longint2)));
                EXPECTS((not is(unsigned_int1, unsigned_longint2)));

                EXPECTS((not is(unsigned_int1, signed_longlongint2)));
                EXPECTS((not is(unsigned_int1, unsigned_longlongint2)));

                // signed longint
                EXPECTS((not is(signed_longint1, char2)));

                EXPECTS((not is(signed_longint1, signed_char2)));
                EXPECTS((not is(signed_longint1, unsigned_char2)));

                EXPECTS((not is(signed_longint1, signed_short2)));
                EXPECTS((not is(signed_longint1, unsigned_short2)));

                EXPECTS((not is(signed_longint1, unsigned_int2)));

                EXPECTS((not is(signed_longint1, unsigned_longint2)));

                EXPECTS((not is(signed_longint1, signed_longlongint2)));
                EXPECTS((not is(signed_longint1, unsigned_longlongint2)));

                // unsigned longint
                EXPECTS((not is(unsigned_longint1, char2)));
                EXPECTS((not is(unsigned_longint1, signed_char2)));
                EXPECTS((not is(unsigned_longint1, unsigned_char2)));

                EXPECTS((not is(unsigned_longint1, signed_short2)));
                EXPECTS((not is(unsigned_longint1, unsigned_short2)));

                EXPECTS((not is(unsigned_longint1, signed_int2)));
                EXPECTS((not is(unsigned_longint1, unsigned_int2)));

                EXPECTS((not is(unsigned_longint1, signed_longint2)));

                EXPECTS((not is(unsigned_longint1, signed_longlongint2)));
                EXPECTS((not is(unsigned_longint1, unsigned_longlongint2)));

                // signed longlongint
                EXPECTS((not is(signed_longlongint1, char2)));

                EXPECTS((not is(signed_longlongint1, signed_char2)));
                EXPECTS((not is(signed_longlongint1, unsigned_char2)));

                EXPECTS((not is(signed_longlongint1, signed_short2)));
                EXPECTS((not is(signed_longlongint1, unsigned_short2)));

                EXPECTS((not is(signed_longlongint1, unsigned_int2)));

                EXPECTS((not is(signed_longlongint1, signed_longint2)));
                EXPECTS((not is(signed_longlongint1, unsigned_longint2)));

                EXPECTS((not is(signed_longlongint1, unsigned_longlongint2)));

                // unsigned longlongint
                EXPECTS((not is(unsigned_longlongint1, char2)));
                EXPECTS((not is(unsigned_longlongint1, signed_char2)));
                EXPECTS((not is(unsigned_longlongint1, unsigned_char2)));

                EXPECTS((not is(unsigned_longlongint1, signed_short2)));
                EXPECTS((not is(unsigned_longlongint1, unsigned_short2)));

                EXPECTS((not is(unsigned_longlongint1, signed_int2)));
                EXPECTS((not is(unsigned_longlongint1, unsigned_int2)));

                EXPECTS((not is(unsigned_longlongint1, signed_longint2)));
                EXPECTS((not is(unsigned_longlongint1, unsigned_longint2)));

                EXPECTS((not is(unsigned_longlongint1, signed_longlongint2)));
            } },
          }
    };
} // namespace
