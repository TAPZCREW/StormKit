module;

#include <pthread.h>
#include <sys/prctl.h>

module stormkit.core;

import std;

namespace stormkit { inline namespace core {
    namespace details {
        ////////////////////////////////////////
        ////////////////////////////////////////
        auto set_thread_name(pthread_t id, string_view name) noexcept -> void {
            pthread_setname_np(id, std::data(name));
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        auto get_thread_name(pthread_t id) noexcept -> string {
            auto name = array<char, 256> {};
            pthread_getname_np(id, std::data(name), std::size(name));
            return string { std::begin(name), std::begin(name) + std::strlen(std::data(name)) };
        }
    } // namespace details

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto set_current_thread_name(string_view name) noexcept -> void {
        prctl(PR_SET_NAME, stdr::data(name), 0, 0, 0);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto set_thread_name(std::thread& thread, string_view name) noexcept -> void {
        const auto id = thread.native_handle();
        details::set_thread_name(id, name);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto set_thread_name(std::jthread& thread, string_view name) noexcept -> void {
        const auto id = thread.native_handle();
        details::set_thread_name(id, name);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto get_current_thread_name() noexcept -> string {
        const auto id = pthread_self();
        return details::get_thread_name(id);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto get_thread_name(std::thread& thread) noexcept -> string {
        const auto id = thread.native_handle();
        return details::get_thread_name(id);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto get_thread_name(std::jthread& thread) noexcept -> string {
        const auto id = thread.native_handle();
        return details::get_thread_name(id);
    }
}} // namespace stormkit::core
