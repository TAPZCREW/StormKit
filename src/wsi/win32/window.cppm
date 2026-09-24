// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform/windows.hpp>

#include <stormkit/core/platform_macro.hpp>

#include <stormkit/core/contract_macro.hpp>

export module stormkit.wsi:win32.window;

import std;

import stormkit.core;
import stormkit.wsi;

import :common.window_base;

// import :win32.keyboard;
// import :win32.mouse;

export namespace stormkit::wsi::win32 {
    class Window: public ::stormkit::wsi::common::WindowBase {
      public:
        explicit Window(WM wm) noexcept;
        ~Window() noexcept;

        Window(const Window&) noexcept;
        auto operator=(const Window&) noexcept -> Window&;

        Window(Window&&) noexcept;
        auto operator=(Window&&) noexcept -> Window&;

        auto open(string title, const math::uextent2& size, WindowFlag flags) noexcept
          -> void;
        auto close() noexcept -> void;

        auto handle_events() noexcept -> void;

        auto clear(const ucolor_rgb& color) noexcept -> void;
        auto fill_framebuffer(array_view<const ucolor_rgb> colors) noexcept -> void;

        auto set_title(string title) noexcept -> void;
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
        auto native_handle() const noexcept -> NativeHandle;

        auto update_geometry(const math::uextent2& extent) noexcept -> void;

        auto win32_state(this auto& self) noexcept -> decltype(auto);
        auto state(this auto& self) noexcept -> decltype(auto);

        auto gdi_frame_data(this auto& self) noexcept -> decltype(auto);

        auto gdiinit() noexcept -> void;

      private:
        struct {
            DWORD style;
            DWORD style_ex;

            bool external_context = false;
            bool mouse_inside     = false;
            bool resizing         = false;

            math::uextent2 extent;
            math::uextent2 last_extent;

            DWORD tls_index = 0;

            bool mouse_tracked = false;
        } m_win32_state;

        HWND m_window_handle = nullptr;

        using Hdc = RAIICapsule<HDC, CreateCompatibleDC, DeleteDC, struct HdcTag, nullptr>;
        using HBitmap
          = RAIICapsule<HBITMAP, CreateDIBSection, DeleteObject, struct HBitmapTag, nullptr>;

        struct GDIFrameData {
            GDIFrameData();

            GDIFrameData(const GDIFrameData&)                    = delete;
            auto operator=(const GDIFrameData&) -> GDIFrameData& = delete;

            GDIFrameData(GDIFrameData&&) noexcept;
            auto operator=(GDIFrameData&&) noexcept -> GDIFrameData&;

            ~GDIFrameData() noexcept;

            Hdc                context    = Hdc::empty();
            HBitmap            bitmap     = HBitmap::empty();
            std::atomic<void*> pixels_ptr = nullptr;

            math::extent2<LONG> extent;
        } m_gdi_frame_data;
    };
} // namespace stormkit::wsi::win32

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::wsi::win32 {
    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto Window::is_mouse_inside() const noexcept -> bool {
        return m_win32_state.mouse_inside;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::set_mouse_inside(bool inside) noexcept -> void {
        m_win32_state.mouse_inside = inside;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto Window::native_handle() const noexcept -> NativeHandle {
        return std::bit_cast<NativeHandle>(m_window_handle);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::update_geometry(const math::uextent2& extent) noexcept -> void {
        m_state.extent       = extent;
        m_win32_state.extent = extent;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::begin_resize() noexcept -> void {
        m_win32_state.resizing = true;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::resize(u32 width, u32 height) noexcept -> void {
        m_win32_state.last_extent.width  = width;
        m_win32_state.last_extent.height = height;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::end_resize() noexcept -> void {
        m_win32_state.resizing = false;

        if (m_win32_state.last_extent != extent()) {
            m_win32_state.last_extent = extent();

            resized_event({ m_win32_state.last_extent.width, m_win32_state.last_extent.height });
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::win32_state(this auto& self) noexcept -> decltype(auto) {
        return std::forward_like<decltype(self)>(self.m_win32_state);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::state(this auto& self) noexcept -> decltype(auto) {
        return std::forward_like<decltype(self)>(self.m_state);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::gdi_frame_data(this auto& self) noexcept -> decltype(auto) {
        return std::forward_like<decltype(self)>(self.m_gdi_frame_data);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline Window::GDIFrameData::GDIFrameData() = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline Window::GDIFrameData::GDIFrameData(GDIFrameData&& other) noexcept
        : context { std::move(other.context) }, bitmap { std::move(other.bitmap) },
          pixels_ptr { other.pixels_ptr.load() }, extent { other.extent } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::GDIFrameData::operator=(GDIFrameData&& other) noexcept -> GDIFrameData& {
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
    inline Window::GDIFrameData::~GDIFrameData() noexcept = default;

} // namespace stormkit::wsi::win32
