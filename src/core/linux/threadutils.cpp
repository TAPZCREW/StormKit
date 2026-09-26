module;

#include <pthread.h>
#include <sys/prctl.h>

module stormkit.core.parallelism.threadutils;

import std;

namespace stdr = std::ranges;

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
            pthread_getname_np(id, stdr::data(name), stdr::size(name));
            return string { stdr::begin(name), stdr::begin(name) + std::strlen(stdr::data(name)) };
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
    auto get_thread_name(const std::thread& thread) noexcept -> string {
        // const_cast because native_handle() is not marked const, but we are not modifying the thread
        // so should be fine
        const auto id = const_cast<std::thread&>(thread).native_handle();
        return details::get_thread_name(id);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto get_thread_name(const std::jthread& thread) noexcept -> string {
        // const_cast because native_handle() is not marked const, but we are not modifying the thread
        // so should be fine
        const auto id = const_cast<std::jthread&>(thread).native_handle();
        return details::get_thread_name(id);
    }
}} // namespace stormkit::core
