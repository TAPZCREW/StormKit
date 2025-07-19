// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

module stormkit.core;

import :containers.shmbuffer;

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    SHMBuffer::~SHMBuffer() {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto SHMBuffer::allocate_buffer(bool named) noexcept -> std::expected<void, SHMBuffer::Error> {
        (void)named;

        return {};
    }
}} // namespace stormkit::core
