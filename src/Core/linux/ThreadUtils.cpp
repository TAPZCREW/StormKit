module;

#include <pthread.h>

module stormkit.Core;

import std;

namespace stormkit { inline namespace core {
    namespace details {
        ////////////////////////////////////////
        ////////////////////////////////////////
        auto setThreadName(pthread_t id, std::string_view name) noexcept -> void {
            pthread_setname_np(id, std::data(name));
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        auto getThreadName(pthread_t id) noexcept -> std::string {
            auto name = std::array<char, 256> {};
            pthread_getname_np(id, std::data(name), std::size(name));
            return std::string { std::begin(name),
                                 std::begin(name) + std::strlen(std::data(name)) };
        }
    } // namespace details

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto setCurrentThreadName(std::string_view name) noexcept -> void {
        const auto id = pthread_self();
        details::setThreadName(id, name);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto setThreadName(std::thread& thread, std::string_view name) noexcept -> void {
        const auto id = thread.native_handle();
        details::setThreadName(id, name);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto setThreadName(std::jthread& thread, std::string_view name) noexcept -> void {
        const auto id = thread.native_handle();
        details::setThreadName(id, name);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto getCurrentThreadName() noexcept -> std::string {
        const auto id = pthread_self();
        return details::getThreadName(id);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto getThreadName(std::thread& thread) noexcept -> std::string {
        const auto id = thread.native_handle();
        return details::getThreadName(id);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto getThreadName(std::jthread& thread) noexcept -> std::string {
        const auto id = thread.native_handle();
        return details::getThreadName(id);
    }
}} // namespace stormkit::core
