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

module stormkit.Wsi;

import std;

import stormkit.Core;

import :Linux.Common.XKB;
import :Linux.Wayland.WindowImpl;
import :Linux.Wayland.Callbacks;
import :Linux.Wayland.Log;

namespace stormkit::wsi::linux::wayland {
    /////////////////////////////////////
    /////////////////////////////////////
    auto outputGeometryHandler(void*       data,
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
    auto outputModeHandler(void*      data,
                           wl_output* wl_output,
                           UInt32     flags,
                           Int32      width,
                           Int32      height,
                           Int32      refresh) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto outputDoneHandler(void* data, wl_output* wl_output) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto outputScaleHandler(void* data, wl_output* wl_output, Int32 factor) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto seatCapabilitiesHandler(void* data, wl_seat* seat, UInt32 capabilities) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto seatNameHandler(void* data, wl_seat* seat, const char* name) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointerEnterHandler(void*       data,
                             wl_pointer* pointer,
                             UInt32      serial,
                             wl_surface* surface,
                             wl_fixed_t  surface_x,
                             wl_fixed_t  surface_y) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointerLeaveHandler(void*       data,
                             wl_pointer* pointer,
                             UInt32      serial,
                             wl_surface* surface) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointerMotionHandler(void*       data,
                              wl_pointer* pointer,
                              UInt32      time,
                              wl_fixed_t  surface_x,
                              wl_fixed_t  surface_y) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointerButtonHandler(void*       data,
                              wl_pointer* pointer,
                              UInt32      serial,
                              UInt32      time,
                              UInt32      button,
                              UInt32      state) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointerAxisHandler(void*       data,
                            wl_pointer* pointer,
                            UInt32      time,
                            UInt32      axis,
                            wl_fixed_t  value) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointerFrameHandler(void* data, wl_pointer* pointer) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointerAxisSourceHandler(void*       data,
                                  wl_pointer* pointer,
                                  UInt32      axis_source) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointerAxisStopHandler(void*       data,
                                wl_pointer* pointer,
                                UInt32      time,
                                UInt32      axis) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointerAxisDiscreteHandler(void*       data,
                                    wl_pointer* pointer,
                                    UInt32      axis,
                                    Int32       discrete) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboardKeymapHandler(void*        data,
                               wl_keyboard* keyboard,
                               UInt32       format,
                               Int32        fd,
                               UInt32       size) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboardEnterHandler(void*        data,
                              wl_keyboard* keyboard,
                              UInt32       serial,
                              wl_surface*  surface,
                              wl_array*    keys) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboardLeaveHandler(void*        data,
                              wl_keyboard* keyboard,
                              UInt32       serial,
                              wl_surface*  surface) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboardKeyHandler(void*        data,
                            wl_keyboard* keyboard,
                            UInt32       serial,
                            UInt32       time,
                            UInt32       key,
                            UInt32       state) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboardModifiersHandler(void*        data,
                                  wl_keyboard* keyboard,
                                  UInt32       serial,
                                  UInt32       mods_depressed,
                                  UInt32       mods_latcher,
                                  UInt32       mods_locked,
                                  UInt32       group) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboardRepeatInfoHandler(void*        data,
                                   wl_keyboard* keyboard,
                                   Int32        rate,
                                   Int32        delay) noexcept -> void;

    namespace {
        auto globals = Globals {};

        constinit const auto stormkit_registry_listener
            = wl_registry_listener { .global        = registryHandler,
                                     .global_remove = registryRemoverHandler };

        constinit const auto stormkit_surface_listener
            = wl_surface_listener { .enter = surfaceEnterHandler, .leave = surfaceLeaveHandler };

        constinit const auto stormkit_xdg_surface_listener
            = xdg_surface_listener { .configure = surfaceConfigureHandler };

        constinit const auto stormkit_xdg_toplevel_listener
            = xdg_toplevel_listener { .configure = toplevelConfigureHandler,
                                      .close     = toplevelCloseHandler };

        constinit const auto stormkit_shell_listener
            = xdg_wm_base_listener { .ping = shellPingHandler };

        constinit const auto stormkit_shell_surface_listener
            = wl_shell_surface_listener { .ping       = shellPingHandler,
                                          .configure  = shellSurfaceConfigureHandler,
                                          .popup_done = nullptr };

        constinit const auto stormkit_relative_pointer_listener
            = zwp_relative_pointer_v1_listener { .relative_motion
                                                 = relativePointerRelativeMotionHandler };

        constinit const auto stormkit_locked_pointer_listener
            = zwp_locked_pointer_v1_listener { .locked   = lockedPointerLockedHandler,
                                               .unlocked = lockedPointerUnlockedHandler };
    } // namespace

    void init() {
        globals.display.reset(wl_display_connect(nullptr));

        if (globals.display) dlog("Wayland context initialized");
        else {
            flog("Failed to initialize wayland");
            std::exit(EXIT_FAILURE);
        }

        globals.registry.reset(wl_display_get_registry(globals.display.get()));

        wl_registry_add_listener(globals.registry.get(), &stormkit_registry_listener, &globals);

        wl_display_dispatch(globals.display.get());
        wl_display_roundtrip(globals.display.get());

        if (globals.compositor) dlog("Wayland compositor found !");
        else {
            flog("Failed to find a Wayland compositor");
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

        m_xkb_context = common::getXKBContext();

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
    auto WindowImpl::create(std::string          title,
                            const math::ExtentU& extent,
                            WindowStyle          style) -> void {
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
            createXDGShell();
        } else {
            dlog("XDGShell not found, falling back to WLShell");

            if (globals.wayland_shell) createWaylandShell();
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
    auto WindowImpl::pollEvent(Event& event) noexcept -> bool {
        while (!m_configured) wl_display_dispatch(globals.display.get());

        while (wl_display_prepare_read(globals.display.get()) != 0)
            wl_display_dispatch_pending(globals.display.get());

        wl_display_flush(globals.display.get());
        wl_display_read_events(globals.display.get());
        wl_display_dispatch_pending(globals.display.get());

        return WindowImplBase::pollEvent(event);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::waitEvent(Event& event) noexcept -> bool {
        while (!m_configured) wl_display_dispatch(globals.display.get());

        while (wl_display_prepare_read(globals.display.get()) != 0)
            wl_display_dispatch(globals.display.get());
        wl_display_flush(globals.display.get());

        return WindowImplBase::waitEvent(event);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::setTitle(std::string title) noexcept -> void {
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
    auto WindowImpl::setExtent([[maybe_unused]] const math::ExtentU& extent) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::setFullscreenEnabled(bool enabled) noexcept -> void {
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
    auto WindowImpl::lockMouse() noexcept -> void {
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

        m_mouse_locked = true;

        hideMouse();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::unlockMouse() noexcept -> void {
        if (!m_open) return;

        m_locked_mouse_position = m_mouse_state.position_in_window;
        m_locked_pointer.reset();
        m_relative_pointer.reset();

        m_mouse_locked = false;

        unhideMouse();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::hideMouse() noexcept -> void {
        if (!m_open) return;
        wl_pointer_set_cursor(m_pointer, m_pointer_serial, nullptr, 0, 0);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::unhideMouse() noexcept -> void {
        if (!m_open) return;
        wl_pointer_set_cursor(m_pointer, m_pointer_serial, m_cursor_surface.get(), 0, 0);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::nativeHandle() const noexcept -> NativeHandle {
        return std::bit_cast<NativeHandle>(&m_handles);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::setKeyRepeatEnabled([[maybe_unused]] bool enabled) noexcept -> void {
        elog("wayland::WindowImpl::setKeyRepeatEnabled isn't yet implemented");
        m_key_repeat_enabled = false;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::setVirtualKeyboardVisible([[maybe_unused]] bool visible) noexcept -> void {
        elog("wayland::WindowImpl::setVirtualKeyboardVisible isn't yet implemented");
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::setMousePosition(const math::Vector2I& position) noexcept -> void {
        if (!m_open) return;
        if (m_mouse_locked) {
            zwp_locked_pointer_v1_set_cursor_position_hint(m_locked_pointer.get(),
                                                           wl_fixed_from_int(position.x),
                                                           wl_fixed_from_int(position.y));
            wl_surface_commit(m_surface.get());
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::setMousePositionOnDesktop(
        [[maybe_unused]] const math::Vector2U& position) noexcept -> void {
        elog("wayland::WindowImpl::setMousePositionOnDesktop isn't yet implemented");
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::getMonitorSettings() -> std::vector<Monitor> {
        if (!globals.display) init();

        auto output = transform(globals.monitors, [](const auto& pair) { return pair.second; });

        return output;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::surfaceOutputEnter([[maybe_unused]] wl_surface* surface,
                                        wl_output*                   output) noexcept -> void {
        m_current_output.reset(output);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::surfaceConfigure(xdg_surface* surface, std::uint32_t serial) noexcept -> void {
        dlog("XDG surface configure, serial: {}", serial);

        xdg_surface_ack_configure(surface, serial);

        wl_surface_damage(m_surface.get(), 0, 0, m_extent.width, m_extent.height);
        createPixelBuffer();

        m_configured = true;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::toplevelConfigure([[maybe_unused]] xdg_toplevel* xdg_tl,
                                       std::int32_t                   width,
                                       std::int32_t                   height,
                                       wl_array*                      state) noexcept -> void {
        dlog("XDG Shell configure: {}:{}", width, height);

        m_open    = true;
        m_visible = width > 0 && height > 0;

        auto data = static_cast<xdg_toplevel_state*>(state->data);
        for (auto i : range(state->size)) {
            const auto state = data[i];

            switch (state) {
                case XDG_TOPLEVEL_STATE_MAXIMIZED: WindowImplBase::maximizeEvent(); break;
                case XDG_TOPLEVEL_STATE_RESIZING: {
                    WindowImplBase::resizeEvent(width, height);
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
    auto WindowImpl::toplevelClose([[maybe_unused]] xdg_toplevel* xdg_tl) noexcept -> void {
        m_open          = false;
        m_visible       = false;
        m_extent.width  = 0;
        m_extent.height = 0;

        WindowImplBase::closeEvent();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::shellSurfaceConfigure([[maybe_unused]] wl_shell_surface* xdg_tl,
                                           [[maybe_unused]] std::uint32_t     edges,
                                           std::int32_t                       width,
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
    auto
        WindowImpl::seatCapabilities([[maybe_unused]] wl_seat*      seat,
                                     [[maybe_unused]] std::uint32_t capabilities) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::pointerEnter([[maybe_unused]] wl_pointer* pointer,
                                  std::uint32_t                serial,
                                  [[maybe_unused]] wl_surface* surface,
                                  [[maybe_unused]] wl_fixed_t  surface_x,
                                  [[maybe_unused]] wl_fixed_t  surface_y) noexcept -> void {
        m_pointer_serial = serial;

        if (!m_mouse_locked) unhideMouse();
        else
            hideMouse();

        WindowImplBase::mouseEnteredEvent();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::pointerLeave([[maybe_unused]] wl_pointer*   pointer,
                                  [[maybe_unused]] std::uint32_t serial,
                                  [[maybe_unused]] wl_surface*   surface) noexcept -> void {
        m_pointer_serial = serial;

        WindowImplBase::mouseExitedEvent();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::pointerMotion([[maybe_unused]] wl_pointer*   pointer,
                                   [[maybe_unused]] std::uint32_t time,
                                   wl_fixed_t                     surface_x,
                                   wl_fixed_t                     surface_y) noexcept -> void {
        if (m_mouse_locked) return;

        m_mouse_state.position_in_window.x = wl_fixed_to_int(surface_x);
        m_mouse_state.position_in_window.y = wl_fixed_to_int(surface_y);

        WindowImplBase::mouseMoveEvent(m_mouse_state.position_in_window.x,
                                       m_mouse_state.position_in_window.y);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::pointerButton([[maybe_unused]] wl_pointer*   pointer,
                                   [[maybe_unused]] std::uint32_t serial,
                                   [[maybe_unused]] std::uint32_t time,
                                   std::uint32_t                  button,
                                   std::uint32_t                  state) noexcept -> void {
#define BUTTON_HANDLER(a, b)                                                          \
    case a: {                                                                         \
        auto it  = std::ranges::find_if(m_mouse_state.button_state,                   \
                                       [](const auto& s) { return s.button == a; }); \
        it->down = down;                                                              \
        if (down)                                                                     \
            WindowImplBase::mouseDownEvent(b,                                         \
                                           m_mouse_state.position_in_window.x,        \
                                           m_mouse_state.position_in_window.y);       \
        else                                                                          \
            WindowImplBase::mouseUpEvent(b,                                           \
                                         m_mouse_state.position_in_window.x,          \
                                         m_mouse_state.position_in_window.y);         \
        break;                                                                        \
    }

        m_pointer_serial = serial;

        const auto down = !!state;

        switch (button) {
            BUTTON_HANDLER(BTN_LEFT, MouseButton::Left)
            BUTTON_HANDLER(BTN_RIGHT, MouseButton::Right)
            BUTTON_HANDLER(BTN_MIDDLE, MouseButton::Middle)
            BUTTON_HANDLER(BTN_FORWARD, MouseButton::Button1)
            BUTTON_HANDLER(BTN_BACK, MouseButton::Button2)
            default:
                if (down)
                    WindowImplBase::mouseDownEvent(MouseButton::Unknow,
                                                   m_mouse_state.position_in_window.x,
                                                   m_mouse_state.position_in_window.y);
                else
                    WindowImplBase::mouseUpEvent(MouseButton::Unknow,
                                                 m_mouse_state.position_in_window.x,
                                                 m_mouse_state.position_in_window.y);
        }

#undef BUTTON_HANDLER
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::keyboardKeymap([[maybe_unused]] wl_keyboard* keyboard,
                                    std::uint32_t                 format,
                                    std::int32_t                  fd,
                                    std::uint32_t                 size) noexcept -> void {
        if (format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
            auto map_shm
                = reinterpret_cast<char*>(mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0));

            updateKeymap(std::string_view { map_shm, size });

            munmap(map_shm, size);
            ::close(fd);
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::keyboardEnter([[maybe_unused]] wl_keyboard*  keyboard,
                                   [[maybe_unused]] std::uint32_t serial,
                                   [[maybe_unused]] wl_surface*   surface,
                                   wl_array*                      keys) noexcept -> void {
        WindowImplBase::gainedFocusEvent();

        auto data = static_cast<std::uint32_t*>(keys->data);
        for (auto i : range(keys->size)) {
            const auto keycode = data[i] + 8;

            auto character = char {};

            const auto symbol = xkb_state_key_get_one_sym(m_xkb_state.get(), keycode);
            xkb_state_key_get_utf8(m_xkb_state.get(), keycode, &character, sizeof(char));

            const auto skey = common::xkbKeyToStormkitKey(symbol);

            WindowImplBase::keyDownEvent(skey, character);
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::keyboardLeave([[maybe_unused]] wl_keyboard*  keyboard,
                                   [[maybe_unused]] std::uint32_t serial,
                                   [[maybe_unused]] wl_surface*   surface) noexcept -> void {
        WindowImplBase::lostFocusEvent();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::keyboardKey([[maybe_unused]] wl_keyboard*  keyboard,
                                 [[maybe_unused]] std::uint32_t serial,
                                 [[maybe_unused]] std::uint32_t time,
                                 std::uint32_t                  key,
                                 std::uint32_t                  state) noexcept -> void {
        auto character = char {};

        const auto keycode = key + 8;

        const auto symbol = xkb_state_key_get_one_sym(m_xkb_state.get(), keycode);
        xkb_state_key_get_utf8(m_xkb_state.get(), keycode, &character, sizeof(char));

        const auto skey = common::xkbKeyToStormkitKey(symbol);

        const auto down = state == WL_KEYBOARD_KEY_STATE_PRESSED;

        auto it = std::ranges::find_if(m_keyboard_state,
                                       [symbol](const auto& s) { return s.key == symbol; });

        it->down = down;
        if (down) WindowImplBase::keyDownEvent(skey, character);
        else
            WindowImplBase::keyUpEvent(skey, character);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::keyboardModifiers([[maybe_unused]] wl_keyboard*  keyboard,
                                       [[maybe_unused]] std::uint32_t serial,
                                       std::uint32_t                  mods_depressed,
                                       std::uint32_t                  mods_latched,
                                       std::uint32_t                  mods_locked,
                                       std::uint32_t                  group) noexcept -> void {
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
    auto WindowImpl::keyboardRepeatInfo([[maybe_unused]] wl_keyboard* keyboard,
                                        [[maybe_unused]] std::int32_t rate,
                                        [[maybe_unused]] std::int32_t delay) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto
        WindowImpl::relativePointerRelativeMotion([[maybe_unused]] zwp_relative_pointer_v1* pointer,
                                                  [[maybe_unused]] std::uint32_t            time_hi,
                                                  [[maybe_unused]] std::uint32_t            time_lw,
                                                  [[maybe_unused]] wl_fixed_t               dx,
                                                  [[maybe_unused]] wl_fixed_t               dy,
                                                  wl_fixed_t dx_unaccel,
                                                  wl_fixed_t dy_unaccel) noexcept -> void {
        m_mouse_state.position_in_window.x += wl_fixed_to_int(dx_unaccel);
        m_mouse_state.position_in_window.y += wl_fixed_to_int(dy_unaccel);

        WindowImplBase::mouseMoveEvent(m_mouse_state.position_in_window.x,
                                       m_mouse_state.position_in_window.y);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::createXDGShell() noexcept -> void {
        xdg_wm_base_add_listener(globals.xdg_shell.get(), &stormkit_shell_listener, nullptr);

        m_xdg_surface.reset(xdg_wm_base_get_xdg_surface(globals.xdg_shell.get(), m_surface.get()));

        xdg_surface_add_listener(m_xdg_surface.get(), &stormkit_xdg_surface_listener, this);

        m_xdg_toplevel.reset(xdg_surface_get_toplevel(m_xdg_surface.get()));

        xdg_toplevel_add_listener(m_xdg_toplevel.get(), &stormkit_xdg_toplevel_listener, this);

        xdg_toplevel_set_title(m_xdg_toplevel.get(), m_title.c_str());
        xdg_toplevel_set_app_id(m_xdg_toplevel.get(), m_title.c_str());

        if (!checkFlag(m_style, WindowStyle::Resizable)) {
            xdg_toplevel_set_min_size(m_xdg_toplevel.get(), m_extent.width, m_extent.height);
            xdg_toplevel_set_max_size(m_xdg_toplevel.get(), m_extent.width, m_extent.height);
        } else {
            const auto fullscreen_size = [] {
                const auto monitors = getMonitorSettings();
                for (const auto& monitor : monitors)
                    if (monitor.flags == Monitor::Flags::Primary) return monitor.extents[0];

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
    auto WindowImpl::createWaylandShell() noexcept -> void {
        m_wlshell_surface.reset(
            wl_shell_get_shell_surface(globals.wayland_shell.get(), m_surface.get()));
        wl_shell_surface_add_listener(m_wlshell_surface.get(),
                                      &stormkit_shell_surface_listener,
                                      this);
        wl_shell_surface_set_toplevel(m_wlshell_surface.get());

        wl_surface_damage(m_surface.get(), 0, 0, m_extent.width, m_extent.height);
        createPixelBuffer();
    }

    auto WindowImpl::createPixelBuffer() noexcept -> void {
        const auto buffer_size   = m_extent.width * m_extent.height * 4;
        const auto buffer_stride = m_extent.width * 4;

        auto fd = syscall(SYS_memfd_create, "buffer", 0);
        ftruncate(fd, buffer_size);

        [[maybe_unused]] auto* data
            = mmap(nullptr, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

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
    auto WindowImpl::updateKeymap(std::string_view keymap_string) noexcept -> void {
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
            = wl_output_listener { .geometry = outputGeometryHandler,
                                   .mode     = outputModeHandler,
                                   .done     = outputDoneHandler,
                                   .scale    = outputScaleHandler };

        constinit const auto stormkit_seat_listener
            = wl_seat_listener { .capabilities = seatCapabilitiesHandler, .name = seatNameHandler };

        constinit const auto stormkit_pointer_listener
            = wl_pointer_listener { .enter         = pointerEnterHandler,
                                    .leave         = pointerLeaveHandler,
                                    .motion        = pointerMotionHandler,
                                    .button        = pointerButtonHandler,
                                    .axis          = pointerAxisHandler,
                                    .frame         = pointerFrameHandler,
                                    .axis_source   = pointerAxisSourceHandler,
                                    .axis_stop     = pointerAxisStopHandler,
                                    .axis_discrete = pointerAxisDiscreteHandler };

        constinit const auto stormkit_keyboard_listener
            = wl_keyboard_listener { .keymap      = keyboardKeymapHandler,
                                     .enter       = keyboardEnterHandler,
                                     .leave       = keyboardLeaveHandler,
                                     .key         = keyboardKeyHandler,
                                     .modifiers   = keyboardModifiersHandler,
                                     .repeat_info = keyboardRepeatInfoHandler };

        // TODO support touchscreens
        [[maybe_unused]] constinit const auto stormkit_touchscreen_listener = wl_touch_listener {};
    } // namespace

    /////////////////////////////////////
    /////////////////////////////////////
    auto registryHandler(void*        data,
                         wl_registry* registry,
                         UInt32       id,
                         const char*  interface,
                         UInt32       version) noexcept -> void {
        auto& globals = *static_cast<Globals*>(data);

#define BIND(n, t, v) n.reset(static_cast<t*>(wl_registry_bind(registry, id, &t##_interface, v)));

        dlog("Wayland registry acquired {} (id: {}, version: {})", interface, id, version);

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
    auto registryRemoverHandler([[maybe_unused]] void*        data,
                                [[maybe_unused]] wl_registry* registry,
                                UInt32                        id) noexcept -> void {
        dlog("Wayland registry lost {}", id);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto outputGeometryHandler(void*                       data,
                               [[maybe_unused]] wl_output* output,
                               [[maybe_unused]] Int32      x,
                               [[maybe_unused]] Int32      y,
                               [[maybe_unused]] Int32      pwidth,
                               [[maybe_unused]] Int32      pheight,
                               [[maybe_unused]] Int32      subpixels,
                               const char*                 make,
                               const char*                 model,
                               [[maybe_unused]] Int32      transform) noexcept -> void {
        auto& globals = *static_cast<Globals*>(data);
        auto& monitor = globals.monitors[output];

        monitor.name = std::format("{} {}", make, model);

        for (auto& [_, m] : globals.monitors) { m.flags = Monitor::Flags::None; }

        if (&monitor == &globals.monitors.begin()->second) monitor.flags = Monitor::Flags::Primary;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto outputModeHandler(void*                       data,
                           [[maybe_unused]] wl_output* wl_output,
                           [[maybe_unused]] UInt32     flags,
                           Int32                       width,
                           Int32                       height,
                           [[maybe_unused]] Int32      refresh) noexcept -> void {
        auto& globals = *static_cast<Globals*>(data);
        auto& monitor = globals.monitors[wl_output];

        monitor.extents.emplace_back(as<UInt32>(width), as<UInt32>(height));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto outputDoneHandler([[maybe_unused]] void*      data,
                           [[maybe_unused]] wl_output* wl_output) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto outputScaleHandler([[maybe_unused]] void*      data,
                            [[maybe_unused]] wl_output* wl_output,
                            [[maybe_unused]] Int32      factor) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto surfaceEnterHandler(void* data, wl_surface* surface, wl_output* output) noexcept -> void {
        auto* window = static_cast<WindowImpl*>(data);
        window->surfaceOutputEnter(surface, output);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto surfaceLeaveHandler([[maybe_unused]] void*       data,
                             [[maybe_unused]] wl_surface* surface,
                             [[maybe_unused]] wl_output*  output) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto toplevelConfigureHandler(void*         data,
                                  xdg_toplevel* xdg_tl,
                                  Int32         width,
                                  Int32         height,
                                  wl_array*     states) noexcept -> void {
        auto* window = static_cast<WindowImpl*>(data);
        window->toplevelConfigure(xdg_tl, width, height, states);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto toplevelCloseHandler(void* data, xdg_toplevel* xdg_tl) noexcept -> void {
        auto* window = static_cast<WindowImpl*>(data);
        window->toplevelClose(xdg_tl);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto surfaceConfigureHandler(void* data, xdg_surface* surface, UInt32 serial) noexcept -> void {
        auto window = static_cast<WindowImpl*>(data);
        window->surfaceConfigure(surface, serial);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto shellPingHandler([[maybe_unused]] void* data,
                          xdg_wm_base*           xdg_shell,
                          UInt32                 serial) noexcept -> void {
        ilog("Ping received from shell");

        xdg_wm_base_pong(xdg_shell, serial);
    }

    auto shellSurfaceConfigureHandler(void*             data,
                                      wl_shell_surface* shell_surface,
                                      UInt32            edges,
                                      Int32             width,
                                      Int32             height) noexcept -> void {
        auto* window = static_cast<WindowImpl*>(data);
        window->shellSurfaceConfigure(shell_surface, edges, width, height);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto shellPingHandler([[maybe_unused]] void* data,
                          wl_shell_surface*      shell_surface,
                          UInt32                 serial) noexcept -> void {
        ilog("Ping received from shell");

        wl_shell_surface_pong(shell_surface, serial);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto seatCapabilitiesHandler(void*                     data,
                                 [[maybe_unused]] wl_seat* seat,
                                 UInt32                    capabilities) noexcept -> void {
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
    auto seatNameHandler([[maybe_unused]] void*    data,
                         [[maybe_unused]] wl_seat* seat,
                         const char*               name) noexcept -> void {
        dlog("WL Seat found! {}", name);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointerEnterHandler(void*       data,
                             wl_pointer* pointer,
                             UInt32      serial,
                             wl_surface* surface,
                             wl_fixed_t  surface_x,
                             wl_fixed_t  surface_y) noexcept -> void {
        if (data == nullptr) return;

        auto* window = static_cast<WindowImpl*>(data);
        window->pointerEnter(pointer, serial, surface, surface_x, surface_y);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointerLeaveHandler(void*       data,
                             wl_pointer* pointer,
                             UInt32      serial,
                             wl_surface* surface) noexcept -> void {
        if (data == nullptr) return;

        auto* window = static_cast<WindowImpl*>(data);
        window->pointerLeave(pointer, serial, surface);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointerMotionHandler(void*       data,
                              wl_pointer* pointer,
                              UInt32      time,
                              wl_fixed_t  surface_x,
                              wl_fixed_t  surface_y) noexcept -> void {
        if (data == nullptr) return;

        auto* window = static_cast<WindowImpl*>(data);
        window->pointerMotion(pointer, time, surface_x, surface_y);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointerButtonHandler(void*       data,
                              wl_pointer* pointer,
                              UInt32      serial,
                              UInt32      time,
                              UInt32      button,
                              UInt32      state) noexcept -> void {
        if (data == nullptr) return;

        auto* window = static_cast<WindowImpl*>(data);
        window->pointerButton(pointer, serial, time, button, state);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointerAxisHandler([[maybe_unused]] void*       data,
                            [[maybe_unused]] wl_pointer* pointer,
                            [[maybe_unused]] UInt32      time,
                            [[maybe_unused]] UInt32      axis,
                            [[maybe_unused]] wl_fixed_t  value) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointerFrameHandler([[maybe_unused]] void*       data,
                             [[maybe_unused]] wl_pointer* pointer) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointerAxisSourceHandler([[maybe_unused]] void*       data,
                                  [[maybe_unused]] wl_pointer* pointer,
                                  [[maybe_unused]] UInt32      axis_source) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointerAxisStopHandler([[maybe_unused]] void*       data,
                                [[maybe_unused]] wl_pointer* pointer,
                                [[maybe_unused]] UInt32      time,
                                [[maybe_unused]] UInt32      axis) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto pointerAxisDiscreteHandler([[maybe_unused]] void*       data,
                                    [[maybe_unused]] wl_pointer* pointer,
                                    [[maybe_unused]] UInt32      axis,
                                    [[maybe_unused]] Int32       discrete) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboardKeymapHandler(void*        data,
                               wl_keyboard* keyboard,
                               UInt32       format,
                               Int32        fd,
                               UInt32       size) noexcept -> void {
        if (data == nullptr) return;

        auto* window = static_cast<WindowImpl*>(data);
        window->keyboardKeymap(keyboard, format, fd, size);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboardEnterHandler(void*        data,
                              wl_keyboard* keyboard,
                              UInt32       serial,
                              wl_surface*  surface,
                              wl_array*    keys) noexcept -> void {
        if (data == nullptr) return;

        auto* window = static_cast<WindowImpl*>(data);
        window->keyboardEnter(keyboard, serial, surface, keys);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboardLeaveHandler(void*        data,
                              wl_keyboard* keyboard,
                              UInt32       serial,
                              wl_surface*  surface) noexcept -> void {
        if (data == nullptr) return;

        auto* window = static_cast<WindowImpl*>(data);
        window->keyboardLeave(keyboard, serial, surface);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboardKeyHandler(void*        data,
                            wl_keyboard* keyboard,
                            UInt32       serial,
                            UInt32       time,
                            UInt32       key,
                            UInt32       state) noexcept -> void {
        if (data == nullptr) return;

        auto* window = static_cast<WindowImpl*>(data);
        window->keyboardKey(keyboard, serial, time, key, state);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboardModifiersHandler(void*        data,
                                  wl_keyboard* keyboard,
                                  UInt32       serial,
                                  UInt32       mods_depressed,
                                  UInt32       mods_latcher,
                                  UInt32       mods_locked,
                                  UInt32       group) noexcept -> void {
        if (data == nullptr) return;

        auto* window = static_cast<WindowImpl*>(data);
        window
            ->keyboardModifiers(keyboard, serial, mods_depressed, mods_latcher, mods_locked, group);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto keyboardRepeatInfoHandler(void*        data,
                                   wl_keyboard* keyboard,
                                   Int32        rate,
                                   Int32        delay) noexcept -> void {
        if (data == nullptr) return;

        auto* window = static_cast<WindowImpl*>(data);
        window->keyboardRepeatInfo(keyboard, rate, delay);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto relativePointerRelativeMotionHandler(void*                    data,
                                              zwp_relative_pointer_v1* pointer,
                                              UInt32                   time_hi,
                                              UInt32                   time_lw,
                                              wl_fixed_t               dx,
                                              wl_fixed_t               dy,
                                              wl_fixed_t               dx_unaccel,
                                              wl_fixed_t dy_unaccel) noexcept -> void {
        auto* window = static_cast<WindowImpl*>(data);
        window->relativePointerRelativeMotion(pointer,
                                              time_hi,
                                              time_lw,
                                              dx,
                                              dy,
                                              dx_unaccel,
                                              dy_unaccel);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto lockedPointerLockedHandler([[maybe_unused]] void*                  data,
                                    [[maybe_unused]] zwp_locked_pointer_v1* locked_pointer) noexcept
        -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto lockedPointerUnlockedHandler(
        [[maybe_unused]] void*                  data,
        [[maybe_unused]] zwp_locked_pointer_v1* locked_pointer) noexcept -> void {
    }
} // namespace stormkit::wsi::linux::wayland
