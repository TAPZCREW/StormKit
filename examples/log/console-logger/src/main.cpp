// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.core;
import stormkit.log;

#include <stormkit/main/main_macro.hpp>

using namespace stormkit;

struct Bar {
    std::string d = "FooBar";
};

template<class CharT>
struct std::formatter<Bar, CharT>: std::formatter<std::basic_string<CharT>, CharT> {
    template<class FormatContext>
    auto format(const Bar& data, FormatContext& ctx) const -> decltype(ctx.out()) {
        auto&& out = ctx.out();
        return format_to(out, "[Bar: d = {}]", data.d);
    }
};

struct Foo {
    UInt32 a = 0u;
    float  b = 2.3f;
    Bar    c = Bar {};
};

template<class CharT>
struct std::formatter<Foo, CharT>: std::formatter<std::basic_string<CharT>, CharT> {
    template<class FormatContext>
    auto format(const Foo& data, FormatContext& ctx) const -> decltype(ctx.out()) {
        auto&& out = ctx.out();
        return format_to(out, "[Foo: a = {}, b = {}, c = {}]", data.a, data.b, data.c);
    }
};

////////////////////////////////////////
////////////////////////////////////////
auto main(std::span<const std::string_view>) -> int {
    using log::operator""_module;

    auto logger = log::Logger::create_logger_instance<log::ConsoleLogger>();

    log::Logger::ilog("This is an information");
    log::Logger::dlog("This is a debug information");
    log::Logger::wlog("This is a warning");
    log::Logger::elog("This is an error");
    log::Logger::flog("This is a fatal error");

    const auto integer = 5u;
    const auto real    = 8.5f;
    log::Logger::ilog("you can format your string like this {}, {}", integer, real);

    log::Logger::ilog("Foo"_module, "you can specify a module");

    const auto foo = Foo {};
    log::Logger::ilog("you can format complexes structures\n{}", foo);

    return 0;
}
