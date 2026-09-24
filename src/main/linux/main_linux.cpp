// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;
import stormkit.core;

#include <version>

using namespace stormkit;

extern auto user_main(array_view<const string_view>) -> i32;

auto main(i32 argc, char** argv) -> i32 {
    setup_signal_handler();
    set_current_thread_name("stormkit:main_thread");

    auto args = dynarray<string_view> {};

    for (auto i : range(argc)) args.emplace_back(argv[i]);

    return user_main(args);
}
