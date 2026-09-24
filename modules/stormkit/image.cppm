// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>

#include <stormkit/image/api.hpp>

export module stormkit.image;

import std;

import stormkit.core;
import stormkit.math;

namespace stdfs = std::filesystem;

export namespace stormkit::image {
    enum class image_format : u8 {
        R8_SNORM     = 0,
        RG8_SNORM    = 1,
        RGB8_SNORM   = 2,
        RGBA8_SNORM  = 3,
        R8_UNORM     = 4,
        RG8_UNORM    = 5,
        RGB8_UNORM   = 6,
        RGBA8_UNORM  = 7,
        R16_SNORM    = 8,
        RG16_SNORM   = 9,
        RGB16_SNORM  = 10,
        RGBA16_SNORM = 11,
        R16_UNORM    = 12,
        RG16_UNORM   = 13,
        RGB16_UNORM  = 14,
        RGBA16_UNORM = 15,
        RGBA4_UNORM  = 17,
        BGR8_UNORM   = 20,
        BGRA8_UNORM  = 21,
        R8I          = 22,
        RG8I         = 23,
        RGB8I        = 24,
        RGBA8I       = 25,
        R8U          = 26,
        RG8U         = 27,
        RGB8U        = 28,
        RGBA8U       = 29,
        R16I         = 30,
        RG16I        = 31,
        RGB16I       = 32,
        RGBA16I      = 33,
        R16U         = 34,
        RG16U        = 35,
        RGB16U       = 36,
        RGBA16U      = 37,
        R32I         = 38,
        RG32I        = 39,
        RGB32I       = 40,
        RGBA32I      = 41,
        R32U         = 42,
        RG32U        = 43,
        RGB32U       = 44,
        RGBA32U      = 45,
        R16F         = 47,
        RG16F        = 48,
        RGB16F       = 49,
        RGBA16F      = 50,
        R32F         = 51,
        RG32F        = 52,
        RGB32F       = 53,
        RGBA32F      = 54,
        SRGB8        = 56,
        SRGBA8       = 57,
        SBGR8        = 58,
        SBGRA8       = 59,
        UNDEFINED    = 254,
    };

    enum class image_codec : u8 {
        AUTODETECT = 0,
        JPEG       = 1,
        PNG        = 2,
        TARGA      = 3,
        PPM        = 4,
        HDR        = 5,
        KTX        = 6,
        QOI        = 7,
        UNKNOWN    = 255,
    };

    enum class image_codec_format : u8 {
        BINARY = 0,
        ASCII  = 1,
    };

    enum class image_status_code : u8 {
        NOT_IMPLEMENTED = 0,
        FAILED_TO_PARSE = 1,
        FAILED_TO_SAVE  = 2,
        FILE_NOT_FOUND  = 3,
        INVALID_FORMAT  = 4,
        UNKNOWN         = std::numeric_limits<u8>::max(),
    };

    constexpr auto status_code(image_status_code) noexcept -> system_error2::quick_status_code_from_enum_code<image_status_code>;
} // namespace stormkit::image

export namespace stormkit::image {
    class STORMKIT_IMAGE_API image {
      public:
        template<typename T>
        using result = core::system_result<T>;

        struct image_data_t {
            math::uextent3 extent            = { .width = 0u, .height = 0u };
            u32            channel_count     = 0u;
            u32            bytes_per_channel = 0u;
            u32            layers            = 1u;
            u32            faces             = 1u;
            u32            mip_levels        = 1u;
            image_format   format            = image_format::UNDEFINED;

            dynarray<byte> data = {};
        };

        image() noexcept;
        explicit image(image_data_t&& data) noexcept;
        image(meta::in<math::uextent3> extent, image_format format) noexcept;
        image(const stdfs::path& filepath, image_codec codec = image_codec::AUTODETECT) noexcept;
        image(array_view<const byte> data, image_codec codec = image_codec::AUTODETECT) noexcept;
        ~image() noexcept;

        image(const image& rhs) noexcept;
        auto operator=(const image& rhs) noexcept -> image&;

        image(image&& rhs) noexcept;
        auto operator=(image&& rhs) noexcept -> image&;

        [[nodiscard]]
        auto load_from_file(const stdfs::path& filepath, image_codec codec = image_codec::AUTODETECT) noexcept
          -> result<void>;
        [[nodiscard]]
        auto load_from_memory(array_view<const byte> data, image_codec codec = image_codec::AUTODETECT) noexcept -> result<void>;
        [[nodiscard]]
        auto save_to_file(const stdfs::path& filename,
                          image_codec                  codec,
                          image_codec_format           args = image_codec_format::BINARY) const noexcept -> result<void>;

        [[nodiscard]]
        auto save_to_memory(image_codec codec, image_codec_format args = image_codec_format::BINARY) const noexcept
          -> result<dynarray<byte>>;

        auto create(meta::in<math::uextent3> extent, image_format format) noexcept -> void;

        [[nodiscard]]
        auto convert_to(image_format format) const noexcept -> image;
        [[nodiscard]]
        auto scale(meta::in<math::uextent3> scale_to) const noexcept -> image;
        [[nodiscard]]
        auto flip_x() const noexcept -> image;
        [[nodiscard]]
        auto flip_y() const noexcept -> image;
        [[nodiscard]]
        auto flip_z() const noexcept -> image;
        [[nodiscard]]
        auto rotate_90() const noexcept -> image;
        [[nodiscard]]
        auto rotate_180() const noexcept -> image;
        [[nodiscard]]
        auto rotate_270() const noexcept -> image;

        template<typename Self>
        [[nodiscard]]
        auto pixel(this Self& self, usize id, u32 layer = 0u, u32 face = 0u, u32 level = 0u) noexcept
          -> array_view<meta::forward_const_to<Self, byte>>;

        template<typename Self>
        [[nodiscard]]
        auto pixel(this Self& self, meta::in<math::uvec3> position, u32 layer = 0u, u32 face = 0u, u32 level = 0u) noexcept
          -> array_view<meta::forward_const_to<Self, byte>>;

        [[nodiscard]]
        auto extent(u32 level = 0u) const noexcept -> math::uextent3;

        [[nodiscard]]
        auto channel_count() const noexcept -> u32;

        [[nodiscard]]
        auto bytes_per_channel() const noexcept -> u32;

        [[nodiscard]]
        auto layers() const noexcept -> u32;

        [[nodiscard]]
        auto faces() const noexcept -> u32;

        [[nodiscard]]
        auto mip_levels() const noexcept -> u32;

        [[nodiscard]]
        auto format() const noexcept -> image_format;

        [[nodiscard]]
        auto size() const noexcept -> usize;
        [[nodiscard]]
        auto size(u32 layer, u32 face, u32 level) const noexcept -> usize;
        [[nodiscard]]
        auto size(u32 layer, u32 face) const noexcept -> usize;
        [[nodiscard]]
        auto size(u32 layer) const noexcept -> usize;

        template<typename Self>
        [[nodiscard]]
        auto data(this Self& self) noexcept -> array_view<meta::forward_const_to<Self, byte>>;

        template<typename Self>
        [[nodiscard]]
        auto data(this Self& self, u32 layer, u32 face, u32 level) noexcept -> array_view<meta::forward_const_to<Self, byte>>;

        // template<typename Self>
        // [[nodiscard]]
        // auto begin(this Self& self) noexcept;
        // template<typename Self>
        // [[nodiscard]]
        // auto begin(this Self& self, u32 layer, u32 face, u32 level) noexcept;

        // [[nodiscard]]
        // auto cbegin() const noexcept;
        // [[nodiscard]]
        // auto cbegin(u32 layer, u32 face, u32 level) const noexcept;

        // template<typename Self>
        // [[nodiscard]]
        // auto end() noexcept;
        // template<typename Self>
        // [[nodiscard]]
        // auto end(u32 layer, u32 face, u32 level) noexcept;

        // [[nodiscard]]
        // auto cend() const noexcept;
        // [[nodiscard]]
        // auto cend(u32 layer, u32 face, u32 level) const noexcept;

        [[nodiscard]]
        auto image_data() const & noexcept -> const image_data_t&;

        [[nodiscard]]
        auto image_data() && noexcept -> image_data_t;

      private:
        image_data_t m_data;
    };

    [[nodiscard]]
    constexpr auto get_format_channel_count(image_format format) noexcept -> u8;
    [[nodiscard]]
    constexpr auto get_format_component_size(image_format format) noexcept -> u8;

    // template<typename CharT>
    // constexpr auto tag_invoke(format_as_fn<CharT>, meta::in<image::Error> value, meta::format_context auto& ctx)
    //   -> decltype(ctx.out());
} // namespace stormkit::image

export template<>
struct system_error2::quick_status_code_from_enum<stormkit::image::image_status_code>
    : quick_status_code_from_enum_defaults<stormkit::image::image_status_code> {
    static constexpr auto domain_name = "stormkit Image";
    static constexpr auto domain_uuid = "{0f7f937d-b027-0427-27d2-47b9359122ae}";

    static constexpr auto value_mappings() noexcept -> const std::initializer_list<mapping>&;
};

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace system_error2 {
    /////////////////////////////////////
    /////////////////////////////////////
    inline constexpr auto quick_status_code_from_enum<stormkit::image::image_status_code>::value_mappings() noexcept
      -> const std::initializer_list<mapping>& {
        static constexpr auto mapper = std::initializer_list<mapping> {
            { stormkit::image::image_status_code::NOT_IMPLEMENTED, "Not implemented", { errc::not_supported }             },
            { stormkit::image::image_status_code::FAILED_TO_PARSE, "Failed to parse", { errc::not_supported }             },
            { stormkit::image::image_status_code::FAILED_TO_SAVE,  "Failed to save",  { errc::io_error }                  },
            { stormkit::image::image_status_code::FILE_NOT_FOUND,  "File not found",  { errc::no_such_file_or_directory } },
            { stormkit::image::image_status_code::INVALID_FORMAT,  "Invalid format",  { errc::not_supported }             },
            { stormkit::image::image_status_code::UNKNOWN,         "Unknown error",   { errc::state_not_recoverable }     },
        };
        return mapper;
    }
} // namespace system_error2

namespace stormkit::image {
    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto status_code(image_status_code code) noexcept
      -> system_error2::quick_status_code_from_enum_code<image_status_code> {
        return code;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Self>
    inline auto image::pixel(this Self& self, usize index, u32 layer, u32 face, u32 level) noexcept
      -> array_view<meta::forward_const_to<Self, byte>> {
        EXPECTS(self.m_data.mip_levels > level);
        EXPECTS(self.m_data.faces > face);
        EXPECTS(self.m_data.layers > layer);

        auto&& _data = self.data(layer, face, level);

        EXPECTS(index < self.m_data.extent.width * self.m_data.extent.height * self.m_data.extent.depth);

        const auto block_size = self.m_data.channel_count * self.m_data.bytes_per_channel;

        return array_view<meta::forward_const_to<Self, byte>> { std::data(_data) + index * block_size, block_size };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Self>
    inline auto image::pixel(this Self& self, meta::in<math::uvec3> position, u32 layer, u32 face, u32 level) noexcept
      -> array_view<meta::forward_const_to<Self, byte>> {
        const auto mip_extent = self.extent(level);
        const auto id = position.x() + (position.y() * mip_extent.width) + (mip_extent.width * mip_extent.height * position.z());

        return self.pixel(id, layer, face, level);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto image::extent(u32 level) const noexcept -> math::uextent3 {
        EXPECTS(m_data.mip_levels > level);

        return { .width  = std::max(1u, m_data.extent.width >> level),
                 .height = std::max(1u, m_data.extent.height >> level),
                 .depth  = std::max(1u, m_data.extent.depth >> level) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto image::channel_count() const noexcept -> u32 {
        return m_data.channel_count;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto image::bytes_per_channel() const noexcept -> u32 {
        return m_data.bytes_per_channel;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto image::layers() const noexcept -> u32 {
        return m_data.layers;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto image::faces() const noexcept -> u32 {
        return m_data.faces;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto image::mip_levels() const noexcept -> u32 {
        return m_data.mip_levels;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto image::format() const noexcept -> image_format {
        return m_data.format;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto image::size() const noexcept -> usize {
        return std::size(m_data.data);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto image::size(u32 layer, // TODO Use layer and face to get correct size
                            u32 face,
                            u32 level) const noexcept -> usize {
        EXPECTS(m_data.mip_levels > level);
        EXPECTS(m_data.faces > face);
        EXPECTS(m_data.layers > layer);

        const auto mip_extent = extent(level);

        return mip_extent.width * mip_extent.height * mip_extent.depth * m_data.channel_count * m_data.bytes_per_channel;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto image::size(u32 layer, u32 face) const noexcept -> usize {
        auto _size = usize { 0u };
        for (auto i : range(m_data.mip_levels)) _size += size(layer, face, i);

        return _size;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto image::size(u32 layer) const noexcept -> usize {
        auto _size = usize { 0u };
        for (auto i : range(m_data.faces)) _size += size(layer, i);

        return _size;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Self>
    STORMKIT_FORCE_INLINE
    inline auto image::data(this Self& self) noexcept -> array_view<meta::forward_const_to<Self, byte>> {
        return array_view<meta::forward_const_to<Self, byte>> { std::forward_like<Self&>(self.m_data.data) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Self>
    inline auto image::data(this Self& self, u32 layer, u32 face, u32 level) noexcept
      -> array_view<meta::forward_const_to<Self, byte>> {
        const auto mip_size = self.size(layer, face, level);

        auto offset = usize { 0 };
        for (auto i : range(layer)) offset += self.size(i);
        for (auto j : range(face)) offset += self.size(layer, j);
        for (auto k : range(level)) offset += self.size(layer, face, k);

        return array_view<meta::forward_const_to<Self, byte>> { std::data(self.m_data.data) + offset, mip_size };
    }

    // /////////////////////////////////////
    // /////////////////////////////////////
    // template<typename Self>
    // inline auto image::begin() noexcept {
    //     return std::begin(m_data.data);
    // }

    // /////////////////////////////////////
    // /////////////////////////////////////
    // template<typename Self>
    // inline auto image::begin(u32 layer, u32 face, u32 level) noexcept {
    //     EXPECTS(m_data.mip_levels > level);
    //     EXPECTS(m_data.faces > face);
    //     EXPECTS(m_data.layers > layer);

    //    return std::begin(data(layer, face, level));
    // }

    // /////////////////////////////////////
    // /////////////////////////////////////
    // inline auto image::cbegin() const noexcept {
    //     return std::cbegin(m_data.data);
    // }

    // /////////////////////////////////////
    // /////////////////////////////////////
    // inline auto image::cbegin(u32 layer, u32 face, u32 level) const noexcept {
    //     EXPECTS(m_data.mip_levels > level);
    //     EXPECTS(m_data.faces > face);
    //     EXPECTS(m_data.layers > layer);

    //    return std::cbegin(data(layer, face, level));
    // }

    // /////////////////////////////////////
    // /////////////////////////////////////
    // template<typename Self>
    // inline auto image::end() noexcept {
    //     return std::end(m_data.data);
    // }

    // /////////////////////////////////////
    // /////////////////////////////////////
    // template<typename Self>
    // inline auto image::end(u32 layer, u32 face, u32 level) noexcept {
    //     EXPECTS(m_data.mip_levels > level);
    //     EXPECTS(m_data.faces > face);
    //     EXPECTS(m_data.layers > layer);

    //    return std::end(data(layer, face, level));
    // }

    // /////////////////////////////////////
    // /////////////////////////////////////
    // inline auto image::cend() const noexcept {
    //     return std::cend(m_data.data);
    // }

    // /////////////////////////////////////
    // /////////////////////////////////////
    // inline auto image::cend(u32 layer, u32 face, u32 level) const noexcept {
    //     EXPECTS(m_data.mip_levels > level);
    //     EXPECTS(m_data.faces > face);
    //     EXPECTS(m_data.layers > layer);

    //    return std::cend(data(layer, face, level));
    // }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto image::image_data() const & noexcept -> const image_data_t& {
        return m_data;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto image::image_data() && noexcept -> image_data_t {
        return std::move(m_data);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_CONST
    constexpr auto get_format_channel_count(image_format format) noexcept -> u8 {
        switch (format) {
            case image_format::R8_SNORM:
            case image_format::R8_UNORM:
            case image_format::R16_SNORM:
            case image_format::R16_UNORM:
            case image_format::R8I:
            case image_format::R8U:
            case image_format::R16I:
            case image_format::R16U:
            case image_format::R32I:
            case image_format::R32U:
            case image_format::R16F:
            case image_format::R32F: return 1;

            case image_format::RG8_SNORM:
            case image_format::RG8_UNORM:
            case image_format::RG16_SNORM:
            case image_format::RG16_UNORM:
            case image_format::RG8I:
            case image_format::RG8U:
            case image_format::RG16I:
            case image_format::RG16U:
            case image_format::RG32I:
            case image_format::RG32U:
            case image_format::RG16F:
            case image_format::RG32F: return 2;

            case image_format::RGB8_SNORM:
            case image_format::RGB8_UNORM:
            case image_format::RGB16_SNORM:
            case image_format::RGB16_UNORM:
            case image_format::BGR8_UNORM:
            case image_format::RGB8I:
            case image_format::RGB8U:
            case image_format::RGB16I:
            case image_format::RGB16U:
            case image_format::RGB32I:
            case image_format::RGB32U:
            case image_format::RGB16F:
            case image_format::RGB32F:
            case image_format::SRGB8:
            case image_format::SBGR8: return 3;

            case image_format::RGBA8_SNORM:
            case image_format::RGBA8_UNORM:
            case image_format::RGBA16_SNORM:
            case image_format::RGBA16_UNORM:
            case image_format::BGRA8_UNORM:
            case image_format::RGBA8I:
            case image_format::RGBA8U:
            case image_format::RGBA16I:
            case image_format::RGBA16U:
            case image_format::RGBA32I:
            case image_format::RGBA32U:
            case image_format::RGBA16F:
            case image_format::RGBA32F:
            case image_format::SRGBA8:
            case image_format::SBGRA8: return 4;

            default: break;
        }

        return 0u;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_CONST
    constexpr auto get_format_component_size(image_format format) noexcept -> u8 {
        switch (format) {
            case image_format::R8_SNORM:
            case image_format::R8_UNORM:
            case image_format::RG8_SNORM:
            case image_format::RG8_UNORM:
            case image_format::R8I:
            case image_format::R8U:
            case image_format::RG8I:
            case image_format::RG8U:
            case image_format::RGB8_SNORM:
            case image_format::RGB8_UNORM:
            case image_format::BGR8_UNORM:
            case image_format::RGB8I:
            case image_format::RGB8U:
            case image_format::RGBA8_SNORM:
            case image_format::RGBA8_UNORM:
            case image_format::RGBA16_SNORM:
            case image_format::BGRA8_UNORM:
            case image_format::SRGB8:
            case image_format::SBGR8:
            case image_format::SRGBA8:
            case image_format::SBGRA8: return 1u;

            case image_format::R16_SNORM:
            case image_format::R16_UNORM:
            case image_format::R16I:
            case image_format::R16U:
            case image_format::RG16_SNORM:
            case image_format::RG16_UNORM:
            case image_format::RG16I:
            case image_format::RG16U:
            case image_format::RG16F:
            case image_format::RGB16I:
            case image_format::RGB16U:
            case image_format::RGB16F:
            case image_format::RGBA16I:
            case image_format::RGBA16U:
            case image_format::RGBA16F:
            case image_format::R16F: return 2u;

            case image_format::R32I:
            case image_format::R32U:
            case image_format::R32F:
            case image_format::RG32I:
            case image_format::RG32U:
            case image_format::RG32F:
            case image_format::RGB16_SNORM:
            case image_format::RGB32I:
            case image_format::RGB32U:
            case image_format::RGB32F:
            case image_format::RGBA8I:
            case image_format::RGBA8U:
            case image_format::RGBA32I:
            case image_format::RGBA32U:
            case image_format::RGBA32F: return 4u;

            default: break;
        }

        return 0u;
    }

    // /////////////////////////////////////
    // /////////////////////////////////////
    // template<typename CharT>
    // constexpr auto tag_invoke(format_as_fn<CharT>, meta::in<image::Error> value, meta::format_context auto& ctx)
    //   -> decltype(ctx.out()) {
    //     return std::format_to(out, "{}", error.str_error);
    // }
} // namespace stormkit::image
