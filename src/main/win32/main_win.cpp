// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#include <windows.h>
#undef __nullnullterminated

#include <cstdio>
#include <cstdlib>

import std;

import stormkit.core;

#include <version>

namespace {
    constexpr auto BUF_SIZE = 1024;
}

extern auto user_main(std::span<const std::string_view>) -> int;

auto __stdcall main(int argc, char** argv) -> int {
    std::locale::global(std::locale { "" });
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    setvbuf(stderr,
            nullptr,
            _IOFBF,
            BUF_SIZE); // on Windows 10+ we need buffering or console will get 1
                       // byte at a time (screwing up utf-8 encoding)
    setvbuf(stdout, nullptr, _IOFBF, BUF_SIZE);

    stormkit::setup_signal_handler();
    stormkit::set_current_thread_name("MainThread");

    auto args = std::vector<std::string_view> {};
    args.reserve(argc);

    for (auto&& i : stormkit::range(argc)) args.emplace_back(argv[i]);

    return user_main(args);
}

auto __stdcall WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) -> int {
    const auto argc = __argc;
    const auto argv = __argv;

    std::locale::global(std::locale { "" });
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    setvbuf(stderr,
            nullptr,
            _IOFBF,
            BUF_SIZE); // on Windows 10+ we need buffering or console will get 1
                       // byte at a time (screwing up utf-8 encoding)
    setvbuf(stdout, nullptr, _IOFBF, BUF_SIZE);

    stormkit::setup_signal_handler();
    stormkit::set_current_thread_name("MainThread");

    auto args = std::vector<std::string_view> {};
    args.reserve(argc);

    for (auto&& i : stormkit::range(argc)) args.emplace_back(argv[i]);

    return user_main(args);
}
