#include "stormkit-Core.hpp"

import stormkit.Core;

auto setupSignalHandler() -> void {
    stormkit::setupSignalHandler();
}

auto setCurrentThreadName(std::string_view name) -> void {
    stormkit::setCurrentThreadName(name);
}
