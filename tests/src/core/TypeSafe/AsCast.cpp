// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.Core;

import Test;

using namespace stormkit::core;

#define expects(x) test::expects(x, #x)

namespace {
    auto _ = test::TestSuite {
        "Core.TypeSafe",
        {
          { "AsCast.same_type",
              [] static noexcept {
                  auto foo = Int { 9 };
                  expects(as<Int>(Int { 1 }) == 1);
                  expects(&as<Int>(foo) == &foo);
              } },
          { "AsCast.is_as_variant",
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

                  expects(is<Bar>(bar.get()));
                  expects(not is<BarFoo>(bar.get()));
                  expects(is<Bar>(*bar));
                  expects(not is<BarFoo>(*bar));
              } },
          { "AsCast.as_runtime_polymorphic",
              [] static noexcept {
                  struct Foo {
                      virtual ~Foo() {}
                      virtual auto foo() -> int = 0;
                  };
                  struct Bar: Foo {
                      auto foo() -> int override { return 1; }
                  };
                  struct BarFoo: Foo {
                      auto foo() -> int override { return 5; }
                  };

                  auto bar = std::unique_ptr<Foo> { std::make_unique<Bar>() };

                  expects(as<Bar>(bar)->foo() == 1);
                  expects(as<Bar>(*bar).foo() == 1);
                  expects(as<Bar>(bar.get())->foo() == 1);
                  expects(as<Bar>(*(bar.get())).foo() == 1);
              } },
          { "AsCast.as_bytes",
              [] static noexcept {
                  struct Foo {
                      int  a;
                      int  b;
                      char c;
                  };

                  auto bar          = Foo { 9, 3, 'x' };
                  auto bar_byte_ptr = as<std::byte*>(&bar);
                  auto bar_ptr      = as<Foo*>(bar_byte_ptr);
                  auto bar_borrowed = as<Borrowed<Foo>>(bar_byte_ptr);

                  auto bar2          = std::make_unique<Foo>(9, 3, 'x');
                  auto bar_byte_ptr2 = as<std::byte*>(bar2);
                  auto bar_ptr2      = as<Foo*>(bar_byte_ptr2);

                  auto bar3          = std::make_shared<Foo>(9, 3, 'x');
                  auto bar_byte_ptr3 = as<std::byte*>(bar3);
                  auto bar_ptr3      = as<Foo*>(bar_byte_ptr3);

                  auto bar4          = Borrowed { bar };
                  auto bar_byte_ptr4 = as<std::byte*>(bar4);
                  auto bar_ptr4      = as<Foo*>(bar_byte_ptr4);

                  expects(bar_byte_ptr != nullptr);
                  expects(bar_ptr != nullptr);
                  expects(bar_ptr->a == 9);
                  expects(bar_ptr->b == 3);
                  expects(bar_ptr->c == 'x');
                  expects(bar_borrowed->a == 9);
                  expects(bar_borrowed->b == 3);
                  expects(bar_borrowed->c == 'x');
                  expects(bar_byte_ptr2 != nullptr);
                  expects(bar_ptr2->a == 9);
                  expects(bar_ptr2->b == 3);
                  expects(bar_ptr2->c == 'x');
                  expects(bar_byte_ptr3 != nullptr);
                  expects(bar_ptr3->a == 9);
                  expects(bar_ptr3->b == 3);
                  expects(bar_ptr3->c == 'x');
                  expects(bar_byte_ptr4 != nullptr);
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
          { "AsCast.as_integers",
              [] static noexcept {
                  static_assert(IsNarrowing<int, short>);
                  static_assert(not IsNarrowing<int, unsigned int>);
                  static_assert(not IsNarrowing<int, unsigned int>);
                  static_assert(not IsNarrowing<float, unsigned int>);
                  static_assert(not IsNarrowing<float, int>);
                  static_assert(not IsNarrowing<unsigned int, float>);

                  using schar = signed char;
                  expects(as<signed char>(127) == schar { 127 });
                  expects(as<signed char>(-80) != schar { -81 });

                  expects(as<int>(8u) == 8);
                  expects(as<int>(1820) != 7);

                  expects(as<unsigned int>(14) == 14u);

                  expects(is(as<float>(19), 19.f));
                  expects(is(as<float>(-5), -5.));
              } },
          { "AsCast.as_bytes",
              [] static noexcept {
                  using uchar = unsigned char;
                  using schar = signed char;

                  static_assert(IsSafeCastableToByte<char>);
                  static_assert(IsSafeCastableToByte<signed char>);
                  static_assert(not IsSafeCastableToByte<short>);
                  static_assert(not IsSafeCastableToByte<int>);
                  static_assert(not IsSafeCastableToByte<long>);
                  static_assert(not IsSafeCastableToByte<long long>);
                  static_assert(IsSafeCastableToByte<unsigned char>);
                  static_assert(not IsSafeCastableToByte<unsigned short>);
                  static_assert(not IsSafeCastableToByte<unsigned int>);
                  static_assert(not IsSafeCastableToByte<unsigned long>);
                  static_assert(not IsSafeCastableToByte<unsigned long long>);

                  expects(as<std::byte>(uchar { 8 }) == std::byte { 8 });
                  expects(as<std::byte>(schar { 8 }) == std::byte { 8 });
                  expects(as<std::byte>(char { 8 }) == std::byte { 8 });
                  expects(as<int>(std::byte { 8 }) == 8);
              } },
          { "AsCast.as_explicit_cast",
              [] static noexcept {
                  struct Bar {
                      operator bool() { return true; }
                  };
                  struct Foo {
                      explicit operator Bar() const { return Bar {}; }
                  };
                  struct FooBar {
                      operator bool() { return true; }
                      explicit FooBar(Foo& foo) {}
                  };

                  auto foo = Foo {};
                  expects(as<Bar>(Foo {}));
                  expects(as<Bar>(foo));
                  expects(as<Bar>(std::as_const(foo)));
                  expects(as<FooBar>(foo));
                  expects(as<Bar>(std::move(foo)));
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
          }
    };
} // namespace
