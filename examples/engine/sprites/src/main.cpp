// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.core;
import stormkit.Main;
import stormkit.log;
import stormkit.wsi;

import SpritesApp;

#include <stormkit/core/platform_macro.hpp>
#include <stormkit/main/main_macro.hpp>

////////////////////////////////////////
////////////////////////////////////////
auto main(std::span<const std::string_view> args) -> int {
    using namespace stormkit;

    wsi::parse_args(args);

    auto logger = log::Logger::create_logger_instance<log::ConsoleLogger>();
    logger.ilog("Using StormKit {}.{}.{}\n    branch: {}\n    commit_hash: {}\n    built with {}",
                STORMKIT_MAJOR_VERSION,
                STORMKIT_MINOR_VERSION,
                STORMKIT_PATCH_VERSION,
                STORMKIT_GIT_BRANCH,
                STORMKIT_GIT_COMMIT_HASH,
                STORMKIT_COMPILER);

    try {
        auto app = SpritesApp {};
        return app.run(args);
    } catch (const std::exception& e) {
        logger.flog("{}", e.what());
        return -1;
    } catch (...) {
        logger.flog("Uncaught exception occured !");
        return -1;
    }
}
