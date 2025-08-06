// Copyright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

extern "C" {
#include <xkbcommon/xkbcommon-x11.h>
#include <xkbcommon/xkbcommon.h>

#include <xcb/randr.h>
#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xcb_xrm.h>
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

import :linux.x11.window_impl;
import :linux.x11.context;
import :linux.x11.xcb;
import :linux.x11.log;
import :linux.x11.utils;

namespace stormkit::wsi::linux::x11 {
    namespace {
        [[maybe_unused]]
        constexpr auto WM_HINTS_STR            = std::string_view("_MOTIF_WM_HINTS");
        constexpr auto WM_PROTOCOLS            = std::string_view("WM_PROTOCOLS");
        constexpr auto WM_DELETE_WINDOW        = std::string_view("WM_DELETE_WINDOW");
        constexpr auto WM_STATE_STR            = std::string_view("_NET_WM_STATE");
        constexpr auto WM_STATE_FULLSCREEN_STR = std::string_view("_NET_WM_STATE_FULLSCREEN");

        constexpr auto MWM_HINTS_FUNCTIONS   = 1 << 0;
        constexpr auto MWM_HINTS_DECORATIONS = 1 << 1;

        constexpr auto MWM_DECOR_BORDER   = 1 << 1;
        constexpr auto MWM_DECOR_RESIZE   = 1 << 2;
        constexpr auto MWM_DECOR_TITLE    = 1 << 3;
        constexpr auto MWM_DECOR_MENU     = 1 << 4;
        constexpr auto MWM_DECOR_MINIMIZE = 1 << 5;
        constexpr auto MWM_DECOR_MAXIMIZE = 1 << 6;

        constexpr auto MWM_FUNC_RESIZE   = 1 << 1;
        constexpr auto MWM_FUNC_MOVE     = 1 << 2;
        constexpr auto MWM_FUNC_MINIMIZE = 1 << 3;
        constexpr auto MWM_FUNC_MAXIMIZE = 1 << 4;
        constexpr auto MWM_FUNC_CLOSE    = 1 << 5;

        constexpr auto _NET_WM_STATE_REMOVE = 0; // remove/unset property
        constexpr auto _NET_WM_STATE_ADD    = 1; // add/set property
        [[maybe_unused]]
        constexpr auto _NET_WM_STATE_TOGGLE = 2; // toggle property

        constexpr auto MOUSE_RAW_EVENTS    = u32 { XCB_INPUT_XI_EVENT_MASK_RAW_BUTTON_PRESS
                                                | XCB_INPUT_XI_EVENT_MASK_RAW_BUTTON_RELEASE
                                                | XCB_INPUT_XI_EVENT_MASK_RAW_MOTION };
        constexpr auto KEYBOARD_RAW_EVENTS = u32 { XCB_INPUT_XI_EVENT_MASK_RAW_KEY_PRESS
                                                   | XCB_INPUT_XI_EVENT_MASK_RAW_KEY_RELEASE };
        constexpr auto KEYBOARD_EVENTS     = u32 { XCB_INPUT_XI_EVENT_MASK_KEY_PRESS
                                               | XCB_INPUT_XI_EVENT_MASK_KEY_RELEASE };

        constexpr auto XINPUT_MASK_MODIFIERS = u32 { XCB_INPUT_MODIFIER_MASK_ANY };

        constexpr const auto EVENTS = XCB_EVENT_MASK_FOCUS_CHANGE
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

        constexpr auto REQUIRED_EVENTS = u16 { XCB_XKB_EVENT_TYPE_NEW_KEYBOARD_NOTIFY
                                               | XCB_XKB_EVENT_TYPE_MAP_NOTIFY
                                               | XCB_XKB_EVENT_TYPE_STATE_NOTIFY };

    } // namespace

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_CONST STORMKIT_FORCE_INLINE
    inline auto is_ext_event(xcb_generic_event_t* event, int opcode) noexcept -> bool {
        return std::bit_cast<xcb_ge_generic_event_t*>(event)->extension == opcode;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    WindowImpl::~WindowImpl() noexcept {
        xcb_flush(xcb::get_globals().connection);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::create(std::string               title,
                            const math::Extent2<u32>& extent,
                            WindowFlag                flags) noexcept -> void {
        auto& connection = xcb::get_globals().connection;

        auto screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
        m_window    = xcb::Window { std::in_place, connection };

        m_state.dpi = xcb::get_xft_value<f32>("Xft.dpi").value_or(96.f) / 96.f;

        const auto width  = extent.width * m_state.dpi;
        const auto height = extent.height * m_state.dpi;

        {
            u32 value_list[] = { screen->white_pixel, screen->black_pixel, EVENTS };

            auto cookie = xcb_create_window_checked(connection,
                                                    XCB_COPY_FROM_PARENT,
                                                    m_window,
                                                    screen->root,
                                                    0,
                                                    0,
                                                    as<u16>(width),
                                                    as<u16>(height),
                                                    1,
                                                    XCB_WINDOW_CLASS_INPUT_OUTPUT,
                                                    screen->root_visual,
                                                    XCB_CW_BACK_PIXEL
                                                      | XCB_CW_BORDER_PIXEL
                                                      | XCB_CW_EVENT_MASK,
                                                    value_list);

            auto error = xcb_request_check(connection, cookie);
            if (error) [[unlikely]] {
                m_window.reset();
                elog("Failed to create window\n    > reason: {}",
                     xcb::get_error(as_ref_mut(error)));
                return;
            }
        }

        m_state.extent             = extent;
        m_state.framebuffer_extent = extent;
        m_state.framebuffer_extent.width *= m_state.dpi;
        m_state.framebuffer_extent.height *= m_state.dpi;

        // init key_symbol map, this is needed to extract the keysymbol from event
        m_key_symbols = xcb::KeySymbols { std::in_place, connection };
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
            const auto reply  = Reply { std::in_place, connection, cookie, nullptr };

            ensures(reply != nullptr, "Failed to enable XCB_XKB extension");
            ensures(reply.handle()->supported,
                    "The XCB_XKB extension is not supported on this X server");
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
            const auto reply  = Reply { std::in_place, connection, cookie, nullptr };

            ensures(reply != nullptr, "Failed to query XCB_XFIXES extension version");
            dlog("XCB_XFIXES extension loaded, version: {}.{}",
                 reply.handle()->major_version,
                 reply.handle()->minor_version);
        }

        auto xcb_input_ext_reply = xcb_get_extension_data(connection, &xcb_input_id);
        ensures(xcb_input_ext_reply, "Failed to enable XCB_INPUT extension");
        {
            using Reply = RAIICapsule<xcb_input_xi_query_version_reply_t*,
                                      xcb_input_xi_query_version_reply,
                                      std::free,
                                      struct InputXIQueryTag>;

            const auto cookie = xcb_input_xi_query_version(connection, 2, XCB_INPUT_MINOR_VERSION);
            const auto reply  = Reply { std::in_place, connection, cookie, nullptr };

            ensures(reply != nullptr, "Failed to query XCB_INPUT_XI extension version");
            dlog("XCB_INPUT_XI extension loaded, version: {}.{}",
                 reply.handle()->major_version,
                 reply.handle()->minor_version);

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

        if (check_flag_bit(flags, WindowFlag::TITLE_BAR)) {
            window_hints.decorations |= MWM_DECOR_BORDER | MWM_DECOR_TITLE | MWM_DECOR_MENU;
            window_hints.functions |= MWM_FUNC_MOVE;
        }

        if (check_flag_bit(flags, WindowFlag::CLOSE)) {
            window_hints.decorations |= 0;
            window_hints.functions |= MWM_FUNC_CLOSE;
        }

        if (check_flag_bit(flags, WindowFlag::MINIMIZABLE)) {
            window_hints.decorations |= MWM_DECOR_MINIMIZE;
            window_hints.functions |= MWM_FUNC_MINIMIZE;
        }

        if (check_flag_bit(flags, WindowFlag::RESIZEABLE)) {
            window_hints.decorations |= MWM_DECOR_RESIZE | MWM_DECOR_MAXIMIZE;
            window_hints.functions |= MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE;
        } else {
            auto size_hints = xcb_size_hints_t {};

            xcb_icccm_size_hints_set_min_size(&size_hints, as<i32>(width), as<i32>(height));
            xcb_icccm_size_hints_set_max_size(&size_hints, as<i32>(width), as<i32>(height));

            xcb_icccm_set_wm_normal_hints(connection, m_window, &size_hints);
        }

        xcb::get_atom(WM_HINTS_STR, false)
          .transform([this, &window_hints, &connection](auto&& atom) noexcept {
              xcb_change_property(connection,
                                  XCB_PROP_MODE_REPLACE,
                                  m_window,
                                  atom,
                                  atom,
                                  32,
                                  5,
                                  &window_hints);
          })
          .transform_error(xcb::atom_error(WM_STATE_STR));

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

        xcb::get_atom(WM_STATE_STR, false)
          .transform([this, &connection](auto&& atom) noexcept {
              xcb_change_property(connection,
                                  XCB_PROP_MODE_REPLACE,
                                  m_window,
                                  atom,
                                  XCB_ATOM_ATOM,
                                  32,
                                  0,
                                  nullptr);
          })
          .transform_error(xcb::atom_error(WM_STATE_STR));

        xcb_map_window(connection, m_window);

        xcb_flush(connection);

        m_title = std::move(title);
        m_open  = true;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::close() noexcept -> void {
        m_window.reset();

        m_title.clear();
        m_open                     = false;
        m_key_repeat_enabled       = false;
        m_virtual_keyboard_visible = false;

        m_xi_opcode = 0;

        m_key_symbols.reset();

        m_state = {};

        m_keyboard_state = {};
        m_mouse_state    = {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::clear(const RGBColorU&) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::set_pixels_to(std::span<const RGBColorU>) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::poll_event(Event& event) noexcept -> bool {
        using Event = RAIICapsule<xcb_generic_event_t*,
                                  xcb_poll_for_event,
                                  std::free,
                                  struct EventTag>;

        auto& globals = xcb::get_globals();

        for (auto xevent = Event { std::in_place, globals.connection }; xevent;
             xevent.reset(xcb_poll_for_event(globals.connection)))
            process_events(xevent);

        if (m_mouse_state.locked) {
            xcb_warp_pointer(globals.connection,
                             XCB_NONE,
                             m_window,
                             0,
                             0,
                             0,
                             0,
                             as<i16>(m_mouse_state.locked_at.x * m_state.dpi),
                             as<i16>(m_mouse_state.locked_at.y * m_state.dpi));

            if (m_mouse_state.hidden) xcb_xfixes_hide_cursor(globals.connection, m_window);
            else
                xcb_xfixes_show_cursor(globals.connection, m_window);
        }

        return WindowImplBase::poll_event(event);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::wait_event(Event& event) noexcept -> bool {
        using Event = RAIICapsule<xcb_generic_event_t*,
                                  xcb_poll_for_event,
                                  std::free,
                                  struct EventTag>;

        auto& globals = xcb::get_globals();

        auto xevent = Event {};
        while (!xevent) xevent = Event { std::in_place, globals.connection };

        process_events(xevent);

        return WindowImplBase::wait_event(event);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::set_title(std::string title) noexcept -> void {
        auto& globals = xcb::get_globals();

        xcb_change_property(globals.connection,
                            XCB_PROP_MODE_REPLACE,
                            m_window,
                            XCB_ATOM_WM_NAME,
                            XCB_ATOM_STRING,
                            8,
                            as<u32>(title.length()),
                            title.c_str());

        xcb_flush(globals.connection);

        m_title = std::move(title);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::set_fullscreen(bool enabled) noexcept -> void {
        xcb::get_atom(WM_STATE_FULLSCREEN_STR, false)
          .transform_error(xcb::atom_error(WM_STATE_FULLSCREEN_STR))
          .and_then([](auto&& fullscreen_atom) static noexcept {
              return xcb::get_atom(WM_STATE_STR, false)
                .transform(monadic::as_tuple(std::move(fullscreen_atom)));
          })
          .transform_error(xcb::atom_error(WM_STATE_STR))
          .transform(monadic::unpack_tuple_to([this,
                                               enabled](auto&& fullscreen_atom, auto&& state_atom) {
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
                             XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
                               | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY,
                             std::bit_cast<const char*>(&ev));

              xcb_flush(globals.connection);
              m_state.fullscreen = enabled;
          }));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::confine_mouse(bool confined, u32) noexcept -> void {
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

        if (m_mouse_state.hidden) xcb_xfixes_hide_cursor(globals.connection, m_window);
        else
            xcb_xfixes_show_cursor(globals.connection, m_window);

        xcb_flush(globals.connection);

        m_mouse_state.confined = confined;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::lock_mouse(bool locked, u32) noexcept -> void {
        auto& globals = xcb::get_globals();

        if (locked) {
            m_mouse_state.locked_at = m_mouse_state.last_position;
            xcb_warp_pointer(globals.connection,
                             XCB_NONE,
                             m_window,
                             0,
                             0,
                             0,
                             0,
                             as<i16>(m_mouse_state.locked_at.x * m_state.dpi),
                             as<i16>(m_mouse_state.locked_at.y * m_state.dpi));
        }

        if (m_mouse_state.hidden) xcb_xfixes_hide_cursor(globals.connection, m_window);
        else
            xcb_xfixes_show_cursor(globals.connection, m_window);

        xcb_flush(globals.connection);
        m_mouse_state.locked = locked;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::hide_mouse(bool hidden, u32) noexcept -> void {
        auto& globals = xcb::get_globals();

        if (hidden) xcb_xfixes_hide_cursor(globals.connection, m_window);
        else
            xcb_xfixes_show_cursor(globals.connection, m_window);

        xcb_flush(globals.connection);

        m_mouse_state.hidden = hidden;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::set_relative_mouse(bool relative, u32) noexcept -> void {
        m_mouse_state.relative = relative;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::set_key_repeat(bool enabled, u32) noexcept -> void {
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

        m_key_repeat_enabled = enabled;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::show_virtual_keyboard(bool) noexcept -> void {
        elog("x11::WindowImpl::show_virtual_keyboard isn't yet implemented");
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::set_mouse_position(const math::vec2i& position, u32) noexcept -> void {
        auto& globals = xcb::get_globals();

        xcb_warp_pointer(globals.connection,
                         XCB_NONE,
                         m_window,
                         0,
                         0,
                         0,
                         0,
                         as<i16>(position.x * m_state.dpi),
                         as<i16>(position.y * m_state.dpi));

        xcb_flush(globals.connection);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::set_mouse_position_on_desktop(const math::vec2u& position, u32) noexcept
      -> void {
        auto& globals = xcb::get_globals();

        const auto default_screen_id = 0;
        auto       root_window       = default_root_window(globals.connection, default_screen_id);

        xcb_warp_pointer(globals.connection,
                         XCB_NONE,
                         root_window,
                         0,
                         0,
                         0,
                         0,
                         as<i16>(position.x),
                         as<i16>(position.y));

        xcb_flush(globals.connection);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::get_monitor_settings() -> std::vector<Monitor> {
        auto& globals = xcb::get_globals();

        using Monitors = RAIICapsule<xcb_randr_get_monitors_reply_t*,
                                     xcb_randr_get_monitors_reply,
                                     std::free,
                                     struct MonitorTag>;
        using Output   = RAIICapsule<xcb_randr_get_output_info_reply_t*,
                                     xcb_randr_get_output_info_reply,
                                     std::free,
                                     struct OutputTag>;
        using CRTC     = RAIICapsule<xcb_randr_get_crtc_info_reply_t*,
                                     xcb_randr_get_crtc_info_reply,
                                     std::free,
                                     struct CRTCTag>;

        const auto root = xcb_setup_roots_iterator(xcb_get_setup(globals.connection)).data;

        auto xcb_monitors = Monitors { std::in_place,
                                       globals.connection,
                                       xcb_randr_get_monitors(globals.connection, root->root, 0),
                                       nullptr };
        auto monitors     = std::vector<Monitor> {};

        auto xcb_monitor_iter = xcb_randr_get_monitors_monitors_iterator(xcb_monitors);
        for (auto i = 0; xcb_monitor_iter.rem;
             xcb_randr_monitor_info_next(&xcb_monitor_iter), ++i) {
            auto monitor_info = xcb_monitor_iter.data;

            xcb_randr_select_input(globals.connection, root->root, true);

            auto& monitor = monitors.emplace_back();
            if (monitor_info->primary) monitor.flags = Monitor::Flags::PRIMARY;

            auto name = xcb::get_atom_name(monitor_info->name);
            if (!name) monitor.name = std::format("Monitor {}", i);
            else
                monitor.name = std::move(*name);

            auto len     = xcb_randr_monitor_info_outputs_length(monitor_info);
            auto outputs = xcb_randr_monitor_info_outputs(monitor_info);

            for (auto j : range(len)) {
                auto output_cookie = xcb_randr_get_output_info(globals.connection,
                                                               outputs[j],
                                                               xcb_monitors.handle()->timestamp);
                auto output = Output { std::in_place, globals.connection, output_cookie, nullptr };

                if (!output) continue;
                if (output.handle()->connection != XCB_RANDR_CONNECTION_CONNECTED) continue;
                if (output.handle()->crtc != XCB_NONE) continue;

                auto crtc_cookie = xcb_randr_get_crtc_info(globals.connection,
                                                           output.handle()->crtc,
                                                           output.handle()->timestamp);
                auto crtc        = CRTC { std::in_place, globals.connection, crtc_cookie, nullptr };

                if (crtc == nullptr) {}

                monitor.extents.emplace_back(math::Extent2<u32> { as<u32>(crtc.handle()->width),
                                                                  as<u32>(crtc.handle()->height) });
            }

            if (stdr::empty(monitor.extents))
                monitor.extents
                  .emplace_back(math::Extent2<u32> { as<u32>(xcb_monitor_iter.data->width),
                                                     as<u32>(xcb_monitor_iter.data->height) });
        }
        return monitors;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::process_events(xcb_generic_event_t* event) -> void {
        auto*      xevent        = event;
        const auto response_type = xevent->response_type & ~0x80;

        switch (response_type) {
            case XCB_KEY_PRESS: {
                auto kevent = std::bit_cast<xcb_key_press_event_t*>(xevent);
                handle_key_event(kevent->detail, true);
                break;
            }
            case XCB_KEY_RELEASE: {
                auto kevent = std::bit_cast<xcb_key_release_event_t*>(xevent);
                handle_key_event(kevent->detail, false);
                break;
            }
            case XCB_MOTION_NOTIFY: {
                auto mouse_event = std::bit_cast<xcb_motion_notify_event_t*>(xevent);

                const auto x = narrow<u32>(mouse_event->event_x / m_state.dpi);
                const auto y = narrow<u32>(mouse_event->event_y / m_state.dpi);

                if (m_mouse_state.locked)
                    if (x == m_mouse_state.locked_at.x and y == m_mouse_state.locked_at.y) break;

                if (m_mouse_state.relative) {
                    const auto dx = x - m_mouse_state.last_position.x;
                    const auto dy = y - m_mouse_state.last_position.y;

                    WindowImplBase::mouse_move_event(dx, dy);
                } else
                    WindowImplBase::mouse_move_event(x, y);

                m_mouse_state.last_position.x = x;
                m_mouse_state.last_position.y = y;

                break;
            }
            case XCB_BUTTON_PRESS: {
                auto button_event = std::bit_cast<xcb_button_press_event_t*>(xevent);

                auto button = button_event->detail;
                WindowImplBase::mouse_down_event(x11_button_to_stormkit(button),
                                                 button_event->event_x / m_state.dpi,
                                                 button_event->event_y / m_state.dpi);
                break;
            }
            case XCB_BUTTON_RELEASE: {
                auto button_event = std::bit_cast<xcb_button_press_event_t*>(xevent);

                auto button = button_event->detail;
                WindowImplBase::mouse_up_event(x11_button_to_stormkit(button),
                                               button_event->event_x,
                                               button_event->event_y);
                break;
            }
            case XCB_CONFIGURE_NOTIFY: {
                auto configure_event = std::bit_cast<xcb_configure_notify_event_t*>(xevent);

                if ((configure_event->width != m_state.extent.width)
                    || (configure_event->height != m_state.extent.height)) {
                    m_state.extent.width  = configure_event->width;
                    m_state.extent.height = configure_event->height;
                    WindowImplBase::resize_event(configure_event->width, configure_event->height);
                }
                break;
            }
            case XCB_ENTER_NOTIFY: WindowImplBase::mouse_entered_event(); break;
            case XCB_LEAVE_NOTIFY: WindowImplBase::mouse_exited_event(); break;
            case XCB_VISIBILITY_NOTIFY: // TODO XCB_VISIBILITY_NOTIFY
                /*if(xevent.xvisibility.wsi.== m_window
                        m_is_visible = xevent.xvisibility.state !=
                   VisibilityFullyObscured;*/
                break;
            case XCB_DESTROY_NOTIFY: // TODO XCB_DESTROY_NOTIFY
                break;
            case XCB_CLIENT_MESSAGE: {
                xcb::get_atom(WM_DELETE_WINDOW, false)
                  .transform([this](auto&&) noexcept { WindowImplBase::close_event(); })
                  .transform_error(xcb::atom_error(WM_DELETE_WINDOW));
                break;
            }
            case XCB_MAPPING_NOTIFY: {
                auto mapping_notify_event = std::bit_cast<xcb_mapping_notify_event_t*>(xevent);

                if (mapping_notify_event->request != XCB_MAPPING_POINTER) {
                    xcb_refresh_keyboard_mapping(m_key_symbols, mapping_notify_event);
                    update_keymap();
                }
                break;
            }
            case XCB_GE_GENERIC: {
                if (is_ext_event(xevent, m_xi_opcode)) {
                    auto xievent = std::bit_cast<xcb_ge_generic_event_t*>(xevent);
                    switch (xievent->event_type) {
                        case XCB_INPUT_KEY_PRESS: [[fallthrough]];
                        case XCB_INPUT_RAW_KEY_PRESS: {
                            auto _xievent = std::bit_cast<xcb_input_key_press_event_t*>(xevent);
                            handle_key_event(as<xcb_keycode_t>(_xievent->detail), true);
                            break;
                        }
                        case XCB_INPUT_KEY_RELEASE: [[fallthrough]];
                        case XCB_INPUT_RAW_KEY_RELEASE: {
                            auto _xievent = std::bit_cast<xcb_input_key_press_event_t*>(xevent);
                            handle_key_event(as<xcb_keycode_t>(_xievent->detail), false);
                            break;
                        }
                        case XCB_INPUT_BUTTON_PRESS: [[fallthrough]];
                        case XCB_INPUT_RAW_BUTTON_PRESS: {
                            auto
                              button_event = std::bit_cast<xcb_input_button_press_event_t*>(xevent);

                            auto button = button_event->detail;
                            WindowImplBase::
                              mouse_down_event(x11_button_to_stormkit(as<xcb_button_t>(button)),
                                               button_event->event_x,
                                               button_event->event_y);
                            break;
                        }
                        case XCB_INPUT_BUTTON_RELEASE: [[fallthrough]];
                        case XCB_INPUT_RAW_BUTTON_RELEASE: {
                            auto button_event = std::bit_cast<
                              xcb_input_button_release_event_t*>(xevent);

                            auto button = button_event->detail;
                            WindowImplBase::
                              mouse_down_event(x11_button_to_stormkit(as<xcb_button_t>(button)),
                                               button_event->event_x,
                                               button_event->event_y);
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::update_keymap() -> void {
        auto& globals = xcb::get_globals();

        const auto device_id = xkb_x11_get_core_keyboard_device_id(globals.connection);

        m_keyboard_state.keymap = common::xkb::Keymap {
            xkb_x11_keymap_new_from_device(globals.xkb_context,
                                           globals.connection,
                                           device_id,
                                           XKB_KEYMAP_COMPILE_NO_FLAGS)
        };
        if (not m_keyboard_state.keymap) {
            elog("Failed to compile a keymap");
            return;
        }

        m_keyboard_state.state = common::xkb::State {
            xkb_x11_state_new_from_device(m_keyboard_state.keymap, globals.connection, device_id)
        };
        if (not m_keyboard_state.state) {
            elog("Failed to create XKB state");
            return;
        }

        m_handles.state = m_keyboard_state.state;

        m_keyboard_state.mods = common::xkb::Mods {
            .shift   = xkb_keymap_mod_get_index(m_keyboard_state.keymap, XKB_MOD_NAME_SHIFT),
            .lock    = xkb_keymap_mod_get_index(m_keyboard_state.keymap, XKB_MOD_NAME_CAPS),
            .control = xkb_keymap_mod_get_index(m_keyboard_state.keymap, XKB_MOD_NAME_CTRL),
            .mod1    = xkb_keymap_mod_get_index(m_keyboard_state.keymap, "Mod1"),
            .mod2    = xkb_keymap_mod_get_index(m_keyboard_state.keymap, "Mod2"),
            .mod3    = xkb_keymap_mod_get_index(m_keyboard_state.keymap, "Mod3"),
            .mod4    = xkb_keymap_mod_get_index(m_keyboard_state.keymap, "Mod4"),
            .mod5    = xkb_keymap_mod_get_index(m_keyboard_state.keymap, "Mod5")
        };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::handle_key_event(xcb_keycode_t keycode, bool down) noexcept -> void {
        auto symbol = xkb_keysym_t {};

        auto character = char {};
        symbol         = xkb_state_key_get_one_sym(m_keyboard_state.state, keycode);
        xkb_state_key_get_utf8(m_keyboard_state.state,
                               keycode,
                               std::bit_cast<char*>(&character),
                               sizeof(char));

        auto key = common::xkb_key_to_stormkit(symbol);

        if (down) WindowImplBase::key_down_event(key, character);
        else
            WindowImplBase::key_up_event(key, character);
    }
} // namespace stormkit::wsi::linux::x11
