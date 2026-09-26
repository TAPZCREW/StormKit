// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stormkit/core/platform_macro.hpp>

module stormkit.core.containers.shmbuffer;

namespace stdr = std::ranges;

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    shm_buffer::~shm_buffer() {
        if (m_handle) {
            munmap(stdr::data(m_data), m_size);
            shm_unlink(stdr::data(m_name));
            m_handle = nullptr;
            m_size   = 0;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto shm_buffer::do_init(usize size, string name, io::access access) noexcept -> system_result<void> {
        m_size                = size;
        m_name                = std::move(name);
        m_access              = access;
        const auto shm_access = (has_flag_bit(m_access, io::access::WRITE) ? O_RDWR : O_RDONLY)
                                | ((m_handle != nullptr) ? O_TRUNC : O_CREAT);

        const auto mode = init_by<mode_t>([access = m_access](auto& mode) noexcept {
            if (has_flag_bit(access, io::access::READ)) mode |= S_IRUSR;
            if (has_flag_bit(access, io::access::WRITE)) mode |= S_IWUSR;
        });

        m_handle = reinterpret_cast<void*>(iptr { shm_open(stdr::data(m_name), shm_access, mode) });
        if (m_handle == nullptr) return std::unexpected { error_code::from_errno() };
        const auto fd = unchecked_narrow<i32>(std::bit_cast<iptr>(m_handle));

        const auto ret = ftruncate(fd, as<off_t>(m_size));
        if (ret < 0) return std::unexpected { error_code::from_errno() };

        const auto prot_access = init_by<i32>([access = m_access](auto& prot_access) noexcept {
            if (has_flag_bit(access, io::access::READ)) prot_access |= PROT_READ;
            if (has_flag_bit(access, io::access::WRITE)) prot_access |= PROT_WRITE;
        });

        auto buf = mmap(nullptr, m_size, prot_access, MAP_SHARED, fd, 0);
        if (buf == nullptr) return std::unexpected { error_code::from_errno() };

        m_data = { reinterpret_cast<byte*>(buf), m_size };

        return {};
    }
}} // namespace stormkit::core
