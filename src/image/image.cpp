// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/try_expected.hpp>

module stormkit.image;

import std;

import stormkit.core;

import :hdr;
import :jpg;
import :ktx;
import :png;
import :ppm;
import :qoi;
import :tga;

namespace stdr  = std::ranges;
namespace stdfs = std::filesystem;

namespace stormkit::image {
    namespace details {
        using namespace stormkit::literals;
        inline constexpr auto
          ktx_header = into<array>(as_bytes, { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A });

        inline constexpr auto png_header = into<array>(as_bytes, { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A });

        inline constexpr auto qoi_header = into<array>(as_bytes, { 0x71, 0x6f, 0x69, 0x66 });

        inline constexpr auto jpeg_header = into<array>(as_bytes, { 0xFF, 0xD8 });

        auto filename_to_codec(const stdfs::path& filename) noexcept -> image_codec {
            EXPECTS(stdfs::exists(filename));
            EXPECTS(filename.has_extension());
            EXPECTS(!stdfs::is_directory(filename));
            EXPECTS(stdfs::is_regular_file(filename));

            const auto ext = to_lower(filename.extension().string());

            if (ext == ".jpg" or ext == ".jpeg") return image_codec::jpeg;
            else if (ext == ".png")
                return image_codec::png;
            else if (ext == ".tga" or ext == ".targa")
                return image_codec::targa;
            else if (ext == ".ppm")
                return image_codec::ppm;
            else if (ext == ".hdr")
                return image_codec::hdr;
            else if (ext == ".ktx")
                return image_codec::ktx;
            else if (ext == ".qoi")
                return image_codec::qoi;

            return image_codec::unknown;
        }

        auto header_to_codec(array_view<const byte> data) noexcept -> image_codec {
            EXPECTS(stdr::size(data) >= 12);

            if (std::memcmp(stdr::data(data), stdr::data(ktx_header), stdr::size(ktx_header)) == 0) return image_codec::ktx;
            else if (std::memcmp(stdr::data(data), stdr::data(png_header), stdr::size(png_header)) == 0)
                return image_codec::png;
            else if (std::memcmp(stdr::data(data), stdr::data(jpeg_header), stdr::size(jpeg_header)) == 0)
                return image_codec::jpeg;
            else if (std::memcmp(stdr::data(data), stdr::data(qoi_header), stdr::size(qoi_header)) == 0)
                return image_codec::qoi;

            return image_codec::unknown;
        }

        auto map(array_view<const byte> bytes, u32 source_count, u32 destination_count) noexcept -> dynarray<byte> {
            EXPECTS(source_count <= 4u and source_count > 0u and destination_count <= 4u and destination_count > 0u);

            static constexpr auto byte_1_min = std::numeric_limits<u8>::min();
            static constexpr auto byte_1_max = std::numeric_limits<u8>::max();
            static constexpr auto byte_2_min = std::numeric_limits<u16>::min();
            static constexpr auto byte_2_max = std::numeric_limits<u16>::max();
            static constexpr auto byte_4_min = std::numeric_limits<u32>::min();
            static constexpr auto byte_4_max = std::numeric_limits<u32>::max();

            auto data = dynarray<byte> {};
            data.resize(stdr::size(bytes) * destination_count);

            if (source_count == 1u and destination_count == 2u) {
                const auto input_it  = std::bit_cast<const u8*>(stdr::data(data));
                auto       output_it = std::bit_cast<u16*>(stdr::data(data));

                for (auto i : range(stdr::size(bytes)))
                    output_it[i] = math::scale<u16>(input_it[i], byte_1_min, byte_1_max, byte_2_min, byte_2_max);
            } else if (source_count == 1u and destination_count == 4u) {
                const auto input_it  = std::bit_cast<const u8*>(stdr::data(data));
                auto       output_it = std::bit_cast<u32*>(stdr::data(data));

                for (auto i : range(stdr::size(bytes)))
                    output_it[i] = math::scale<u32>(input_it[i], byte_1_min, byte_1_max, byte_4_min, byte_4_max);
            } else if (source_count == 2u and destination_count == 1u) {
                const auto input_it  = std::bit_cast<const u16*>(stdr::data(data));
                auto       output_it = std::bit_cast<u8*>(stdr::data(data));

                for (auto i : range(stdr::size(bytes)))
                    output_it[i] = math::scale<u8>(input_it[i], byte_2_min, byte_2_max, byte_1_min, byte_1_max);
            } else if (source_count == 2u and destination_count == 4u) {
                const auto input_it  = std::bit_cast<const u16*>(stdr::data(data));
                auto       output_it = std::bit_cast<u32*>(stdr::data(data));

                for (auto i : range(stdr::size(bytes)))
                    output_it[i] = math::scale<u32>(input_it[i], byte_2_min, byte_2_max, byte_4_min, byte_4_max);
            } else if (source_count == 4u and destination_count == 1u) {
                const auto input_it  = std::bit_cast<const u32*>(stdr::data(data));
                auto       output_it = std::bit_cast<u8*>(stdr::data(data));

                for (auto i : range(stdr::size(bytes)))
                    output_it[i] = math::scale<u8>(input_it[i], byte_4_min, byte_4_max, byte_1_min, byte_1_max);
            } else if (source_count == 4u and destination_count == 2u) {
                const auto input_it  = std::bit_cast<const u32*>(stdr::data(data));
                auto       output_it = std::bit_cast<u16*>(stdr::data(data));

                for (auto i : range(stdr::size(bytes)))
                    output_it[i] = math::scale<u16>(input_it[i], byte_4_min, byte_4_max, byte_2_min, byte_2_max);
            } else
                data = { stdr::begin(bytes), stdr::end(bytes) };

            return data;
        }
    } // namespace details

    /////////////////////////////////////
    /////////////////////////////////////
    image::image() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    image::image(image_data_t&& data) noexcept : image {} {
        m_data = std::move(data);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    image::image(meta::in<math::uextent3> extent, image_format format) noexcept : image {} {
        create(extent, format);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    image::image(const stdfs::path& filepath, image_codec codec) noexcept : image {} {
        const auto _ = load_from_file(filepath, codec);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    image::image(array_view<const byte> data, image_codec codec) noexcept : image {} {
        const auto _ = load_from_memory(data, codec);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    image::image(const image& rhs) noexcept = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto image::operator=(const image& rhs) noexcept -> image& = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    image::image(image&&) noexcept = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto image::operator=(image&&) noexcept -> image& = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    image::~image() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto image::load_from_file(const stdfs::path& filepath_, image_codec codec) noexcept -> result<void> {
        const auto filepath = stdfs::canonical(filepath_);

        EXPECTS(codec != image_codec::unknown);
        EXPECTS(not std::empty(filepath));

        if (not stdfs::exists(filepath)) {
            return std::unexpected { status_code(image_status_code::file_not_found) };
            // return std::unexpected<Error> { std::in_place,
            //                                 Error::Reason::FILE_NOT_FOUND,
            //                                 std::format("Failed to open file {}\n    > Incorrect path", filepath.string()) };
        }

        TryTo(filedata, io::readfile(filepath));
        if (codec == image_codec::autodetect) codec = details::filename_to_codec(filepath);
        switch (codec) {
            case image_codec::jpeg: {
                TryTo(result, details::load_jpg(filedata));
                *this = std::move(result);
                return {};
            }
            case image_codec::png: {
                TryTo(result, details::load_png(filedata));
                *this = std::move(result);
                return {};
            }
            case image_codec::targa: {
                TryTo(result, details::load_tga(filedata));
                *this = std::move(result);
                return {};
            }
            case image_codec::ppm: {
                TryTo(result, details::load_ppm(filedata));
                *this = std::move(result);
                return {};
            }
            case image_codec::hdr: {
                TryTo(result, details::load_hdr(filedata));
                *this = std::move(result);
                return {};
            }
            case image_codec::ktx: {
                TryTo(result, details::load_ktx(filedata));
                *this = std::move(result);
                return {};
            }
            case image_codec::qoi: {
                TryTo(result, details::load_qoi(filedata));
                *this = std::move(result);
                return {};
            }
            default: break;
        }

        return std::unexpected { status_code(image_status_code::invalid_format) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto image::load_from_memory(array_view<const byte> data, image_codec codec) noexcept -> result<void> {
        EXPECTS(codec != image_codec::unknown);
        EXPECTS(not std::empty(data));

        // if (codec == image_codec::autodetect) codec = details::header_to_codec(data);
        // switch (codec) {
        //     CASE_DO (JPEG, load_jpg, "JPEG")
        //     CASE_DO (PNG, load_png, "PNG")
        //     CASE_DO (TARGA, load_tga, "TARGA")
        //     CASE_DO (PPM, load_ppm, "PPM")
        //     CASE_DO (HDR, load_hdr, "HDR")
        //     CASE_DO (KTX, load_ktx, "KTX")
        //     CASE_DO (QOI, load_qoi, "QOI")
        //     default: break;
        // }

        // return std::unexpected<Error> { std::in_place,
        //                                 Error::Reason::INVALID_FORMAT,
        //                                 "Failed to load image\n    > Invalid format" };
        if (codec == image_codec::autodetect) codec = details::header_to_codec(data);
        switch (codec) {
            case image_codec::jpeg: {
                TryTo(result, details::load_jpg(data));
                *this = std::move(result);
                return {};
            }
            case image_codec::png: {
                TryTo(result, details::load_png(data));
                *this = std::move(result);
                return {};
            }
            case image_codec::targa: {
                TryTo(result, details::load_tga(data));
                *this = std::move(result);
                return {};
            }
            case image_codec::ppm: {
                TryTo(result, details::load_ppm(data));
                *this = std::move(result);
                return {};
            }
            case image_codec::hdr: {
                TryTo(result, details::load_hdr(data));
                *this = std::move(result);
                return {};
            }
            case image_codec::ktx: {
                TryTo(result, details::load_ktx(data));
                *this = std::move(result);
                return {};
            }
            case image_codec::qoi: {
                TryTo(result, details::load_qoi(data));
                *this = std::move(result);
                return {};
            }
            default: break;
        }

        return std::unexpected { status_code(image_status_code::invalid_format) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto image::save_to_file(const stdfs::path& filepath_, image_codec codec, image_codec_format format) const noexcept
      -> result<void> {
        auto filepath = stdfs::canonical(filepath_.parent_path()) / filepath_.filename();

        EXPECTS(codec != image_codec::unknown);
        EXPECTS(codec != image_codec::autodetect);
        EXPECTS(not std::empty(filepath));
        EXPECTS(not std::empty(m_data.data));
        EXPECTS(stdfs::exists(filepath.root_directory()));

        auto out = result<void> { std::unexpect, status_code(image_status_code::invalid_format) };

        switch (codec) {
            case image_codec::jpeg: out = details::save_jpg(*this, filepath); break;
            case image_codec::png: out = details::save_png(*this, filepath); break;
            case image_codec::targa: out = details::save_tga(*this, filepath); break;
            case image_codec::ppm: out = details::save_ppm(*this, format, filepath); break;
            case image_codec::hdr: out = details::save_hdr(*this, filepath); break;
            case image_codec::ktx: out = details::save_ktx(*this, filepath); break;
            case image_codec::qoi: out = details::save_qoi(*this, filepath); break;
            default: break;
        }

        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto image::save_to_memory(image_codec codec, image_codec_format format) const noexcept -> result<dynarray<byte>> {
        EXPECTS(codec != image_codec::unknown);
        EXPECTS(codec != image_codec::autodetect);
        EXPECTS(not std::empty(m_data.data));

        auto out = result<dynarray<byte>> { std::unexpect, status_code(image_status_code::invalid_format) };

        switch (codec) {
            case image_codec::jpeg: out = details::save_jpg(*this); break;
            case image_codec::png: out = details::save_png(*this); break;
            case image_codec::targa: out = details::save_tga(*this); break;
            case image_codec::ppm: out = details::save_ppm(*this, format); break;
            case image_codec::hdr: out = details::save_hdr(*this); break;
            case image_codec::ktx: out = details::save_ktx(*this); break;
            case image_codec::qoi: out = details::save_qoi(*this); break;
            default: break;
        }

        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto image::create(meta::in<math::uextent3> extent, image_format format) noexcept -> void {
        EXPECTS(extent.width > 0u and extent.height > 0u and extent.depth > 0u and format != image_format::undefined);
        m_data.data.clear();

        m_data.extent            = extent;
        m_data.channel_count     = get_format_channel_count(format);
        m_data.bytes_per_channel = get_format_component_size(format);
        m_data.layers            = 1u;
        m_data.faces             = 1u;
        m_data.mip_levels        = 1u;
        m_data.format            = format;

        m_data.data.resize(m_data.extent.width
                           * m_data.extent.height
                           * m_data.extent.depth
                           * m_data.layers
                           * m_data.faces
                           * m_data.mip_levels
                           * m_data.channel_count
                           * m_data.bytes_per_channel);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto image::convert_to(image_format format) const noexcept -> image {
        EXPECTS(!std::empty(m_data.data));
        EXPECTS(format != image_format::undefined);

        if (m_data.format == format) return *this;

        auto image_data = image_data_t {
            .extent            = m_data.extent,
            .channel_count     = get_format_channel_count(format),
            .bytes_per_channel = get_format_component_size(format),
            .layers            = m_data.layers,
            .faces             = m_data.faces,
            .mip_levels        = m_data.mip_levels,
            .format            = format
        };

        /*const auto channel_delta =
            static_cast<u8>(std::max(0,
                                              static_cast<i8>(image_data.channel_count) -
                                                  static_cast<i8>(m_data.channel_count)));*/
        const auto pixel_count = m_data.extent.width * m_data.extent.height * m_data.extent.depth;

        image_data.data.resize(pixel_count * image_data.channel_count * image_data.bytes_per_channel, byte { 255u });

        auto new_image = image { std::move(image_data) };

        for (auto [layer, face, level, i] : multi_range(new_image.layers(), new_image.faces(), new_image.layers(), pixel_count)) {
            const auto new_from_image = details::map(pixel(as<usize>(i), layer, face, level),
                                                     m_data.bytes_per_channel,
                                                     new_image.bytes_per_channel());
            auto       new_to_image   = new_image.pixel(as<usize>(i), layer, face, level);

            stdr::copy_n(stdr::begin(new_from_image),
                         std::min(m_data.channel_count, new_image.channel_count()),
                         stdr::begin(new_to_image));
        }

        return new_image;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto image::scale(meta::in<math::uextent3>) const noexcept -> image {
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto image::flip_x() const noexcept -> image {
        auto image_data = image_data_t {
            .extent            = m_data.extent,
            .channel_count     = m_data.channel_count,
            .bytes_per_channel = m_data.bytes_per_channel,
            .layers            = m_data.layers,
            .faces             = m_data.faces,
            .mip_levels        = m_data.mip_levels,
            .format            = m_data.format
        };

        image_data.data.resize(stdr::size(m_data.data));

        auto new_image = image { std::move(image_data) };

        for (auto [layer, face, mip, x, y, z] :
             multi_range(m_data.layers,
                         m_data.faces,
                         m_data.mip_levels,
                         m_data.extent.width,
                         m_data.extent.height,
                         m_data.extent.depth)) {
            const auto inv_x  = m_data.extent.width - x - 1u;
            auto       output = new_image.pixel({ inv_x, y, z }, layer, face, mip);
            // const auto data  = pixel({ x, y, z }, layer, face, mip);

            stdr::copy(pixel({ x, y, z }, layer, face, mip), stdr::begin(output));
        }

        return new_image;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto image::flip_y() const noexcept -> image {
        auto image_data = image_data_t {
            .extent            = m_data.extent,
            .channel_count     = m_data.channel_count,
            .bytes_per_channel = m_data.bytes_per_channel,
            .layers            = m_data.layers,
            .faces             = m_data.faces,
            .mip_levels        = m_data.mip_levels,
            .format            = m_data.format
        };

        image_data.data.resize(stdr::size(m_data.data));

        auto new_image = image { std::move(image_data) };

        for (auto [layer, face, mip, x, y, z] :
             multi_range(m_data.layers,
                         m_data.faces,
                         m_data.mip_levels,
                         m_data.extent.width,
                         m_data.extent.height,
                         m_data.extent.depth)) {
            const auto inv_y  = m_data.extent.height - 1u - y;
            auto       output = new_image.pixel({ x, inv_y, z }, layer, face, mip);

            stdr::copy(pixel({ x, y, z }, layer, face, mip), stdr::begin(output));
        }

        return new_image;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto image::flip_z() const noexcept -> image {
        auto image_data = image_data_t {
            .extent            = m_data.extent,
            .channel_count     = m_data.channel_count,
            .bytes_per_channel = m_data.bytes_per_channel,
            .layers            = m_data.layers,
            .faces             = m_data.faces,
            .mip_levels        = m_data.mip_levels,
            .format            = m_data.format
        };
        image_data.data.resize(stdr::size(m_data.data));

        auto new_image = image { std::move(image_data) };

        for (auto [layer, face, mip, x, y, z] :
             multi_range(m_data.layers,
                         m_data.faces,
                         m_data.mip_levels,
                         m_data.extent.width,
                         m_data.extent.height,
                         m_data.extent.depth)) {
            const auto inv_z  = m_data.extent.depth - 1u - z;
            auto       output = new_image.pixel({ x, z, inv_z }, layer, face, mip);

            stdr::copy(pixel({ x, y, z }, layer, face, mip), stdr::begin(output));
        }

        return new_image;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto image::rotate_90() const noexcept -> image {
        auto image_data = image_data_t {
            .extent            = m_data.extent,
            .channel_count     = m_data.channel_count,
            .bytes_per_channel = m_data.bytes_per_channel,
            .layers            = m_data.layers,
            .faces             = m_data.faces,
            .mip_levels        = m_data.mip_levels,
            .format            = m_data.format
        };

        image_data.data.resize(stdr::size(m_data.data));

        // TODO implement

        return image { std::move(image_data) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto image::rotate_180() const noexcept -> image {
        auto image_data = image_data_t {
            .extent            = m_data.extent,
            .channel_count     = m_data.channel_count,
            .bytes_per_channel = m_data.bytes_per_channel,
            .layers            = m_data.layers,
            .faces             = m_data.faces,
            .mip_levels        = m_data.mip_levels,
            .format            = m_data.format
        };

        image_data.data.resize(stdr::size(m_data.data));

        return image { std::move(image_data) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto image::rotate_270() const noexcept -> image {
        auto image_data = image_data_t {
            .extent            = m_data.extent,
            .channel_count     = m_data.channel_count,
            .bytes_per_channel = m_data.bytes_per_channel,
            .layers            = m_data.layers,
            .faces             = m_data.faces,
            .mip_levels        = m_data.mip_levels,
            .format            = m_data.format
        };

        image_data.data.resize(stdr::size(m_data.data));

        return image { std::move(image_data) };
    }

} // namespace stormkit::image
