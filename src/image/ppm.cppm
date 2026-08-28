// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>

export module stormkit.image:ppm;

import std;

import stormkit.core;
import stormkit.image;

export namespace stormkit::image::details {
    [[nodiscard]]
    auto load_ppm(byte_view data) noexcept -> std::expected<image::Image, image::Image::Error>;

    [[nodiscard]]
    auto save_ppm(const image::Image& image, image::Image::CodecTs args, const std::filesystem::path& filepath) noexcept
      -> std::expected<void, image::Image::Error>;

    [[nodiscard]]
    auto save_ppm(const image::Image& image, image::Image::CodecTs args) noexcept
      -> std::expected<byte_dynarray, image::Image::Error>;
} // namespace stormkit::image::details

using namespace std::literals;

namespace stormkit::image::details {
    template<class E>
    using Unexpected = std::unexpected<E>;
    using Error      = image::Image::Error;
    using Reason     = image::Image::Error::Reason;
    using Format     = image::Image::Format;

    /////////////////////////////////////
    /////////////////////////////////////
    auto load_ppm(byte_view) noexcept -> std::expected<image::Image, image::Image::Error> {
        assert(false, "Not implemented yet !");
        return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto save_ppm(const image::Image& image, image::Image::CodecTs args, const std::filesystem::path& filepath) noexcept
      -> std::expected<void, image::Image::Error> {
        return save_ppm(image, args)
          .and_then([&filepath](auto&& val) noexcept {
              return io::write(filepath, val).transform_error([](auto&&) static noexcept {
                  return Error { .reason = Error::Reason::FAILED_TO_SAVE, .str_error = "" };
              });
          })
          .transform(monadic::discard());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto save_ppm(const image::Image& image, image::Image::CodecTs args) noexcept
      -> std::expected<byte_dynarray, image::Image::Error> {
        const auto  output_image = image.convert_to(Format::RGB8_UNORM);
        const auto& data         = output_image.image_data();

        auto output = byte_dynarray {};
        if (args == image::Image::CodecTs::ASCII) {
            auto result = std::format("P3\n{}\n{}\n255\n"sv, data.extent.width, data.extent.height);

            const auto& extent = output_image.extent();
            for (auto [i, j] : multi_range(extent.height, extent.width)) {
                const auto pixel = output_image.pixel(i * output_image.extent().width + j);

                result += std::format("{} {} {}\n"sv, as<u16>(pixel[0]), as<u16>(pixel[1]), as<u16>(pixel[2]));

                if (j == extent.width) result += '\n';
            }

            output.reserve(std::size(result));
            std::ranges::copy(as<array_view>(as_bytes, result), std::back_inserter(output));
        } else if (args == image::Image::CodecTs::BINARY) {
            auto header = std::format("P3\n{}\n{}\n255\n"sv, data.extent.width, data.extent.height);
            output.reserve(std::size(output) + std::size(output_image));

            std::ranges::copy(as<array_view>(as_bytes, header), std::back_inserter(output));
            std::ranges::copy(output_image, std::back_inserter(output));
        }

        return output;
    }
} // namespace stormkit::image::details
