// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#include <stormkit/core/contract_macro.hpp>

export module stormkit.wsi:win32.window;

import std;

import stormkit.core;
import stormkit.core.win32;
import stormkit.wsi;

import :common.window_base;

export namespace stormkit::wsi::win32 {
    class window: public ::stormkit::wsi::common::window_base {
      public:
        explicit window(window_manager wm) noexcept;
        ~window() noexcept;

        window(const window&) noexcept;
        auto operator=(const window&) noexcept -> window&;

        window(window&&) noexcept;
        auto operator=(window&&) noexcept -> window&;

        auto open(string&& title, const math::uextent2& size, window_flag flags) noexcept -> void;
        auto close() noexcept -> void;

        auto handle_events() noexcept -> void;

        auto clear(const ucolor_rgb& color) noexcept -> void;
        auto fill_framebuffer(array_view<const ucolor_rgb> colors) noexcept -> void;

        auto set_title(string&& title) noexcept -> void;
        auto set_extent(const math::uextent2& extent) noexcept -> void;
        auto set_fullscreen(bool fullscreen) noexcept -> void;

        auto confine_mouse(bool confined, u8 mouse_id) noexcept -> void;
        [[nodiscard]]
        auto is_mouse_confined(u8 mouse_id) const noexcept -> bool;

        auto lock_mouse(bool locked, u8 mouse_id) noexcept -> void;
        [[nodiscard]]
        auto is_mouse_locked(u8 mouse_id) const noexcept -> bool;

        auto hide_mouse(bool hidden, u8 mouse_id) noexcept -> void;
        [[nodiscard]]
        auto is_mouse_hidden(u8 mouse_id) const noexcept -> bool;

        auto set_relative_mouse(bool enabled, u8 mouse_id) noexcept -> void;
        [[nodiscard]]
        auto is_mouse_relative(u8 mouse_id) const noexcept -> bool;

        auto set_key_repeat(bool enabled, u8 keyboard_id) noexcept -> void;
        [[nodiscard]]
        auto is_key_repeat_enabled(u8 keyboard_id) const noexcept -> bool;

        auto show_virtual_keyboard(bool visible) noexcept -> void;
        [[nodiscard]]
        auto is_virtual_keyboard_visible() const noexcept -> bool;

        auto set_mouse_position(const math::ivec2& position, u8 mouse_id) noexcept -> void;

        [[nodiscard]]
        auto is_mouse_inside() const noexcept -> bool;
        auto set_mouse_inside(bool inside) noexcept -> void;

        auto begin_resize() noexcept -> void;
        auto resize(u32 width, u32 height) noexcept -> void;
        auto end_resize() noexcept -> void;

        [[nodiscard]]
        auto native_handle() const noexcept -> native_handle_type;

        auto update_geometry(const math::uextent2& extent) noexcept -> void;

        auto win32_state(this auto& self) noexcept -> decltype(auto);
        auto state(this auto& self) noexcept -> decltype(auto);

        auto gdi_frame_data(this auto& self) noexcept -> decltype(auto);

        auto gdiinit() noexcept -> void;

      private:
        struct {
            ::win32::DWORD style;
            ::win32::DWORD style_ex;

            bool external_context = false;
            bool mouse_inside     = false;
            bool resizing         = false;

            math::uextent2 extent;
            math::uextent2 last_extent;

            ::win32::DWORD tls_index = 0;

            bool mouse_tracked = false;
        } win32_state_;

        ::win32::HWND window_handle_ = nullptr;

        using hdc = raii_capsule<::win32::HDC, ::win32::CreateCompatibleDC, ::win32::DeleteDC, struct hdc_tag, nullptr>;
        using hbitmap
          = raii_capsule<::win32::HBITMAP, ::win32::CreateDIBSection, ::win32::DeleteObject, struct hbitmap_tag, nullptr>;

        struct GDIFrameData {
            GDIFrameData();

            GDIFrameData(const GDIFrameData&)                    = delete;
            auto operator=(const GDIFrameData&) -> GDIFrameData& = delete;

            GDIFrameData(GDIFrameData&&) noexcept;
            auto operator=(GDIFrameData&&) noexcept -> GDIFrameData&;

            ~GDIFrameData() noexcept;

            hdc                context    = hdc::empty();
            hbitmap            bitmap     = hbitmap::empty();
            std::atomic<void*> pixels_ptr = nullptr;

            math::extent2<::win32::LONG> extent;
        } gdi_frame_data_;
    };
} // namespace stormkit::wsi::win32

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::wsi::win32 {
    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::is_mouse_inside() const noexcept -> bool {
        return win32_state_.mouse_inside;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::set_mouse_inside(bool inside) noexcept -> void {
        win32_state_.mouse_inside = inside;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::native_handle() const noexcept -> native_handle_type {
        return std::bit_cast<native_handle_type>(window_handle_);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::update_geometry(const math::uextent2& extent) noexcept -> void {
        state_.extent       = extent;
        win32_state_.extent = extent;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::begin_resize() noexcept -> void {
        win32_state_.resizing = true;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::resize(u32 width, u32 height) noexcept -> void {
        win32_state_.last_extent.width  = width;
        win32_state_.last_extent.height = height;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::end_resize() noexcept -> void {
        win32_state_.resizing = false;

        if (win32_state_.last_extent != extent()) {
            win32_state_.last_extent = extent();

            resized_event(win32_state_.last_extent);
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::win32_state(this auto& self) noexcept -> decltype(auto) {
        return std::forward_like<decltype(self)>(self.win32_state_);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::state(this auto& self) noexcept -> decltype(auto) {
        return std::forward_like<decltype(self)>(self.state_);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::gdi_frame_data(this auto& self) noexcept -> decltype(auto) {
        return std::forward_like<decltype(self)>(self.gdi_frame_data_);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline window::GDIFrameData::GDIFrameData() = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline window::GDIFrameData::GDIFrameData(GDIFrameData&& other) noexcept
        : context { std::move(other.context) },
          bitmap { std::move(other.bitmap) },
          pixels_ptr { other.pixels_ptr.load() },
          extent { other.extent } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto window::GDIFrameData::operator=(GDIFrameData&& other) noexcept -> GDIFrameData& {
        if (&other == this) return *this;

        context    = std::move(other.context);
        bitmap     = std::move(other.bitmap);
        pixels_ptr = other.pixels_ptr.load();
        extent     = other.extent;

        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline window::GDIFrameData::~GDIFrameData() noexcept = default;

} // namespace stormkit::wsi::win32
