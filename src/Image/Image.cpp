// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module stormkit.Image;

import std;

import stormkit.core;

import :HDRImage;
import :JPEGImage;
import :KTXImage;
import :PNGImage;
import :PPMImage;
import :QOIImage;
import :TARGAImage;

namespace stormkit::image {
    namespace details {
        using namespace stormkit::literals;
        inline constexpr auto KTX_HEADER
            = into_bytes(0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A);

        inline constexpr auto PNG_HEADER
            = into_bytes(0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A);

        inline constexpr auto QOI_HEADER = into_bytes(0x71, 0x6f, 0x69, 0x66);

        inline constexpr auto JPEG_HEADER = into_bytes(0xFF, 0xD8);

        auto filenameToCodec(const std::filesystem::path& filename) noexcept -> Image::Codec {
            expects(std::filesystem::exists(filename));
            expects(filename.has_extension());
            expects(!std::filesystem::is_directory(filename));
            expects(std::filesystem::is_regular_file(filename));

            const auto ext = filename.extension().string();

            if (to_lower(ext) == ".jpg" or to_lower(ext) == ".jpeg") return Image::Codec::JPEG;
            else if (to_lower(ext) == ".png")
                return Image::Codec::PNG;
            else if (to_lower(ext) == ".tga" or to_lower(ext) == ".targa")
                return Image::Codec::TARGA;
            else if (to_lower(ext) == ".ppm")
                return Image::Codec::PPM;
            else if (to_lower(ext) == ".hdr")
                return Image::Codec::HDR;
            else if (to_lower(ext) == ".ktx")
                return Image::Codec::KTX;
            else if (to_lower(ext) == ".qoi")
                return Image::Codec::QOI;

            return Image::Codec::Unknown;
        }

        auto headerToCodec(std::span<const Byte> data) noexcept -> Image::Codec {
            expects(std::size(data) >= 12);

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

            return Image::Codec::Unknown;
        }

        auto map(std::span<const Byte> bytes,
                 UInt32                source_count,
                 UInt32                destination_count) noexcept -> std::vector<Byte> {
            expects(source_count <= 4u
                    and source_count > 0u
                    and destination_count <= 4u
                    and destination_count > 0u);

            static constexpr auto BYTE_1_MIN = std::numeric_limits<UInt8>::min();
            static constexpr auto BYTE_1_MAX = std::numeric_limits<UInt8>::max();
            static constexpr auto BYTE_2_MIN = std::numeric_limits<UInt16>::min();
            static constexpr auto BYTE_2_MAX = std::numeric_limits<UInt16>::max();
            static constexpr auto BYTE_4_MIN = std::numeric_limits<UInt32>::min();
            static constexpr auto BYTE_4_MAX = std::numeric_limits<UInt32>::max();

            auto data = std::vector<Byte> {};
            data.resize(std::size(bytes) * destination_count);

            if (source_count == 1u and destination_count == 2u) {
                const auto input_it  = std::bit_cast<const UInt8*>(std::data(data));
                auto       output_it = std::bit_cast<UInt16*>(std::data(data));

                for (auto i : range(std::size(bytes)))
                    output_it[i] = math::scale<UInt16>(input_it[i],
                                                       BYTE_1_MIN,
                                                       BYTE_1_MAX,
                                                       BYTE_2_MIN,
                                                       BYTE_2_MAX);
            } else if (source_count == 1u and destination_count == 4u) {
                const auto input_it  = std::bit_cast<const UInt8*>(std::data(data));
                auto       output_it = std::bit_cast<UInt32*>(std::data(data));

                for (auto i : range(std::size(bytes)))
                    output_it[i] = math::scale<UInt32>(input_it[i],
                                                       BYTE_1_MIN,
                                                       BYTE_1_MAX,
                                                       BYTE_4_MIN,
                                                       BYTE_4_MAX);
            } else if (source_count == 2u and destination_count == 1u) {
                const auto input_it  = std::bit_cast<const UInt16*>(std::data(data));
                auto       output_it = std::bit_cast<UInt8*>(std::data(data));

                for (auto i : range(std::size(bytes)))
                    output_it[i] = math::scale<UInt8>(input_it[i],
                                                      BYTE_2_MIN,
                                                      BYTE_2_MAX,
                                                      BYTE_1_MIN,
                                                      BYTE_1_MAX);
            } else if (source_count == 2u and destination_count == 4u) {
                const auto input_it  = std::bit_cast<const UInt16*>(std::data(data));
                auto       output_it = std::bit_cast<UInt32*>(std::data(data));

                for (auto i : range(std::size(bytes)))
                    output_it[i] = math::scale<UInt32>(input_it[i],
                                                       BYTE_2_MIN,
                                                       BYTE_2_MAX,
                                                       BYTE_4_MIN,
                                                       BYTE_4_MAX);
            } else if (source_count == 4u and destination_count == 1u) {
                const auto input_it  = std::bit_cast<const UInt32*>(std::data(data));
                auto       output_it = std::bit_cast<UInt8*>(std::data(data));

                for (auto i : range(std::size(bytes)))
                    output_it[i] = math::scale<UInt8>(input_it[i],
                                                      BYTE_4_MIN,
                                                      BYTE_4_MAX,
                                                      BYTE_1_MIN,
                                                      BYTE_1_MAX);
            } else if (source_count == 4u and destination_count == 2u) {
                const auto input_it  = std::bit_cast<const UInt32*>(std::data(data));
                auto       output_it = std::bit_cast<UInt16*>(std::data(data));

                for (auto i : range(std::size(bytes)))
                    output_it[i] = math::scale<UInt16>(input_it[i],
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
    Image::Image(const math::ExtentU& extent, Format format) noexcept : Image {} {
        create(extent, format);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    Image::Image(const std::filesystem::path& filepath, Image::Codec codec) noexcept : Image {} {
        const auto _ = loadFromFile(filepath, codec);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    Image::Image(std::span<const Byte> data, Image::Codec codec) noexcept : Image {} {
        const auto _ = loadFromMemory(data, codec);
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
    auto Image::loadFromFile(std::filesystem::path filepath, Image::Codec codec) noexcept
        -> std::expected<void, Error> {
        filepath = std::filesystem::canonical(filepath);

        expects(codec != Image::Codec::Unknown);
        expects(!std::empty(filepath));

        if (!std::filesystem::exists(filepath)) {
            return std::unexpected<Error> {
                std::in_place,
                Error::Reason::File_Not_Found,
                std::format("Failed to open file {}\n    > Incorrect path", filepath.string())
            };
        }

        const auto data = [&filepath]() {
            auto       stream = std::ifstream { filepath, std::ios::binary | std::ios::ate };
            const auto size   = stream.tellg();

            return read(stream, size);
        }();

        if (codec == Image::Codec::Autodetect) codec = details::filenameToCodec(filepath);
        switch (codec) {
            CASE_DO (JPEG, loadJPG)
                ;
            CASE_DO (PNG, loadPNG)
                ;
            CASE_DO (TARGA, loadTGA)
                ;
            CASE_DO (PPM, loadPPM)
                ;
            CASE_DO (HDR, loadHDR)
                ;
            CASE_DO (KTX, loadKTX)
                ;
            CASE_DO (QOI, loadQOI)
                ;
            default: break;
        }

        return std::unexpected<Error> {
            std::in_place,
            Error::Reason::Invalid_Format,
            std::format("Failed to save image from {}\n    > Invalid format", filepath.string())
        };
    }

#undef CASE_DO
#define CASE_DO(_E, _Func, _Name)                                                     \
    case Image::Codec::_E: {                                                          \
        auto result = details::_Func(data);                                           \
        if (!result) {                                                                \
            return std::unexpected<Error> { std::in_place,                            \
                                            result.error().reason,                    \
                                            std::format("Failed to load " _Name       \
                                                        " image from data\n    > {}", \
                                                        result.error().str_error) };  \
        }                                                                             \
        *this = std::move(*result);                                                   \
        return {};                                                                    \
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Image::loadFromMemory(std::span<const Byte> data, Image::Codec codec) noexcept
        -> std::expected<void, Error> {
        expects(codec != Image::Codec::Unknown);
        expects(!std::empty(data));

        if (codec == Image::Codec::Autodetect) codec = details::headerToCodec(data);
        switch (codec) {
            CASE_DO (JPEG, loadJPG, "JPEG")
                ;
            CASE_DO (PNG, loadPNG, "PNG")
                ;
            CASE_DO (TARGA, loadTGA, "TARGA")
                ;
            CASE_DO (PPM, loadPPM, "PPM")
                ;
            CASE_DO (HDR, loadHDR, "HDR")
                ;
            CASE_DO (KTX, loadKTX, "KTX")
                ;
            CASE_DO (QOI, loadQOI, "QOI")
                ;
            default: break;
        }

        return std::unexpected<Error> { std::in_place,
                                        Error::Reason::Invalid_Format,
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
    auto Image::saveToFile(std::filesystem::path filepath,
                           Codec                 codec,
                           CodecArgs args) const noexcept -> std::expected<void, Error> {
        filepath = std::filesystem::canonical(filepath.parent_path()) / filepath.filename();

        expects(codec != Image::Codec::Unknown);
        expects(codec != Image::Codec::Autodetect);
        expects(!std::empty(filepath));
        expects(!std::empty(m_data.data));
        expects(std::filesystem::exists(filepath.root_directory()));

        switch (codec) {
            CASE_DO (JPEG, saveJPG)
                ;
            CASE_DO (PNG, savePNG)
                ;
            CASE_DO (TARGA, saveTGA)
                ;
            CASE_ARGS_DO (PPM, savePPM)
                ;
            CASE_DO (HDR, saveHDR)
                ;
            CASE_DO (KTX, saveKTX)
                ;
            CASE_DO (QOI, saveQOI)
                ;
            default: break;
        }

        return std::unexpected<Error> {
            std::in_place,
            Error::Reason::Invalid_Format,
            std::format("Failed to save image to {}\n    > Invalid format", filepath.string())
        };
    }

#undef CASE_DO
#undef CASE_ARGS_DO
#define CASE_DO(_E, _Func, _Name)                                                             \
    case Image::Codec::_E: {                                                                  \
        auto result = details::_Func(*this);                                                  \
        if (!result) {                                                                        \
            return std::unexpected<Error> { std::in_place,                                    \
                                            result.error().reason,                            \
                                            std::format("Failed to load " _Name               \
                                                        " image from data\n    > {}",         \
                                                        result.error().str_error) };          \
        }                                                                                     \
        return std::expected<std::vector<Byte>, Error> { std::in_place, std::move(*result) }; \
    }
#define CASE_ARGS_DO(_E, _Func, _Name)                                                        \
    case Image::Codec::_E: {                                                                  \
        auto result = details::_Func(*this, std::move(args));                                 \
        if (!result) {                                                                        \
            return std::unexpected<Error> { std::in_place,                                    \
                                            result.error().reason,                            \
                                            std::format("Failed to load " _Name               \
                                                        " image from data\n    > {}",         \
                                                        result.error().str_error) };          \
        }                                                                                     \
        return std::expected<std::vector<Byte>, Error> { std::in_place, std::move(*result) }; \
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Image::saveToMemory(Codec codec, CodecArgs args) const noexcept
        -> std::expected<std::vector<Byte>, Error> {
        expects(codec != Image::Codec::Unknown);
        expects(codec != Image::Codec::Autodetect);
        expects(!std::empty(m_data.data));

        auto output = std::vector<Byte> {};

        switch (codec) {
            CASE_DO (JPEG, saveJPG, "JPEG")
                ;
            CASE_DO (PNG, savePNG, "PNG")
                ;
            CASE_DO (TARGA, saveTGA, "TARGA")
                ;
            CASE_ARGS_DO (PPM, savePPM, "PPM")
                ;
            CASE_DO (HDR, saveHDR, "HDR")
                ;
            CASE_DO (KTX, saveKTX, "KTX")
                ;
            CASE_DO (QOI, saveQOI, "QOI")
                ;
            default: break;
        }

        return std::unexpected<Error> { std::in_place,
                                        Error::Reason::Invalid_Format,
                                        "Failed to save image\n    > Invalid format" };
    }

#undef CASE_ARGS_DO
#undef CASE_DO

    /////////////////////////////////////
    /////////////////////////////////////
    auto Image::create(math::ExtentU extent, Format format) noexcept -> void {
        expects(extent.width > 0u
                and extent.height > 0u
                and extent.depth > 0u
                and format != Format::Undefined);
        m_data.data.clear();

        m_data.extent            = extent;
        m_data.channel_count     = getChannelCountFor(format);
        m_data.bytes_per_channel = getSizeof(format);
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
    auto Image::toFormat(Format format) const noexcept -> Image {
        expects(!std::empty(m_data.data));
        expects(format != Format::Undefined);

        if (m_data.format == format) return *this;

        auto image_data = ImageData { .extent            = m_data.extent,
                                      .channel_count     = getChannelCountFor(format),
                                      .bytes_per_channel = getSizeof(format),
                                      .layers            = m_data.layers,
                                      .faces             = m_data.faces,
                                      .mip_levels        = m_data.mip_levels,
                                      .format            = format };

        /*const auto channel_delta =
            static_cast<UInt8>(std::max(0,
                                              static_cast<Int8>(image_data.channel_count) -
                                                  static_cast<Int8>(m_data.channel_count)));*/
        const auto pixel_count = m_data.extent.width * m_data.extent.height * m_data.extent.depth;

        image_data.data.resize(pixel_count
                                   * image_data.channel_count
                                   * image_data.bytes_per_channel,
                               Byte { 255u });

        auto image = Image { std::move(image_data) };

        for (auto [layer, face, level, i] :
             multi_range(image.layers(), image.faces(), image.layers(), pixel_count)) {
            const auto from_image = details::map(pixel(as<RangeExtent>(i), layer, face, level),
                                                 m_data.bytes_per_channel,
                                                 image.bytesPerChannel());
            auto       to_image   = image.pixel(as<RangeExtent>(i), layer, face, level);

            std::ranges::copy_n(std::ranges::begin(from_image),
                                std::min(m_data.channel_count, image.channelCount()),
                                std::ranges::begin(to_image));
        }

        return image;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Image::scale(const math::ExtentU&) const noexcept -> Image {
        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Image::flipX() const noexcept -> Image {
        auto image_data = ImageData { .extent            = m_data.extent,
                                      .channel_count     = m_data.channel_count,
                                      .bytes_per_channel = m_data.bytes_per_channel,
                                      .layers            = m_data.layers,
                                      .faces             = m_data.faces,
                                      .mip_levels        = m_data.mip_levels,
                                      .format            = m_data.format };

        image_data.data.resize(std::size(m_data.data));

        auto image = Image { std::move(image_data) };

        for (auto [layer, face, mip, x, y, z] : multi_range(m_data.layers,
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
    auto Image::flipY() const noexcept -> Image {
        auto image_data = ImageData { .extent            = m_data.extent,
                                      .channel_count     = m_data.channel_count,
                                      .bytes_per_channel = m_data.bytes_per_channel,
                                      .layers            = m_data.layers,
                                      .faces             = m_data.faces,
                                      .mip_levels        = m_data.mip_levels,
                                      .format            = m_data.format };

        image_data.data.resize(std::size(m_data.data));

        auto image = Image { std::move(image_data) };

        for (auto [layer, face, mip, x, y, z] : multi_range(m_data.layers,
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
    auto Image::flipZ() const noexcept -> Image {
        auto image_data = ImageData { .extent            = m_data.extent,
                                      .channel_count     = m_data.channel_count,
                                      .bytes_per_channel = m_data.bytes_per_channel,
                                      .layers            = m_data.layers,
                                      .faces             = m_data.faces,
                                      .mip_levels        = m_data.mip_levels,
                                      .format            = m_data.format };
        image_data.data.resize(std::size(m_data.data));

        auto image = Image { std::move(image_data) };

        for (auto [layer, face, mip, x, y, z] : multi_range(m_data.layers,
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
    auto Image::rotate90() const noexcept -> Image {
        auto image_data = ImageData { .extent            = m_data.extent,
                                      .channel_count     = m_data.channel_count,
                                      .bytes_per_channel = m_data.bytes_per_channel,
                                      .layers            = m_data.layers,
                                      .faces             = m_data.faces,
                                      .mip_levels        = m_data.mip_levels,
                                      .format            = m_data.format };

        image_data.data.resize(std::size(m_data.data));

        // TODO implement

        return Image { std::move(image_data) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Image::rotate180() const noexcept -> Image {
        auto image_data = ImageData { .extent            = m_data.extent,
                                      .channel_count     = m_data.channel_count,
                                      .bytes_per_channel = m_data.bytes_per_channel,
                                      .layers            = m_data.layers,
                                      .faces             = m_data.faces,
                                      .mip_levels        = m_data.mip_levels,
                                      .format            = m_data.format };

        image_data.data.resize(std::size(m_data.data));

        return Image { std::move(image_data) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Image::rotate270() const noexcept -> Image {
        auto image_data = ImageData { .extent            = m_data.extent,
                                      .channel_count     = m_data.channel_count,
                                      .bytes_per_channel = m_data.bytes_per_channel,
                                      .layers            = m_data.layers,
                                      .faces             = m_data.faces,
                                      .mip_levels        = m_data.mip_levels,
                                      .format            = m_data.format };

        image_data.data.resize(std::size(m_data.data));

        return Image { std::move(image_data) };
    }

} // namespace stormkit::image
