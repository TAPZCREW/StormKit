#ifdef STORMKIT_BUILD_MODULES
import std;

import stormkit.Core;
import stormkit.Log;
import stormkit.Main;

import App;
#else
    #include <stormkit/Core.hpp>
    #include <stormkit/Log.hpp>
    #include <stormkit/Main.hpp>

    #include "App.mpp"
#endif

auto main([[maybe_unused]] std::span<const std::string_view> args) -> int {
    using namespace stormkit;

    setupSignalHandler();

    log::LogHandler::setupDefaultLogger();

    auto app = App {};
    return app.run(argc, argv);
}
