module;

#include <windows.h>
#undef __nullnullterminated

module stormkit.Core;

import std;

inline constexpr auto MS_VC_EXCEPTION = DWORD { 0x406D1388 };

#pragma pack(push, 8)

struct ThreadNameInfo {
    DWORD  dwType = 0x1000;
    LPCSTR szName;
    DWORD  dwThreadHandle;
    DWORD  dwFlags = 0;
};

#pragma pack(pop)

namespace stormkit { inline namespace core {
    namespace details {
        ////////////////////////////////////////
        ////////////////////////////////////////
        auto setThreadName(HANDLE handle, std::string_view name) noexcept -> void {
            const auto id   = ::GetThreadhandle(handle);
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
        auto setThreadName(HANDLE handle, std::string_view name) noexcept -> void {
            const auto id   = ::GetThreadhandle(handle);
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
        auto getThreadName(HANDLE handle) noexcept -> std::string {
            using PWSTRP    = PWSTR*;
            auto       data = PWSTRP { nullptr };
            const auto hr   = GetThreadDescription(handle, &data);

            auto out = std::string {};

            if (SUCCEEDED(hr)) {
                out = wideToAscii(data);
                LocalFree(data);
            }

            return {};
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        auto getThreadName(HANDLE handle) noexcept -> std::string {
            const auto id = ::GetThreadhandle(handle);
            return getThreadName(id);
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        auto getThreadHandle(const T& thread) {
            return reinterpret_cast<HANDLE>(thread.native_handle());
        }
    } // namespace details

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto setCurrentThreadName(std::string_view name) noexcept -> void {
        const auto handle = ::GetCurrentThread();
        details::setThreadName(handle, name);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto setThreadName(std::thread& thread, std::string_view name) noexcept -> void {
        const auto handle = getThreadHandle(thread);
        details::setThreadName(handle, name);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto setThreadName(std::jthread& thread, std::string_view name) noexcept -> void {
        const auto handle = getThreadHandle(thread);
        details::setThreadName(handle, name);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto getCurrentThreadName() noexcept -> std::string {
        const auto handle = ::GetCurrentThread();
        return details::getThreadName(handle);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto getThreadName(std::thread& thread) noexcept -> std::string {
        const auto handle = getThreadHandle(thread);
        return details::getThreadName(handle);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto getThreadName(std::jthread& thread) noexcept -> std::string {
        const auto handle = getThreadHandle(thread);
        return details::getThreadName(handle);
    }
}} // namespace stormkit::core
