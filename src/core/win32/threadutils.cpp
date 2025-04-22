module;

#include <windows.h>
#undef __nullnullterminated

module stormkit.core;

import std;

inline constexpr auto MS_VC_EXCEPTION = DWORD { 0x406D1388 };

#pragma pack(push, 8)

struct ThreadNameInfo {
    DWORD  dwType = 0x1000;
    LPCSTR szName;
    DWORD  dwThreadId;
    DWORD  dwFlags = 0;
};

#pragma pack(pop)

namespace stormkit { inline namespace core {
    namespace details {
        ////////////////////////////////////////
        ////////////////////////////////////////
        auto set_thread_name(HANDLE handle, std::string_view name) noexcept -> void {
            const auto id   = ::GetThreadId(handle);
            auto       info = ThreadNameInfo { .szName = std::data(name), .dwThreadId = id };

            __try {
                RaiseException(MS_VC_EXCEPTION,
                               0,
                               sizeof(info) / sizeof(ULONG_PTR),
                               reinterpret_cast<ULONG_PTR*>(&info));
            } __except (EXCEPTION_EXECUTE_HANDLER) {}
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        auto get_thread_name(HANDLE handle) noexcept -> std::string {
            auto       data = PWSTR { nullptr };
            const auto hr   = GetThreadDescription(handle, &data);

            auto out = std::string {};

            if (SUCCEEDED(hr)) {
                out = wide_to_ascii(data);
                LocalFree(data);
            }

            return {};
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        auto getThreadHandle(T& thread) {
            return reinterpret_cast<HANDLE>(thread.native_handle());
        }
    } // namespace details

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto set_current_thread_name(std::string_view name) noexcept -> void {
        const auto handle = ::GetCurrentThread();
        details::set_thread_name(handle, name);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto set_thread_name(std::thread& thread, std::string_view name) noexcept -> void {
        const auto handle = details::getThreadHandle(thread);
        details::set_thread_name(handle, name);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto set_thread_name(std::jthread& thread, std::string_view name) noexcept -> void {
        const auto handle = details::getThreadHandle(thread);
        details::set_thread_name(handle, name);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto get_current_thread_name() noexcept -> std::string {
        const auto handle = ::GetCurrentThread();
        return details::get_thread_name(handle);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto get_thread_name(std::thread& thread) noexcept -> std::string {
        const auto handle = details::getThreadHandle(thread);
        return details::get_thread_name(handle);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto get_thread_name(std::jthread& thread) noexcept -> std::string {
        const auto handle = details::getThreadHandle(thread);
        return details::get_thread_name(handle);
    }
}} // namespace stormkit::core
