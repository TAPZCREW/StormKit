module;

extern "C" {
#include "ThreadUtilsImpl.h"
}

module stormkit.Core;

import std;

import :Parallelism.ThreadUtils;

namespace stormkit { inline namespace core {
    ////////////////////////////////////////
    ////////////////////////////////////////
    auto setCurrentThreadName(std::string_view name) noexcept -> void {
        setCurrentNSThreadName(std::data(name));
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto setThreadName([[maybe_unused]] std::thread&     thread,
                       [[maybe_unused]] std::string_view name) noexcept -> void {
        // auto id = thread.native_handle();
        // details::setThreadName(id, name);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto setThreadName([[maybe_unused]] std::jthread&    thread,
                       [[maybe_unused]] std::string_view name) noexcept -> void {
        // auto id = thread.native_handle();
        // details::setThreadName(id, name);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto getCurrentThreadName() noexcept -> std::string {
        return {};
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto getThreadName(std::thread& _) noexcept -> std::string {
        return {};
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto getThreadName(std::jthread& _) noexcept -> std::string {
        return {};
    }
}} // namespace stormkit::core
