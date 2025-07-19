// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <windows.h>

#include <WinNT.h>

module stormkit.core;

import :containers.shmbuffer;

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    SHMBuffer::~SHMBuffer() {
        if (m_handle) {
            UnmapViewOfFile(stdr::data(m_data));
            CloseHandle(std::bit_cast<HANDLE>(m_handle));
            m_handle = nullptr;
            m_size   = 0;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto SHMBuffer::allocate_buffer() noexcept -> std::expected<void, std::error_code> {
        const auto page_access = (check_flag_bit(m_access, Access::WRITE) ? PAGE_READWRITE
                                                                          : PAGE_READONLY);

        // TODO handle reallocation
        m_handle = ::CreateFileMapping(INVALID_HANDLE_VALUE,
                                       nullptr,
                                       page_access,
                                       0,
                                       as<DWORD>(m_size),
                                       stdr::data(m_name));
        if (m_handle == nullptr)
            return std::unexpected {
                std::error_code { as<i32>(GetLastError()), std::system_category() }
            };

        const auto file_access = init<u32>([access = m_access](auto& file_access) noexcept {
            if (check_flag_bit(access, Access::READ)) file_access |= FILE_MAP_READ;
            if (check_flag_bit(access, Access::WRITE)) file_access |= FILE_MAP_WRITE;
        });

        auto buf = ::MapViewOfFile(m_handle, file_access, 0, 0, as<DWORD>(m_size));
        if (buf == nullptr)
            return std::unexpected {
                std::error_code { as<i32>(GetLastError()), std::system_category() }
            };

        m_data = { std::bit_cast<Byte*>(buf), m_size };

        return {};
    }
}} // namespace stormkit::core
