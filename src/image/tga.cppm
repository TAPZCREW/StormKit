// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>

export module stormkit.image:tga;

import std;

import stormkit.core;

import stormkit.image;

namespace stdfs = std::filesystem;

export namespace stormkit::image::details {
    [[nodiscard]]
    auto load_tga(array_view<const byte>) noexcept -> image::result<image>;

    [[nodiscard]]
    auto save_tga(const image&, const stdfs::path&) noexcept -> image::result<void>;

    [[nodiscard]]
    auto save_tga(const image&) noexcept -> image::result<dynarray<byte>>;
} // namespace stormkit::image::details

namespace stormkit::image::details {
    /////////////////////////////////////
    /////////////////////////////////////
    auto load_tga(array_view<const byte>) noexcept -> image::result<image> {
        assert(false, "Not implemented yet !");
        return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto save_tga(const image&, const stdfs::path&) noexcept -> image::result<void> {
        assert(false, "Not implemented yet !");
        return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto save_tga(const image&) noexcept -> image::result<dynarray<byte>> {
        assert(false, "Not implemented yet !");
        return {};
    }
} // namespace stormkit::image::details
