// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <climits>
#include <cstdlib>
#include <cuchar>

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.string.encodings;

import std;

#ifdef STORMKIT_OS_WINDOWS
import stormkit.core.win32;
#endif

import stormkit.core.types;
import stormkit.core.typesafe;

export namespace stormkit { inline namespace core {
    auto ascii_to_utf16(string_view) -> u16string;
    auto utf16_to_ascii(u16string_view) -> string;

    auto ascii_to_wide(string_view) -> wstring;
    auto wide_to_ascii(wstring_view) -> string;

    auto ascii_to_utf8(string_view) -> u8string;
    auto utf8_to_ascii(u8string_view) -> string;

#ifdef STORMKIT_COMPILER_MSVC
    auto to_native_encoding(string_view) -> u16string;
#else
    auto to_native_encoding(string_view) -> u8string;
#endif
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stdr = std::ranges;

// TODO make proper support of char8_t / char16_t / char32_t on macOS / iOS / tvOS
namespace stormkit { inline namespace core {
    ////////////////////////////////////////
    ////////////////////////////////////////
    inline auto ascii_to_utf16(string_view input) -> u16string {
        auto output = u16string {};
#if not defined(STORMKIT_COMPILER_CLANG)
        auto state = std::mbstate_t {};
        output.resize(stdr::size(input));

        auto len      = 0ull;
        auto input_it = stdr::data(input);
        while ((len = std::mbrtoc16(reinterpret_cast<char16_t*>(stdr::data(output)), input_it, MB_CUR_MAX, &state)) > 0ull)
            input_it += len;
#else
        output.resize(stdr::size(input));
        auto i = 0;
        for (const auto& c : input) output[i] = unchecked_narrow<char16_t>(c);
#endif

        return output;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    inline auto utf16_to_ascii(u16string_view input) -> string {
        auto output = string {};
#if not defined(STORMKIT_COMPILER_CLANG)
        auto state = std::mbstate_t {};
        output.resize(stdr::size(input));

        for (const auto& c : input) auto _ = std::c16rtomb(reinterpret_cast<char*>(stdr::data(output)), c, &state);
#else
        output.resize(stdr::size(input));
        auto i = 0;
        for (const auto& c : input) output[i] = unchecked_narrow<char>(c);
#endif

        return output;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    inline auto ascii_to_wide(string_view input) -> wstring {
        [[maybe_unused]]
        auto output = wstring {};
#if defined(STORMKIT_OS_WINDOWS)
        auto count = ::win32::MultiByteToWideChar(::win32::CP_ACP, 0, stdr::data(input), stdr::size(input), nullptr, 0);
        output.resize(count);

        ::win32::MultiByteToWideChar(::win32::CP_UTF8,
                                     0,
                                     stdr::data(input),
                                     stdr::size(input),
                                     stdr::data(output),
                                     stdr::size(output));
#elif defined(STORMKIT_OS_MACOS)
        output.resize(stdr::size(input));
        auto i = 0;
        for (const auto& c : input) output[i] = unchecked_narrow<wchar_t>(c);
#else
        auto state = std::mbstate_t {};
        output.resize(stdr::size(input));

        auto len      = 0ull;
        auto input_it = stdr::data(input);
        auto i        = 0;
        while ((len = std::mbrtoc8(reinterpret_cast<char8_t*>(stdr::data(output)) + i++, input_it, MB_CUR_MAX, &state)) > 0ull)
            input_it += len;
#endif

        return output;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    inline auto wide_to_ascii(wstring_view input) -> string {
        [[maybe_unused]]
        auto output = string {};
#if defined(STORMKIT_OS_WINDOWS)
        auto count = ::win32::
          WideCharToMultiByte(::win32::CP_ACP, 0, stdr::data(input), stdr::size(input), nullptr, 0, nullptr, nullptr);
        output.resize(count);

        ::win32::WideCharToMultiByte(::win32::CP_UTF8,
                                     0,
                                     stdr::data(input),
                                     stdr::size(input),
                                     stdr::data(output),
                                     stdr::size(output),
                                     nullptr,
                                     nullptr);
#elif defined(STORMKIT_OS_MACOS)
        output.resize(stdr::size(input));
        auto i = 0;
        for (const auto& c : input) output[i] = unchecked_narrow<char>(c);
#else
        auto state = std::mbstate_t {};
        output.resize(stdr::size(input));

        for (const auto& c : input) auto _ = std::c8rtomb(stdr::data(output), unchecked_narrow<char>(c), &state);
#endif

        return output;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    inline auto ascii_to_utf8(string_view input) -> u8string {
        [[maybe_unused]]
        auto output = u8string {};
        output.resize(stdr::size(input) * unchecked_narrow<usize>(MB_LEN_MAX));

#if defined(STORMKIT_COMPILER_MSVC)
        auto bytes = bytes_of(output);
        stdr::copy(bytes_of(input), stdr::begin(bytes));
#elif defined(STORMKIT_COMPILER_CLANG)
        output = reinterpret_cast<const char8_t*>(stdr::data(input));
#else
        auto state    = std::mbstate_t {};
        auto len      = 0ull;
        auto input_it = stdr::data(input);
        auto i        = 0;
        while ((len = std::mbrtoc8(reinterpret_cast<char8_t*>(stdr::data(output)) + i++, input_it, MB_CUR_MAX, &state)) > 0ull)
            input_it += len;
#endif

        output.shrink_to_fit();

        return output;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    inline auto utf8_to_ascii(u8string_view input) -> string {
        [[maybe_unused]]
        auto output = string {};
        output.resize(stdr::size(input));

#if defined(STORMKIT_COMPILER_MSVC)
        auto bytes = bytes_of(output);
        stdr::copy(bytes_of(input), stdr::begin(bytes));
#elif defined(STORMKIT_COMPILER_CLANG)
        output = reinterpret_cast<const char*>(stdr::data(input));
#else
        auto state = std::mbstate_t {};
        for (const auto& c : input) std::c8rtomb(stdr::data(output), c, &state);
#endif

        output.shrink_to_fit();

        return output;
    }

#ifdef STORMKIT_COMPILER_MSVC
    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto to_native_encoding(string_view input) -> u16string {
        return ascii_to_utf16(input);
    }
#else
    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto to_native_encoding(string_view input) -> u8string {
        return ascii_to_utf8(input);
    }
#endif
}} // namespace stormkit::core
