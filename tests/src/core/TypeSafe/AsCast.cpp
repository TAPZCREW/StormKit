// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.Core;

import Test;

using namespace stormkit::core;

#define expects(x) test::expects(x, #x)

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
        "Core.TypeSafe",
        {
          { "AsCast.same_type",
              [] static noexcept {
                  auto foo = Int { 9 };
                  expects(as<Int>(Int { 1 }) == 1);
                  expects(&as<Int>(foo) == &foo);
              } },
          { "AsCast.is_variant",
              [] static noexcept {
                  auto variant = std::variant<bool, int> { false };

                  expects(is<bool>(variant));
                  expects(not is<int>(variant));
              } },
          { "AsCast.as_variant",
              [] static noexcept {
                  using Variant1 = std::variant<bool, int>;

                  auto       variant  = Variant1 { false };
                  const auto variant2 = Variant1 { 9 };

                  expects(as<int>(variant2) == 9);
                  expects(as<bool>(variant) == false);
                  as<bool>(variant) = true;
                  expects(as<bool>(variant) == true);
                  expects(as<bool>(std::move(variant)) == true);
                  expects(as<int>(std::move(variant2)) == 9);
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

                  expects(is<Bar>(bar));
                  expects(not is<BarFoo>(bar));
                  expects(is<Bar>(bar.get()));
                  expects(not is<BarFoo>(bar.get()));
                  expects(is<Bar>(*bar));
                  expects(not is<BarFoo>(*bar));
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

                  expects(as<Bar*>(bar)->foo() == 1);
                  expects(as<Ref<Bar>>(bar)->foo() == 1);
                  expects(as<Ref<const Bar>>(std::as_const(bar))->foo() == 1);
                  expects(as<Bar&>(*bar).foo() == 1);
                  expects(as<Ref<Bar>>(bar.get())->foo() == 1);
                  expects(as<Bar&>(*(bar.get())).foo() == 1);
              } },
          { "AsCast.as_byte(s)",
              [] static noexcept {
                  struct Foo {
                      int  a;
                      int  b;
                      char c;
                  };

                  auto bar           = Foo { 9, 3, 'x' };
                  auto bar_byte_view = as<ByteView>(bar);
                  auto bar_ptr       = as<Foo*>(bar_byte_view);

                  auto bar2           = std::make_unique<Foo>(9, 3, 'x');
                  auto bar_byte_view2 = as<ByteView>(bar2);
                  auto bar_ptr2       = as<Ref<Foo>>(bar_byte_view2);

                  auto bar3           = std::make_shared<Foo>(9, 3, 'x');
                  auto bar_byte_view3 = as<ByteView>(bar3);
                  auto bar_ptr3       = as<Ref<Foo>>(bar_byte_view3);

                  auto bar4           = borrow(bar);
                  auto bar_byte_view4 = as<ByteView>(bar4);

                  auto bar_ptr4       = as<Ref<Foo>>(bar_byte_view4);

                  auto bar5           = &bar;
                  auto bar_byte_view5 = as<ByteView>(bar5);
                  auto bar_ptr5       = as<Ref<Foo>>(bar_byte_view5);

                  expects(std::data(bar_byte_view) != nullptr);
                  expects(bar_ptr->a == 9);
                  expects(bar_ptr->b == 3);
                  expects(bar_ptr->c == 'x');
                  expects(std::data(bar_byte_view2) != nullptr);
                  expects(bar_ptr2->a == 9);
                  expects(bar_ptr2->b == 3);
                  expects(bar_ptr2->c == 'x');
                  expects(std::data(bar_byte_view3) != nullptr);
                  expects(bar_ptr3->a == 9);
                  expects(bar_ptr3->b == 3);
                  expects(bar_ptr3->c == 'x');
                  expects(std::data(bar_byte_view4) != nullptr);
                  expects(bar_ptr4->a == 9);
                  expects(bar_ptr4->b == 3);
                  expects(bar_ptr4->c == 'x');
              } },
          { "AsCast.as_enumeration",
              [] static noexcept {
                  enum Foo : Int {
                      A = 5
                  };
                  enum class Bar : UInt {
                      A = 5
                  };

                  auto foo = Foo::A;
                  expects(as<Underlying>(foo) == 5);
                  expects(as<Underlying>(Bar::A) == 5);
                  expects(as<Int>(foo) == 5);
                  expects(as<UInt>(Bar::A) == 5);
                  expects(as<Foo>(as<Int>(foo)) == Foo::A);
                  expects(as<Underlying>(auto(foo)) == 5);
                  expects(as<Int>(auto(foo)) == 5);
                  expects(as<Foo>(as<Int>(auto(foo))) == Foo::A);
              } },
          { "AsCast.is_arithmetic",
              [] static noexcept {
                  expects(is(7, 7));
                  expects(is(19.f, 19.f));
                  expects(is(-5., -5.));
              } },
          { "AsCast.as_arithmetic",
              [] static noexcept {
                  static_assert(IsSignNarrowing<int, unsigned int>);
                  static_assert(IsSignNarrowing<unsigned int, int>);
                  static_assert(not IsSignNarrowing<char, unsigned int>);
                  static_assert(IsSignNarrowing<unsigned int, char>);
                  static_assert(IsNarrowing<short, int>);
                  static_assert(not IsNarrowing<int, short>);
                  static_assert(IsNarrowing<int, unsigned int>);
                  static_assert(IsNarrowing<unsigned int, int>);

                  using schar = signed char;
                  expects(as<signed char>(127) == schar { 127 });
                  expects(as<signed char>(-80) != schar { -81 });

                  expects(as<int>(8u) == 8);
                  expects(as<int>(1820) != 7);

                  expects(as<unsigned int>(14) == 14u);

                  expects(is(as<float>(19), 19));
                  expects(is(as<float>(-5), -5.));
              } },
          { "AsCast.as_byte(s)",
              [] static noexcept {
                  using uchar = unsigned char;
                  using schar = signed char;

                  static_assert(IsByteNarrowing<std::byte, int>);
                  static_assert(not IsByteNarrowing<int, std::byte>);

                  static_assert(isSafeNarrowing<std::byte>(5));
                  static_assert(not isSafeNarrowing<std::byte>(-5));
                  static_assert(isSafeNarrowing<int>(std::byte { 5 }));
                  static_assert(not isSafeNarrowing<std::byte>(1000));

                  expects(as<std::byte>(uchar { 8 }) == std::byte { 8 });
                  expects(as<std::byte>(schar { 8 }) == std::byte { 8 });
                  expects(as<std::byte>(char { 8 }) == std::byte { 8 });
                  expects(as<int>(std::byte { 8 }) == 8);
              } },
          { "AsCast.narrow_integers",
              [] static noexcept {
                  expects(narrow<char>(int { 1 }));
                  expects(narrow<char>(int { 1 }));
                  expects(narrow<char>(int { 1 }));
                  expects(narrow<char>(int { 1 }));
                  expects(narrow<char>(int { 1 }));
                  expects(narrow<char>(int { 1 }));
              } },
          { "AsCast.is_same_float",
              [] {
                  auto float1 = 0.1f;
                  auto float2 = 0.2f;

                  auto double1 = 0.1;
                  auto double2 = 0.2;

                  auto longdouble1 = 0.1l;
                  auto longdouble2 = 0.2l;

                  expects((is(float1, float1)));
                  expects((is(double1, double1)));
                  expects((is(longdouble1, longdouble1)));

                  expects((not is(float1, float2)));
                  expects((not is(float1, double2)));
                  expects((not is(longdouble1, longdouble2)));

                  float1 = 100.1f;
                  float2 = 100.2f;

                  double1 = 1000.1;
                  double2 = 1000.2;

                  longdouble1 = 10000.1l;
                  longdouble2 = 10000.2l;

                  expects((is(float1, float1)));
                  expects((is(double1, double1)));
                  expects((is(longdouble1, longdouble1)));

                  expects((not is(float1, float2)));
                  expects((not is(double1, double2)));
                  expects((not is(longdouble1, longdouble2)));
              } },
          { "AsCast.is_different_float",
              [] {
                  auto float1 = 0.1f;
                  auto float2 = 0.2f;

                  auto double1 = 0.1;
                  auto double2 = 0.2;

                  auto longdouble1 = 0.1l;
                  auto longdouble2 = 0.2l;

                  expects((is(float1, double1)));
                  expects((is(double1, float1)));
                  expects((is(float1, longdouble1)));
                  expects((is(longdouble1, float1)));
                  expects((is(double1, longdouble1)));
                  expects((is(longdouble1, double1)));

                  expects((not is(float1, double2)));
                  expects((not is(double2, float1)));
                  expects((not is(float2, double1)));
                  expects((not is(double1, float2)));
                  expects((not is(float1, longdouble2)));
                  expects((not is(longdouble2, float1)));
                  expects((not is(float2, longdouble1)));
                  expects((not is(longdouble1, float2)));
                  expects((not is(double1, longdouble2)));
                  expects((not is(longdouble2, double1)));
                  expects((not is(double2, longdouble1)));
                  expects((not is(longdouble1, double2)));

                  float1 = 1000.1f;
                  float2 = 1000.2f;

                  double1 = 1000.1;
                  double2 = 1000.2;

                  longdouble1 = 1000.1l;
                  longdouble2 = 1000.2l;

                  expects((is(float1, double1)));
                  expects((is(double1, float1)));
                  expects((is(float1, longdouble1)));
                  expects((is(longdouble1, float1)));
                  expects((is(double1, longdouble1)));
                  expects((is(longdouble1, double1)));

                  expects((not is(float1, double2)));
                  expects((not is(double2, float1)));
                  expects((not is(float2, double1)));
                  expects((not is(double1, float2)));
                  expects((not is(float1, longdouble2)));
                  expects((not is(longdouble2, float1)));
                  expects((not is(float2, longdouble1)));
                  expects((not is(longdouble1, float2)));
                  expects((not is(double1, longdouble2)));
                  expects((not is(longdouble2, double1)));
                  expects((not is(double2, longdouble1)));
                  expects((not is(longdouble1, double2)));
              } },
          { "AsCast.is_same_integer",
              [] {
                  expects((is(char1, char1)));

                  expects((is(signed_char1, signed_char1)));
                  expects((is(unsigned_char1, unsigned_char1)));

                  expects((is(signed_short1, signed_short1)));
                  expects((is(unsigned_short1, unsigned_short1)));

                  expects((is(signed_int1, signed_int1)));
                  expects((is(unsigned_int1, unsigned_int1)));

                  expects((is(signed_longint1, signed_longint1)));
                  expects((is(unsigned_longint1, unsigned_longint1)));

                  expects((is(signed_longlongint1, signed_longlongint1)));
                  expects((is(unsigned_longlongint1, unsigned_longlongint1)));

                  expects((not is(char1, char2)));

                  expects((not is(signed_char1, signed_char2)));
                  expects((not is(unsigned_char1, unsigned_char2)));

                  expects((not is(signed_short1, signed_short2)));
                  expects((not is(unsigned_short1, unsigned_short2)));

                  expects((not is(signed_int1, signed_int2)));
                  expects((not is(unsigned_int1, unsigned_int2)));

                  expects((not is(signed_longint1, signed_longint2)));
                  expects((not is(unsigned_longint1, unsigned_longint2)));

                  expects((not is(signed_longlongint1, signed_longlongint2)));
                  expects((not is(unsigned_longlongint1, unsigned_longlongint2)));
              } },
          { "AsCast.is_different_integer",
              [] {
                  // char
                  expects((is(char1, signed_char1)));
                  expects((is(char1, unsigned_char1)));

                  expects((is(char1, signed_short1)));
                  expects((is(char1, unsigned_short1)));

                  expects((is(char1, signed_int1)));
                  expects((is(char1, unsigned_int1)));

                  expects((is(char1, signed_longint1)));
                  expects((is(char1, unsigned_longint1)));

                  expects((is(char1, signed_longlongint1)));
                  expects((is(char1, unsigned_longlongint1)));

                  // signed char
                  expects((is(signed_char1, char1)));
                  expects((is(signed_char1, unsigned_char1)));

                  expects((is(signed_char1, signed_short1)));
                  expects((is(signed_char1, unsigned_short1)));

                  expects((is(signed_char1, signed_int1)));
                  expects((is(signed_char1, unsigned_int1)));

                  expects((is(signed_char1, signed_longint1)));
                  expects((is(signed_char1, unsigned_longint1)));

                  expects((is(signed_char1, signed_longlongint1)));
                  expects((is(signed_char1, unsigned_longlongint1)));

                  // unsigned char
                  expects((is(unsigned_char1, char1)));
                  expects((is(unsigned_char1, signed_char1)));

                  expects((is(unsigned_char1, signed_short1)));
                  expects((is(unsigned_char1, unsigned_short1)));

                  expects((is(unsigned_char1, signed_int1)));
                  expects((is(unsigned_char1, unsigned_int1)));

                  expects((is(unsigned_char1, signed_longint1)));
                  expects((is(unsigned_char1, unsigned_longint1)));

                  expects((is(unsigned_char1, signed_longlongint1)));
                  expects((is(unsigned_char1, unsigned_longlongint1)));

                  // signed short
                  expects((is(signed_short1, char1)));

                  expects((is(signed_short1, signed_char1)));
                  expects((is(signed_short1, unsigned_char1)));

                  expects((is(signed_short1, unsigned_short1)));

                  expects((is(signed_short1, signed_int1)));
                  expects((is(signed_short1, unsigned_int1)));

                  expects((is(signed_short1, signed_longint1)));
                  expects((is(signed_short1, unsigned_longint1)));

                  expects((is(signed_short1, signed_longlongint1)));
                  expects((is(signed_short1, unsigned_longlongint1)));

                  // unsigned short
                  expects((is(unsigned_short1, char1)));
                  expects((is(unsigned_short1, signed_char1)));
                  expects((is(unsigned_short1, unsigned_char1)));

                  expects((is(unsigned_short1, signed_short1)));

                  expects((is(unsigned_short1, signed_int1)));
                  expects((is(unsigned_short1, unsigned_int1)));

                  expects((is(unsigned_short1, signed_longint1)));
                  expects((is(unsigned_short1, unsigned_longint1)));

                  expects((is(unsigned_short1, signed_longlongint1)));
                  expects((is(unsigned_short1, unsigned_longlongint1)));

                  // signed int
                  expects((is(signed_int1, char1)));

                  expects((is(signed_int1, signed_char1)));
                  expects((is(signed_int1, unsigned_char1)));

                  expects((is(signed_int1, signed_short1)));
                  expects((is(signed_int1, unsigned_short1)));

                  expects((is(signed_int1, unsigned_int1)));

                  expects((is(signed_int1, signed_longint1)));
                  expects((is(signed_int1, unsigned_longint1)));

                  expects((is(signed_int1, signed_longlongint1)));
                  expects((is(signed_int1, unsigned_longlongint1)));

                  // unsigned int
                  expects((is(unsigned_int1, char1)));
                  expects((is(unsigned_int1, signed_char1)));
                  expects((is(unsigned_int1, unsigned_char1)));

                  expects((is(unsigned_int1, signed_short1)));
                  expects((is(unsigned_int1, unsigned_short1)));

                  expects((is(unsigned_int1, signed_int1)));

                  expects((is(unsigned_int1, signed_longint1)));
                  expects((is(unsigned_int1, unsigned_longint1)));

                  expects((is(unsigned_int1, signed_longlongint1)));
                  expects((is(unsigned_int1, unsigned_longlongint1)));

                  // signed longint
                  expects((is(signed_longint1, char1)));

                  expects((is(signed_longint1, signed_char1)));
                  expects((is(signed_longint1, unsigned_char1)));

                  expects((is(signed_longint1, signed_short1)));
                  expects((is(signed_longint1, unsigned_short1)));

                  expects((is(signed_longint1, unsigned_int1)));

                  expects((is(signed_longint1, unsigned_longint1)));

                  expects((is(signed_longint1, signed_longlongint1)));
                  expects((is(signed_longint1, unsigned_longlongint1)));

                  // unsigned longint
                  expects((is(unsigned_longint1, char1)));
                  expects((is(unsigned_longint1, signed_char1)));
                  expects((is(unsigned_longint1, unsigned_char1)));

                  expects((is(unsigned_longint1, signed_short1)));
                  expects((is(unsigned_longint1, unsigned_short1)));

                  expects((is(unsigned_longint1, signed_int1)));
                  expects((is(unsigned_longint1, unsigned_int1)));

                  expects((is(unsigned_longint1, signed_longint1)));

                  expects((is(unsigned_longint1, signed_longlongint1)));
                  expects((is(unsigned_longint1, unsigned_longlongint1)));

                  // signed longlongint
                  expects((is(signed_longlongint1, char1)));

                  expects((is(signed_longlongint1, signed_char1)));
                  expects((is(signed_longlongint1, unsigned_char1)));

                  expects((is(signed_longlongint1, signed_short1)));
                  expects((is(signed_longlongint1, unsigned_short1)));

                  expects((is(signed_longlongint1, unsigned_int1)));

                  expects((is(signed_longlongint1, signed_longint1)));
                  expects((is(signed_longlongint1, unsigned_longint1)));

                  expects((is(signed_longlongint1, unsigned_longlongint1)));

                  // unsigned longlongint
                  expects((is(unsigned_longlongint1, char1)));
                  expects((is(unsigned_longlongint1, signed_char1)));
                  expects((is(unsigned_longlongint1, unsigned_char1)));

                  expects((is(unsigned_longlongint1, signed_short1)));
                  expects((is(unsigned_longlongint1, unsigned_short1)));

                  expects((is(unsigned_longlongint1, signed_int1)));
                  expects((is(unsigned_longlongint1, unsigned_int1)));

                  expects((is(unsigned_longlongint1, signed_longint1)));
                  expects((is(unsigned_longlongint1, unsigned_longint1)));

                  expects((is(unsigned_longlongint1, signed_longlongint1)));

                  // char
                  expects((not is(char1, signed_char2)));
                  expects((not is(char1, unsigned_char2)));

                  expects((not is(char1, signed_short2)));
                  expects((not is(char1, unsigned_short2)));

                  expects((not is(char1, signed_int2)));
                  expects((not is(char1, unsigned_int2)));

                  expects((not is(char1, signed_longint2)));
                  expects((not is(char1, unsigned_longint2)));

                  expects((not is(char1, signed_longlongint2)));
                  expects((not is(char1, unsigned_longlongint2)));

                  // signed char
                  expects((not is(signed_char1, char2)));
                  expects((not is(signed_char1, unsigned_char2)));

                  expects((not is(signed_char1, signed_short2)));
                  expects((not is(signed_char1, unsigned_short2)));

                  expects((not is(signed_char1, signed_int2)));
                  expects((not is(signed_char1, unsigned_int2)));

                  expects((not is(signed_char1, signed_longint2)));
                  expects((not is(signed_char1, unsigned_longint2)));

                  expects((not is(signed_char1, signed_longlongint2)));
                  expects((not is(signed_char1, unsigned_longlongint2)));

                  // unsigned char
                  expects((not is(unsigned_char1, char2)));
                  expects((not is(unsigned_char1, signed_char2)));

                  expects((not is(unsigned_char1, signed_short2)));
                  expects((not is(unsigned_char1, unsigned_short2)));

                  expects((not is(unsigned_char1, signed_int2)));
                  expects((not is(unsigned_char1, unsigned_int2)));

                  expects((not is(unsigned_char1, signed_longint2)));
                  expects((not is(unsigned_char1, unsigned_longint2)));

                  expects((not is(unsigned_char1, signed_longlongint2)));
                  expects((not is(unsigned_char1, unsigned_longlongint2)));

                  // signed short
                  expects((not is(signed_short1, char2)));

                  expects((not is(signed_short1, signed_char2)));
                  expects((not is(signed_short1, unsigned_char2)));

                  expects((not is(signed_short1, unsigned_short2)));

                  expects((not is(signed_short1, signed_int2)));
                  expects((not is(signed_short1, unsigned_int2)));

                  expects((not is(signed_short1, signed_longint2)));
                  expects((not is(signed_short1, unsigned_longint2)));

                  expects((not is(signed_short1, signed_longlongint2)));
                  expects((not is(signed_short1, unsigned_longlongint2)));

                  // unsigned short
                  expects((not is(unsigned_short1, char2)));
                  expects((not is(unsigned_short1, signed_char2)));
                  expects((not is(unsigned_short1, unsigned_char2)));

                  expects((not is(unsigned_short1, signed_short2)));

                  expects((not is(unsigned_short1, signed_int2)));
                  expects((not is(unsigned_short1, unsigned_int2)));

                  expects((not is(unsigned_short1, signed_longint2)));
                  expects((not is(unsigned_short1, unsigned_longint2)));

                  expects((not is(unsigned_short1, signed_longlongint2)));
                  expects((not is(unsigned_short1, unsigned_longlongint2)));

                  // signed int
                  expects((not is(signed_int1, char2)));

                  expects((not is(signed_int1, signed_char2)));
                  expects((not is(signed_int1, unsigned_char2)));

                  expects((not is(signed_int1, signed_short2)));
                  expects((not is(signed_int1, unsigned_short2)));

                  expects((not is(signed_int1, unsigned_int2)));

                  expects((not is(signed_int1, signed_longint2)));
                  expects((not is(signed_int1, unsigned_longint2)));

                  expects((not is(signed_int1, signed_longlongint2)));
                  expects((not is(signed_int1, unsigned_longlongint2)));

                  // unsigned int
                  expects((not is(unsigned_int1, char2)));
                  expects((not is(unsigned_int1, signed_char2)));
                  expects((not is(unsigned_int1, unsigned_char2)));

                  expects((not is(unsigned_int1, signed_short2)));
                  expects((not is(unsigned_int1, unsigned_short2)));

                  expects((not is(unsigned_int1, signed_int2)));

                  expects((not is(unsigned_int1, signed_longint2)));
                  expects((not is(unsigned_int1, unsigned_longint2)));

                  expects((not is(unsigned_int1, signed_longlongint2)));
                  expects((not is(unsigned_int1, unsigned_longlongint2)));

                  // signed longint
                  expects((not is(signed_longint1, char2)));

                  expects((not is(signed_longint1, signed_char2)));
                  expects((not is(signed_longint1, unsigned_char2)));

                  expects((not is(signed_longint1, signed_short2)));
                  expects((not is(signed_longint1, unsigned_short2)));

                  expects((not is(signed_longint1, unsigned_int2)));

                  expects((not is(signed_longint1, unsigned_longint2)));

                  expects((not is(signed_longint1, signed_longlongint2)));
                  expects((not is(signed_longint1, unsigned_longlongint2)));

                  // unsigned longint
                  expects((not is(unsigned_longint1, char2)));
                  expects((not is(unsigned_longint1, signed_char2)));
                  expects((not is(unsigned_longint1, unsigned_char2)));

                  expects((not is(unsigned_longint1, signed_short2)));
                  expects((not is(unsigned_longint1, unsigned_short2)));

                  expects((not is(unsigned_longint1, signed_int2)));
                  expects((not is(unsigned_longint1, unsigned_int2)));

                  expects((not is(unsigned_longint1, signed_longint2)));

                  expects((not is(unsigned_longint1, signed_longlongint2)));
                  expects((not is(unsigned_longint1, unsigned_longlongint2)));

                  // signed longlongint
                  expects((not is(signed_longlongint1, char2)));

                  expects((not is(signed_longlongint1, signed_char2)));
                  expects((not is(signed_longlongint1, unsigned_char2)));

                  expects((not is(signed_longlongint1, signed_short2)));
                  expects((not is(signed_longlongint1, unsigned_short2)));

                  expects((not is(signed_longlongint1, unsigned_int2)));

                  expects((not is(signed_longlongint1, signed_longint2)));
                  expects((not is(signed_longlongint1, unsigned_longint2)));

                  expects((not is(signed_longlongint1, unsigned_longlongint2)));

                  // unsigned longlongint
                  expects((not is(unsigned_longlongint1, char2)));
                  expects((not is(unsigned_longlongint1, signed_char2)));
                  expects((not is(unsigned_longlongint1, unsigned_char2)));

                  expects((not is(unsigned_longlongint1, signed_short2)));
                  expects((not is(unsigned_longlongint1, unsigned_short2)));

                  expects((not is(unsigned_longlongint1, signed_int2)));
                  expects((not is(unsigned_longlongint1, unsigned_int2)));

                  expects((not is(unsigned_longlongint1, signed_longint2)));
                  expects((not is(unsigned_longlongint1, unsigned_longint2)));

                  expects((not is(unsigned_longlongint1, signed_longlongint2)));
              } },
          }
    };
} // namespace
