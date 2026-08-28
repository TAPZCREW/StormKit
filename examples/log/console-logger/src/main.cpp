// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit;

#include <stormkit/main/main_macro.hpp>

using namespace stormkit;

struct Bar {
    string d = "FooBar";
};

template<typename CharT, typename FormatContext>
constexpr auto tag_invoke(format_as_fn<CharT>, const Bar& value, FormatContext& ctx) -> decltype(ctx.out()) {
    return std::format_to(ctx.out(), "[Bar d: {}]", value.d);
}

struct Foo {
    u32 a = 0u;
    f32 b = 2.3f;
    Bar c = Bar {};
};

template<typename CharT, typename FormatContext>
constexpr auto tag_invoke(format_as_fn<CharT>, const Foo& value, FormatContext& ctx) -> decltype(ctx.out()) {
    return std::format_to(ctx.out(), "[Foo a: {}, b: {}, c: {}]", value.a, value.b, value.c);
}

////////////////////////////////////////
////////////////////////////////////////
auto main(array_view<const string_view> args) -> int {
    using log::operator""_module;

    // force debug
    auto args2 = dynarray<string_view> { std::from_range, args };
    args2.emplace_back("--debug");

    log::parse_args(args2);

    auto logger = log::logger::create_logger_instance<log::console_logger>();

    log::logger::ilog("This is an information");
    log::logger::dlog("This is a debug information");
    log::logger::wlog("This is a warning");
    log::logger::elog("This is an error");
    log::logger::flog("This is a fatal error");

    const auto integer = 5u;
    const auto real    = 8.5f;
    log::logger::ilog("use std::format internally {}, {}", integer, real);

    log::logger::ilog("Foo"_module, "you can specify a module");

    const auto foo = Foo {};
    log::logger::ilog("compatible with cpo format_as\n    {}", foo);

    return 0;
}
