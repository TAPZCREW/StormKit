// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.core;
import stormkit.Main;
import stormkit.Log;
import stormkit.Wsi;

import TriangleApp;

#include <stormkit/Core/PlatformMacro.hpp>
#include <stormkit/Main/MainMacro.hpp>

////////////////////////////////////////
////////////////////////////////////////
auto main(std::span<const std::string_view> args) -> int {
    using namespace stormkit;

    wsi::parseArgs(args);

    auto logger = log::Logger::createLoggerInstance<log::ConsoleLogger>();
    logger.ilog("Using StormKit {}.{}.{}\n    branch: {}\n    commit_hash: {}\n    built with {}",
                STORMKIT_MAJOR_VERSION,
                STORMKIT_MINOR_VERSION,
                STORMKIT_PATCH_VERSION,
                STORMKIT_GIT_BRANCH,
                STORMKIT_GIT_COMMIT_HASH,
                STORMKIT_COMPILER);

    try {
        auto app = TriangleApp {};
        return app.run(args);
    } catch (const std::exception& e) {
        logger.flog("{}", e.what());
    } catch (...) {
        logger.flog("Uncaught exception occured !");
    }
    return -1;
}
