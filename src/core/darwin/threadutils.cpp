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
    auto set_current_thread_name(std::string_view name) noexcept -> void {
        setCurrentNSThreadName(std::data(name));
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto set_thread_name(std::thread&, std::string_view) noexcept -> void {
        // auto id = thread.native_handle();
        // details::set_thread_name(id, name);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto set_thread_name(std::jthread&, std::string_view) noexcept -> void {
        // auto id = thread.native_handle();
        // details::set_thread_name(id, name);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto get_current_thread_name() noexcept -> std::string {
        return {};
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto get_thread_name(std::thread&) noexcept -> std::string {
        return {};
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto get_thread_name(std::jthread&) noexcept -> std::string {
        return {};
    }
}} // namespace stormkit::core
