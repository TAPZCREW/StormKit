// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>

export module stormkit.image:qoi;

import std;

import stormkit.core;
import stormkit.image;

export namespace stormkit::image::details {
    [[nodiscard]]
    auto load_qoi(byte_view data) noexcept -> std::expected<image::Image, image::Image::Error>;

    [[nodiscard]]
    auto save_qoi(const image::Image& image, const std::filesystem::path& filepath) noexcept
      -> std::expected<void, image::Image::Error>;

    [[nodiscard]]
    auto save_qoi(const image::Image& image) noexcept -> std::expected<byte_dynarray, image::Image::Error>;
} // namespace stormkit::image::details

using namespace std::literals;

namespace stdr = std::ranges;

namespace stormkit::image::details {
    template<class E>
    using Unexpected = std::unexpected<E>;
    using Error      = image::Image::Error;
    using Reason     = image::Image::Error::Reason;

    struct QOIHeader {
        array<byte, 4> magic;
        u32            width;
        u32            height;
        u8             channels;
        u8             colorspace;
    };

    namespace {
        constexpr auto SIZE_OF_HEADER = 14;

        constexpr auto CHANNELS_TO_FORMAT = make_static_hash_map<i32, array<image::Image::Format, 2>>({
          { 3, array { image::Image::Format::SRGB8, image::Image::Format::RGB8_UNORM }   },
          { 4, array { image::Image::Format::SRGBA8, image::Image::Format::RGBA8_UNORM } }
        });

        constexpr auto END_OF_FILE = into<bytes_view>({ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 });

        constexpr auto PIXEL_CACHE_SIZE = 64u;
    } // namespace

    enum class QOI_OPERATION : u8 {
        RGB   = 0b11111110,
        RGBA  = 0b11111111,
        INDEX = 0b00000000,
        DIFF  = 0b01000000,
        LUMA  = 0b10000000,
        RUN   = 0b11000000,
    };

    union Pixel {
        struct {
            u8 r = 0;
            u8 g = 0;
            u8 b = 0;
            u8 a = 0;
        } rgba;

        array<u8, 4> data;
    };

    /////////////////////////////////////
    /////////////////////////////////////
    constexpr auto indexHash(const Pixel& pixel) noexcept {
        return (pixel.rgba.r * 3u + pixel.rgba.g * 5u + pixel.rgba.b * 7u + pixel.rgba.a * 11u) % PIXEL_CACHE_SIZE;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto load_qoi(byte_view data) noexcept -> std::expected<image::Image, image::Image::Error> {
        const auto  raw_header = data.subspan(SIZE_OF_HEADER);
        const auto* header     = std::bit_cast<const QOIHeader*>(stdr::data(raw_header));

        const auto extent   = math::uextent3 { .width = byte_swap(header->width), .height = byte_swap(header->height) };
        const auto channels = header->channels;
        const auto format   = CHANNELS_TO_FORMAT.at(header->channels)[header->colorspace];

        auto pixel_cache = array<Pixel, PIXEL_CACHE_SIZE> {};

        const auto chunks = array_view { std::bit_cast<const u8*>(stdr::data(data)) + SIZE_OF_HEADER,
                                         stdr::size(data) - SIZE_OF_HEADER };

        const auto output_size = extent.width * extent.height * channels;

        auto output = byte_dynarray {};
        output.reserve(output_size);

        auto previous_pixel = Pixel { .rgba = { .a = 255 } };

        auto run = 0;

        const auto diff = 4 - channels;
        auto       it   = stdr::begin(chunks);

        const auto chunks_size = output_size - stdr::size(END_OF_FILE);
        for (auto _ : range(output_size, channels)) {
            const auto tag = *it;

            const auto position = as<usize>(std::distance(stdr::begin(chunks), it));

            if (run > 0) --run;
            else if (std::memcmp(&*it, std::data(END_OF_FILE), stdr::size(END_OF_FILE)) == 0) [[unlikely]] {
                it = stdr::cend(chunks);
            } else if (position < chunks_size) {
                ++it;
                if (static_cast<QOI_OPERATION>(tag) == QOI_OPERATION::RGB) {
                    previous_pixel.rgba.r = *it;
                    previous_pixel.rgba.g = *(it + 1);
                    previous_pixel.rgba.b = *(it + 2);

                    it += 3;
                } else if (static_cast<QOI_OPERATION>(tag) == QOI_OPERATION::RGBA) {
                    previous_pixel.rgba.r = *it;
                    previous_pixel.rgba.g = *(it + 1);
                    previous_pixel.rgba.b = *(it + 2);
                    previous_pixel.rgba.a = *(it + 3);

                    it += 4;
                } else {
#define CHECK(op) (tag & 0b11000000) == static_cast<u8>(op)
                    if (CHECK(QOI_OPERATION::INDEX)) {
                        const auto index = tag;

                        previous_pixel = pixel_cache[index];
                    } else if (CHECK(QOI_OPERATION::DIFF)) {
                        const auto r_diff = as<u8>(((tag >> 4) & 0x03) - 2);
                        const auto g_diff = as<u8>(((tag >> 2) & 0x03) - 2);
                        const auto b_diff = as<u8>((tag & 0x03) - 2);

                        previous_pixel.rgba.r += r_diff;
                        previous_pixel.rgba.g += g_diff;
                        previous_pixel.rgba.b += b_diff;

                    } else if (CHECK(QOI_OPERATION::LUMA)) {
                        const auto g_diff = (tag & 0x3f) - 32;

                        const auto current_r = ((*it) >> 4) & 0x0f;
                        const auto current_b = (*it) & 0x0f;

                        previous_pixel.rgba.r += as<u8>(g_diff - 8 + current_r);
                        previous_pixel.rgba.g += as<u8>(g_diff);
                        previous_pixel.rgba.b += as<u8>(g_diff - 8 + current_b);

                        ++it;
                    } else if (CHECK(QOI_OPERATION::RUN)) {
                        run = (tag & 0x3f);
                    }
#undef CHECK
                }

                auto& cached = pixel_cache[indexHash(previous_pixel)];
                cached       = previous_pixel;
            }

            stdr::transform(stdr::begin(previous_pixel.data),
                            stdr::end(previous_pixel.data) - diff,
                            std::back_inserter(output),
                            monadic::as<byte>());
        }

        auto image_data = image::Image::ImageData {
            .extent            = extent,
            .channel_count     = channels,
            .bytes_per_channel = getSizeof(format),
            .layers            = 1u,
            .faces             = 1u,
            .mip_levels        = 1u,
            .format            = format,
            .data              = std::move(output)

        };

        return image::Image { std::move(image_data) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto save_qoi(const image::Image&, const std::filesystem::path&) noexcept -> std::expected<void, image::Image::Error> {
        assert(false, "Not implemented yet !");
        return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    [[nodiscard]]
    auto save_qoi(const image::Image&) noexcept -> std::expected<byte_dynarray, image::Image::Error> {
        assert(false, "Not implemented yet !");
        return {};
    }
} // namespace stormkit::image::details
