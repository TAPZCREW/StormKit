// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module stormkit.core.containers.shmbuffer;

import stormkit.core.win32;

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    shm_buffer::~shm_buffer() {
        if (m_handle) {
            win32::UnmapViewOfFile(stdr::data(m_data));
            win32::CloseHandle(std::bit_cast<win32::HANDLE>(m_handle));
            m_handle = nullptr;
            m_size   = 0;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto shm_buffer::do_init(usize size, string name, io::access access) noexcept -> system_result<void> {
        m_size                 = size;
        m_name                 = std::move(name);
        m_access               = access;
        const auto page_access = (has_flag_bit(m_access, io::access::WRITE) ? win32::PAGE_READWRITE : win32::PAGE_READONLY);

        // TODO handle reallocation
        m_handle = win32::CreateFileMappingA(win32::INVALID_HANDLE_VALUE,
                                             nullptr,
                                             as<win32::DWORD>(page_access),
                                             0,
                                             as<win32::DWORD>(m_size),
                                             stdr::data(m_name));
        if (m_handle == nullptr) return std::unexpected { error_code::from_win32() };

        const auto file_access = init_by<u32>([access = m_access](auto& file_access) noexcept {
            if (has_flag_bit(access, io::access::READ)) file_access |= win32::FILE_MAP_READ;
            if (has_flag_bit(access, io::access::WRITE)) file_access |= win32::FILE_MAP_WRITE;
        });

        auto buf = win32::MapViewOfFile(m_handle, file_access, 0, 0, as<win32::DWORD>(m_size));
        if (buf == nullptr) return std::unexpected { error_code::from_win32() };

        m_data = { reinterpret_cast<byte*>(buf), m_size };

        return {};
    }
}} // namespace stormkit::core
