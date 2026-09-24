// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

export module stormkit.image:hdr;

import std;

import stormkit.core;
import stormkit.image;

namespace stdfs = std::filesystem;

export namespace stormkit::image::details {
    [[nodiscard]]
    auto load_hdr(array_view<const byte>) noexcept -> image::result<image>;

    [[nodiscard]]
    auto save_hdr(const image&, const stdfs::path&) noexcept -> image::result<void>;

    [[nodiscard]]
    auto save_hdr(const image&) noexcept -> image::result<dynarray<byte>>;
} // namespace stormkit::image::details

namespace stormkit::image::details {
    /////////////////////////////////////
    /////////////////////////////////////
    auto load_hdr(array_view<const byte>) noexcept -> image::result<image> {
        assert(false, "Not implemented yet !");
        return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto save_hdr(const image&, const stdfs::path&) noexcept -> image::result<void> {
        assert(false, "Not implemented yet !");
        return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto save_hdr(const image&) noexcept -> image::result<dynarray<byte>> {
        assert(false, "Not implemented yet !");
        return {};
    }
} // namespace stormkit::image::details
