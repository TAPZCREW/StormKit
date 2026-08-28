module;

extern "C" {
#include "threadutils_impl.h"
}

module stormkit.core;

import std;

import :parallelism.threadutils;

namespace stormkit { inline namespace core {
    ////////////////////////////////////////
    ////////////////////////////////////////
    auto set_current_thread_name(string_view name) noexcept -> void {
        setCurrentNSThreadName(std::data(name));
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto set_thread_name(std::thread&, string_view) noexcept -> void {
        // auto id = thread.native_handle();
        // details::set_thread_name(id, name);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto set_thread_name(std::jthread&, string_view) noexcept -> void {
        // auto id = thread.native_handle();
        // details::set_thread_name(id, name);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto get_current_thread_name() noexcept -> string {
        return {};
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto get_thread_name(std::thread&) noexcept -> string {
        return {};
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto get_thread_name(std::jthread&) noexcept -> string {
        return {};
    }
}} // namespace stormkit::core
