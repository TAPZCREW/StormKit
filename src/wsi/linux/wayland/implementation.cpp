module;

#include <sys/mman.h>
#include <syscall.h>
#include <unistd.h>

#include <linux/input-event-codes.h>
#include <xkbcommon/xkbcommon.h>

#include <wayland-client.h>
#include <wayland-cursor.h>

#include <pointer-constraints-unstable-v1.h>
#include <relative-pointer-unstable-v1.h>
#include <xdg-decoration-unstable-v1.h>
#include <xdg-shell.h>

module stormkit.wsi;

import std;

import stormkit.core;

import :linux.common.xkb;
import :linux.wayland.window_impl;
import :linux.wayland.callbacks;
import :linux.wayland.log;

namespace stormkit::wsi::linux::wayland {
    /////////////////////////////////////
    /////////////////////////////////////
    auto output_geometry_handler(void*       data,
                                 wl_output*  output,
                                 Int32       x,
                                 Int32       y,
                                 Int32       pwidth,
                                 Int32       pheight,
                                 Int32       subpixels,
                                 const char* make,
                                 const char* model,
                                 Int32       transform) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto output_mode_handler(void*      data,
                             wl_output* wl_output,
                             UInt32     flags,
                             Int32      width,
                             Int32      height,
                             Int32      refresh) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto output_done_handler(void* data, wl_output* wl_output) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto output_scale_handler(void* data, wl_output* wl_output, Int32 factor) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto seat_capabilities_handler(void* data, wl_seat* seat, UInt32 capabilities) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto seat_name_handler(void* data, wl_seat* seat, const char* name) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_enter_handler(void*       data,
                               wl_pointer* pointer,
                               UInt32      serial,
                               wl_surface* surface,
                               wl_fixed_t  surface_x,
                               wl_fixed_t  surface_y) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_leave_handler(void*       data,
                               wl_pointer* pointer,
                               UInt32      serial,
                               wl_surface* surface) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_motion_handler(void*       data,
                                wl_pointer* pointer,
                                UInt32      time,
                                wl_fixed_t  surface_x,
                                wl_fixed_t  surface_y) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_button_handler(void*       data,
                                wl_pointer* pointer,
                                UInt32      serial,
                                UInt32      time,
                                UInt32      button,
                                UInt32      state) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_axis_handler(void*       data,
                              wl_pointer* pointer,
                              UInt32      time,
                              UInt32      axis,
                              wl_fixed_t  value) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_frame_handler(void* data, wl_pointer* pointer) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_axis_source_handler(void* data, wl_pointer* pointer, UInt32 axis_source) noexcept
        -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_axis_stop_handler(void*       data,
                                   wl_pointer* pointer,
                                   UInt32      time,
                                   UInt32      axis) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_axis_discrete_handler(void*       data,
                                       wl_pointer* pointer,
                                       UInt32      axis,
                                       Int32       discrete) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboard_keymap_handler(void*        data,
                                 wl_keyboard* keyboard,
                                 UInt32       format,
                                 Int32        fd,
                                 UInt32       size) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboard_enter_handler(void*        data,
                                wl_keyboard* keyboard,
                                UInt32       serial,
                                wl_surface*  surface,
                                wl_array*    keys) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboard_leave_handler(void*        data,
                                wl_keyboard* keyboard,
                                UInt32       serial,
                                wl_surface*  surface) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboard_key_handler(void*        data,
                              wl_keyboard* keyboard,
                              UInt32       serial,
                              UInt32       time,
                              UInt32       key,
                              UInt32       state) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboard_modifiers_handler(void*        data,
                                    wl_keyboard* keyboard,
                                    UInt32       serial,
                                    UInt32       mods_depressed,
                                    UInt32       mods_latcher,
                                    UInt32       mods_locked,
                                    UInt32       group) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboard_repeat_info_handler(void*        data,
                                      wl_keyboard* keyboard,
                                      Int32        rate,
                                      Int32        delay) noexcept -> void;

    namespace {
        auto globals = Globals {};

        constinit const auto stormkit_registry_listener
            = wl_registry_listener { .global        = registry_handler,
                                     .global_remove = registry_remover_handler };

        constinit const auto stormkit_surface_listener
            = wl_surface_listener { .enter = surface_enter_handler,
                                    .leave = surface_leave_handler };

        constinit const auto stormkit_xdg_surface_listener
            = xdg_surface_listener { .configure = surface_configure_handler };

        constinit const auto stormkit_xdg_toplevel_listener
            = xdg_toplevel_listener { .configure = top_level_configure_handler,
                                      .close     = top_level_close_handler };

        constinit const auto stormkit_shell_listener
            = xdg_wm_base_listener { .ping = shell_ping_handler };

        constinit const auto stormkit_shell_surface_listener
            = wl_shell_surface_listener { .ping       = shell_ping_handler,
                                          .configure  = shell_surface_configure_handler,
                                          .popup_done = nullptr };

        constinit const auto stormkit_relative_pointer_listener
            = zwp_relative_pointer_v1_listener { .relative_motion
                                                 = relative_pointer_relative_motion_handler };

        constinit const auto stormkit_locked_pointer_listener
            = zwp_locked_pointer_v1_listener { .locked   = locker_pointer_locker_handler,
                                               .unlocked = locker_pointer_unlocker_handler };
    } // namespace

    void init() {
        globals.display.reset(wl_display_connect(nullptr));

        if (globals.display) dlog("WAYLAND context initialized");
        else {
            flog("Failed to initialize wayland");
            std::exit(EXIT_FAILURE);
        }

        globals.registry.reset(wl_display_get_registry(globals.display.get()));

        wl_registry_add_listener(globals.registry.get(), &stormkit_registry_listener, &globals);

        wl_display_dispatch(globals.display.get());
        wl_display_roundtrip(globals.display.get());

        if (globals.compositor) dlog("WAYLAND compositor found !");
        else {
            flog("Failed to find a WAYLAND compositor");
            std::exit(EXIT_FAILURE);
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    WindowImpl::WindowImpl() {
        if (!globals.display) init();

        m_keyboard_state = std::array {
            KeyState { XKB_KEY_a,            false },
             KeyState { XKB_KEY_b,            false },
            KeyState { XKB_KEY_c,            false },
             KeyState { XKB_KEY_d,            false },
            KeyState { XKB_KEY_e,            false },
             KeyState { XKB_KEY_f,            false },
            KeyState { XKB_KEY_g,            false },
             KeyState { XKB_KEY_h,            false },
            KeyState { XKB_KEY_i,            false },
             KeyState { XKB_KEY_j,            false },
            KeyState { XKB_KEY_k,            false },
             KeyState { XKB_KEY_l,            false },
            KeyState { XKB_KEY_m,            false },
             KeyState { XKB_KEY_n,            false },
            KeyState { XKB_KEY_o,            false },
             KeyState { XKB_KEY_p,            false },
            KeyState { XKB_KEY_q,            false },
             KeyState { XKB_KEY_r,            false },
            KeyState { XKB_KEY_s,            false },
             KeyState { XKB_KEY_t,            false },
            KeyState { XKB_KEY_u,            false },
             KeyState { XKB_KEY_v,            false },
            KeyState { XKB_KEY_w,            false },
             KeyState { XKB_KEY_x,            false },
            KeyState { XKB_KEY_y,            false },
             KeyState { XKB_KEY_z,            false },
            KeyState { XKB_KEY_0,            false },
             KeyState { XKB_KEY_1,            false },
            KeyState { XKB_KEY_2,            false },
             KeyState { XKB_KEY_3,            false },
            KeyState { XKB_KEY_4,            false },
             KeyState { XKB_KEY_5,            false },
            KeyState { XKB_KEY_6,            false },
             KeyState { XKB_KEY_7,            false },
            KeyState { XKB_KEY_8,            false },
             KeyState { XKB_KEY_9,            false },
            KeyState { XKB_KEY_Escape,       false },
             KeyState { XKB_KEY_Control_L,    false },
            KeyState { XKB_KEY_Shift_L,      false },
             KeyState { XKB_KEY_Alt_L,        false },
            KeyState { XKB_KEY_Super_L,      false },
             KeyState { XKB_KEY_Control_R,    false },
            KeyState { XKB_KEY_Shift_R,      false },
             KeyState { XKB_KEY_Alt_R,        false },
            KeyState { XKB_KEY_Super_R,      false },
             KeyState { XKB_KEY_Menu,         false },
            KeyState { XKB_KEY_bracketleft,  false },
             KeyState { XKB_KEY_bracketright, false },
            KeyState { XKB_KEY_semicolon,    false },
             KeyState { XKB_KEY_comma,        false },
            KeyState { XKB_KEY_period,       false },
             KeyState { XKB_KEY_quoteleft,    false },
            KeyState { XKB_KEY_slash,        false },
             KeyState { XKB_KEY_backslash,    false },
            KeyState { XKB_KEY_dead_grave,   false },
             KeyState { XKB_KEY_equal,        false },
            KeyState { XKB_KEY_hyphen,       false },
             KeyState { XKB_KEY_space,        false },
            KeyState { XKB_KEY_Return,       false },
             KeyState { XKB_KEY_BackSpace,    false },
            KeyState { XKB_KEY_Tab,          false },
             KeyState { XKB_KEY_Page_Up,      false },
            KeyState { XKB_KEY_Page_Down,    false },
             KeyState { XKB_KEY_Begin,        false },
            KeyState { XKB_KEY_End,          false },
             KeyState { XKB_KEY_Home,         false },
            KeyState { XKB_KEY_Insert,       false },
             KeyState { XKB_KEY_Delete,       false },
            KeyState { XKB_KEY_KP_Add,       false },
             KeyState { XKB_KEY_KP_Subtract,  false },
            KeyState { XKB_KEY_KP_Multiply,  false },
             KeyState { XKB_KEY_KP_Divide,    false },
            KeyState { XKB_KEY_Left,         false },
             KeyState { XKB_KEY_Right,        false },
            KeyState { XKB_KEY_Up,           false },
             KeyState { XKB_KEY_Down,         false },
            KeyState { XKB_KEY_KP_0,         false },
             KeyState { XKB_KEY_KP_1,         false },
            KeyState { XKB_KEY_KP_2,         false },
             KeyState { XKB_KEY_KP_3,         false },
            KeyState { XKB_KEY_KP_4,         false },
             KeyState { XKB_KEY_KP_5,         false },
            KeyState { XKB_KEY_KP_6,         false },
             KeyState { XKB_KEY_KP_7,         false },
            KeyState { XKB_KEY_KP_8,         false },
             KeyState { XKB_KEY_KP_9,         false },
            KeyState { XKB_KEY_F1,           false },
             KeyState { XKB_KEY_F2,           false },
            KeyState { XKB_KEY_F3,           false },
             KeyState { XKB_KEY_F4,           false },
            KeyState { XKB_KEY_F5,           false },
             KeyState { XKB_KEY_F6,           false },
            KeyState { XKB_KEY_F7,           false },
             KeyState { XKB_KEY_F8,           false },
            KeyState { XKB_KEY_F9,           false },
             KeyState { XKB_KEY_F10,          false },
            KeyState { XKB_KEY_F11,          false },
             KeyState { XKB_KEY_F12,          false },
            KeyState { XKB_KEY_F13,          false },
             KeyState { XKB_KEY_F14,          false },
            KeyState { XKB_KEY_F15,          false },
             KeyState { XKB_KEY_Pause,        false },
        };

        m_xkb_context = common::get_xkb_context();

        m_cursor_theme.reset(wl_cursor_theme_load(nullptr, 24, globals.shm.get()));
        auto cursor = wl_cursor_theme_get_cursor(m_cursor_theme.get(), "default");

        if (!cursor) cursor = wl_cursor_theme_get_cursor(m_cursor_theme.get(), "left_ptr");

        auto cursor_image = cursor->images[0];

        m_cursor_buffer.reset(wl_cursor_image_get_buffer(cursor_image));

        m_cursor_surface.reset(wl_compositor_create_surface(globals.compositor.get()));
        wl_surface_attach(m_cursor_surface.get(), m_cursor_buffer.get(), 0, 0);
        wl_surface_commit(m_cursor_surface.get());

        m_handles.display = globals.display.get();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    WindowImpl::WindowImpl(std::string title, const math::ExtentU& extent, WindowStyle style)
        : WindowImpl {} {
        create(std::move(title), extent, style);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    WindowImpl::~WindowImpl() {
        wl_display_flush(globals.display.get());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    WindowImpl::WindowImpl(WindowImpl&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::operator=(WindowImpl&&) noexcept -> WindowImpl& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::create(std::string title, const math::ExtentU& extent, WindowStyle style)
        -> void {
        m_title  = title;
        m_extent = extent;
        m_style  = style;

        m_locked_mouse_position.x = m_extent.width / 2;
        m_locked_mouse_position.y = m_extent.height / 2;

        m_surface.reset(wl_compositor_create_surface(globals.compositor.get()));

        if (!std::empty(globals.pointers)) {
            m_pointer.reset(globals.pointers.front().get());
            wl_pointer_set_user_data(m_pointer, this);
        } else
            wlog("No pointer found");
        if (!std::empty(globals.keyboards)) {
            m_keyboard.reset(globals.keyboards.front().get());
            wl_keyboard_set_user_data(m_keyboard, this);
        } else
            wlog("No keyboard found");
        if (!std::empty(globals.touchscreens)) {
            m_touchscreen.reset(globals.touchscreens.front().get());
            wl_touch_set_user_data(m_touchscreen, this);
        } else
            wlog("No touchscreen found");

        if (globals.xdg_shell) {
            dlog("XDG shell found !");
            create_xdg_shell();
        } else {
            dlog("XDGShell not found, falling back to WLShell");

            if (globals.wayland_shell) create_wayland_shell();
            else {
                flog("WLShell not found, aborting...");
                std::exit(EXIT_FAILURE);
            }
        }

        wl_surface_add_listener(m_surface.get(), &stormkit_surface_listener, this);

        m_handles.surface = m_surface.get();

        m_title      = std::move(title);
        m_open       = true;
        m_visible    = true;
        m_configured = true;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::close() noexcept -> void {
        wl_display_flush(globals.display.get());

        // Fake Buffer
        m_buffer.release();

        // WP
        m_locked_pointer.reset();
        m_relative_pointer.reset();

        // Events
        if (m_pointer) wl_pointer_set_user_data(m_pointer, nullptr);

        if (m_keyboard) wl_keyboard_set_user_data(m_keyboard, nullptr);

        if (m_touchscreen) wl_touch_set_user_data(m_touchscreen, nullptr);

        m_pointer.reset();
        m_pointer_serial = 0u;
        m_keyboard.reset();
        m_touchscreen.reset();

        // XDG
        globals.xdg_decoration_manager.reset();
        m_xdg_toplevel.reset();
        m_xdg_surface.reset();

        // WL_Shell
        m_wlshell_surface.reset();

        // Base_Surface
        m_surface.reset();

        m_current_output.reset();

        m_title.clear();
        m_open       = false;
        m_visible    = false;
        m_configured = false;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::poll_event(Event& event) noexcept -> bool {
        while (!m_configured) wl_display_dispatch(globals.display.get());

        while (wl_display_prepare_read(globals.display.get()) != 0)
            wl_display_dispatch_pending(globals.display.get());

        wl_display_flush(globals.display.get());
        wl_display_read_events(globals.display.get());
        wl_display_dispatch_pending(globals.display.get());

        return WindowImplBase::poll_event(event);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::wait_event(Event& event) noexcept -> bool {
        while (!m_configured) wl_display_dispatch(globals.display.get());

        while (wl_display_prepare_read(globals.display.get()) != 0)
            wl_display_dispatch(globals.display.get());
        wl_display_flush(globals.display.get());

        return WindowImplBase::wait_event(event);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::set_title(std::string title) noexcept -> void {
        if (!m_open) return;
        m_title = std::move(title);

        if (m_xdg_toplevel) {
            xdg_toplevel_set_title(m_xdg_toplevel.get(), m_title.c_str());
            xdg_toplevel_set_app_id(m_xdg_toplevel.get(), m_title.c_str());
        } else {
            wl_shell_surface_set_title(m_wlshell_surface.get(), m_title.c_str());
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::set_extent(const math::ExtentU&) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::toggle_fullscreen(bool enabled) noexcept -> void {
        if (!m_open) return;
        if (m_xdg_toplevel) {
            if (enabled) xdg_toplevel_set_fullscreen(m_xdg_toplevel.get(), m_current_output);
            else
                xdg_toplevel_unset_fullscreen(m_xdg_toplevel.get());
        } else {
            // TODO implement for wl_shell_surface
        }

        m_fullscreen = enabled;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::lock_mouse() noexcept -> void {
        if (!m_open) return;
        if (!globals.relative_pointer_manager) {
            elog("Can't lock mouse, {} protocol is not present",
                 zwp_relative_pointer_manager_v1_interface.name);
            return;
        }
        if (!globals.pointer_constraints) {
            elog("Can't lock mouse, {} protocol is not present",
                 zwp_pointer_constraints_v1_interface.name);
            return;
        }

        m_mouse_state.position_in_window = m_locked_mouse_position;

        m_relative_pointer.reset(zwp_relative_pointer_manager_v1_get_relative_pointer(
            globals.relative_pointer_manager.get(),
            m_pointer));
        zwp_relative_pointer_v1_add_listener(m_relative_pointer.get(),
                                             &stormkit_relative_pointer_listener,
                                             this);

        m_locked_pointer.reset(zwp_pointer_constraints_v1_lock_pointer(
            globals.pointer_constraints.get(),
            m_surface.get(),
            m_pointer,
            nullptr,
            ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT));

        zwp_locked_pointer_v1_add_listener(m_locked_pointer.get(),
                                           &stormkit_locked_pointer_listener,
                                           this);

        m_is_mouse_locked = true;

        hide_mouse();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::unlock_mouse() noexcept -> void {
        if (!m_open) return;

        m_locked_mouse_position = m_mouse_state.position_in_window;
        m_locked_pointer.reset();
        m_relative_pointer.reset();

        m_is_mouse_locked = false;

        unhide_mouse();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::hide_mouse() noexcept -> void {
        if (!m_open) return;
        wl_pointer_set_cursor(m_pointer, m_pointer_serial, nullptr, 0, 0);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::unhide_mouse() noexcept -> void {
        if (!m_open) return;
        wl_pointer_set_cursor(m_pointer, m_pointer_serial, m_cursor_surface.get(), 0, 0);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::native_handle() const noexcept -> NativeHandle {
        return std::bit_cast<NativeHandle>(&m_handles);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::toggle_key_repeat(bool) noexcept -> void {
        elog("wayland::WindowImpl::toggle_key_repeat isn't yet implemented");
        m_key_repeat_enabled = false;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::toggle_virtual_keyboard_visibility(bool) noexcept -> void {
        elog("wayland::WindowImpl::toggle_virtual_keyboard_visibility isn't yet implemented");
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::set_mouse_position(const math::Vector2I& position) noexcept -> void {
        if (!m_open) return;
        if (m_is_mouse_locked) {
            zwp_locked_pointer_v1_set_cursor_position_hint(m_locked_pointer.get(),
                                                           wl_fixed_to_int(position.x),
                                                           wl_fixed_to_int(position.y));
            wl_surface_commit(m_surface.get());
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::set_mouse_position_on_desktop(const math::Vector2U&) noexcept -> void {
        elog("wayland::WindowImpl::set_mouse_position_on_desktop isn't yet implemented");
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::get_monitor_settings() -> std::vector<Monitor> {
        if (!globals.display) init();

        auto output = transform(globals.monitors, [](const auto& pair) { return pair.second; });

        return output;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::surface_output_enter(wl_surface*, wl_output* output) noexcept -> void {
        m_current_output.reset(output);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::surface_configure(xdg_surface* surface, std::uint32_t serial) noexcept
        -> void {
        dlog("XDG surface configure, serial: {}", serial);

        xdg_surface_ack_configure(surface, serial);

        wl_surface_damage(m_surface.get(), 0, 0, m_extent.width, m_extent.height);
        create_pixel_buffer();

        m_configured = true;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::top_level_configure(xdg_toplevel*,
                                         std::int32_t width,
                                         std::int32_t height,
                                         wl_array*    state) noexcept -> void {
        dlog("XDG Shell configure: {}:{}", width, height);

        m_open    = true;
        m_visible = width > 0 && height > 0;

        auto data = static_cast<xdg_toplevel_state*>(state->data);
        for (auto i : range(state->size)) {
            const auto state = data[i];

            switch (state) {
                case XDG_TOPLEVEL_STATE_MAXIMIZED: WindowImplBase::maximize_event(); break;
                case XDG_TOPLEVEL_STATE_RESIZING: {
                    WindowImplBase::resize_event(width, height);
                    break;
                }
                default: break;
            }
        }

        if (width <= 0 || height <= 0) return;

        m_extent.width  = width;
        m_extent.height = height;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::top_level_close(xdg_toplevel*) noexcept -> void {
        m_open          = false;
        m_visible       = false;
        m_extent.width  = 0;
        m_extent.height = 0;

        WindowImplBase::close_event();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::shell_surface_configure(wl_shell_surface*,
                                             std::uint32_t,
                                             std::int32_t width,
                                             std::int32_t height) noexcept -> void {
        dlog("WL Shell configure: {}:{}", width, height);

        m_open    = true;
        m_visible = width > 0 && height > 0;

        if (width <= 0 || height <= 0) return;

        m_extent.width  = width;
        m_extent.height = height;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::seat_capabilities(wl_seat*, std::uint32_t) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::pointer_enter(wl_pointer*,
                                   std::uint32_t serial,
                                   wl_surface*,
                                   wl_fixed_t,
                                   wl_fixed_t) noexcept -> void {
        m_pointer_serial = serial;

        if (!m_is_mouse_locked) unhide_mouse();
        else
            hide_mouse();

        WindowImplBase::mouse_entered_event();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::pointer_leave(wl_pointer*, std::uint32_t serial, wl_surface*) noexcept
        -> void {
        m_pointer_serial = serial;

        WindowImplBase::mouse_exited_event();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::pointer_motion(wl_pointer*,
                                    std::uint32_t,
                                    wl_fixed_t surface_x,
                                    wl_fixed_t surface_y) noexcept -> void {
        if (m_is_mouse_locked) return;

        m_mouse_state.position_in_window.x = wl_fixed_from_int(surface_x);
        m_mouse_state.position_in_window.y = wl_fixed_from_int(surface_y);

        WindowImplBase::mouse_move_event(m_mouse_state.position_in_window.x,
                                         m_mouse_state.position_in_window.y);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::pointer_button(wl_pointer*,
                                    std::uint32_t serial,
                                    std::uint32_t,
                                    std::uint32_t button,
                                    std::uint32_t state) noexcept -> void {
#define BUTTON_HANDLER(a, b)                                                          \
    case a: {                                                                         \
        auto it  = std::ranges::find_if(m_mouse_state.button_state,                   \
                                       [](const auto& s) { return s.button == a; }); \
        it->down = down;                                                              \
        if (down)                                                                     \
            WindowImplBase::mouse_down_event(b,                                       \
                                             m_mouse_state.position_in_window.x,      \
                                             m_mouse_state.position_in_window.y);     \
        else                                                                          \
            WindowImplBase::mouse_up_event(b,                                         \
                                           m_mouse_state.position_in_window.x,        \
                                           m_mouse_state.position_in_window.y);       \
        break;                                                                        \
    }

        m_pointer_serial = serial;

        const auto down = !!state;

        switch (button) {
            BUTTON_HANDLER(BTN_LEFT, MouseButton::LEFT)
            BUTTON_HANDLER(BTN_RIGHT, MouseButton::RIGHT)
            BUTTON_HANDLER(BTN_MIDDLE, MouseButton::MIDDLE)
            BUTTON_HANDLER(BTN_FORWARD, MouseButton::BUTTON_1)
            BUTTON_HANDLER(BTN_BACK, MouseButton::BUTTON_2)
            default:
                if (down)
                    WindowImplBase::mouse_down_event(MouseButton::UNKNOWN,
                                                     m_mouse_state.position_in_window.x,
                                                     m_mouse_state.position_in_window.y);
                else
                    WindowImplBase::mouse_up_event(MouseButton::UNKNOWN,
                                                   m_mouse_state.position_in_window.x,
                                                   m_mouse_state.position_in_window.y);
        }

#undef BUTTON_HANDLER
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::keyboard_keymap(wl_keyboard*,
                                     std::uint32_t format,
                                     std::int32_t  fd,
                                     std::uint32_t size) noexcept -> void {
        if (format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
            auto map_shm
                = reinterpret_cast<char*>(mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0));

            update_keymap(std::string_view { map_shm, size });

            munmap(map_shm, size);
            ::close(fd);
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::keyboard_enter(wl_keyboard*,
                                    std::uint32_t,
                                    wl_surface*,
                                    wl_array* keys) noexcept -> void {
        WindowImplBase::gained_focus_event();

        auto data = static_cast<std::uint32_t*>(keys->data);
        for (auto i : range(keys->size)) {
            const auto keycode = data[i] + 8;

            auto character = char {};

            const auto symbol = xkb_state_key_get_one_sym(m_xkb_state.get(), keycode);
            xkb_state_key_get_utf8(m_xkb_state.get(), keycode, &character, sizeof(char));

            const auto skey = common::xkb_key_to_stormkit(symbol);

            WindowImplBase::key_down_event(skey, character);
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::keyboard_leave(wl_keyboard*, std::uint32_t, wl_surface*) noexcept -> void {
        WindowImplBase::lost_focus_event();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::keyboard_key(wl_keyboard*,
                                  std::uint32_t,
                                  std::uint32_t,
                                  std::uint32_t key,
                                  std::uint32_t state) noexcept -> void {
        auto character = char {};

        const auto keycode = key + 8;

        const auto symbol = xkb_state_key_get_one_sym(m_xkb_state.get(), keycode);
        xkb_state_key_get_utf8(m_xkb_state.get(), keycode, &character, sizeof(char));

        const auto skey = common::xkb_key_to_stormkit(symbol);

        const auto down = state == WL_KEYBOARD_KEY_STATE_PRESSED;

        auto it = std::ranges::find_if(m_keyboard_state,
                                       [symbol](const auto& s) { return s.key == symbol; });

        it->down = down;
        if (down) WindowImplBase::key_down_event(skey, character);
        else
            WindowImplBase::key_up_event(skey, character);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::keyboard_modifiers(wl_keyboard*,
                                        std::uint32_t,
                                        std::uint32_t mods_depressed,
                                        std::uint32_t mods_latched,
                                        std::uint32_t mods_locked,
                                        std::uint32_t group) noexcept -> void {
        xkb_state_update_mask(m_xkb_state.get(),
                              mods_depressed,
                              mods_latched,
                              mods_locked,
                              0,
                              0,
                              group);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::keyboard_repeat_info(wl_keyboard*, std::int32_t, std::int32_t) noexcept
        -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::relative_pointer_relative_motion(zwp_relative_pointer_v1*,
                                                      std::uint32_t,
                                                      std::uint32_t,
                                                      wl_fixed_t,
                                                      wl_fixed_t,
                                                      wl_fixed_t dx_unaccel,
                                                      wl_fixed_t dy_unaccel) noexcept -> void {
        m_mouse_state.position_in_window.x += wl_fixed_from_int(dx_unaccel);
        m_mouse_state.position_in_window.y += wl_fixed_from_int(dy_unaccel);

        WindowImplBase::mouse_move_event(m_mouse_state.position_in_window.x,
                                         m_mouse_state.position_in_window.y);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::create_xdg_shell() noexcept -> void {
        xdg_wm_base_add_listener(globals.xdg_shell.get(), &stormkit_shell_listener, nullptr);

        m_xdg_surface.reset(xdg_wm_base_get_xdg_surface(globals.xdg_shell.get(), m_surface.get()));

        xdg_surface_add_listener(m_xdg_surface.get(), &stormkit_xdg_surface_listener, this);

        m_xdg_toplevel.reset(xdg_surface_get_toplevel(m_xdg_surface.get()));

        xdg_toplevel_add_listener(m_xdg_toplevel.get(), &stormkit_xdg_toplevel_listener, this);

        xdg_toplevel_set_title(m_xdg_toplevel.get(), m_title.c_str());
        xdg_toplevel_set_app_id(m_xdg_toplevel.get(), m_title.c_str());

        if (!checkFlag(m_style, WindowStyle::RESIZEABLE)) {
            xdg_toplevel_set_min_size(m_xdg_toplevel.get(), m_extent.width, m_extent.height);
            xdg_toplevel_set_max_size(m_xdg_toplevel.get(), m_extent.width, m_extent.height);
        } else {
            const auto fullscreen_size = [] {
                const auto monitors = get_monitor_settings();
                for (const auto& monitor : monitors)
                    if (monitor.flags == Monitor::Flags::PRIMARY) return monitor.extents[0];

                return math::ExtentU { 1, 1 };
            }();

            xdg_toplevel_set_min_size(m_xdg_toplevel.get(), 1, 1);
            xdg_toplevel_set_max_size(m_xdg_toplevel.get(),
                                      fullscreen_size.width,
                                      fullscreen_size.height);
        }

        if (globals.xdg_decoration_manager) {
            zxdg_decoration_manager_v1_get_toplevel_decoration(globals.xdg_decoration_manager.get(),
                                                               m_xdg_toplevel.get());
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::create_wayland_shell() noexcept -> void {
        m_wlshell_surface.reset(
            wl_shell_get_shell_surface(globals.wayland_shell.get(), m_surface.get()));
        wl_shell_surface_add_listener(m_wlshell_surface.get(),
                                      &stormkit_shell_surface_listener,
                                      this);
        wl_shell_surface_set_toplevel(m_wlshell_surface.get());

        wl_surface_damage(m_surface.get(), 0, 0, m_extent.width, m_extent.height);
        create_pixel_buffer();
    }

    auto WindowImpl::create_pixel_buffer() noexcept -> void {
        const auto buffer_size   = m_extent.width * m_extent.height * 4;
        const auto buffer_stride = m_extent.width * 4;

        auto fd = syscall(SYS_memfd_create, "buffer", 0);
        ftruncate(fd, buffer_size);

        auto _ = mmap(nullptr, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        m_shm_pool.reset(wl_shm_create_pool(globals.shm.get(), fd, buffer_size));

        m_buffer.reset(wl_shm_pool_create_buffer(m_shm_pool.get(),
                                                 0,
                                                 m_extent.width,
                                                 m_extent.height,
                                                 buffer_stride,
                                                 WL_SHM_FORMAT_XRGB8888));

        wl_surface_attach(m_surface.get(), m_buffer.get(), 0, 0);
        wl_surface_commit(m_surface.get());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::update_keymap(std::string_view keymap_string) noexcept -> void {
        m_xkb_keymap.reset(xkb_keymap_new_from_string(m_xkb_context,
                                                      std::data(keymap_string),
                                                      XKB_KEYMAP_FORMAT_TEXT_V1,
                                                      XKB_KEYMAP_COMPILE_NO_FLAGS));

        if (!m_xkb_keymap) {
            elog("Failed to compile a keymap");
            return;
        }

        m_xkb_state.reset(xkb_state_new(m_xkb_keymap.get()));

        if (!m_xkb_state) {
            elog("Failed to create XKB state");
            return;
        }

        m_xkb_mods = common::XKBMods {
            .shift   = xkb_keymap_mod_get_index(m_xkb_keymap.get(), XKB_MOD_NAME_SHIFT),
            .lock    = xkb_keymap_mod_get_index(m_xkb_keymap.get(), XKB_MOD_NAME_CAPS),
            .control = xkb_keymap_mod_get_index(m_xkb_keymap.get(), XKB_MOD_NAME_CTRL),
            .mod1    = xkb_keymap_mod_get_index(m_xkb_keymap.get(), "Mod1"),
            .mod2    = xkb_keymap_mod_get_index(m_xkb_keymap.get(), "Mod2"),
            .mod3    = xkb_keymap_mod_get_index(m_xkb_keymap.get(), "Mod3"),
            .mod4    = xkb_keymap_mod_get_index(m_xkb_keymap.get(), "Mod4"),
            .mod5    = xkb_keymap_mod_get_index(m_xkb_keymap.get(), "Mod5")
        };
    }

    namespace {
        constinit const auto stormkit_output_listener
            = wl_output_listener { .geometry = output_geometry_handler,
                                   .mode     = output_mode_handler,
                                   .done     = output_done_handler,
                                   .scale    = output_scale_handler };

        constinit const auto stormkit_seat_listener
            = wl_seat_listener { .capabilities = seat_capabilities_handler,
                                 .name         = seat_name_handler };

        constinit const auto stormkit_pointer_listener
            = wl_pointer_listener { .enter         = pointer_enter_handler,
                                    .leave         = pointer_leave_handler,
                                    .motion        = pointer_motion_handler,
                                    .button        = pointer_button_handler,
                                    .axis          = pointer_axis_handler,
                                    .frame         = pointer_frame_handler,
                                    .axis_source   = pointer_axis_source_handler,
                                    .axis_stop     = pointer_axis_stop_handler,
                                    .axis_discrete = pointer_axis_discrete_handler };

        constinit const auto stormkit_keyboard_listener
            = wl_keyboard_listener { .keymap      = keyboard_keymap_handler,
                                     .enter       = keyboard_enter_handler,
                                     .leave       = keyboard_leave_handler,
                                     .key         = keyboard_key_handler,
                                     .modifiers   = keyboard_modifiers_handler,
                                     .repeat_info = keyboard_repeat_info_handler };

        // TODO support touchscreens
        [[maybe_unused]]
        constinit const auto stormkit_touchscreen_listener
            = wl_touch_listener {};
    } // namespace

    /////////////////////////////////////
    /////////////////////////////////////
    auto registry_handler(void*        data,
                          wl_registry* registry,
                          UInt32       id,
                          const char*  interface,
                          UInt32       version) noexcept -> void {
        auto& globals = *static_cast<Globals*>(data);

#define BIND(n, t, v) n.reset(static_cast<t*>(wl_registry_bind(registry, id, &t##_interface, v)));

        dlog("WAYLAND registry acquired {} (id: {}, version: {})", interface, id, version);

        const auto size = std::char_traits<char>::length(interface);

        const auto interface_name = std::string_view { interface, size };

        if (interface_name == wl_compositor_interface.name)
            BIND(globals.compositor, wl_compositor, 3)
        else if (interface_name == wl_output_interface.name) {
            auto& output = globals.outputs.emplace_back(
                static_cast<wl_output*>(wl_registry_bind(registry, id, &wl_output_interface, 2)));
            wl_output_add_listener(output.get(), &stormkit_output_listener, &globals);
        } else if (interface_name == xdg_wm_base_interface.name)
            BIND(globals.xdg_shell, xdg_wm_base, 1)
        else if (interface_name == zxdg_decoration_manager_v1_interface.name)
            BIND(globals.xdg_decoration_manager, zxdg_decoration_manager_v1, 1)
        else if (interface_name == wl_shell_interface.name)
            BIND(globals.wayland_shell, wl_shell, 1)
        else if (interface_name == wl_shm_interface.name)
            BIND(globals.shm, wl_shm, 1)
        else if (interface_name == wl_seat_interface.name) {
            BIND(globals.seat, wl_seat, 5)
            wl_seat_add_listener(globals.seat.get(), &stormkit_seat_listener, &globals);
        } else if (interface_name == zwp_pointer_constraints_v1_interface.name)
            BIND(globals.pointer_constraints, zwp_pointer_constraints_v1, 1)
        else if (interface_name == zwp_relative_pointer_manager_v1_interface.name)
            BIND(globals.relative_pointer_manager, zwp_relative_pointer_manager_v1, 1)
#undef BIND
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto registry_remover_handler(void*, wl_registry*, UInt32 id) noexcept -> void {
        dlog("WAYLAND registry lost {}", id);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto output_geometry_handler(void*      data,
                                 wl_output* output,
                                 Int32,
                                 Int32,
                                 Int32,
                                 Int32,
                                 Int32,
                                 const char* make,
                                 const char* model,
                                 Int32) noexcept -> void {
        auto& globals = *static_cast<Globals*>(data);
        auto& monitor = globals.monitors[output];

        monitor.name = std::format("{} {}", make, model);

        for (auto& [_, m] : globals.monitors) { m.flags = Monitor::Flags::NONE; }

        if (&monitor == &globals.monitors.begin()->second) monitor.flags = Monitor::Flags::PRIMARY;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto output_mode_handler(void*      data,
                             wl_output* wl_output,
                             UInt32,
                             Int32 width,
                             Int32 height,
                             Int32) noexcept -> void {
        auto& globals = *static_cast<Globals*>(data);
        auto& monitor = globals.monitors[wl_output];

        monitor.extents.emplace_back(as<UInt32>(width), as<UInt32>(height));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto output_done_handler(void*, wl_output*) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto output_scale_handler(void*, wl_output*, Int32) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto surface_enter_handler(void* data, wl_surface* surface, wl_output* output) noexcept
        -> void {
        auto* window = static_cast<WindowImpl*>(data);
        window->surface_output_enter(surface, output);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto surface_leave_handler(void*, wl_surface*, wl_output*) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto top_level_configure_handler(void*         data,
                                     xdg_toplevel* xdg_tl,
                                     Int32         width,
                                     Int32         height,
                                     wl_array*     states) noexcept -> void {
        auto* window = static_cast<WindowImpl*>(data);
        window->top_level_configure(xdg_tl, width, height, states);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto top_level_close_handler(void* data, xdg_toplevel* xdg_tl) noexcept -> void {
        auto* window = static_cast<WindowImpl*>(data);
        window->top_level_close(xdg_tl);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto surface_configure_handler(void* data, xdg_surface* surface, UInt32 serial) noexcept
        -> void {
        auto window = static_cast<WindowImpl*>(data);
        window->surface_configure(surface, serial);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto shell_ping_handler(void*, xdg_wm_base* xdg_shell, UInt32 serial) noexcept -> void {
        ilog("Ping received from shell");

        xdg_wm_base_pong(xdg_shell, serial);
    }

    auto shell_surface_configure_handler(void*             data,
                                         wl_shell_surface* shell_surface,
                                         UInt32            edges,
                                         Int32             width,
                                         Int32             height) noexcept -> void {
        auto* window = static_cast<WindowImpl*>(data);
        window->shell_surface_configure(shell_surface, edges, width, height);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto shell_ping_handler(void*, wl_shell_surface* shell_surface, UInt32 serial) noexcept
        -> void {
        ilog("Ping received from shell");

        wl_shell_surface_pong(shell_surface, serial);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto seat_capabilities_handler(void* data, wl_seat*, UInt32 capabilities) noexcept -> void {
        auto& globals = *static_cast<Globals*>(data);

        if ((capabilities & WL_SEAT_CAPABILITY_KEYBOARD) > 0) {
            auto& keyboard
                = globals.keyboards.emplace_back(wl_seat_get_keyboard(globals.seat.get()));
            wl_keyboard_add_listener(keyboard.get(), &stormkit_keyboard_listener, nullptr);
        }

        if ((capabilities & WL_SEAT_CAPABILITY_POINTER) > 0) {
            auto& pointer = globals.pointers.emplace_back(wl_seat_get_pointer(globals.seat.get()));
            wl_pointer_add_listener(pointer.get(), &stormkit_pointer_listener, nullptr);
        }

        if ((capabilities & WL_SEAT_CAPABILITY_TOUCH) > 0) {
            globals.touchscreens.emplace_back(wl_seat_get_touch(globals.seat.get()));
            // wl_touch_add_listener(touchscreen, &stormkit_touchscreen_listener, this);
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto seat_name_handler(void*, wl_seat*, const char* name) noexcept -> void {
        dlog("WL Seat found! {}", name);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_enter_handler(void*       data,
                               wl_pointer* pointer,
                               UInt32      serial,
                               wl_surface* surface,
                               wl_fixed_t  surface_x,
                               wl_fixed_t  surface_y) noexcept -> void {
        if (data == nullptr) return;

        auto* window = static_cast<WindowImpl*>(data);
        window->pointer_enter(pointer, serial, surface, surface_x, surface_y);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_leave_handler(void*       data,
                               wl_pointer* pointer,
                               UInt32      serial,
                               wl_surface* surface) noexcept -> void {
        if (data == nullptr) return;

        auto* window = static_cast<WindowImpl*>(data);
        window->pointer_leave(pointer, serial, surface);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_motion_handler(void*       data,
                                wl_pointer* pointer,
                                UInt32      time,
                                wl_fixed_t  surface_x,
                                wl_fixed_t  surface_y) noexcept -> void {
        if (data == nullptr) return;

        auto* window = static_cast<WindowImpl*>(data);
        window->pointer_motion(pointer, time, surface_x, surface_y);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_button_handler(void*       data,
                                wl_pointer* pointer,
                                UInt32      serial,
                                UInt32      time,
                                UInt32      button,
                                UInt32      state) noexcept -> void {
        if (data == nullptr) return;

        auto* window = static_cast<WindowImpl*>(data);
        window->pointer_button(pointer, serial, time, button, state);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_axis_handler(void*, wl_pointer*, UInt32, UInt32, wl_fixed_t) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_frame_handler(void*, wl_pointer*) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_axis_source_handler(void*, wl_pointer*, UInt32) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_axis_stop_handler(void*, wl_pointer*, UInt32, UInt32) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointer_axis_discrete_handler(void*, wl_pointer*, UInt32, Int32) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboard_keymap_handler(void*        data,
                                 wl_keyboard* keyboard,
                                 UInt32       format,
                                 Int32        fd,
                                 UInt32       size) noexcept -> void {
        if (data == nullptr) return;

        auto* window = static_cast<WindowImpl*>(data);
        window->keyboard_keymap(keyboard, format, fd, size);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboard_enter_handler(void*        data,
                                wl_keyboard* keyboard,
                                UInt32       serial,
                                wl_surface*  surface,
                                wl_array*    keys) noexcept -> void {
        if (data == nullptr) return;

        auto* window = static_cast<WindowImpl*>(data);
        window->keyboard_enter(keyboard, serial, surface, keys);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboard_leave_handler(void*        data,
                                wl_keyboard* keyboard,
                                UInt32       serial,
                                wl_surface*  surface) noexcept -> void {
        if (data == nullptr) return;

        auto* window = static_cast<WindowImpl*>(data);
        window->keyboard_leave(keyboard, serial, surface);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboard_key_handler(void*        data,
                              wl_keyboard* keyboard,
                              UInt32       serial,
                              UInt32       time,
                              UInt32       key,
                              UInt32       state) noexcept -> void {
        if (data == nullptr) return;

        auto* window = static_cast<WindowImpl*>(data);
        window->keyboard_key(keyboard, serial, time, key, state);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboard_modifiers_handler(void*        data,
                                    wl_keyboard* keyboard,
                                    UInt32       serial,
                                    UInt32       mods_depressed,
                                    UInt32       mods_latcher,
                                    UInt32       mods_locked,
                                    UInt32       group) noexcept -> void {
        if (data == nullptr) return;

        auto* window = static_cast<WindowImpl*>(data);
        window->keyboard_modifiers(keyboard,
                                   serial,
                                   mods_depressed,
                                   mods_latcher,
                                   mods_locked,
                                   group);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboard_repeat_info_handler(void*        data,
                                      wl_keyboard* keyboard,
                                      Int32        rate,
                                      Int32        delay) noexcept -> void {
        if (data == nullptr) return;

        auto* window = static_cast<WindowImpl*>(data);
        window->keyboard_repeat_info(keyboard, rate, delay);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto relative_pointer_relative_motion_handler(void*                    data,
                                                  zwp_relative_pointer_v1* pointer,
                                                  UInt32                   time_hi,
                                                  UInt32                   time_lw,
                                                  wl_fixed_t               dx,
                                                  wl_fixed_t               dy,
                                                  wl_fixed_t               dx_unaccel,
                                                  wl_fixed_t dy_unaccel) noexcept -> void {
        auto* window = static_cast<WindowImpl*>(data);
        window->relative_pointer_relative_motion(pointer,
                                                 time_hi,
                                                 time_lw,
                                                 dx,
                                                 dy,
                                                 dx_unaccel,
                                                 dy_unaccel);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto locker_pointer_locker_handler(void*, zwp_locked_pointer_v1*) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto locker_pointer_unlocker_handler(void*, zwp_locked_pointer_v1*) noexcept -> void {
    }
} // namespace stormkit::wsi::linux::wayland
