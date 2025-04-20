// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;
import stormkit.Core;

#include <version>

extern auto userMain(std::span<const std::string_view>) -> int;

auto main(int argc, char** argv) -> int {
    stormkit::setupSignalHandler();
    stormkit::setCurrentThreadName("MainThread");

    auto args = std::vector<std::string_view> {};

    for (auto i : stormkit::range(argc)) args.emplace_back(argv[i]);

    return userMain(args);
}
