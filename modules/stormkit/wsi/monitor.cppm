// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#include <stormkit/wsi/api.hpp>

export module stormkit.wsi:monitor;

import std;

import stormkit.core;

export {
    namespace stormkit::wsi {
        struct Monitor {
            enum class Flags {
                NONE = 0,
                PRIMARY,
            };

            Flags  flags = Flags::NONE;
            string name;

            dynarray<math::uextent2> extents;
            u32                      scale_factor = 1;

            [[nodiscard]]
            constexpr auto operator<=>(const Monitor& other) const noexcept -> std::strong_ordering;

            [[nodiscard]]
            constexpr auto operator==(const Monitor& other) const noexcept -> bool;

            void* native_handle = nullptr;
        };

        constexpr auto as_string(Monitor::Flags flags) noexcept -> string_view;
        constexpr auto to_string(Monitor::Flags flags) noexcept -> string;

        auto to_string(const Monitor& monitor) noexcept -> string;

        template<typename FormatContext>
        auto format_as(const Monitor& monitor, FormatContext& ctx) noexcept -> decltype(ctx.out());

        [[nodiscard]]
        STORMKIT_WSI_API auto get_monitors(bool update = false) noexcept -> array_view<const Monitor>;

        [[nodiscard]]
        STORMKIT_WSI_API auto get_primary_monitor() noexcept -> const Monitor&;
    } // namespace stormkit::wsi

    template<>
    inline constexpr auto stormkit::core::meta::FLAG_TRAIT<stormkit::wsi::Monitor::Flags> = true;
} // namespace stormkit::wsi

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::wsi {
    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_PURE
    constexpr auto Monitor::operator<=>(const Monitor& other) const noexcept -> std::strong_ordering {
        if (auto ret = flags <=> other.flags; ret != 0) return ret;

#ifdef STORMKIT_COMPILER_CLANG
        if (auto ret = name.compare(other.name); ret != 0) {
            if (ret < 0) return std::strong_ordering::less;
            else if (ret > 0)
                return std::strong_ordering::greater;
        }
#else
        if (auto ret = name <=> other.name; ret != 0) return ret;
#endif

        if (auto ret = std::size(extents) <=> std::size(other.extents); ret != 0) return ret;

        for (auto i : range(std::size(extents))) {
            if (auto ret = extents[i].width <=> other.extents[i].width; ret != 0) return ret;
            if (auto ret = extents[i].height <=> other.extents[i].height; ret != 0) return ret;
        }

        return std::strong_ordering::equal;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_PURE
    constexpr auto Monitor::operator==(const Monitor& other) const noexcept -> bool {
        if (flags != other.flags) return false;
        if (name != other.name) return false;
        if (std::size(extents) != std::size(other.extents)) return false;
        for (auto i : range(std::size(extents))) {
            if (extents[i].width != other.extents[i].width) return false;
            if (extents[i].height != other.extents[i].height) return false;
        }

        return true;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto as_string(Monitor::Flags flags) noexcept -> string_view {
        switch (flags) {
            case Monitor::Flags::NONE: return "Monitor::Flags::NONE";
            case Monitor::Flags::PRIMARY: return "Monitor::Flags::PRIMARY";
            default: break;
        }

        std::unreachable();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    constexpr auto to_string(Monitor::Flags flags) noexcept -> string {
        return string { as_string(flags) };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto to_string(const Monitor& monitor) noexcept -> string {
        return std::format("{}", monitor);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename FormatContext>
    STORMKIT_FORCE_INLINE
    inline auto format_as(const Monitor& monitor, FormatContext& ctx) noexcept -> decltype(ctx.out()) {
        return std::format_to(ctx.out(),
                              "[Monitor name: {}, flags: {}, extents: {}, scale_factor: {}]",
                              monitor.name,
                              monitor.flags,
                              monitor.extents,
                              monitor.scale_factor);
    }
} // namespace stormkit::wsi
