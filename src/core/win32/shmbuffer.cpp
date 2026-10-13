// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module stormkit.core.containers.shmbuffer;

import stormkit.core.win32;

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    shm_buffer::~shm_buffer() {
        if (handle_) {
            win32::UnmapViewOfFile(stdr::data(data_));
            win32::CloseHandle(std::bit_cast<win32::HANDLE>(handle_));
            handle_ = nullptr;
            size_   = 0;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto shm_buffer::do_init(usize size, string&& name, io::access access) noexcept -> system_result<void> {
        size_                  = size;
        name_                  = std::move(name);
        access_                = access;
        const auto page_access = (has_flag_bit(access_, io::access::write) ? win32::PAGE_READWRITE : win32::PAGE_READONLY);

        // TODO handle reallocation
        handle_ = win32::CreateFileMappingA(win32::INVALID_HANDLE_VALUE,
                                            nullptr,
                                            as<win32::DWORD>(page_access),
                                            0,
                                            as<win32::DWORD>(size_),
                                            stdr::data(name_));
        if (handle_ == nullptr) return std::unexpected { error_code::from_win32() };

        const auto file_access = init_by<u32>([access = access_](auto& file_access) noexcept {
            if (has_flag_bit(access, io::access::read)) file_access |= win32::FILE_MAP_READ;
            if (has_flag_bit(access, io::access::write)) file_access |= win32::FILE_MAP_WRITE;
        });

        auto buf = win32::MapViewOfFile(handle_, file_access, 0, 0, as<win32::DWORD>(size_));
        if (buf == nullptr) return std::unexpected { error_code::from_win32() };

        data_ = { reinterpret_cast<byte*>(buf), size_ };

        return {};
    }
}} // namespace stormkit::core
