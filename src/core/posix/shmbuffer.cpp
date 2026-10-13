// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
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
        if (handle_) {
            munmap(stdr::data(data_), size_);
            shm_unlink(stdr::data(name_));
            handle_ = nullptr;
            size_   = 0;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto shm_buffer::do_init(usize size, string&& name, io::access access) noexcept -> system_result<void> {
        size_                 = size;
        name_                 = std::move(name);
        access_               = access;
        const auto shm_access = (has_flag_bit(access_, io::access::write) ? O_RDWR : O_RDONLY)
                                | ((handle_ != nullptr) ? O_TRUNC : O_CREAT);

        const auto mode = init_by<mode_t>([access = access_](auto& mode) noexcept {
            if (has_flag_bit(access, io::access::read)) mode |= S_IRUSR;
            if (has_flag_bit(access, io::access::write)) mode |= S_IWUSR;
        });

        handle_ = reinterpret_cast<void*>(iptr { shm_open(stdr::data(name_), shm_access, mode) });
        if (handle_ == nullptr) return std::unexpected { error_code::from_errno() };

        const auto fd  = unchecked_narrow<i32>(std::bit_cast<iptr>(handle_));
        const auto ret = ftruncate(fd, as<off_t>(size_));
        if (ret < 0) return std::unexpected { error_code::from_errno() };

        const auto prot_access = init_by<i32>([access = access_](auto& prot_access) noexcept {
            if (has_flag_bit(access, io::access::read)) prot_access |= PROT_READ;
            if (has_flag_bit(access, io::access::write)) prot_access |= PROT_WRITE;
        });

        auto buf = mmap(nullptr, size_, prot_access, MAP_SHARED, fd, 0);
        if (buf == nullptr) return std::unexpected { error_code::from_errno() };

        data_ = { reinterpret_cast<byte*>(buf), size_ };

        return {};
    }
}} // namespace stormkit::core
