// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>

#include <stormkit/image/api.hpp>

export module stormkit.image;

import std;

import stormkit.core;
import stormkit.math.extent;
import stormkit.math.linear;

namespace stdfs = std::filesystem;

export namespace stormkit::image {
    enum class image_format : u8 {
        r8_snorm     = 0,
        rg8_snorm    = 1,
        rgb8_snorm   = 2,
        rgba8_snorm  = 3,
        r8_unorm     = 4,
        rg8_unorm    = 5,
        rgb8_unorm   = 6,
        rgba8_unorm  = 7,
        r16_snorm    = 8,
        rg16_snorm   = 9,
        rgb16_snorm  = 10,
        rgba16_snorm = 11,
        r16_unorm    = 12,
        rg16_unorm   = 13,
        rgb16_unorm  = 14,
        rgba16_unorm = 15,
        rgba4_unorm  = 17,
        bgr8_unorm   = 20,
        bgra8_unorm  = 21,
        r8i          = 22,
        rg8i         = 23,
        rgb8i        = 24,
        rgba8i       = 25,
        r8u          = 26,
        rg8u         = 27,
        rgb8u        = 28,
        rgba8u       = 29,
        r16i         = 30,
        rg16i        = 31,
        rgb16i       = 32,
        rgba16i      = 33,
        r16u         = 34,
        rg16u        = 35,
        rgb16u       = 36,
        rgba16u      = 37,
        r32i         = 38,
        rg32i        = 39,
        rgb32i       = 40,
        rgba32i      = 41,
        r32u         = 42,
        rg32u        = 43,
        rgb32u       = 44,
        rgba32u      = 45,
        r16f         = 47,
        rg16f        = 48,
        rgb16f       = 49,
        rgba16f      = 50,
        r32f         = 51,
        rg32f        = 52,
        rgb32f       = 53,
        rgba32f      = 54,
        srgb8        = 56,
        srgba8       = 57,
        sbgr8        = 58,
        sbgra8       = 59,
        undefined    = 254,
    };

    enum class image_codec : u8 {
        autodetect = 0,
        jpeg       = 1,
        png        = 2,
        targa      = 3,
        ppm        = 4,
        hdr        = 5,
        ktx        = 6,
        qoi        = 7,
        unknown    = 255,
    };

    enum class image_codec_format : u8 {
        binary = 0,
        ascii  = 1,
    };

    enum class image_status_code : u8 {
        not_implemented = 0,
        failed_to_parse = 1,
        failed_to_save  = 2,
        file_not_found  = 3,
        invalid_format  = 4,
        unknown         = std::numeric_limits<u8>::max(),
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
            image_format   format            = image_format::undefined;

            dynarray<byte> data = {};
        };

        image() noexcept;
        explicit image(image_data_t&& data) noexcept;
        image(meta::in<math::uextent3> extent, image_format format) noexcept;
        image(const stdfs::path& filepath, image_codec codec = image_codec::autodetect) noexcept;
        image(array_view<const byte> data, image_codec codec = image_codec::autodetect) noexcept;
        ~image() noexcept;

        image(const image& rhs) noexcept;
        auto operator=(const image& rhs) noexcept -> image&;

        image(image&& rhs) noexcept;
        auto operator=(image&& rhs) noexcept -> image&;

        [[nodiscard]]
        auto load_from_file(const stdfs::path& filepath, image_codec codec = image_codec::autodetect) noexcept -> result<void>;
        [[nodiscard]]
        auto load_from_memory(array_view<const byte> data, image_codec codec = image_codec::autodetect) noexcept -> result<void>;
        [[nodiscard]]
        auto save_to_file(const stdfs::path& filename,
                          image_codec        codec,
                          image_codec_format args = image_codec_format::binary) const noexcept -> result<void>;

        [[nodiscard]]
        auto save_to_memory(image_codec codec, image_codec_format args = image_codec_format::binary) const noexcept
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
    static constexpr auto domain_name = "stormkit image";
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
            { stormkit::image::image_status_code::not_implemented, "Not implemented", { errc::not_supported }             },
            { stormkit::image::image_status_code::failed_to_parse, "Failed to parse", { errc::not_supported }             },
            { stormkit::image::image_status_code::failed_to_save,  "Failed to save",  { errc::io_error }                  },
            { stormkit::image::image_status_code::file_not_found,  "File not found",  { errc::no_such_file_or_directory } },
            { stormkit::image::image_status_code::invalid_format,  "Invalid format",  { errc::not_supported }             },
            { stormkit::image::image_status_code::unknown,         "Unknown error",   { errc::state_not_recoverable }     },
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
            case image_format::r8_snorm:
            case image_format::r8_unorm:
            case image_format::r16_snorm:
            case image_format::r16_unorm:
            case image_format::r8i:
            case image_format::r8u:
            case image_format::r16i:
            case image_format::r16u:
            case image_format::r32i:
            case image_format::r32u:
            case image_format::r16f:
            case image_format::r32f: return 1;

            case image_format::rg8_snorm:
            case image_format::rg8_unorm:
            case image_format::rg16_snorm:
            case image_format::rg16_unorm:
            case image_format::rg8i:
            case image_format::rg8u:
            case image_format::rg16i:
            case image_format::rg16u:
            case image_format::rg32i:
            case image_format::rg32u:
            case image_format::rg16f:
            case image_format::rg32f: return 2;

            case image_format::rgb8_snorm:
            case image_format::rgb8_unorm:
            case image_format::rgb16_snorm:
            case image_format::rgb16_unorm:
            case image_format::bgr8_unorm:
            case image_format::rgb8i:
            case image_format::rgb8u:
            case image_format::rgb16i:
            case image_format::rgb16u:
            case image_format::rgb32i:
            case image_format::rgb32u:
            case image_format::rgb16f:
            case image_format::rgb32f:
            case image_format::srgb8:
            case image_format::sbgr8: return 3;

            case image_format::rgba8_snorm:
            case image_format::rgba8_unorm:
            case image_format::rgba16_snorm:
            case image_format::rgba16_unorm:
            case image_format::bgra8_unorm:
            case image_format::rgba8i:
            case image_format::rgba8u:
            case image_format::rgba16i:
            case image_format::rgba16u:
            case image_format::rgba32i:
            case image_format::rgba32u:
            case image_format::rgba16f:
            case image_format::rgba32f:
            case image_format::srgba8:
            case image_format::sbgra8: return 4;

            default: break;
        }

        return 0u;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_CONST
    constexpr auto get_format_component_size(image_format format) noexcept -> u8 {
        switch (format) {
            case image_format::r8_snorm:
            case image_format::r8_unorm:
            case image_format::rg8_snorm:
            case image_format::rg8_unorm:
            case image_format::r8i:
            case image_format::r8u:
            case image_format::rg8i:
            case image_format::rg8u:
            case image_format::rgb8_snorm:
            case image_format::rgb8_unorm:
            case image_format::bgr8_unorm:
            case image_format::rgb8i:
            case image_format::rgb8u:
            case image_format::rgba8_snorm:
            case image_format::rgba8_unorm:
            case image_format::rgba16_snorm:
            case image_format::bgra8_unorm:
            case image_format::srgb8:
            case image_format::sbgr8:
            case image_format::srgba8:
            case image_format::sbgra8: return 1u;

            case image_format::r16_snorm:
            case image_format::r16_unorm:
            case image_format::r16i:
            case image_format::r16u:
            case image_format::rg16_snorm:
            case image_format::rg16_unorm:
            case image_format::rg16i:
            case image_format::rg16u:
            case image_format::rg16f:
            case image_format::rgb16i:
            case image_format::rgb16u:
            case image_format::rgb16f:
            case image_format::rgba16i:
            case image_format::rgba16u:
            case image_format::rgba16f:
            case image_format::r16f: return 2u;

            case image_format::r32i:
            case image_format::r32u:
            case image_format::r32f:
            case image_format::rg32i:
            case image_format::rg32u:
            case image_format::rg32f:
            case image_format::rgb16_snorm:
            case image_format::rgb32i:
            case image_format::rgb32u:
            case image_format::rgb32f:
            case image_format::rgba8i:
            case image_format::rgba8u:
            case image_format::rgba32i:
            case image_format::rgba32u:
            case image_format::rgba32f: return 4u;

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
