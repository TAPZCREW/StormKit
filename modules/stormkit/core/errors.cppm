// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>

#ifdef STORMKIT_OS_WINDOWS
    #include <stormkit/core/platform/windows.hpp>
#endif

export module stormkit.core.errors;

import std;

import stormkit.core.types;

export import stormkit.core.status_code;
import stormkit.core.meta.type_query;

export {
    namespace stormkit { inline namespace core {
        template<typename Val, typename Err>
        using expected = std::expected<Val, Err>;
#if not(defined(__clang__) or defined(__GNUC__)) and STORMKIT_TRYX_COROUTINES
        template<typename VAL, typename ERR>
        class expected: public std::expected<VAL, ERR> {
          public:
            using std::expected<VAL, ERR>::expected;

            struct promise_type {
                constexpr auto initial_suspend() const noexcept -> std::suspend_never { return {}; }

                constexpr auto final_suspend() const noexcept -> std::suspend_never { return {}; }

                constexpr auto get_return_object() noexcept -> expected { return expected { this }; }

                template<typename... Ts>
                constexpr auto return_value(Ts&&... args) noexcept {
                    EXPECTS(expected_ptr != nullptr);
                    *expected_ptr = expected { std::in_place, std::forward<Ts>(args)... };
                }

                constexpr auto return_value(ERR error) noexcept {
                    EXPECTS(expected_ptr != nullptr);
                    *expected_ptr = expected {
                        std::unexpected<ERR> { std::in_place, std::move(error) }
                    };
                }

                [[noreturn]]
                auto unhandled_exception() const noexcept {
                    std::abort();
                }

                expected* expected_ptr = nullptr;
            };

            constexpr expected(promise_type* promise) noexcept : expected {} { promise->expected_ptr = this; }

            constexpr auto await_ready() const noexcept -> bool { return this->has_value(); }

            constexpr auto await_resume() noexcept -> VAL { return std::move(*this).value(); }

            template<typename Promise>
            constexpr auto await_suspend(std::coroutine_handle<Promise> handle) noexcept -> bool {
                handle.promise().return_value(this->error());
                handle.destroy();
                return true;
            }
        };
#endif
        using system_code = system_error2::system_code;

        namespace error_code {
#ifdef STORMKIT_OS_WINDOWS
            auto from_win32() noexcept -> system_code;
            auto from_ntstatus(long code) noexcept -> system_code;
#endif
            auto from_errno() noexcept -> system_code;
            auto from_stderrc(std::errc code) noexcept -> system_code;
        } // namespace error_code

        template<typename T>
        using system_result = expected<T, system_code>;
    }} // namespace stormkit::core
}

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stdr = std::ranges;

namespace stormkit { inline namespace core {
    namespace error_code {
#ifdef STORMKIT_OS_WINDOWS
        ////////////////////////////////////////
        ////////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto from_win32() noexcept -> system_code {
            return system_error2::win32_code { GetLastError() };
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto from_ntstatus(long status) noexcept -> system_code {
            return system_error2::nt_code { status };
        }
#endif

        ////////////////////////////////////////
        ////////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto from_errno() noexcept -> system_code {
            return system_error2::posix_code { errno };
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto from_stderrc(std::errc code) noexcept -> system_code {
            return system_error2::posix_code { static_cast<i32>(code) };
        }
    } // namespace error_code

    // ////////////////////////////////////////
    // ////////////////////////////////////////
    // template<typename CharT, typename FormatContext>
    // STORMKIT_FORCE_INLINE
    // constexpr auto tag_invoke(format_as_fn<CharT>, meta::in<system_code> error, FormatContext& ctx) -> decltype(ctx.out()) {
    //     return std::format_to(ctx.out(),
    //                           "{:#x} ({})",
    //                           static_cast<unsigned long long>(error.value()),
    //                           std::string_view { error.message() });
    // }
}} // namespace stormkit::core
