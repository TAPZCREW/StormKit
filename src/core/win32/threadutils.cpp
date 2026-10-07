module stormkit.core.parallelism.threadutils;

import std;

import stormkit.core.win32;
import stormkit.core.string.encodings;

#pragma clang diagnostic ignored "-Wlanguage-extension-token"

inline constexpr auto MS_VC_EXCEPTION = win32::DWORD { 0x406D1388 };

#pragma pack(push, 8)

struct ThreadNameInfo {
    win32::DWORD  dwType = 0x1000;
    win32::LPCSTR szName;
    win32::DWORD  dwThreadId;
    win32::DWORD  dwFlags = 0;
};

#pragma pack(pop)

namespace stormkit { inline namespace core {
    namespace details {
        ////////////////////////////////////////
        ////////////////////////////////////////
        auto set_thread_name(win32::HANDLE handle, string_view name) noexcept -> void {
            const auto id   = win32::GetThreadId(handle);
            auto       info = ThreadNameInfo { .szName = std::data(name), .dwThreadId = id };

            __try {
                win32::RaiseException(MS_VC_EXCEPTION,
                                      0,
                                      sizeof(info) / sizeof(win32::ULONG_PTR),
                                      reinterpret_cast<win32::ULONG_PTR*>(&info));
            } __except (win32::EXCEPTION_EXECUTE_HANDLER) {}
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        auto get_thread_name(win32::HANDLE handle) noexcept -> string {
            auto       data = win32::PWSTR { nullptr };
            const auto hr   = win32::GetThreadDescription(handle, &data);

            auto out = string {};

            if (hr >= 0) {
                out = wide_to_ascii(data);
                win32::LocalFree(data);
            }

            return {};
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T>
        auto get_thread_handle(const T& thread) {
            return reinterpret_cast<win32::HANDLE>(const_cast<T&>(thread).native_handle());
        }
    } // namespace details

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto set_current_thread_name(string_view name) noexcept -> void {
        const auto handle = win32::GetCurrentThread();
        details::set_thread_name(handle, name);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto set_thread_name(std::thread& thread, string_view name) noexcept -> void {
        const auto handle = details::get_thread_handle(thread);
        details::set_thread_name(handle, name);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto set_thread_name(std::jthread& thread, string_view name) noexcept -> void {
        const auto handle = details::get_thread_handle(thread);
        details::set_thread_name(handle, name);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto get_current_thread_name() noexcept -> string {
        const auto handle = win32::GetCurrentThread();
        return details::get_thread_name(handle);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto get_thread_name(const std::thread& thread) noexcept -> string {
        const auto handle = details::get_thread_handle(thread);
        return details::get_thread_name(handle);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto get_thread_name(const std::jthread& thread) noexcept -> string {
        const auto handle = details::get_thread_handle(thread);
        return details::get_thread_name(handle);
    }
}} // namespace stormkit::core
