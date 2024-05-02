// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.Core;

import Test;

using namespace stormkit::core;

#define expects(x) test::expects(x, #x)

namespace {
    auto _ =
        test::TestSuite { "Core.AsCast",
                          {
                              { "is<std::variant>",
                                [] static noexcept {
                                    using Variant1 = std::variant<bool, int>;

                                    auto variant = Variant1 { false };

                                    expects(is<bool>(variant));
                                    expects(not is<int>(variant));
                                } },
                              { "is<IsPolymorphic>",
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
                              { "as<std::variant>",
                                [] static noexcept {
                                    using Variant1 = std::variant<bool, int>;

                                    auto variant = Variant1 { false };

                                    // std::variant &
                                    as<bool>(variant) = true;
                                    expects(as<bool>(variant) == true);

                                    // std::variant &&
                                    expects(as<bool>(std::move(variant)) == true);

                                    const auto variant2 = Variant1 { 9 };
                                    // const std::variant &
                                    expects(as<int>(variant2) == 9);

                                    // const std::variant &&
                                    expects(as<int>(std::move(variant2)) == 9);
                                } },
                              { "as<IsPolymorphic>",
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

                                    static_assert(IsPolymorphic<RemoveIndirectionsType<Bar*>>);
                                    static_assert(IsPolymorphic<ElementType<PureType<Foo*>>>);
                                    static_assert(Is<RawType<Bar*>, ElementType<PureType<Foo*>>>);

                                    AsCaster<Bar*, Foo*>::operator()(bar.get());

                                    expects(as<Bar*>(bar)->foo() == 1);
                                    expects(as<Bar&>(bar).foo() == 1);
                                    expects(as<Bar*>(bar.get())->foo() == 1);
                                    expects(as<Bar&>(bar.get()).foo() == 1);
                                    expects(as<Bar*>(*bar)->foo() == 1);
                                    expects(as<Bar&>(*bar).foo() == 1);
                                } },
                              { "as<Byte>",
                                [] static noexcept {
                                    struct Foo {
                                        int  a;
                                        int  b;
                                        char c;
                                    };

                                    auto bar          = Foo { 9, 3, 'x' };
                                    auto bar_byte_ptr = as<std::byte*>(&bar);
                                    expects(bar_byte_ptr != nullptr);

                                    bar_byte_ptr = as<std::byte*>(bar);
                                    expects(bar_byte_ptr != nullptr);

                                    auto bar_ptr = as<Foo*>(bar_byte_ptr);
                                    expects(bar_ptr != nullptr);
                                    expects(bar_ptr->a == 9);
                                    expects(bar_ptr->b == 3);
                                    expects(bar_ptr->c == 'x');

                                    bar_ptr = as<Foo*>(*bar_byte_ptr);
                                    expects(bar_ptr != nullptr);
                                    expects(bar_ptr->a == 9);
                                    expects(bar_ptr->b == 3);
                                    expects(bar_ptr->c == 'x');
                                } },
                              { "as<IsEnumeration>",
                                [] static noexcept {
                                    enum Foo : int {
                                        A = 5
                                    };
                                    enum class Bar : unsigned int {
                                        A = 5
                                    };

                                    auto foo = Foo::A;
                                    expects(as<Underlying>(foo) == 5);
                                    expects(as<Underlying>(Bar::A) == 5);
                                    expects(as<int>(foo) == 5);
                                    expects(as<unsigned int>(Bar::A) == 5);
                                    expects(as<Foo>(as<int>(foo)) == Foo::A);
                                } },
                              { "as.explicit_cast",
                                [] static noexcept {
                                    struct Foo {};
                                    struct Bar {
                                        Bar(Foo) {};
                                        operator bool() { return true; }
                                    };

                                    auto foo = Foo {};

                                    expects(as<Bar>(Foo {}));
                                    expects(as<Bar>(foo));
                                    expects(as<Bar>(std::as_const(foo)));
                                    expects(as<Bar>(std::move(foo)));
                                } },
                              { "as<Integers>",
                                [] static noexcept {
                                    static_assert(IsNarrowing<int, short>);
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
                                    expects(as<unsigned int>(-1) != 1);

                                    expects(is(as<float>(19), 19.f));
                                    expects(is(as<float>(-5), -5.));
                                } },
                          } };
} // namespace
