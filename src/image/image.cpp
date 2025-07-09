// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>

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

namespace stormkit::image {
    namespace details {
        using namespace stormkit::literals;
        inline constexpr auto KTX_HEADER
          = into_bytes({ 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A });

        inline constexpr auto PNG_HEADER = into_bytes({ 0x89,
                                                        0x50,
                                                        0x4E,
                                                        0x47,
                                                        0x0D,
                                                        0x0A,
                                                        0x1A,
                                                        0x0A });

        inline constexpr auto QOI_HEADER = into_bytes({ 0x71, 0x6f, 0x69, 0x66 });

        inline constexpr auto JPEG_HEADER = into_bytes({ 0xFF, 0xD8 });

        auto filename_to_codec(const std::filesystem::path& filename) noexcept -> Image::Codec {
            EXPECTS(std::filesystem::exists(filename));
            EXPECTS(filename.has_extension());
            EXPECTS(!std::filesystem::is_directory(filename));
            EXPECTS(std::filesystem::is_regular_file(filename));

            const auto ext = to_lower(filename.extension().string());

            if (ext == ".jpg" or ext == ".jpeg") return Image::Codec::JPEG;
            else if (ext == ".png")
                return Image::Codec::PNG;
            else if (ext == ".tga" or ext == ".targa")
                return Image::Codec::TARGA;
            else if (ext == ".ppm")
                return Image::Codec::PPM;
            else if (ext == ".hdr")
                return Image::Codec::HDR;
            else if (ext == ".ktx")
                return Image::Codec::KTX;
            else if (ext == ".qoi")
                return Image::Codec::QOI;

            return Image::Codec::UNKNOWN;
        }

        auto header_to_codec(std::span<const Byte> data) noexcept -> Image::Codec {
            EXPECTS(std::size(data) >= 12);

            if (std::memcmp(std::data(data), std::data(KTX_HEADER), std::size(KTX_HEADER)) == 0)
                return Image::Codec::KTX;
            else if (std::memcmp(std::data(data), std::data(PNG_HEADER), std::size(PNG_HEADER))
                     == 0)
                return Image::Codec::PNG;
            else if (std::memcmp(std::data(data), std::data(JPEG_HEADER), std::size(JPEG_HEADER))
                     == 0)
                return Image::Codec::JPEG;
            else if (std::memcmp(std::data(data), std::data(QOI_HEADER), std::size(QOI_HEADER))
                     == 0)
                return Image::Codec::QOI;

            return Image::Codec::UNKNOWN;
        }

        auto map(std::span<const Byte> bytes, u32 source_count, u32 destination_count) noexcept
          -> std::vector<Byte> {
            EXPECTS(source_count <= 4u
                    and source_count > 0u
                    and destination_count <= 4u
                    and destination_count > 0u);

            static constexpr auto BYTE_1_MIN = std::numeric_limits<u8>::min();
            static constexpr auto BYTE_1_MAX = std::numeric_limits<u8>::max();
            static constexpr auto BYTE_2_MIN = std::numeric_limits<u16>::min();
            static constexpr auto BYTE_2_MAX = std::numeric_limits<u16>::max();
            static constexpr auto BYTE_4_MIN = std::numeric_limits<u32>::min();
            static constexpr auto BYTE_4_MAX = std::numeric_limits<u32>::max();

            auto data = std::vector<Byte> {};
            data.resize(std::size(bytes) * destination_count);

            if (source_count == 1u and destination_count == 2u) {
                const auto input_it  = std::bit_cast<const u8*>(std::data(data));
                auto       output_it = std::bit_cast<u16*>(std::data(data));

                for (auto i : range(std::size(bytes)))
                    output_it[i] = math::scale<u16>(input_it[i],
                                                    BYTE_1_MIN,
                                                    BYTE_1_MAX,
                                                    BYTE_2_MIN,
                                                    BYTE_2_MAX);
            } else if (source_count == 1u and destination_count == 4u) {
                const auto input_it  = std::bit_cast<const u8*>(std::data(data));
                auto       output_it = std::bit_cast<u32*>(std::data(data));

                for (auto i : range(std::size(bytes)))
                    output_it[i] = math::scale<u32>(input_it[i],
                                                    BYTE_1_MIN,
                                                    BYTE_1_MAX,
                                                    BYTE_4_MIN,
                                                    BYTE_4_MAX);
            } else if (source_count == 2u and destination_count == 1u) {
                const auto input_it  = std::bit_cast<const u16*>(std::data(data));
                auto       output_it = std::bit_cast<u8*>(std::data(data));

                for (auto i : range(std::size(bytes)))
                    output_it[i] = math::scale<u8>(input_it[i],
                                                   BYTE_2_MIN,
                                                   BYTE_2_MAX,
                                                   BYTE_1_MIN,
                                                   BYTE_1_MAX);
            } else if (source_count == 2u and destination_count == 4u) {
                const auto input_it  = std::bit_cast<const u16*>(std::data(data));
                auto       output_it = std::bit_cast<u32*>(std::data(data));

                for (auto i : range(std::size(bytes)))
                    output_it[i] = math::scale<u32>(input_it[i],
                                                    BYTE_2_MIN,
                                                    BYTE_2_MAX,
                                                    BYTE_4_MIN,
                                                    BYTE_4_MAX);
            } else if (source_count == 4u and destination_count == 1u) {
                const auto input_it  = std::bit_cast<const u32*>(std::data(data));
                auto       output_it = std::bit_cast<u8*>(std::data(data));

                for (auto i : range(std::size(bytes)))
                    output_it[i] = math::scale<u8>(input_it[i],
                                                   BYTE_4_MIN,
                                                   BYTE_4_MAX,
                                                   BYTE_1_MIN,
                                                   BYTE_1_MAX);
            } else if (source_count == 4u and destination_count == 2u) {
                const auto input_it  = std::bit_cast<const u32*>(std::data(data));
                auto       output_it = std::bit_cast<u16*>(std::data(data));

                for (auto i : range(std::size(bytes)))
                    output_it[i] = math::scale<u16>(input_it[i],
                                                    BYTE_4_MIN,
                                                    BYTE_4_MAX,
                                                    BYTE_2_MIN,
                                                    BYTE_2_MAX);
            } else
                data = { std::ranges::begin(bytes), std::ranges::end(bytes) };

            return data;
        }
    } // namespace details

    /////////////////////////////////////
    /////////////////////////////////////
    Image::Image() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    Image::Image(ImageData&& data) noexcept : Image {} {
        m_data = std::move(data);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    Image::Image(const math::Extent3<u32>& extent, Format format) noexcept : Image {} {
        create(extent, format);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    Image::Image(const std::filesystem::path& filepath, Image::Codec codec) noexcept : Image {} {
        const auto _ = load_from_file(filepath, codec);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    Image::Image(std::span<const Byte> data, Image::Codec codec) noexcept : Image {} {
        const auto _ = load_from_memory(data, codec);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    Image::Image(const Image& rhs) noexcept = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto Image::operator=(const Image& rhs) noexcept -> Image& = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    Image::Image(Image&&) noexcept = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto Image::operator=(Image&&) noexcept -> Image& = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    Image::~Image() noexcept = default;

#define CASE_DO(_E, _Func)                                                                  \
    case Image::Codec::_E: {                                                                \
        auto result = details::_Func(data);                                                 \
        if (!result) {                                                                      \
            return std::unexpected<Error> { std::in_place,                                  \
                                            result.error().reason,                          \
                                            std::format("Failed to load file {}\n    > {}", \
                                                        filepath.string(),                  \
                                                        result.error().str_error) };        \
        }                                                                                   \
        *this = std::move(*result);                                                         \
        return {};                                                                          \
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Image::load_from_file(std::filesystem::path filepath, Image::Codec codec) noexcept
      -> std::expected<void, Error> {
        filepath = std::filesystem::canonical(filepath);

        EXPECTS(codec != Image::Codec::UNKNOWN);
        EXPECTS(!std::empty(filepath));

        if (!std::filesystem::exists(filepath)) {
            return std::unexpected<Error> {
                std::in_place,
                Error::Reason::FILE_NOT_FOUND,
                std::format("Failed to open file {}\n    > Incorrect path", filepath.string())
            };
        }

        return io::readfile(filepath)
          .transform_error([](auto&& error) static noexcept -> Error {
              return { Error::Reason::UNKNOWN, error.message() };
          })
          .and_then([this,
                     &filepath,
                     codec](auto&& data) mutable noexcept -> std::expected<void, Error> {
              if (codec == Image::Codec::AUTODETECT) codec = details::filename_to_codec(filepath);
              switch (codec) {
                  CASE_DO (JPEG, load_jpg)
                      ;
                  CASE_DO (PNG, load_png)
                      ;
                  CASE_DO (TARGA, load_tga)
                      ;
                  CASE_DO (PPM, load_ppm)
                      ;
                  CASE_DO (HDR, load_hdr)
                      ;
                  CASE_DO (KTX, load_ktx)
                      ;
                  CASE_DO (QOI, load_qoi)
                      ;
                  default: break;
              }

              return std::unexpected<Error> {
                  std::in_place,
                  Error::Reason::INVALID_FORMAT,
                  std::format("Failed to open image from {}\n    > Invalid format",
                              filepath.string())
              };
          });
    }

#undef CASE_DO
#define CASE_DO(_E, _Func, _Name)                                                 \
    case Image::Codec::_E: {                                                      \
        auto result = details::_Func(data);                                       \
        if (!result) {                                                            \
            return std::unexpected<Error> {                                       \
                std::in_place,                                                    \
                result.error().reason,                                            \
                std::format("Failed to load " _Name " image from data\n    > {}", \
                            result.error().str_error)                             \
            };                                                                    \
        }                                                                         \
        *this = std::move(*result);                                               \
        return {};                                                                \
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Image::load_from_memory(std::span<const Byte> data, Image::Codec codec) noexcept
      -> std::expected<void, Error> {
        EXPECTS(codec != Image::Codec::UNKNOWN);
        EXPECTS(!std::empty(data));

        if (codec == Image::Codec::AUTODETECT) codec = details::header_to_codec(data);
        switch (codec) {
            CASE_DO (JPEG, load_jpg, "JPEG")
                ;
            CASE_DO (PNG, load_png, "PNG")
                ;
            CASE_DO (TARGA, load_tga, "TARGA")
                ;
            CASE_DO (PPM, load_ppm, "PPM")
                ;
            CASE_DO (HDR, load_hdr, "HDR")
                ;
            CASE_DO (KTX, load_ktx, "KTX")
                ;
            CASE_DO (QOI, load_qoi, "QOI")
                ;
            default: break;
        }

        return std::unexpected<Error> { std::in_place,
                                        Error::Reason::INVALID_FORMAT,
                                        "Failed to load image\n    > Invalid format" };
    }

#undef CASE_DO
#define CASE_DO(_E, _Func)                                                                     \
    case Image::Codec::_E: {                                                                   \
        auto result = details::_Func(*this, filepath);                                         \
        if (!result) {                                                                         \
            return std::unexpected<Error> { std::in_place,                                     \
                                            result.error().reason,                             \
                                            std::format("Failed to save to file {}\n    > {}", \
                                                        filepath.string(),                     \
                                                        result.error().str_error) };           \
        }                                                                                      \
        return {};                                                                             \
    }
#define CASE_ARGS_DO(_E, _Func)                                                                \
    case Image::Codec::_E: {                                                                   \
        auto result = details::_Func(*this, std::move(args), filepath);                        \
        if (!result) {                                                                         \
            return std::unexpected<Error> { std::in_place,                                     \
                                            result.error().reason,                             \
                                            std::format("Failed to save to file {}\n    > {}", \
                                                        filepath.string(),                     \
                                                        result.error().str_error) };           \
        }                                                                                      \
        return {};                                                                             \
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Image::save_to_file(std::filesystem::path filepath,
                             Codec                 codec,
                             CodecArgs args) const noexcept -> std::expected<void, Error> {
        filepath = std::filesystem::canonical(filepath.parent_path()) / filepath.filename();

        EXPECTS(codec != Image::Codec::UNKNOWN);
        EXPECTS(codec != Image::Codec::AUTODETECT);
        EXPECTS(!std::empty(filepath));
        EXPECTS(!std::empty(m_data.data));
        EXPECTS(std::filesystem::exists(filepath.root_directory()));

        switch (codec) {
            CASE_DO (JPEG, save_jpg)
                ;
            CASE_DO (PNG, save_png)
                ;
            CASE_DO (TARGA, save_tga)
                ;
            CASE_ARGS_DO (PPM, save_ppm)
                ;
            CASE_DO (HDR, save_hdr)
                ;
            CASE_DO (KTX, save_ktx)
                ;
            CASE_DO (QOI, save_qoi)
                ;
            default: break;
        }

        return std::unexpected<Error> {
            std::in_place,
            Error::Reason::INVALID_FORMAT,
            std::format("Failed to save image to {}\n    > Invalid format", filepath.string())
        };
    }

#undef CASE_DO
#undef CASE_ARGS_DO
#define CASE_DO(_E, _Func, _Name)                                                             \
    case Image::Codec::_E: {                                                                  \
        auto result = details::_Func(*this);                                                  \
        if (!result) {                                                                        \
            return std::unexpected<Error> {                                                   \
                std::in_place,                                                                \
                result.error().reason,                                                        \
                std::format("Failed to load " _Name " image from data\n    > {}",             \
                            result.error().str_error)                                         \
            };                                                                                \
        }                                                                                     \
        return std::expected<std::vector<Byte>, Error> { std::in_place, std::move(*result) }; \
    }
#define CASE_ARGS_DO(_E, _Func, _Name)                                                        \
    case Image::Codec::_E: {                                                                  \
        auto result = details::_Func(*this, std::move(args));                                 \
        if (!result) {                                                                        \
            return std::unexpected<Error> {                                                   \
                std::in_place,                                                                \
                result.error().reason,                                                        \
                std::format("Failed to load " _Name " image from data\n    > {}",             \
                            result.error().str_error)                                         \
            };                                                                                \
        }                                                                                     \
        return std::expected<std::vector<Byte>, Error> { std::in_place, std::move(*result) }; \
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Image::save_to_memory(Codec codec, CodecArgs args) const noexcept
      -> std::expected<std::vector<Byte>, Error> {
        EXPECTS(codec != Image::Codec::UNKNOWN);
        EXPECTS(codec != Image::Codec::AUTODETECT);
        EXPECTS(!std::empty(m_data.data));

        auto output = std::vector<Byte> {};

        switch (codec) {
            CASE_DO (JPEG, save_jpg, "JPEG")
                ;
            CASE_DO (PNG, save_png, "PNG")
                ;
            CASE_DO (TARGA, save_tga, "TARGA")
                ;
            CASE_ARGS_DO (PPM, save_ppm, "PPM")
                ;
            CASE_DO (HDR, save_hdr, "HDR")
                ;
            CASE_DO (KTX, save_ktx, "KTX")
                ;
            CASE_DO (QOI, save_qoi, "QOI")
                ;
            default: break;
        }

        return std::unexpected<Error> { std::in_place,
                                        Error::Reason::INVALID_FORMAT,
                                        "Failed to save image\n    > Invalid format" };
    }

#undef CASE_ARGS_DO
#undef CASE_DO

    /////////////////////////////////////
    /////////////////////////////////////
    auto Image::create(math::Extent3<u32> extent, Format format) noexcept -> void {
        EXPECTS(extent.width > 0u
                and extent.height > 0u
                and extent.depth > 0u
                and format != Format::UNDEFINED);
        m_data.data.clear();

        m_data.extent            = extent;
        m_data.channel_count     = get_format_channel_count(format);
        m_data.bytes_per_channel = getSizeof(format);
        m_data.layers            = 1u;
        m_data.faces             = 1u;
        m_data.mip_levels        = 1u;
        m_data.format            = format;

        m_data.data
          .resize(m_data.extent.width
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
    auto Image::convert_to(Format format) const noexcept -> Image {
        EXPECTS(!std::empty(m_data.data));
        EXPECTS(format != Format::UNDEFINED);

        if (m_data.format == format) return *this;

        auto image_data = ImageData {
            .extent            = m_data.extent,
            .channel_count     = get_format_channel_count(format),
            .bytes_per_channel = getSizeof(format),
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

        image_data.data
          .resize(pixel_count * image_data.channel_count * image_data.bytes_per_channel,
                  Byte { 255u });

        auto image = Image { std::move(image_data) };

        for (auto [layer, face, level, i] :
             multi_range(image.layers(), image.faces(), image.layers(), pixel_count)) {
            const auto from_image = details::map(pixel(as<usize>(i), layer, face, level),
                                                 m_data.bytes_per_channel,
                                                 image.bytesPerChannel());
            auto       to_image   = image.pixel(as<usize>(i), layer, face, level);

            std::ranges::copy_n(std::ranges::begin(from_image),
                                std::min(m_data.channel_count, image.channelCount()),
                                std::ranges::begin(to_image));
        }

        return image;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Image::scale(const math::Extent3<u32>&) const noexcept -> Image {
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Image::flip_x() const noexcept -> Image {
        auto image_data = ImageData {
            .extent            = m_data.extent,
            .channel_count     = m_data.channel_count,
            .bytes_per_channel = m_data.bytes_per_channel,
            .layers            = m_data.layers,
            .faces             = m_data.faces,
            .mip_levels        = m_data.mip_levels,
            .format            = m_data.format
        };

        image_data.data.resize(std::size(m_data.data));

        auto image = Image { std::move(image_data) };

        for (auto [layer, face, mip, x, y, z] :
             multi_range(m_data.layers,
                         m_data.faces,
                         m_data.mip_levels,
                         m_data.extent.width,
                         m_data.extent.height,
                         m_data.extent.depth)) {
            const auto inv_x  = m_data.extent.width - x - 1u;
            auto       output = image.pixel({ inv_x, y, z }, layer, face, mip);
            // const auto data  = pixel({ x, y, z }, layer, face, mip);

            std::ranges::copy(pixel({ x, y, z }, layer, face, mip), std::ranges::begin(output));
        }

        return image;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Image::flip_y() const noexcept -> Image {
        auto image_data = ImageData {
            .extent            = m_data.extent,
            .channel_count     = m_data.channel_count,
            .bytes_per_channel = m_data.bytes_per_channel,
            .layers            = m_data.layers,
            .faces             = m_data.faces,
            .mip_levels        = m_data.mip_levels,
            .format            = m_data.format
        };

        image_data.data.resize(std::size(m_data.data));

        auto image = Image { std::move(image_data) };

        for (auto [layer, face, mip, x, y, z] :
             multi_range(m_data.layers,
                         m_data.faces,
                         m_data.mip_levels,
                         m_data.extent.width,
                         m_data.extent.height,
                         m_data.extent.depth)) {
            const auto inv_y  = m_data.extent.height - 1u - y;
            auto       output = image.pixel({ x, inv_y, z }, layer, face, mip);

            std::ranges::copy(pixel({ x, y, z }, layer, face, mip), std::ranges::begin(output));
        }

        return image;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Image::flip_z() const noexcept -> Image {
        auto image_data = ImageData {
            .extent            = m_data.extent,
            .channel_count     = m_data.channel_count,
            .bytes_per_channel = m_data.bytes_per_channel,
            .layers            = m_data.layers,
            .faces             = m_data.faces,
            .mip_levels        = m_data.mip_levels,
            .format            = m_data.format
        };
        image_data.data.resize(std::size(m_data.data));

        auto image = Image { std::move(image_data) };

        for (auto [layer, face, mip, x, y, z] :
             multi_range(m_data.layers,
                         m_data.faces,
                         m_data.mip_levels,
                         m_data.extent.width,
                         m_data.extent.height,
                         m_data.extent.depth)) {
            const auto inv_z  = m_data.extent.depth - 1u - z;
            auto       output = image.pixel({ x, z, inv_z }, layer, face, mip);

            std::ranges::copy(pixel({ x, y, z }, layer, face, mip), std::ranges::begin(output));
        }

        return image;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Image::rotate_90() const noexcept -> Image {
        auto image_data = ImageData {
            .extent            = m_data.extent,
            .channel_count     = m_data.channel_count,
            .bytes_per_channel = m_data.bytes_per_channel,
            .layers            = m_data.layers,
            .faces             = m_data.faces,
            .mip_levels        = m_data.mip_levels,
            .format            = m_data.format
        };

        image_data.data.resize(std::size(m_data.data));

        // TODO implement

        return Image { std::move(image_data) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Image::rotate_180() const noexcept -> Image {
        auto image_data = ImageData {
            .extent            = m_data.extent,
            .channel_count     = m_data.channel_count,
            .bytes_per_channel = m_data.bytes_per_channel,
            .layers            = m_data.layers,
            .faces             = m_data.faces,
            .mip_levels        = m_data.mip_levels,
            .format            = m_data.format
        };

        image_data.data.resize(std::size(m_data.data));

        return Image { std::move(image_data) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Image::rotate_270() const noexcept -> Image {
        auto image_data = ImageData {
            .extent            = m_data.extent,
            .channel_count     = m_data.channel_count,
            .bytes_per_channel = m_data.bytes_per_channel,
            .layers            = m_data.layers,
            .faces             = m_data.faces,
            .mip_levels        = m_data.mip_levels,
            .format            = m_data.format
        };

        image_data.data.resize(std::size(m_data.data));

        return Image { std::move(image_data) };
    }

} // namespace stormkit::image
