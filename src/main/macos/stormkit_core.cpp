#include "stormkit_core.hpp"

import stormkit.core;

auto setup_signal_handler() -> void {
    stormkit::setup_signal_handler();
}

auto set_current_thread_name(std::string_view name) -> void {
    stormkit::set_current_thread_name(name);
}
