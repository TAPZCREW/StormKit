// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/try_expected.hpp>

export module stormkit.image:ppm;

import std;

import stormkit.core;
import stormkit.image;

namespace stdfs = std::filesystem;

export namespace stormkit::image::details {
    [[nodiscard]]
    auto load_ppm(array_view<const byte>) noexcept -> image::result<image>;

    [[nodiscard]]
    auto save_ppm(const image&, image_codec_format, const stdfs::path&) noexcept -> image::result<void>;

    [[nodiscard]]
    auto save_ppm(const image&, image_codec_format) noexcept -> image::result<dynarray<byte>>;
} // namespace stormkit::image::details

using namespace std::literals;

namespace stdr = std::ranges;

namespace stormkit::image::details {
    /////////////////////////////////////
    /////////////////////////////////////
    auto load_ppm(array_view<const byte>) noexcept -> image::result<image> {
        assert(false, "Not implemented yet !");
        return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto save_ppm(const image& image, image_codec_format format, const stdfs::path& filepath) noexcept
      -> image::result<void> {
        TryTo(bytes, save_ppm(image, format));
        Try(io::writefile(filepath, bytes));
        return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto save_ppm(const image& image, image_codec_format format) noexcept -> image::result<dynarray<byte>> {
        const auto  output_image = image.convert_to(image_format::RGB8_UNORM);
        const auto& data         = output_image.image_data();

        auto output = dynarray<byte> {};
        if (format == image_codec_format::ASCII) {
            auto result = std::format("P3\n{}\n{}\n255\n"sv, data.extent.width, data.extent.height);

            const auto& extent = output_image.extent();
            for (auto [i, j] : multi_range(extent.height, extent.width)) {
                const auto pixel = output_image.pixel(i * output_image.extent().width + j);

                result += std::format("{} {} {}\n"sv, as<u16>(pixel[0]), as<u16>(pixel[1]), as<u16>(pixel[2]));

                if (j == extent.width) result += '\n';
            }

            output.reserve(std::size(result) * sizeof(char));
            stdr::copy(view_of(as_bytes, result), std::back_inserter(output));
        } else if (format == image_codec_format::BINARY) {
            auto header = std::format("P3\n{}\n{}\n255\n"sv, data.extent.width, data.extent.height);
            output.reserve((std::size(header) + std::size(output_image)) * sizeof(char));

            stdr::copy(view_of(as_bytes, header), std::back_inserter(output));
            stdr::copy(output_image.data(), std::back_inserter(output));
        }

        return output;
    }
} // namespace stormkit::image::details
