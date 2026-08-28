// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

export module stormkit.image:hdr;

import std;

import stormkit.core;
import stormkit.image;

export namespace stormkit::image::details {
    [[nodiscard]]
    auto load_hdr(byte_view data) noexcept -> std::expected<image::Image, image::Image::Error>;

    [[nodiscard]]
    auto save_hdr(const image::Image& image, const std::filesystem::path& filepath) noexcept
      -> std::expected<void, image::Image::Error>;

    [[nodiscard]]
    auto save_hdr(const image::Image& image) noexcept -> std::expected<byte_dynarray, image::Image::Error>;
} // namespace stormkit::image::details

namespace stormkit::image::details {
    template<class E>
    using Unexpected = std::unexpected<E>;
    using Error      = image::Image::Error;
    using Reason     = image::Image::Error::Reason;

    /////////////////////////////////////
    /////////////////////////////////////
    auto load_hdr(byte_view) noexcept -> std::expected<image::Image, image::Image::Error> {
        assert(false, "Not implemented yet !");
        return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto save_hdr(const image::Image&, const std::filesystem::path&) noexcept -> std::expected<void, image::Image::Error> {
        assert(false, "Not implemented yet !");
        return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto save_hdr(const image::Image&) noexcept -> std::expected<byte_dynarray, image::Image::Error> {
        assert(false, "Not implemented yet !");
        return {};
    }
} // namespace stormkit::image::details
