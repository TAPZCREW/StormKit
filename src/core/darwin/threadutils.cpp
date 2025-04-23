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
    auto set_thread_name([[maybe_unused]] std::thread&     thread,
                         [[maybe_unused]] std::string_view name) noexcept -> void {
        // auto id = thread.native_handle();
        // details::set_thread_name(id, name);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto set_thread_name([[maybe_unused]] std::jthread&    thread,
                         [[maybe_unused]] std::string_view name) noexcept -> void {
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
    auto get_thread_name(std::thread& _) noexcept -> std::string {
        return {};
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto get_thread_name(std::jthread& _) noexcept -> std::string {
        return {};
    }
}} // namespace stormkit::core
