// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

module stormkit.core;

import :containers.shmbuffer;

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    SHMBuffer::~SHMBuffer() {
        if (m_handle) {
            munmap(stdr::data(m_data), m_size);
            shm_unlink(stdr::data(m_name));
            m_handle = nullptr;
            m_size   = 0;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto SHMBuffer::allocate_buffer() noexcept -> std::expected<void, std::error_code> {
        const auto shm_access = (check_flag_bit(m_access, Access::WRITE) ? O_RDWR : O_RDONLY)
                                | ((m_handle != nullptr) ? O_TRUNC : O_CREAT);

        const auto mode = init<i32>([access = m_access](auto& mode) noexcept {
            if (check_flag_bit(access, Access::READ)) mode |= S_IRUSR;
            if (check_flag_bit(access, Access::WRITE)) mode |= S_IWUSR;
        });

        m_handle = std::bit_cast<void*>(iptr { shm_open(stdr::data(m_name), shm_access, mode) });
        if (not m_handle)
            return std::unexpected {
                std::error_code { as<i32>(errno), std::system_category() }
            };
        const auto fd = narrow<i32>(std::bit_cast<iptr>(m_handle));

        const auto ret = ftruncate(fd, m_size);
        if (ret < 0)
            return std::unexpected {
                std::error_code { as<i32>(errno), std::system_category() }
            };

        const auto prot_access = init<i32>([access = m_access](auto& prot_access) noexcept {
            if (check_flag_bit(access, Access::READ)) prot_access |= PROT_READ;
            if (check_flag_bit(access, Access::WRITE)) prot_access |= PROT_WRITE;
        });

        auto buf = mmap(nullptr, m_size, prot_access, MAP_SHARED, fd, 0);
        if (not buf)
            return std::unexpected {
                std::error_code { as<i32>(errno), std::system_category() }
            };

        m_data = { std::bit_cast<Byte*>(buf), m_size };

        return {};
    }
}} // namespace stormkit::core
