module;

#include <stormkit/core/api.hpp>
#include <stormkit/core/platform_macro.hpp>

#include <csignal>

module stormkit.core.stacktrace;

import std;

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    extern "C" auto terminate_handler() noexcept -> void {
        print_stacktrace(3);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    extern "C" auto signal_handler(i32 signum) noexcept -> void {
        std::signal(signum, SIG_DFL);
        print_stacktrace(3);
        std::raise(SIGABRT);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto setup_signal_handler() noexcept -> void {
        std::set_terminate(&terminate_handler);
        std::signal(SIGSEGV, &signal_handler);
    }
}} // namespace stormkit::core
