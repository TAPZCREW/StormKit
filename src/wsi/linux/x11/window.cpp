// Copyright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

extern "C" {
#include <xkbcommon/xkbcommon-x11.h>
#include <xkbcommon/xkbcommon.h>

#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_image.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xfixes.h>
#include <xcb/xinput.h>

    STORMKIT_PUSH_WARNINGS
#pragma GCC diagnostic ignored "-Wkeyword-macro"
#define explicit _explicit
#include <xcb/xkb.h>
#undef explicit
    STORMKIT_POP_WARNINGS
}

module stormkit.wsi;

import std;

import stormkit.core;

import :linux.common.xkb;

import :linux.x11.window;
import :linux.x11.context;
import :linux.x11.xcb;
import :linux.x11.log;
import :linux.x11.utils;

namespace stdr = std::ranges;
namespace stdv = std::views;

namespace stormkit::wsi::linux::x11 {
    namespace {
        [[maybe_unused]]
        constexpr auto WM_CLASS                = string_view("WM_CLASS");
        constexpr auto WM_HINTS_STR            = string_view("_MOTIF_WM_HINTS");
        constexpr auto WM_PROTOCOLS            = string_view("WM_PROTOCOLS");
        constexpr auto WM_DELETE_WINDOW        = string_view("WM_DELETE_WINDOW");
        constexpr auto WM_STATE_STR            = string_view("_NET_WM_STATE");
        constexpr auto WM_STATE_FULLSCREEN_STR = string_view("_NET_WM_STATE_FULLSCREEN");
        constexpr auto WM_STATE_HIDDEN_STR     = string_view("_NET_WM_STATE_HIDDEN");

        constexpr auto MWM_HINTS_FUNCTIONS   = 1 << 0;
        constexpr auto MWM_HINTS_DECORATIONS = 1 << 1;

        constexpr auto MWM_DECOR_BORDER = 1 << 1;
        constexpr auto MWM_DECOR_RESIZE = 1 << 2;
        constexpr auto MWM_DECOR_TITLE  = 1 << 3;
        constexpr auto MWM_DECOR_MENU   = 1 << 4;
        [[maybe_unused]]
        constexpr auto MWM_DECOR_MINIMIZE = 1 << 5;
        [[maybe_unused]]
        constexpr auto MWM_DECOR_MAXIMIZE = 1 << 6;

        constexpr auto MWM_FUNC_RESIZE = 1 << 1;
        constexpr auto MWM_FUNC_MOVE   = 1 << 2;
        [[maybe_unused]]
        constexpr auto MWM_FUNC_MINIMIZE = 1 << 3;
        constexpr auto MWM_FUNC_MAXIMIZE = 1 << 4;
        constexpr auto MWM_FUNC_CLOSE    = 1 << 5;

        constexpr auto _NET_WM_STATE_REMOVE = 0; // remove/unset property
        constexpr auto _NET_WM_STATE_ADD    = 1; // add/set property
        [[maybe_unused]]
        constexpr auto _NET_WM_STATE_TOGGLE = 2; // toggle property

        constexpr auto MOUSE_RAW_EVENTS = u32 {
            XCB_INPUT_XI_EVENT_MASK_RAW_BUTTON_PRESS
            | XCB_INPUT_XI_EVENT_MASK_RAW_BUTTON_RELEASE
            | XCB_INPUT_XI_EVENT_MASK_RAW_MOTION
        };
        constexpr auto KEYBOARD_RAW_EVENTS = u32 {
            XCB_INPUT_XI_EVENT_MASK_RAW_KEY_PRESS | XCB_INPUT_XI_EVENT_MASK_RAW_KEY_RELEASE
        };
        constexpr auto KEYBOARD_EVENTS = u32 { XCB_INPUT_XI_EVENT_MASK_KEY_PRESS | XCB_INPUT_XI_EVENT_MASK_KEY_RELEASE };

        constexpr auto XINPUT_MASK_MODIFIERS = u32 { XCB_INPUT_MODIFIER_MASK_ANY };

        constexpr const auto
          EVENTS = XCB_EVENT_MASK_FOCUS_CHANGE
                   | XCB_EVENT_MASK_BUTTON_PRESS
                   | XCB_EVENT_MASK_BUTTON_RELEASE
                   | XCB_EVENT_MASK_BUTTON_MOTION
                   | XCB_EVENT_MASK_POINTER_MOTION
                   | XCB_EVENT_MASK_KEY_PRESS
                   | XCB_EVENT_MASK_KEY_RELEASE
                   | XCB_EVENT_MASK_STRUCTURE_NOTIFY
                   | XCB_EVENT_MASK_ENTER_WINDOW
                   | XCB_EVENT_MASK_LEAVE_WINDOW
                   | XCB_EVENT_MASK_VISIBILITY_CHANGE
                   | XCB_EVENT_MASK_PROPERTY_CHANGE
                   | XCB_EVENT_MASK_EXPOSURE;

        constexpr auto REQUIRED_MAP_PARTS = u16 {
            XCB_XKB_MAP_PART_KEY_TYPES
            | XCB_XKB_MAP_PART_KEY_SYMS
            | XCB_XKB_MAP_PART_MODIFIER_MAP
            | XCB_XKB_MAP_PART_EXPLICIT_COMPONENTS
            | XCB_XKB_MAP_PART_KEY_ACTIONS
            | XCB_XKB_MAP_PART_KEY_BEHAVIORS
            | XCB_XKB_MAP_PART_VIRTUAL_MODS
            | XCB_XKB_MAP_PART_VIRTUAL_MOD_MAP
        };

        constexpr auto REQUIRED_EVENTS = u16 {
            XCB_XKB_EVENT_TYPE_NEW_KEYBOARD_NOTIFY | XCB_XKB_EVENT_TYPE_MAP_NOTIFY | XCB_XKB_EVENT_TYPE_STATE_NOTIFY
        };

    } // namespace

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_PURE STORMKIT_FORCE_INLINE
    inline auto is_ext_event(xcb_generic_event_t* event, int opcode) noexcept -> bool {
        return std::bit_cast<xcb_ge_generic_event_t*>(event)->extension == opcode;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    Window::~Window() noexcept {
        xcb_flush(xcb::get_globals().connection);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::open(string title, const math::uextent2& extent, WindowFlag flags) noexcept -> void {
        const auto& connection = xcb::get_globals().connection;

        const auto screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
        m_window          = xcb::Window::create(connection);

        // m_dpi = xcb::get_xft_value<f32>("Xft.dpi").value_or(96.f) / 96.f;

        const auto [width, height] = extent;
        {
            m_color_map = xcb::ColorMap::create(connection);
            xcb_create_colormap(connection, XCB_COLORMAP_ALLOC_NONE, m_color_map, screen->root, screen->root_visual);
            const auto value_list = array<u32, 3> { screen->white_pixel, EVENTS, m_color_map };

            const auto cookie = xcb_create_window_checked(connection,
                                                          screen->root_depth,
                                                          m_window,
                                                          screen->root,
                                                          0,
                                                          0,
                                                          unchecked_narrow<u16>(width),
                                                          unchecked_narrow<u16>(height),
                                                          1,
                                                          XCB_WINDOW_CLASS_INPUT_OUTPUT,
                                                          screen->root_visual,
                                                          XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK | XCB_CW_COLORMAP,
                                                          stdr::data(value_list));

            auto error = xcb_request_check(connection, cookie);
            if (error) [[unlikely]] {
                elog("Failed to create window\n    > reason: {}", xcb::get_error(as_ref_mut(error)));
                return;
            }
        }

        {
            using Reply = RAIICapsule<xcb_get_geometry_reply_t*, xcb_get_geometry_reply, std::free, struct XCBGeometryReplyTag>;

            const auto cookie = xcb_get_geometry(connection, m_window);
            const auto reply  = Reply::create(connection, cookie, nullptr);
            ensures(reply != nullptr, "Failed to query window geometry");
            m_state.extent.width  = reply.handle()->width;
            m_state.extent.height = reply.handle()->height;
        }

        {
            using Reply = RAIICapsule<xcb_get_geometry_reply_t*, xcb_get_geometry_reply, std::free, struct XCBGeometryReplyTag>;

            const auto cookie = xcb_get_geometry(connection, m_window);
            const auto reply  = Reply::create(connection, cookie, nullptr);
            ensures(reply != nullptr, "Failed to query window geometry");
            m_state.extent.width  = reply.handle()->width;
            m_state.extent.height = reply.handle()->height;
        }

        // init key_symbol map, this is needed to extract the keysymbol from event
        m_key_symbols = xcb::KeySymbols::create(connection);
        ensures(m_key_symbols, "Failed to initialize XKB symbols map");

        auto xkb_ext_reply = xcb_get_extension_data(connection, &xcb_xkb_id);
        ensures(xkb_ext_reply, "Failed to get XCB_XKB extension data");
        {
            static constexpr auto major_version = XCB_XKB_MAJOR_VERSION;
            static constexpr auto minor_version = XCB_XKB_MINOR_VERSION;

            using Reply       = RAIICapsule<xcb_xkb_use_extension_reply_t*,
                                            xcb_xkb_use_extension_reply,
                                            std::free,
                                            struct XCBXKBReplyTag>;
            const auto cookie = xcb_xkb_use_extension(connection, major_version, minor_version);
            const auto reply  = Reply::create(connection, cookie, nullptr);

            ensures(reply != nullptr, "Failed to enable XCB_XKB extension");
            ensures(reply.handle()->supported, "The XCB_XKB extension is not supported on this X server");
            dlog("XCB_XKB extension loaded, version: {}.{}", major_version, minor_version);
        }

        auto xfixes_ext_reply = xcb_get_extension_data(connection, &xcb_xfixes_id);
        ensures(xfixes_ext_reply, "Failed to enable XCB_XFIXES extension");
        {
            using Reply = RAIICapsule<xcb_xfixes_query_version_reply_t*,
                                      xcb_xfixes_query_version_reply,
                                      std::free,
                                      struct XFixesQueryTag>;

            const auto cookie = xcb_xfixes_query_version(connection, 4, 0);
            const auto reply  = Reply::create(connection, cookie, nullptr);

            ensures(reply != nullptr, "Failed to query XCB_XFIXES extension version");
            dlog("XCB_XFIXES extension loaded, version: {}.{}", reply.handle()->major_version, reply.handle()->minor_version);
        }

        auto xcb_input_ext_reply = xcb_get_extension_data(connection, &xcb_input_id);
        ensures(xcb_input_ext_reply, "Failed to enable XCB_INPUT extension");
        {
            using Reply = RAIICapsule<xcb_input_xi_query_version_reply_t*,
                                      xcb_input_xi_query_version_reply,
                                      std::free,
                                      struct InputXIQueryTag>;

            const auto cookie = xcb_input_xi_query_version(connection, 2, XCB_INPUT_MINOR_VERSION);
            const auto reply  = Reply::create(connection, cookie, nullptr);

            ensures(reply != nullptr, "Failed to query XCB_INPUT_XI extension version");
            dlog("XCB_INPUT_XI extension loaded, version: {}.{}", reply.handle()->major_version, reply.handle()->minor_version);

            m_xi_opcode = xcb_input_ext_reply->major_opcode;
        }

        update_keymap();

        // XKB events are reported to all interested clients without regard
        // to the current keyboard input focus or grab state
        xcb_xkb_select_events_checked(connection,
                                      XCB_XKB_ID_USE_CORE_KBD,
                                      REQUIRED_EVENTS,
                                      0,
                                      REQUIRED_EVENTS,
                                      REQUIRED_MAP_PARTS,
                                      REQUIRED_MAP_PARTS,
                                      nullptr);

        constexpr auto MASK = KEYBOARD_EVENTS | MOUSE_RAW_EVENTS;
        xcb_input_xi_passive_grab_device(connection,
                                         XCB_CURRENT_TIME,
                                         m_window,
                                         XCB_CURSOR_NONE,
                                         0,
                                         XCB_INPUT_DEVICE_ALL,
                                         1,
                                         1,
                                         XCB_INPUT_GRAB_TYPE_BUTTON | XCB_INPUT_GRAB_TYPE_KEYCODE,
                                         XCB_INPUT_GRAB_MODE_22_ASYNC,
                                         XCB_INPUT_GRAB_MODE_22_ASYNC,
                                         XCB_INPUT_GRAB_OWNER_NO_OWNER,
                                         &MASK,
                                         &XINPUT_MASK_MODIFIERS);

        set_title(std::move(title));

        struct WindowHints {
            uint32_t flags       = 0;
            uint32_t functions   = 0;
            uint32_t decorations = 0;
            int32_t  input_mode  = 0;
            uint32_t state       = 0;
        } window_hints;

        window_hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;

        if (not has_flag_bit(flags, WindowFlag::BORDERLESS)) {
            window_hints.decorations |= MWM_DECOR_BORDER | MWM_DECOR_TITLE | MWM_DECOR_MENU;
            window_hints.functions |= MWM_FUNC_MOVE | MWM_FUNC_CLOSE;
        }

        if (has_flag_bit(flags, WindowFlag::RESIZEABLE)) {
            window_hints.decorations |= MWM_DECOR_RESIZE | MWM_DECOR_MAXIMIZE;
            window_hints.functions |= MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE;
        } else {
            auto size_hints = xcb_size_hints_t {};

            xcb_icccm_size_hints_set_min_size(&size_hints, as<i32>(width), as<i32>(height));
            xcb_icccm_size_hints_set_max_size(&size_hints, as<i32>(width), as<i32>(height));

            xcb_icccm_set_wm_normal_hints(connection, m_window, &size_hints);
        }

        auto _ = xcb::get_atom(WM_CLASS, false)
                   .transform([this, &connection](auto&& atom) noexcept {
                       constexpr auto CLASS_NAME = "StormKit.Window\0StormKit.Window";
                       xcb_change_property(connection, XCB_PROP_MODE_REPLACE, m_window, atom, atom, 8, 32, CLASS_NAME);
                   })
                   .transform_error(xcb::atom_error(WM_HINTS_STR));

        auto _ = xcb::get_atom(WM_HINTS_STR, false)
                   .transform([this, &window_hints, &connection](auto&& atom) noexcept {
                       xcb_change_property(connection, XCB_PROP_MODE_REPLACE, m_window, atom, atom, 32, 5, &window_hints);
                   })
                   .transform_error(xcb::atom_error(WM_HINTS_STR));

        m_handles.connection  = connection;
        m_handles.window      = m_window;
        m_handles.key_symbols = m_key_symbols;

        auto close_atom    = xcb::get_atom(WM_DELETE_WINDOW, false);
        auto protocol_atom = xcb::get_atom(WM_PROTOCOLS, true);
        if (!protocol_atom) [[unlikely]]
            xcb::atom_error(WM_PROTOCOLS)(protocol_atom.error());
        else if (!close_atom) [[unlikely]]
            xcb::atom_error(WM_DELETE_WINDOW)(close_atom.error());
        else
            xcb_change_property(connection,
                                XCB_PROP_MODE_REPLACE,
                                m_window,
                                *protocol_atom,
                                XCB_ATOM_ATOM,
                                32,
                                1,
                                &(*close_atom));

        auto _ = xcb::get_atom(WM_STATE_STR, false)
                   .transform([this, &connection](auto&& atom) noexcept {
                       xcb_change_property(connection, XCB_PROP_MODE_REPLACE, m_window, atom, XCB_ATOM_ATOM, 32, 0, nullptr);
                   })
                   .transform_error(xcb::atom_error(WM_STATE_STR));

        auto _ = xcb::get_atom(WM_STATE_HIDDEN_STR, false).transform_error(xcb::atom_error(WM_STATE_HIDDEN_STR));

        xcb_map_window(connection, m_window);

        xcb_flush(connection);

        if (not has_flag_bit(flags, WindowFlag::EXTERNAL_CONTEXT)) {
            m_graphics_context = xcb::GraphicsContext::create(connection);
            const auto values  = array<u32, 3> { screen->white_pixel, screen->black_pixel, 0_u32 };
            xcb_create_gc(connection,
                          m_graphics_context,
                          m_window,
                          XCB_GC_BACKGROUND | XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES,
                          stdr::data(values));

            update_framebuffer();
        }

        xcb_flush(connection);

        m_state.title = std::move(title);
        m_state.open  = true;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::close() noexcept -> void {
        const auto& connection = xcb::get_globals().connection;
        if (m_window) xcb_unmap_window(connection, m_window);

        m_image            = xcb::Image::empty();
        m_framebuffer      = {};
        m_graphics_context = xcb::GraphicsContext::empty();

        m_window.reset();
        m_color_map.reset();
        m_handles = {};

        m_key_symbols.reset();
        m_keymap.reset();
        m_xkb_state.reset();
        m_mods = {};

        m_xi_opcode       = 0;
        m_dpi             = 1.f;
        m_keyboard_states = {};
        m_mouse_states    = {};
        m_state           = {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::handle_events() noexcept -> void {
        using Event = RAIICapsule<xcb_generic_event_t*, xcb_poll_for_event, std::free, struct EventTag>;

        auto& globals = xcb::get_globals();

        for (auto xevent = Event::create(globals.connection); xevent; xevent.reset(xcb_poll_for_event(globals.connection)))
            process_events(xevent);

        if (m_mouse_states[GLOBAL_MOUSE_ID].locked) {
            const auto locked_at = m_mouse_states[GLOBAL_MOUSE_ID].locked_at.to<f32>();
            xcb_warp_pointer(globals.connection, XCB_NONE, m_window, 0, 0, 0, 0, as<i16>(locked_at.x), as<i16>(locked_at.y));

            if (m_mouse_states[GLOBAL_MOUSE_ID].hidden) xcb_xfixes_hide_cursor(globals.connection, m_window);
            else
                xcb_xfixes_show_cursor(globals.connection, m_window);
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::clear(const ucolor_rgb& color) noexcept -> void {
        expects(m_graphics_context, "clear called on a window opened with EXTERNAL_CONTEXT flag");
        const auto _color = (255_u32 << 24) | as<u32>(color.r) << 16 | as<u32>(color.g) << 8 | color.b;
        stdr::fill(m_framebuffer, _color);

        const auto& connection = xcb::get_globals().connection;
        const auto  cookie     = xcb_image_put(connection, m_window, m_graphics_context, m_image, 0, 0, 0);
        auto        error      = xcb_request_check(connection, cookie);
        if (error) [[unlikely]] { elog("Failed to copy image\n    > reason: {}", xcb::get_error(as_ref_mut(error))); }
        // dlog("cleared");
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::fill_framebuffer(array_view<const ucolor_rgb> pixels) noexcept -> void {
        expects(m_graphics_context, "fill_framebuffer called on a window opened with EXTERNAL_CONTEXT flag");
        const auto count = std::min(stdr::size(pixels), stdr::size(m_framebuffer));
        stdr::copy(pixels | stdv::take(count) | stdv::transform([](const auto& col) static noexcept {
                       return (255_u32 << 24) | as<u32>(col.r) << 16 | as<u32>(col.g) << 8 | col.b;
                   }),
                   stdr::begin(m_framebuffer));

        const auto& connection = xcb::get_globals().connection;
        const auto  cookie     = xcb_image_put(connection, m_window, m_graphics_context, m_image, 0, 0, 0);
        auto        error      = xcb_request_check(connection, cookie);
        if (error) [[unlikely]]
            elog("Failed to copy image\n    > reason: {}", xcb::get_error(as_ref_mut(error)));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_title(string title) noexcept -> void {
        const auto& globals = xcb::get_globals();

        xcb_change_property(globals.connection,
                            XCB_PROP_MODE_REPLACE,
                            m_window,
                            XCB_ATOM_WM_NAME,
                            XCB_ATOM_STRING,
                            8,
                            as<u32>(title.length()),
                            title.c_str());

        xcb_flush(globals.connection);

        m_state.title = std::move(title);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_extent(const math::uextent2& extent) noexcept -> void {
        auto& globals = xcb::get_globals();

        const auto mask   = XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT;
        const auto width  = as<f32>(extent.width);
        const auto height = as<f32>(extent.height);
        const auto values = array<i32, 2> { as<i32>(width), as<i32>(height) };

        xcb_configure_window(globals.connection, m_window, mask, stdr::data(values));

        xcb_flush(globals.connection);

        m_state.extent = extent;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_fullscreen(bool enabled) noexcept -> void {
        auto _ = xcb::get_atom(WM_STATE_FULLSCREEN_STR, false)
                   .transform_error(xcb::atom_error(WM_STATE_FULLSCREEN_STR))
                   .and_then([](auto&& fullscreen_atom) static noexcept {
                       return xcb::get_atom(WM_STATE_STR, false).transform(monadic::as_tuple(std::move(fullscreen_atom)));
                   })
                   .transform_error(xcb::atom_error(WM_STATE_STR))
                   .transform(monadic::unpack_tuple_to([this, enabled](auto&& fullscreen_atom, auto&& state_atom) {
                       auto& globals     = xcb::get_globals();
                       auto  ev          = xcb_client_message_event_t {};
                       ev.response_type  = XCB_CLIENT_MESSAGE;
                       ev.type           = state_atom;
                       ev.format         = 32;
                       ev.window         = m_window;
                       ev.data.data32[0] = enabled ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;
                       ev.data.data32[1] = fullscreen_atom;
                       ev.data.data32[2] = XCB_ATOM_NONE;
                       ev.data.data32[3] = 0;
                       ev.data.data32[4] = 0;

                       xcb_send_event(globals.connection,
                                      1,
                                      m_window,
                                      XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY,
                                      std::bit_cast<const char*>(&ev));

                       xcb_flush(globals.connection);
                       m_state.fullscreen = enabled;
                   }));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::confine_mouse(bool confined, u8) noexcept -> void {
        auto& globals = xcb::get_globals();

        if (confined) {
            auto cookie = xcb_grab_pointer(globals.connection,
                                           1,
                                           m_window,
                                           XCB_EVENT_MASK_BUTTON_PRESS
                                             | XCB_EVENT_MASK_BUTTON_RELEASE
                                             | XCB_EVENT_MASK_BUTTON_MOTION
                                             | XCB_EVENT_MASK_POINTER_MOTION,
                                           XCB_GRAB_MODE_ASYNC,
                                           XCB_GRAB_MODE_ASYNC,
                                           m_window,
                                           XCB_NONE,
                                           XCB_CURRENT_TIME);
            xcb_grab_pointer_reply(globals.connection, cookie, nullptr);
        } else
            xcb_ungrab_pointer(globals.connection, XCB_CURRENT_TIME);

        if (m_mouse_states[GLOBAL_MOUSE_ID].hidden) xcb_xfixes_hide_cursor(globals.connection, m_window);
        else
            xcb_xfixes_show_cursor(globals.connection, m_window);

        xcb_flush(globals.connection);

        m_mouse_states[GLOBAL_MOUSE_ID].confined = confined;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::lock_mouse(bool locked, u8) noexcept -> void {
        auto& globals = xcb::get_globals();

        if (locked) {
            m_mouse_states[GLOBAL_MOUSE_ID].locked_at = m_mouse_states[GLOBAL_MOUSE_ID].last_position;
            const auto locked_at                      = m_mouse_states[GLOBAL_MOUSE_ID].locked_at.to<f32>();
            xcb_warp_pointer(globals.connection, XCB_NONE, m_window, 0, 0, 0, 0, as<i16>(locked_at.x), as<i16>(locked_at.y));
        }

        if (m_mouse_states[GLOBAL_MOUSE_ID].hidden) xcb_xfixes_hide_cursor(globals.connection, m_window);
        else
            xcb_xfixes_show_cursor(globals.connection, m_window);

        xcb_flush(globals.connection);
        m_mouse_states[GLOBAL_MOUSE_ID].locked = locked;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::hide_mouse(bool hidden, u8) noexcept -> void {
        auto& globals = xcb::get_globals();

        if (hidden) xcb_xfixes_hide_cursor(globals.connection, m_window);
        else
            xcb_xfixes_show_cursor(globals.connection, m_window);

        xcb_flush(globals.connection);

        m_mouse_states[GLOBAL_MOUSE_ID].hidden = hidden;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_relative_mouse(bool relative, u8) noexcept -> void {
        m_mouse_states[GLOBAL_MOUSE_ID].relative = relative;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_key_repeat(bool enabled, u8) noexcept -> void {
        auto& globals = xcb::get_globals();

        const auto MASK = ((enabled) ? KEYBOARD_EVENTS : KEYBOARD_RAW_EVENTS) | MOUSE_RAW_EVENTS;

        xcb_input_xi_passive_grab_device(globals.connection,
                                         XCB_CURRENT_TIME,
                                         m_window,
                                         XCB_CURSOR_NONE,
                                         0,
                                         XCB_INPUT_DEVICE_ALL,
                                         1,
                                         1,
                                         XCB_INPUT_GRAB_TYPE_KEYCODE,
                                         XCB_INPUT_GRAB_MODE_22_ASYNC,
                                         XCB_INPUT_GRAB_MODE_22_ASYNC,
                                         XCB_INPUT_GRAB_OWNER_NO_OWNER,
                                         &MASK,
                                         &XINPUT_MASK_MODIFIERS);

        xcb_flush(globals.connection);

        m_keyboard_states[GLOBAL_KEYBOARD_ID].key_repeat = enabled;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::show_virtual_keyboard(bool) noexcept -> void {
        elog("x11::Window::show_virtual_keyboard isn't yet implemented");
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_mouse_position(const math::ivec2& position, u8) noexcept -> void {
        auto& globals = xcb::get_globals();

        const auto _position = position.to<f32>();
        xcb_warp_pointer(globals.connection, XCB_NONE, m_window, 0, 0, 0, 0, as<i16>(_position.x), as<i16>(_position.y));

        xcb_flush(globals.connection);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::process_events(xcb_generic_event_t* event) -> void {
        if (not event) return;

        dlog("process_events");

        auto*      xevent        = event;
        const auto response_type = xevent->response_type & ~0x80;

        switch (response_type) {
            case XCB_KEY_PRESS: {
                auto kevent = std::bit_cast<xcb_key_press_event_t*>(xevent);
                handle_key_event(kevent->detail, true);
            } break;
            case XCB_KEY_RELEASE: {
                auto kevent = std::bit_cast<xcb_key_release_event_t*>(xevent);
                handle_key_event(kevent->detail, false);
            } break;
            case XCB_MOTION_NOTIFY: {
                auto mouse_event = std::bit_cast<xcb_motion_notify_event_t*>(xevent);

                const auto x = unchecked_narrow<u32>(mouse_event->event_x);
                const auto y = unchecked_narrow<u32>(mouse_event->event_y);

                if (m_mouse_states[GLOBAL_MOUSE_ID].locked)
                    if (x == m_mouse_states[GLOBAL_MOUSE_ID].locked_at.x and y == m_mouse_states[GLOBAL_MOUSE_ID].locked_at.y)
                        break;

                if (m_mouse_states[GLOBAL_MOUSE_ID].relative) {
                    const auto dx = x - m_mouse_states[GLOBAL_MOUSE_ID].last_position.x;
                    const auto dy = y - m_mouse_states[GLOBAL_MOUSE_ID].last_position.y;

                    mouse_moved_event(GLOBAL_MOUSE_ID, math::vec2 { dx, dy }.to<i32>());
                } else
                    mouse_moved_event(GLOBAL_MOUSE_ID, math::vec2 { x, y }.to<i32>());

                m_mouse_states[GLOBAL_MOUSE_ID].last_position.x = x;
                m_mouse_states[GLOBAL_MOUSE_ID].last_position.y = y;

            } break;
            case XCB_BUTTON_PRESS: {
                auto button_event = std::bit_cast<xcb_button_press_event_t*>(xevent);

                auto button = button_event->detail;
                mouse_button_down_event(GLOBAL_MOUSE_ID,
                                        x11_button_to_stormkit(button),
                                        math::vec2 { button_event->event_x, button_event->event_y }.to<i32>());
            } break;
            case XCB_BUTTON_RELEASE: {
                auto button_event = std::bit_cast<xcb_button_press_event_t*>(xevent);

                auto button = button_event->detail;
                mouse_button_up_event(GLOBAL_MOUSE_ID,
                                      x11_button_to_stormkit(button),
                                      math::vec2 { button_event->event_x, button_event->event_y }.to<i32>());
            } break;
            case XCB_CONFIGURE_NOTIFY: {
                auto configure_event = std::bit_cast<xcb_configure_notify_event_t*>(xevent);

                if ((configure_event->width != m_state.extent.width) || (configure_event->height != m_state.extent.height)) {
                    m_state.extent = math::extent2 { configure_event->width, configure_event->height }.narrow_to<u32>();

                    if (m_graphics_context) update_framebuffer();

                    resized_event(m_state.extent);
                }
            } break;
            case XCB_CLIENT_MESSAGE: {
                auto client_message_event = std::bit_cast<xcb_client_message_event_t*>(xevent);

                auto _ = xcb::get_atom(WM_DELETE_WINDOW, false)
                           .transform([this, &client_message_event](auto&& atom) noexcept {
                               if (client_message_event->data.data32[0] == atom) closed_event();
                           })
                           .transform_error(xcb::atom_error(WM_DELETE_WINDOW));
            } break;
            case XCB_MAPPING_NOTIFY: {
                auto mapping_notify_event = std::bit_cast<xcb_mapping_notify_event_t*>(xevent);

                if (mapping_notify_event->request != XCB_MAPPING_POINTER) {
                    xcb_refresh_keyboard_mapping(m_key_symbols, mapping_notify_event);
                    update_keymap();
                }
                break;
            } break;
            case XCB_PROPERTY_NOTIFY: {
                auto property_notify_event = std::bit_cast<xcb_property_notify_event_t*>(xevent);
                auto _ = xcb::get_atom(WM_STATE_STR, false).transform([this, property_notify_event](auto wm_state_atom) {
                    if (wm_state_atom == property_notify_event->atom) {
                        auto& globals = xcb::get_globals();
                        const auto
                          cookie = xcb_get_property(globals.connection, false, m_window, wm_state_atom, XCB_ATOM_ATOM, 0, 32);

                        auto       error = xcb::GenericError::empty();
                        auto       reply = xcb_get_property_reply(globals.connection, cookie, &error.handle());
                        const auto value = std::bit_cast<xcb_atom_t*>(xcb_get_property_value(reply));
                        if (value)
                            auto _ = xcb::get_atom(WM_STATE_HIDDEN_STR, false)
                                       .transform([this, &value](auto wm_state_hidden_atom) noexcept {
                                           if (*value == wm_state_hidden_atom) minimized_event();
                                       });
                    }
                });
            } break;
            case XCB_GE_GENERIC: {
                if (is_ext_event(xevent, m_xi_opcode)) {
                    auto xievent = std::bit_cast<xcb_ge_generic_event_t*>(xevent);
                    switch (xievent->event_type) {
                        case XCB_INPUT_KEY_PRESS: [[fallthrough]];
                        case XCB_INPUT_RAW_KEY_PRESS: {
                            auto _xievent = std::bit_cast<xcb_input_key_press_event_t*>(xevent);
                            handle_key_event(as<xcb_keycode_t>(_xievent->detail), true);
                        } break;
                        case XCB_INPUT_KEY_RELEASE: [[fallthrough]];
                        case XCB_INPUT_RAW_KEY_RELEASE: {
                            auto _xievent = std::bit_cast<xcb_input_key_press_event_t*>(xevent);
                            handle_key_event(as<xcb_keycode_t>(_xievent->detail), false);
                        } break;
                        case XCB_INPUT_BUTTON_PRESS: [[fallthrough]];
                        case XCB_INPUT_RAW_BUTTON_PRESS: {
                            auto button_event = std::bit_cast<xcb_input_button_press_event_t*>(xevent);

                            auto button = button_event->detail;
                            WindowBase::mouse_button_down_event(GLOBAL_MOUSE_ID,
                                                                x11_button_to_stormkit(as<xcb_button_t>(button)),
                                                                math::ivec2 { button_event->event_x, button_event->event_y });
                        } break;
                        case XCB_INPUT_BUTTON_RELEASE: [[fallthrough]];
                        case XCB_INPUT_RAW_BUTTON_RELEASE: {
                            auto button_event = std::bit_cast<xcb_input_button_release_event_t*>(xevent);

                            auto button = button_event->detail;
                            WindowBase::mouse_button_up_event(GLOBAL_MOUSE_ID,
                                                              x11_button_to_stormkit(as<xcb_button_t>(button)),
                                                              math::ivec2 { button_event->event_x, button_event->event_y });
                        } break;
                    }
                }
            } break;
            // case XCB_EXPOSE: {
            //     dlog("EXPOSE");
            //     if (m_graphics_context) {
            //         auto& connection           = xcb::get_globals().connection;
            //         const auto [width, height] = m_state.extent.to<u16>();
            //         xcb_copy_area(connection,
            //                       m_pixmap,
            //                       m_window,
            //                       m_graphics_context,
            //                       0,
            //                       0,
            //                       0,
            //                       0,
            //                       width,
            //                       height);
            //         xcb_flush(connection);
            //     }
            // } break;
            default: dlog("UNHANDLED"); break;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::update_keymap() -> void {
        auto& globals = xcb::get_globals();

        const auto device_id = xkb_x11_get_core_keyboard_device_id(globals.connection);

        m_keymap = common::xkb::Keymap::
          take(xkb_x11_keymap_new_from_device(globals.xkb_context, globals.connection, device_id, XKB_KEYMAP_COMPILE_NO_FLAGS));
        if (not m_keymap) {
            elog("Failed to compile a keymap");
            return;
        }

        m_xkb_state = common::xkb::State::take(xkb_x11_state_new_from_device(m_keymap, globals.connection, device_id));
        if (not m_xkb_state) {
            elog("Failed to create XKB state");
            return;
        }

        m_handles.state = m_xkb_state;

        m_mods = common::xkb::Mods {
            .shift   = xkb_keymap_mod_get_index(m_keymap, XKB_MOD_NAME_SHIFT),
            .lock    = xkb_keymap_mod_get_index(m_keymap, XKB_MOD_NAME_CAPS),
            .control = xkb_keymap_mod_get_index(m_keymap, XKB_MOD_NAME_CTRL),
            .mod1    = xkb_keymap_mod_get_index(m_keymap, "Mod1"),
            .mod2    = xkb_keymap_mod_get_index(m_keymap, "Mod2"),
            .mod3    = xkb_keymap_mod_get_index(m_keymap, "Mod3"),
            .mod4    = xkb_keymap_mod_get_index(m_keymap, "Mod4"),
            .mod5    = xkb_keymap_mod_get_index(m_keymap, "Mod5")
        };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::handle_key_event(xcb_keycode_t keycode, bool down) noexcept -> void {
        auto symbol = xkb_keysym_t {};

        auto character = char {};
        symbol         = xkb_state_key_get_one_sym(m_xkb_state, keycode);
        xkb_state_key_get_utf8(m_xkb_state, keycode, std::bit_cast<char*>(&character), sizeof(char));

        auto key = common::xkb_key_to_stormkit(symbol);

        if (down) key_down_event(GLOBAL_KEYBOARD_ID, key, character);
        else
            key_up_event(GLOBAL_KEYBOARD_ID, key, character);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::update_framebuffer() noexcept -> void {
        const auto& connection = xcb::get_globals().connection;
        const auto  screen     = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

        m_image.reset();

        const auto format          = XCB_IMAGE_FORMAT_Z_PIXMAP;
        const auto depth           = screen->root_depth;
        const auto [width, height] = m_state.extent.to<u16>();
        m_image                    = xcb::Image::create(connection, width, height, format, depth, nullptr, 0_u32, nullptr);

        m_framebuffer = { std::bit_cast<u32*>(m_image.handle()->data), m_image.handle()->size / 8 };

        xcb_flush(connection);

        dlog("updated framebuffer to extent: {}:{} with dpi: {} with size: {} {}",
             m_image.handle()->width,
             m_image.handle()->height,
             m_dpi,
             stdr::size(m_framebuffer),
             depth);
    }
} // namespace stormkit::wsi::linux::x11
