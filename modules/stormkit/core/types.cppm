// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#if not defined(__STDCPP_FLOAT32_T__) or not defined(__STDCPP_FLOAT64_T__) or not defined(__STDCPP_FLOAT128_T__)
    #ifdef STORMKIT_GLIBC
        #include <bits/floatn.h>
    #endif
#endif

#ifdef STORMKIT_COMPILER_MSVC
    #include <__msvc_int128.hpp>
#endif

export module stormkit.core.types;

import std;

namespace stdp = std::pmr;

template<typename T>
concept prefer_pass_by_value = sizeof(T) <= (sizeof(void*) * 2) and std::is_trivially_copyable_v<T>;

export {
    using uchar     = unsigned char;
    using ushort    = unsigned short;
    using uint      = unsigned int;
    using ulong     = unsigned long;
    using ulonglong = unsigned long long;

    using schar    = signed char;
    using longlong = signed long long;

    using longdouble = long double;

    namespace stormkit { inline namespace core {
        template<typename T>
        using raw_ptr = T*;

        using u8  = std::uint8_t;
        using u16 = std::uint16_t;
        using u32 = std::uint32_t;
        using u64 = std::uint64_t;
#ifdef __SIZEOF_INT128__
        __extension__ using u128 = unsigned __int128;
#elif defined(STORMKIT_COMPILER_MSVC)
        using u128 = std::_Unsigned128;
#endif

        using i8  = std::int8_t;
        using i16 = std::int16_t;
        using i32 = std::int32_t;
        using i64 = std::int64_t;
#ifdef __SIZEOF_INT128__
        __extension__ using i128 = __int128;
#elif defined(STORMKIT_COMPILER_MSVC)
        using i128 = std::_Signed128;
#else
#endif

        using usize   = std::size_t;
        using isize   = std::ptrdiff_t;
        using ioffset = std::ptrdiff_t;
        using uptr    = std::uintptr_t;
        using iptr    = std::intptr_t;

        using std::byte;

        enum class hash32 : u32 {
        };
        enum class hash64 : u64 {
        };

        using c8    = char8_t;
        using c16   = char16_t;
        using c32   = char32_t;
        using wchar = wchar_t;

#ifdef __STDCPP_FLOAT16_T__
        using f16 = std::float16_t;
#elif not defined(STORMKIT_OS_WINDOWS)
        using f16 = _Float16;
#endif

#ifdef __STDCPP_FLOAT32_T__
        using f32 = std::float32_t;
#else
    #ifdef STORMKIT_GLIBC
        using f32 = _Float32;
    #else
        using f32 = float;
    #endif
#endif
#ifdef __STDCPP_FLOAT64_T__
        using f64 = std::float64_t;
#else
    #ifdef STORMKIT_GLIBC
        using f64 = _Float64;
    #else
        using f64 = double;
    #endif
#endif
#ifdef __STDCPP_FLOAT128_T__
        using f128 = std::float128_t;
#else
        using f128 = long double;
#endif

        using fsecond = std::chrono::duration<f32, std::chrono::seconds::period>;

        using std::array;

        template<typename T, class Allocator = std::allocator<T>>
        using dynarray = std::vector<T, Allocator>;

        template<typename T, std::size_t Extent = std::dynamic_extent>
        using array_view = std::span<T, Extent>;

        template<typename T, class Extents, class LayoutPolicy = std::layout_right, class AccessorPolicy = std::default_accessor<T>>
        using mdarray_view = std::mdspan<T, Extents, LayoutPolicy, AccessorPolicy>;

        using std::basic_string;
        using std::basic_string_view;
        using std::string;
        using std::string_view;
        using std::u16string;
        using std::u16string_view;
        using std::u32string;
        using std::u32string_view;
        using std::u8string;
        using std::u8string_view;
        using std::wstring;
        using std::wstring_view;

        using czstring  = const char*;
        using zstring   = char*;
        using cwzstring = const wchar_t*;
        using wzstring  = wchar_t*;

        namespace pmr {
            template<typename T>
            using dynarray = core::dynarray<T, std::type_identity_t<stdp::polymorphic_allocator<T>>>;

            using stdp::string;
            using stdp::u16string;
            using stdp::u32string;
            using stdp::u8string;
            using stdp::wstring;
        } // namespace pmr

        using source_location_arg = std::
          conditional_t<prefer_pass_by_value<std::source_location>, const std::source_location, const std::source_location&>;

        namespace literals {
            [[nodiscard]]
            constexpr auto operator""_i8(unsigned long long) noexcept -> i8;
            [[nodiscard]]
            constexpr auto operator""_i16(unsigned long long) noexcept -> i16;
            [[nodiscard]]
            constexpr auto operator""_i32(unsigned long long) noexcept -> i32;
            [[nodiscard]]
            constexpr auto operator""_i64(unsigned long long) noexcept -> i64;
            [[nodiscard]]
            constexpr auto operator""_i128(unsigned long long) noexcept -> i128;

            [[nodiscard]]
            constexpr auto operator""_u8(unsigned long long) noexcept -> u8;
            [[nodiscard]]
            constexpr auto operator""_u16(unsigned long long) noexcept -> u16;
            [[nodiscard]]
            constexpr auto operator""_u32(unsigned long long) noexcept -> u32;
            [[nodiscard]]
            constexpr auto operator""_u64(unsigned long long) noexcept -> u64;
            [[nodiscard]]
            constexpr auto operator""_u128(unsigned long long) noexcept -> u128;

            [[nodiscard]]
            constexpr auto operator""_usize(unsigned long long) noexcept -> usize;
            [[nodiscard]]
            constexpr auto operator""_isize(unsigned long long) noexcept -> isize;
            [[nodiscard]]
            constexpr auto operator""_ioffset(unsigned long long) noexcept -> ioffset;
            [[nodiscard]]
            constexpr auto operator""_uptr(unsigned long long) noexcept -> uptr;
            [[nodiscard]]
            constexpr auto operator""_iptr(unsigned long long) noexcept -> iptr;

            [[nodiscard]]
            constexpr auto operator""_b(unsigned long long value) noexcept -> byte;

            [[nodiscard]]
            constexpr auto operator""_kb(unsigned long long x) noexcept -> u64;

            [[nodiscard]]
            constexpr auto operator""_mb(unsigned long long x) noexcept -> u64;

            [[nodiscard]]
            constexpr auto operator""_gb(unsigned long long x) noexcept -> u64;

            [[nodiscard]]
            constexpr auto operator""_kib(unsigned long long x) noexcept -> u64;

            [[nodiscard]]
            constexpr auto operator""_mib(unsigned long long x) noexcept -> u64;

            [[nodiscard]]
            constexpr auto operator""_gib(unsigned long long x) noexcept -> u64;
        } // namespace literals
    }} // namespace stormkit::core
}
////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core { namespace literals {
    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto operator""_i8(unsigned long long value) noexcept -> i8 {
        return static_cast<i8>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto operator""_i16(unsigned long long value) noexcept -> i16 {
        return static_cast<i16>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto operator""_i32(unsigned long long value) noexcept -> i32 {
        return static_cast<i32>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto operator""_i64(unsigned long long value) noexcept -> i64 {
        return static_cast<i64>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto operator""_i128(unsigned long long value) noexcept -> i128 {
        return static_cast<i128>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto operator""_u8(unsigned long long value) noexcept -> u8 {
        return static_cast<u8>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto operator""_u16(unsigned long long value) noexcept -> u16 {
        return static_cast<u16>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto operator""_u32(unsigned long long value) noexcept -> u32 {
        return static_cast<u32>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto operator""_u64(unsigned long long value) noexcept -> u64 {
        return static_cast<u64>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto operator""_u128(unsigned long long value) noexcept -> u128 {
        return static_cast<u128>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto operator""_usize(unsigned long long value) noexcept -> usize {
        return static_cast<usize>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto operator""_isize(unsigned long long value) noexcept -> isize {
        return static_cast<isize>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto operator""_ioffset(unsigned long long value) noexcept -> ioffset {
        return static_cast<ioffset>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto operator""_iptr(unsigned long long value) noexcept -> iptr {
        return static_cast<iptr>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto operator""_uptr(unsigned long long value) noexcept -> uptr {
        return static_cast<uptr>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto operator""_b(unsigned long long value) noexcept -> byte {
        return static_cast<byte>(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto operator""_kb(unsigned long long x) noexcept -> u64 {
        return x * 1000ULL;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto operator""_mb(unsigned long long x) noexcept -> u64 {
        return x * 1000_kb;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto operator""_gb(unsigned long long x) noexcept -> u64 {
        return x * 1000_mb;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto operator""_kib(unsigned long long x) noexcept -> u64 {
        return x * 1024;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto operator""_mib(unsigned long long x) noexcept -> u64 {
        return x * 1024_kib;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto operator""_gib(unsigned long long x) noexcept -> u64 {
        return x * 1024_mib;
    }
}}} // namespace stormkit::core::literals
