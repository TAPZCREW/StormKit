// Copyright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#include <xkbcommon/xkbcommon.h>

#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <xcb/xcb_keysyms.h>
STORMKIT_PUSH_WARNINGS
#pragma GCC diagnostic ignored "-Wkeyword-macro"
#define explicit _explicit
#include <xcb/xkb.h>
#undef explicit
STORMKIT_POP_WARNINGS

export module stormkit.wsi:linux.x11.window;

import std;

import stormkit.core;
import stormkit.wsi;

import :common.window_base;

import :linux.common.xkb;

import :linux.x11.context;
import :linux.x11.xcb;
import :linux.x11.log;
import :linux.x11.utils;

namespace stdr = std::ranges;

export namespace stormkit::wsi::linux::x11 {
    namespace xcb {
        template<auto Destructor>
        inline constexpr auto XCB_DELETER = [](auto val) {
            auto& globals = get_globals();

            Destructor(globals.connection, val);
        };
        using Window
          = RAIICapsule<xcb_window_t, xcb_generate_id, XCB_DELETER<xcb_destroy_window>, struct WindowTag, XCB_WINDOW_NONE>;
        using ColorMap
          = RAIICapsule<xcb_colormap_t, xcb_generate_id, XCB_DELETER<xcb_free_colormap>, struct ColorMapTag, XCB_NONE>;
        using GraphicsContext
          = RAIICapsule<xcb_gcontext_t, xcb_generate_id, XCB_DELETER<xcb_free_gc>, struct GraphicsContextTag, XCB_NONE>;
        using Image  = RAIICapsule<xcb_image_t*, xcb_image_create_native, xcb_image_destroy, struct ImageTag, nullptr>;
        using Pixmap = RAIICapsule<xcb_pixmap_t, xcb_generate_id, XCB_DELETER<xcb_free_pixmap>, struct PixmapTag, XCB_NONE>;
    } // namespace xcb

    class Window: public stormkit::wsi::common::WindowBase {
      public:
        struct Handles {
            xcb_connection_t*  connection;
            xcb_window_t       window;
            xcb_key_symbols_t* key_symbols;
            xkb_state*         state;
        };

        Window() noexcept;
        ~Window() noexcept;

        Window(const Window&) noexcept                    = delete;
        auto operator=(const Window&) noexcept -> Window& = delete;

        Window(Window&&) noexcept;
        auto operator=(Window&&) noexcept -> Window&;

        auto open(string title, const math::uextent2& size, WindowFlag flags) noexcept -> void;
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
        auto native_handle() const noexcept -> NativeHandle;

      private:
        auto process_events(xcb_generic_event_t* xevent) -> void;
        auto update_keymap() -> void;

        auto handle_key_event(xcb_keycode_t keycode, bool up) noexcept -> void;

        auto update_framebuffer() noexcept -> void;

        int m_xi_opcode = 0;

        Handles m_handles;

        xcb::Window          m_window           = xcb::Window::empty();
        xcb::ColorMap        m_color_map        = xcb::ColorMap::empty();
        xcb::GraphicsContext m_graphics_context = xcb::GraphicsContext::empty();
        xcb::Image           m_image            = xcb::Image::empty();
        array_view<u32>      m_framebuffer;
        xcb::Pixmap          m_pixmap      = xcb::Pixmap::empty();
        xcb::KeySymbols      m_key_symbols = xcb::KeySymbols::empty();
        common::xkb::Keymap  m_keymap      = common::xkb::Keymap::empty();
        common::xkb::State   m_xkb_state   = common::xkb::State::empty();
        common::xkb::Mods    m_mods;
        f32                  m_dpi = 1.f;
    };
} // namespace stormkit::wsi::linux::x11

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::wsi::linux::x11 {
    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline Window::Window() noexcept {
        xcb::init();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline Window::Window(Window&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::operator=(Window&&) noexcept -> Window& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::is_mouse_confined(u8) const noexcept -> bool {
        return m_mouse_states[GLOBAL_MOUSE_ID].confined;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto Window::is_mouse_locked(u8) const noexcept -> bool {
        return m_mouse_states[GLOBAL_MOUSE_ID].locked;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto Window::is_mouse_hidden(u8) const noexcept -> bool {
        return m_mouse_states[GLOBAL_MOUSE_ID].hidden;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto Window::is_mouse_relative(u8) const noexcept -> bool {
        return m_mouse_states[GLOBAL_MOUSE_ID].relative;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto Window::is_key_repeat_enabled(u8) const noexcept -> bool {
        return m_keyboard_states[GLOBAL_KEYBOARD_ID].key_repeat;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto Window::is_virtual_keyboard_visible() const noexcept -> bool {
        return false;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto Window::native_handle() const noexcept -> NativeHandle {
        return std::bit_cast<NativeHandle>(&m_handles);
    }
} // namespace stormkit::wsi::linux::x11
