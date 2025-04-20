module;

#include <pthread.h>

module stormkit.core;

import std;

namespace stormkit { inline namespace core {
    namespace details {
        ////////////////////////////////////////
        ////////////////////////////////////////
        auto set_thread_name(pthread_t id, std::string_view name) noexcept -> void {
            pthread_setname_np(id, std::data(name));
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        auto get_thread_name(pthread_t id) noexcept -> std::string {
            auto name = std::array<char, 256> {};
            pthread_getname_np(id, std::data(name), std::size(name));
            return std::string { std::begin(name),
                                 std::begin(name) + std::strlen(std::data(name)) };
        }
    } // namespace details

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto set_current_thread_name(std::string_view name) noexcept -> void {
        const auto id = pthread_self();
        details::set_thread_name(id, name);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto set_thread_name(std::thread& thread, std::string_view name) noexcept -> void {
        const auto id = thread.native_handle();
        details::set_thread_name(id, name);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto set_thread_name(std::jthread& thread, std::string_view name) noexcept -> void {
        const auto id = thread.native_handle();
        details::set_thread_name(id, name);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto get_current_thread_name() noexcept -> std::string {
        const auto id = pthread_self();
        return details::get_thread_name(id);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto get_thread_name(std::thread& thread) noexcept -> std::string {
        const auto id = thread.native_handle();
        return details::get_thread_name(id);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto get_thread_name(std::jthread& thread) noexcept -> std::string {
        const auto id = thread.native_handle();
        return details::get_thread_name(id);
    }
}} // namespace stormkit::core
