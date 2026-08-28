// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>

#include <stormkit/core/try_expected.hpp>

#ifdef STORMKIT_OS_WINDOWS
    #include <stormkit/core/platform/windows.hpp>

    #include <fcntl.h>
    #include <io.h>
    #include <sys/stat.h>
#else
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>
#endif

#include <cstdio>

export module stormkit.core.filesystem;

import std;

import stormkit.core.errors;
import stormkit.core.contract;
import stormkit.core.heap;
import stormkit.core.functional;
import stormkit.core.meta;
import stormkit.core.types;
import stormkit.core.private_tag;
import stormkit.core.typesafe.flags;
import stormkit.core.typesafe.safecasts;
import stormkit.core.containers.safecasts;

export {
    namespace stormkit { inline namespace core { namespace io {
        enum class open_mode {
            BINARY,
            AINSI,
            UTF8,
            WIDE,
        };
    }}} // namespace stormkit::core::io
}

namespace stormkit { inline namespace core { namespace io::meta {
    template<open_mode MODE>
        requires(MODE != open_mode::BINARY)
    using open_mode_to_char_type = core::meta::conditional<MODE == open_mode::WIDE, wchar, char>;
}}} // namespace stormkit::core::io::meta

export {
    namespace stormkit { inline namespace core {
        namespace stdfs = std::filesystem;

        namespace io {
            enum class access {
                READ,
                WRITE,
            };

            template<open_mode MODE>
            class file_descriptor final {
                using private_tag = private_tag<file_descriptor<MODE>>;

              public:
#ifdef STORMKIT_OS_WINDOWS
                using native_handle_type                = HANDLE;
                static inline const auto INVALID_HANDLE = native_handle_type { INVALID_HANDLE_VALUE };
#else
                using native_handle_type             = i32;
                static constexpr auto INVALID_HANDLE = native_handle_type { 0 };
#endif

                explicit file_descriptor(private_tag) noexcept;
                ~file_descriptor() noexcept;

                file_descriptor(file_descriptor&)                    = delete;
                auto operator=(file_descriptor&) -> file_descriptor& = delete;

                file_descriptor(file_descriptor&&) noexcept;
                auto operator=(file_descriptor&&) noexcept -> file_descriptor&;

                static auto open(const stdfs::path& path, access access) noexcept -> system_result<file_descriptor>;
                static auto allocate_and_open(const stdfs::path& path, access access) noexcept
                  -> system_result<heap_ptr<file_descriptor>>;

                auto close() noexcept;

                auto read_to(array_view<byte> out) noexcept -> system_result<usize>;
                auto read_to(array_view<char> out) noexcept -> system_result<usize>
                    requires(MODE == open_mode::UTF8 or MODE == open_mode::AINSI);
                auto read_to(array_view<wchar> out) noexcept -> system_result<usize>
                    requires(MODE == open_mode::WIDE);

                auto write(array_view<byte> bytes) noexcept -> system_result<usize>;
                auto write(array_view<const char> bytes) noexcept -> system_result<usize>
                    requires(MODE == open_mode::UTF8 or MODE == open_mode::AINSI);
                auto write(array_view<const wchar> bytes) noexcept -> system_result<usize>
                    requires(MODE == open_mode::WIDE);

                auto flush() noexcept -> void;

                auto position() const noexcept -> usize;
                auto size() const noexcept -> usize;

                auto native_descriptor() const noexcept -> native_handle_type;

              private:
                auto do_init(const stdfs::path&, access) noexcept -> system_result<void>;

                native_handle_type         m_descriptor = INVALID_HANDLE;
                mutable std::atomic<usize> m_size       = 0;
            };

            using File = file_descriptor<open_mode::BINARY>;
            template<open_mode MODE = open_mode::UTF8>
                requires(MODE != open_mode::BINARY)
            using TextFile = file_descriptor<MODE>;

            template<open_mode MODE = open_mode::UTF8>
            auto readfile_to(const stdfs::path& path, array_view<meta::open_mode_to_char_type<MODE>> output) noexcept
              -> system_result<usize>;
            template<open_mode MODE = open_mode::UTF8>
            auto readfile(const stdfs::path& path) noexcept -> system_result<dynarray<meta::open_mode_to_char_type<MODE>>>;

            auto readfile_to(const stdfs::path& path, array_view<byte> output) noexcept -> system_result<usize>;
            auto readfile(const stdfs::path& path) noexcept -> system_result<dynarray<byte>>;

            template<open_mode MODE = open_mode::UTF8>
            auto writefile(const stdfs::path& path, array_view<const meta::open_mode_to_char_type<MODE>> data) noexcept
              -> system_result<usize>;
            auto writefile(const stdfs::path& path, array_view<byte> data) noexcept -> system_result<usize>;
        } // namespace io
    }} // namespace stormkit::core

    template<>
    inline constexpr auto stormkit::meta::FLAG_TRAIT<stormkit::io::access> = true;
} // namespace stormkit::core::io

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stdr = std::ranges;

namespace stormkit { inline namespace core { namespace io {
    ////////////////////////////////////////
    ////////////////////////////////////////
    template<open_mode MODE>
    STORMKIT_FORCE_INLINE
    inline file_descriptor<MODE>::file_descriptor(private_tag) noexcept {
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<open_mode MODE>
    STORMKIT_FORCE_INLINE
    inline file_descriptor<MODE>::~file_descriptor() noexcept {
        close();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<open_mode MODE>
    STORMKIT_FORCE_INLINE
    inline file_descriptor<MODE>::file_descriptor(file_descriptor&& other) noexcept
        : m_descriptor { std::exchange(other.m_descriptor, INVALID_HANDLE) } {
        m_size = other.m_size.load();
        other.m_size.store(0);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<open_mode MODE>
    STORMKIT_FORCE_INLINE
    inline auto file_descriptor<MODE>::operator=(file_descriptor&& other) noexcept -> file_descriptor& {
        if (this == &other) [[unlikely]]
            return *this;

        m_descriptor = std::exchange(other.m_descriptor, INVALID_HANDLE);
        m_size       = other.m_size.load();
        other.m_size.store(0);

        return *this;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<open_mode MODE>
    STORMKIT_FORCE_INLINE
    inline auto file_descriptor<MODE>::open(const stdfs::path& path, access access) noexcept
      -> system_result<file_descriptor<MODE>> {
        auto out = file_descriptor<MODE> { PRIVATE<file_descriptor<MODE>> };
        Try(out.do_init(path, access));
        return { std::move(out) };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<open_mode MODE>
    STORMKIT_FORCE_INLINE
    inline auto file_descriptor<MODE>::allocate_and_open(const stdfs::path& path, access access) noexcept
      -> system_result<heap_ptr<file_descriptor<MODE>>> {
        auto out = allocate_unsafe<file_descriptor<MODE>>(PRIVATE<file_descriptor<MODE>>);
        Try(out->do_init(path, access));
        return { std::move(out) };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<open_mode MODE>
    STORMKIT_FORCE_INLINE
    inline auto file_descriptor<MODE>::close() noexcept {
        if (m_descriptor != INVALID_HANDLE) {
            flush();
#ifdef STORMKIT_OS_WINDOWS
            CloseHandle
#else
            ::close
#endif
              (m_descriptor);
            m_descriptor = INVALID_HANDLE;
            m_size       = 0;
        }
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<open_mode MODE>
    STORMKIT_FORCE_INLINE
    inline auto file_descriptor<MODE>::read_to(array_view<byte> out) noexcept -> system_result<usize> {
        EXPECTS(m_descriptor != INVALID_HANDLE);
#ifdef STORMKIT_OS_WINDOWS
        // TODO support async WriteFileEx
        auto ret = DWORD { 0 };
        const auto
          succeed = ReadFile(m_descriptor, std::bit_cast<void*>(stdr::data(out)), as<DWORD>(stdr::size(out)), &ret, nullptr);
        if (not succeed) return std::unexpected { error_code::from_win32() };
#else
        const auto ret = ::read(m_descriptor, std::bit_cast<void*>(stdr::data(out)), as<u32>(stdr::size(out)));
        if (ret == -1) return std::unexpected { error_code::from_errno() };
#endif

        return system_result<usize> { std::in_place, as<usize>(ret) };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<open_mode MODE>
    STORMKIT_FORCE_INLINE
    inline auto file_descriptor<MODE>::read_to(array_view<char> out) noexcept -> system_result<usize>
        requires(MODE == open_mode::UTF8 or MODE == open_mode::AINSI)
    {
        return read_to(as<array_view>(as_bytes, out));
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<open_mode MODE>
    STORMKIT_FORCE_INLINE
    inline auto file_descriptor<MODE>::read_to(array_view<wchar> out) noexcept -> system_result<usize>
        requires(MODE == open_mode::WIDE)
    {
        return read_to(as<array_view>(as_bytes, out));
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<open_mode MODE>
    STORMKIT_FORCE_INLINE
    inline auto file_descriptor<MODE>::write(array_view<byte> data) noexcept -> system_result<usize> {
        EXPECTS(m_descriptor != INVALID_HANDLE);
#ifdef STORMKIT_OS_WINDOWS
        // TODO support async WriteFileEx
        auto       ret     = DWORD { 0 };
        const auto succeed = WriteFile(m_descriptor,
                                       std::bit_cast<const void*>(stdr::data(data)),
                                       as<DWORD>(stdr::size(data)),
                                       &ret,
                                       nullptr);
        if (not succeed) return std::unexpected { error_code::from_win32() };
#else
        const auto ret = ::write(m_descriptor, std::bit_cast<const void*>(stdr::data(data)), as<u32>(stdr::size(data)));
        if (ret == -1) return std::unexpected { error_code::from_errno() };
#endif

        return system_result<usize> { std::in_place, as<usize>(ret) };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<open_mode MODE>
    STORMKIT_FORCE_INLINE
    inline auto file_descriptor<MODE>::write(array_view<const char> data) noexcept -> system_result<usize>
        requires(MODE == open_mode::UTF8 or MODE == open_mode::AINSI)
    {
        return write(as<array_view>(as_bytes, data));
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<open_mode MODE>
    STORMKIT_FORCE_INLINE
    inline auto file_descriptor<MODE>::write(array_view<const wchar> data) noexcept -> system_result<usize>
        requires(MODE == open_mode::WIDE)
    {
        return write(as<array_view>(as_bytes, data));
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<open_mode MODE>
    STORMKIT_FORCE_INLINE
    inline auto file_descriptor<MODE>::flush() noexcept -> void {
        EXPECTS(m_descriptor != INVALID_HANDLE);
#ifdef STORMKIT_OS_WINDOWS
        FlushFileBuffers(reinterpret_cast<HANDLE>(m_descriptor));
#else
    #ifdef STORMKIT_OS_LINUX
        fdatasync
    #else
        fsync
    #endif
          (m_descriptor);
#endif
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<open_mode MODE>
    STORMKIT_FORCE_INLINE
    inline auto file_descriptor<MODE>::position() const noexcept -> usize {
        EXPECTS(m_descriptor != INVALID_HANDLE);
#ifdef STORMKIT_OS_WINDOWS
        auto current_position = LARGE_INTEGER { .QuadPart = 0 };
        SetFilePointerEx(m_descriptor, LARGE_INTEGER { .QuadPart = 0 }, &current_position, FILE_CURRENT);
        return as<usize>(current_position.QuadPart);
#else
        return as<usize>(lseek(m_descriptor, 0, SEEK_CUR));
#endif
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<open_mode MODE>
    inline auto file_descriptor<MODE>::size() const noexcept -> usize {
        EXPECTS(m_descriptor != INVALID_HANDLE);
        if (m_size == 0) {
#ifdef STORMKIT_OS_WINDOWS
            auto       size    = LARGE_INTEGER { .QuadPart = 0 };
            const auto succeed = GetFileSizeEx(m_descriptor, &size);
            if (succeed) m_size = as<usize>(size.QuadPart);
#else
            lseek(m_descriptor, 0, SEEK_SET);
            m_size = as<usize>(lseek(m_descriptor, 0, SEEK_END));
            lseek(m_descriptor, as<i32>(old_pos), SEEK_SET);
#endif
        }

        return m_size;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<open_mode MODE>
    STORMKIT_FORCE_INLINE
    inline auto file_descriptor<MODE>::native_descriptor() const noexcept -> native_handle_type {
        EXPECTS(m_descriptor != INVALID_HANDLE);
        return m_descriptor;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<open_mode MODE>
    inline auto file_descriptor<MODE>::do_init(const stdfs::path& path, access access) noexcept -> system_result<void> {
        if (access == access::READ and not stdfs::exists(path))
            return std::unexpected { error_code::from_stderrc(std::errc::no_such_file_or_directory) };

        if (stdfs::is_directory(path)) return std::unexpected { error_code::from_stderrc(std::errc::is_a_directory) };

#ifdef STORMKIT_OS_WINDOWS
        const auto path_ = "\\\\?\\" / path;

        const auto win32_access = [&access]() noexcept -> DWORD {
            if (access == access::READ) return GENERIC_READ;
            else if (access == access::WRITE)
                return GENERIC_WRITE;
            else
                return GENERIC_WRITE | GENERIC_READ;
            std::unreachable();
        }();
        const auto path_as_string = path_.string();
        const auto ret            = CreateFile(stdr::data(path_as_string),
                                               win32_access,
                                               FILE_SHARE_READ,
                                               nullptr,
                                               OPEN_ALWAYS,
                                               FILE_ATTRIBUTE_NORMAL,
                                               INVALID_HANDLE_VALUE);
        if (ret == INVALID_HANDLE_VALUE) return std::unexpected { error_code::from_win32() };
#else
        const auto posix_access = [&access]() noexcept {
            if (access == access::READ) return O_RDONLY;
            else if (access == access::WRITE)
                return O_WRONLY | O_CREAT;
            else
                return O_RDWR;
            std::unreachable();
        }();

        const auto ret = ::open(path.c_str(), posix_access);
        if (ret == -1) return std::unexpected { error_code::from_errno() };
#endif
        m_descriptor = ret;
        return {};
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<open_mode MODE>
    STORMKIT_FORCE_INLINE
    inline auto readfile_to(const stdfs::path& path, array_view<meta::open_mode_to_char_type<MODE>> out) noexcept
      -> system_result<usize> {
        TryTo(file, (TextFile<MODE>::open(path, access::READ)));
        ENSURES(stdr::size(out) >= file.size());
        TryTo(readed, file.read_to(out));
        return { readed };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<open_mode MODE>
    STORMKIT_FORCE_INLINE
    inline auto readfile(const stdfs::path& path) noexcept -> system_result<dynarray<meta::open_mode_to_char_type<MODE>>> {
        TryTo(file, (TextFile<MODE>::open(path, access::READ)));
        auto out = dynarray<meta::open_mode_to_char_type<MODE>> {};
        out.resize(file.size());
        TryTo(readed, file.read_to(out));
        out.resize(readed);
        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto readfile_to(const stdfs::path& path, array_view<byte> out) noexcept -> system_result<usize> {
        TryTo(file, File::open(path, access::READ));
        TryTo(readed, file.read_to(out));
        return { readed };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto readfile(const stdfs::path& path) noexcept -> system_result<dynarray<byte>> {
        TryTo(file, (File::open(path, access::READ)));
        auto out = dynarray<byte> {};
        out.resize(file.size());
        TryTo(readed, file.read_to(out));
        out.resize(readed);
        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<open_mode MODE>
        STORMKIT_FORCE_INLINE
    inline auto writefile(const stdfs::path& path, array_view<const meta::open_mode_to_char_type<MODE>> data) noexcept
      -> system_result<usize> {
        TryTo(file, (TextFile<MODE>::open(path, access::WRITE)));
        TryTo(writed, file.write(data));
        return { writed };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto writefile(const stdfs::path& path, array_view<byte> data) noexcept -> system_result<usize> {
        TryTo(file, (File::open(path, access::WRITE)));
        TryTo(writed, file.write(data));
        return { writed };
    }
}}} // namespace stormkit::core::io
