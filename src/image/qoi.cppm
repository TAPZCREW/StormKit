// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>

export module stormkit.image:qoi;

import std;

import stormkit.core;
import stormkit.image;
import stormkit.math;

namespace stdfs = std::filesystem;

export namespace stormkit::image::details {
    [[nodiscard]]
    auto load_qoi(array_view<const byte>) noexcept -> image::result<image>;

    [[nodiscard]]
    auto save_qoi(const image&, const stdfs::path&) noexcept -> image::result<void>;

    [[nodiscard]]
    auto save_qoi(const image&) noexcept -> image::result<dynarray<byte>>;
} // namespace stormkit::image::details

using namespace std::literals;
using namespace stormkit::literals;

namespace stdr = std::ranges;

namespace stormkit::image::details {
    struct qoi_header {
        array<byte, 4> magic;
        u32            width;
        u32            height;
        u8             channels;
        u8             colorspace;
    };

    namespace {
        constexpr auto SIZE_OF_HEADER = 14_usize;

        constexpr auto CHANNELS_TO_FORMAT = make_static_hash_map<i32, array<image_format, 2>>({
          { 3, array { image_format::SRGB8, image_format::RGB8_UNORM }   },
          { 4, array { image_format::SRGBA8, image_format::RGBA8_UNORM } }
        });

        constexpr auto END_OF_FILE = into<array>(as_bytes, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 });

        constexpr auto PIXEL_CACHE_SIZE = 64_usize;
    } // namespace

    enum class qoi_operation : u8 {
        RGB   = 0b11111110,
        RGBA  = 0b11111111,
        INDEX = 0b00000000,
        DIFF  = 0b01000000,
        LUMA  = 0b10000000,
        RUN   = 0b11000000,
    };

    using pixel = array<u8, 4>;

    /////////////////////////////////////
    /////////////////////////////////////
    constexpr auto index_hash(const pixel& p) noexcept {
        return (p[0] * 3u + p[1] * 5u + p[2] * 7u + p[3] * 11u) % PIXEL_CACHE_SIZE;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto load_qoi(array_view<const byte> data) noexcept -> image::result<image> {
        const auto raw_header = data.subspan(SIZE_OF_HEADER);
        // const auto header     = *std::start_lifetime_as<qoi_header>(stdr::data(raw_header));
        const auto header = init_by<qoi_header>([raw_header](auto& header) noexcept {
            std::memcpy(&header, stdr::data(raw_header), stdr::size(raw_header));
        });

        const auto extent   = math::uextent3 { .width = byte_swap(header.width), .height = byte_swap(header.height) };
        const auto channels = header.channels;
        const auto format   = CHANNELS_TO_FORMAT.at(header.channels)[header.colorspace];

        auto pixel_cache = array<pixel, PIXEL_CACHE_SIZE> {};

        const auto chunks_sizes = stdr::size(data) - SIZE_OF_HEADER;
        // const auto chunks      = array_view { std::start_lifetime_as_array<u8>(stdr::data(data) + SIZE_OF_HEADER, CHUNKS_SIZE),
        //                                       CHUNKS_SIZE };
        const auto chunks      = array_view { std::bit_cast<const u8 *>(stdr::data(data) + SIZE_OF_HEADER),
                                              chunks_sizes };
        const auto output_size = extent.width * extent.height * channels;

        auto output = dynarray<byte> {};
        output.reserve(output_size);

        auto previous_pixel = pixel { 0, 0, 0, 255 };

        auto run = 0;

        const auto diff = 4 - channels;
        auto       it   = stdr::begin(chunks);

        const auto chunks_size = output_size - stdr::size(END_OF_FILE);
        for (auto _ : range<u32>(output_size, channels)) {
            const auto tag = *it;

            const auto position = as<usize>(std::distance(stdr::begin(chunks), it));

            if (run > 0) --run;
            else if (std::memcmp(&*it, std::data(END_OF_FILE), stdr::size(END_OF_FILE)) == 0) [[unlikely]] {
                it = stdr::cend(chunks);
            } else if (position < chunks_size) {
                ++it;
                if (static_cast<qoi_operation>(tag) == qoi_operation::RGB) {
                    previous_pixel[0] = *it;
                    previous_pixel[1] = *(it + 1);
                    previous_pixel[2] = *(it + 2);

                    it += 3;
                } else if (static_cast<qoi_operation>(tag) == qoi_operation::RGBA) {
                    previous_pixel[0] = *it;
                    previous_pixel[1] = *(it + 1);
                    previous_pixel[2] = *(it + 2);
                    previous_pixel[3] = *(it + 3);

                    it += 4;
                } else {
#define CHECK(op) (tag & 0b11000000) == static_cast<u8>(op)
                    if (CHECK(qoi_operation::INDEX)) {
                        const auto index = tag;

                        previous_pixel = pixel_cache[index];
                    } else if (CHECK(qoi_operation::DIFF)) {
                        const auto r_diff = as<u8>(((tag >> 4) & 0x03) - 2);
                        const auto g_diff = as<u8>(((tag >> 2) & 0x03) - 2);
                        const auto b_diff = as<u8>((tag & 0x03) - 2);

                        previous_pixel[0] += r_diff;
                        previous_pixel[1] += g_diff;
                        previous_pixel[2] += b_diff;

                    } else if (CHECK(qoi_operation::LUMA)) {
                        const auto g_diff = (tag & 0x3f) - 32;

                        const auto current_r = ((*it) >> 4) & 0x0f;
                        const auto current_b = (*it) & 0x0f;

                        previous_pixel[0] += as<u8>(g_diff - 8 + current_r);
                        previous_pixel[1] += as<u8>(g_diff);
                        previous_pixel[2] += as<u8>(g_diff - 8 + current_b);

                        ++it;
                    } else if (CHECK(qoi_operation::RUN)) {
                        run = (tag & 0x3f);
                    }
#undef CHECK
                }

                auto& cached = pixel_cache[index_hash(previous_pixel)];
                cached       = previous_pixel;
            }

            stdr::transform(stdr::begin(previous_pixel),
                            stdr::end(previous_pixel) - diff,
                            std::back_inserter(output),
                            monadic::as<byte>());
        }

        auto image_data = image::image_data_t {
            .extent            = extent,
            .channel_count     = channels,
            .bytes_per_channel = get_format_component_size(format),
            .layers            = 1u,
            .faces             = 1u,
            .mip_levels        = 1u,
            .format            = format,
            .data              = std::move(output)

        };

        return image { std::move(image_data) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto save_qoi(const image&, const stdfs::path&) noexcept -> image::result<void> {
        assert(false, "Not implemented yet !");
        return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    [[nodiscard]]
    auto save_qoi(const image&) noexcept -> image::result<dynarray<byte>> {
        assert(false, "Not implemented yet !");
        return {};
    }
} // namespace stormkit::image::details
