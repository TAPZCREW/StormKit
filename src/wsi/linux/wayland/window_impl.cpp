module;

#include <linux/input-event-codes.h>
#include <unistd.h>

#include <xkbcommon/xkbcommon.h>

#include <wayland-client.h>
#include <wayland-cursor.h>
#include <xdg-shell.h>

#include <content-type-v1.h>
#include <cursor-shape-v1.h>
#include <pointer-constraints-unstable-v1.h>
#include <pointer-warp-v1.h>
#include <relative-pointer-unstable-v1.h>
#include <single-pixel-buffer-v1.h>
#include <viewporter.h>
#include <xdg-decoration-unstable-v1.h>

#include <stormkit/core/contract_macro.hpp>

module stormkit.wsi;

import std;

import stormkit.core;

import :linux.wayland.window_impl;
import :linux.wayland;
import :linux.wayland.context;
import :linux.wayland.input;
import :linux.wayland.log;

namespace stdr = std::ranges;
namespace stdv = std::views;

namespace stormkit::wsi::linux::wayland {
    namespace wl {
        auto xdg_surface_configure_handler(void*, xdg_surface*, u32) noexcept -> void;

        auto xdg_top_level_configure_bounds_handler(void*, xdg_toplevel*, i32, i32) noexcept
          -> void;
        auto xdg_top_level_configure_handler(void*, xdg_toplevel*, i32, i32, wl_array*) noexcept
          -> void;
        auto xdg_top_level_close_handler(void*, xdg_toplevel*) noexcept -> void;

        auto xdg_top_level_decoration_configure_handler(void*,
                                                        zxdg_toplevel_decoration_v1*,
                                                        u32) noexcept -> void;

        auto buffer_release_handler(void*, wl_buffer*) noexcept -> void;

        auto surface_enter_handler(void*, wl_surface*, wl_output*) noexcept -> void;
        auto surface_leave_handler(void*, wl_surface*, wl_output*) noexcept -> void;

        namespace {
            constexpr auto g_xdg_surface_listener = xdg_surface_listener {
                .configure = xdg_surface_configure_handler,
            };

            constexpr auto g_xdg_top_level_listener = xdg_toplevel_listener {
                .configure        = xdg_top_level_configure_handler,
                .close            = xdg_top_level_close_handler,
                .configure_bounds = xdg_top_level_configure_bounds_handler,
                .wm_capabilities  = nullptr
            };

            constexpr auto
              g_xdg_top_level_decoration_listener = zxdg_toplevel_decoration_v1_listener {
                  .configure = xdg_top_level_decoration_configure_handler,
              };

            constexpr auto g_buffer_listener = wl_buffer_listener {
                .release = buffer_release_handler
            };

            constexpr auto g_surface_listener = wl_surface_listener {
                .enter                      = surface_enter_handler,
                .leave                      = surface_leave_handler,
                .preferred_buffer_scale     = nullptr,
                .preferred_buffer_transform = nullptr,
            };

            constexpr auto g_confined_pointer_listener = zwp_confined_pointer_v1_listener {
                .confined   = pointer_contraints_confined_handler,
                .unconfined = pointer_contraints_unconfined_handler,
            };

            constexpr auto g_locked_pointer_listener = zwp_locked_pointer_v1_listener {
                .locked   = pointer_contraints_locked_handler,
                .unlocked = pointer_contraints_unlocked_handler,
            };

            constexpr auto g_relative_pointer_listener = zwp_relative_pointer_v1_listener {
                .relative_motion = relative_pointer_relative_motion_handler
            };
        } // namespace
    } // namespace wl

    /////////////////////////////////////
    /////////////////////////////////////
    WindowImpl::WindowImpl() {
        wl::init();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    WindowImpl::WindowImpl(std::string title, const math::Extent2<u32>& extent, WindowFlag flags)
        : WindowImpl {} {
        create(std::move(title), extent, flags);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    WindowImpl::~WindowImpl() {
        auto& globals = wl::get_globals();
        if (globals.display) wl_display_flush(globals.display);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    WindowImpl::WindowImpl(WindowImpl&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::operator=(WindowImpl&&) noexcept -> WindowImpl& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::create(std::string               title,
                            const math::Extent2<u32>& extent,
                            WindowFlag                flags) noexcept -> void {
        auto& globals = wl::get_globals();

        m_surface = wl::Surface { std::in_place, globals.compositor };
        wl_surface_add_listener(m_surface, &wl::g_surface_listener, this);

        m_xdg_surface = wl::XDGSurface { std::in_place, globals.xdg_wm_base, m_surface };
        xdg_surface_add_listener(m_xdg_surface, &wl::g_xdg_surface_listener, this);

        m_xdg_top_level = wl::XDGTopLevel { std::in_place, m_xdg_surface };
        xdg_toplevel_add_listener(m_xdg_top_level, &wl::g_xdg_top_level_listener, this);

        const auto app_id = std::format("StormKit/{}", title);
        xdg_toplevel_set_app_id(m_xdg_top_level, stdr::data(app_id));

        if (globals.decoration_manager) {
            m_xdg_top_level_decoration = wl::XDGTopLevelDecoration { std::in_place,
                                                                     globals.decoration_manager,
                                                                     m_xdg_top_level };
            zxdg_toplevel_decoration_v1_set_mode(m_xdg_top_level_decoration,
                                                 ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
            zxdg_toplevel_decoration_v1_add_listener(m_xdg_top_level_decoration,
                                                     &wl::g_xdg_top_level_decoration_listener,
                                                     nullptr);
        }

        if (globals.content_type_manager) {
            m_content_type = wl::ContentType { std::in_place,
                                               globals.content_type_manager,
                                               m_surface };
            // game
            wp_content_type_v1_set_content_type(m_content_type, 3);
        }

        globals.windows.emplace_back(m_surface, this);

        m_title           = std::move(title);
        m_state.extent    = extent;
        m_state.visible   = true;
        m_state.has_focus = true;
        m_flags           = flags;
        m_open            = true;
        m_handles.display = globals.display;
        m_handles.surface = m_surface;

        if (globals.viewporter) {
            m_viewport = wl::Viewport { std::in_place, globals.viewporter, m_surface };
            wp_viewport_set_destination(m_viewport, m_state.extent.width, m_state.extent.height);
        }

        reallocate_pixel_buffer();
        clear(RGBColorDef::BLACK<u8>);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::close() noexcept -> void {
        auto& globals = wl::get_globals();
        wl_display_flush(globals.display);

        m_state = {};
        m_title.clear();
        m_flags      = {};
        m_open       = false;
        m_configured = false;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::clear(const RGBColorU& color) noexcept -> void {
        const auto value = (color.alpha << 24)
                           + (color.red << 16)
                           + (color.green << 8)
                           + (color.blue);

        auto view = std::span<i32> { std::bit_cast<i32*>(m_shm_buffer.get().begin()),
                                     m_shm_buffer->size() / sizeof(i32) };
        stdr::fill(view, value);

        wl_surface_damage(m_surface,
                          0,
                          0,
                          as<i32>(m_state.extent.width),
                          as<i32>(m_state.extent.height));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::set_pixels_to(std::span<const RGBColorU> colors) noexcept -> void {
        auto view = std::span<i32> { std::bit_cast<i32*>(m_shm_buffer.get().begin()),
                                     m_shm_buffer->size() / sizeof(i32) };
        stdr::copy(colors | stdv::transform([](const auto& color) static noexcept {
                       return (color.alpha << 24)
                              + (color.red << 16)
                              + (color.green << 8)
                              + (color.blue);
                   }),
                   stdr::begin(view));

        wl_surface_damage(m_surface,
                          0,
                          0,
                          as<i32>(m_state.extent.width),
                          as<i32>(m_state.extent.height));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::poll_event(Event& event) noexcept -> bool {
        auto& globals = wl::get_globals();

        while (wl_display_prepare_read(globals.display) != 0)
            wl_display_dispatch_pending(globals.display);

        wl_display_flush(globals.display);
        wl_display_read_events(globals.display);
        wl_display_dispatch_pending(globals.display);

        handle_key_repeat();

        return WindowImplBase::poll_event(event);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::wait_event(Event& event) noexcept -> bool {
        auto& globals = wl::get_globals();

        while (wl_display_prepare_read(globals.display) != 0) wl_display_dispatch(globals.display);
        wl_display_flush(globals.display);

        handle_key_repeat();

        return WindowImplBase::wait_event(event);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::set_title(std::string title) noexcept -> void {
        if (!m_open) return;
        m_title = std::move(title);

        xdg_toplevel_set_title(m_xdg_top_level, stdr::data(m_title));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::set_extent(const math::Extent2<u32>&) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::set_fullscreen(bool enabled) noexcept -> void {
        if (not m_open or not m_current_output) return;

        if (enabled) xdg_toplevel_set_fullscreen(m_xdg_top_level, m_current_output);
        else
            xdg_toplevel_unset_fullscreen(m_xdg_top_level);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::confine_mouse(bool confined, u32 mouse_id) noexcept -> void {
        if (not m_open) return;

        auto& globals = wl::get_globals();
        if (not globals.pointer_constraints) {
            elog("{} protocol is not supported by this DE, can't confine mouse.",
                 zwp_pointer_constraints_v1_interface.name);
            return;
        }

        EXPECTS(mouse_id < globals.pointers.size());
        auto& [pointer, state] = globals.pointers[mouse_id];

        if (not state.serial or check_flag_bit(state.flags, wl::PointerState::Flag::LOCKED)) return;

        if (confined) {
            if (not check_flag_bit(state.flags, wl::PointerState::Flag::CONFINED)) {
                state.confined_pointer = wl::ConfinedPointer {
                    std::in_place, globals.pointer_constraints,
                    m_surface,     pointer,
                    nullptr,       ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_ONESHOT
                };

                zwp_confined_pointer_v1_add_listener(state.confined_pointer,
                                                     &wl::g_confined_pointer_listener,
                                                     &state);
                state.flags |= wl::PointerState::Flag::CONFINED;
            }
        } else {
            state.confined_pointer.reset();

            state.flags &= ~wl::PointerState::Flag::CONFINED;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_CONST
    auto WindowImpl::is_mouse_confined(u32 mouse_id) const noexcept -> bool {
        auto& globals = wl::get_globals();

        EXPECTS(mouse_id < globals.pointers.size());
        const auto& [_, state] = globals.pointers[mouse_id];

        return check_flag_bit(state.flags, wl::PointerState::Flag::CONFINED);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::lock_mouse(bool locked, u32 mouse_id) noexcept -> void {
        if (not m_open) return;

        auto& globals = wl::get_globals();
        if (not globals.pointer_constraints) {
            elog("{} protocol is not supported by this DE, can't lock mouse.",
                 zwp_pointer_constraints_v1_interface.name);
            return;
        }

        EXPECTS(mouse_id < globals.pointers.size());
        auto& [pointer, state] = globals.pointers[mouse_id];

        if (not state.serial or check_flag_bit(state.flags, wl::PointerState::Flag::CONFINED))
            return;

        if (locked) {
            if (not check_flag_bit(state.flags, wl::PointerState::Flag::LOCKED)) {
                state.locked_pointer = wl::LockedPointer {
                    std::in_place, globals.pointer_constraints,
                    m_surface,     pointer,
                    nullptr,       ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT
                };

                zwp_locked_pointer_v1_add_listener(state.locked_pointer,
                                                   &wl::g_locked_pointer_listener,
                                                   &state);

                state.flags |= wl::PointerState::Flag::LOCKED;
            }
        } else {
            state.locked_pointer.reset();

            state.flags &= ~wl::PointerState::Flag::LOCKED;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_CONST
    auto WindowImpl::is_mouse_locked(u32 mouse_id) const noexcept -> bool {
        auto& globals = wl::get_globals();

        EXPECTS(mouse_id < globals.pointers.size());
        const auto& [_, state] = globals.pointers[mouse_id];

        return check_flag_bit(state.flags, wl::PointerState::Flag::LOCKED);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::hide_mouse(bool hidden, u32 mouse_id) noexcept -> void {
        auto& globals = wl::get_globals();

        EXPECTS(mouse_id < globals.pointers.size());
        auto& [pointer, state] = globals.pointers[mouse_id];

        hide_mouse(hidden, pointer, state);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_CONST
    auto WindowImpl::is_mouse_hidden(u32 mouse_id) const noexcept -> bool {
        auto& globals = wl::get_globals();

        EXPECTS(mouse_id < globals.pointers.size());
        auto& [_, state] = globals.pointers[mouse_id];

        return check_flag_bit(state.flags, wl::PointerState::Flag::HIDDEN);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::set_relative_mouse(bool enabled, u32 mouse_id) noexcept -> void {
        if (not m_open) return;

        auto& globals = wl::get_globals();
        if (not globals.relative_pointer_manager) {
            elog("{} protocol is not supported by this DE, can't enable relative mouse.",
                 zwp_relative_pointer_manager_v1_interface.name);
            return;
        }

        EXPECTS(mouse_id < globals.pointers.size());
        auto& [pointer, state] = globals.pointers[mouse_id];

        if (enabled) {
            if (not check_flag_bit(state.flags, wl::PointerState::Flag::RELATIVE)) {
                state.relative_pointer = wl::RelativePointer { std::in_place,
                                                               globals.relative_pointer_manager,
                                                               pointer };
                zwp_relative_pointer_v1_add_listener(state.relative_pointer,
                                                     &wl::g_relative_pointer_listener,
                                                     &state);

                state.flags |= wl::PointerState::Flag::RELATIVE;
            }
        } else {
            state.relative_pointer.reset();
            state.flags &= ~wl::PointerState::Flag::RELATIVE;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_CONST
    auto WindowImpl::is_mouse_relative(u32 mouse_id) const noexcept -> bool {
        auto& globals = wl::get_globals();

        EXPECTS(mouse_id < globals.pointers.size());
        auto& [_, state] = globals.pointers[mouse_id];

        return check_flag_bit(state.flags, wl::PointerState::Flag::RELATIVE);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::set_key_repeat(bool enabled, u32 keyboard_id) noexcept -> void {
        auto& globals = wl::get_globals();

        EXPECTS(keyboard_id < globals.pointers.size());
        auto& [_, state] = globals.keyboards[keyboard_id];

        state.repeat.enabled = enabled;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_CONST
    auto WindowImpl::is_key_repeat_enabled(u32 keyboard_id) const noexcept -> bool {
        auto& globals = wl::get_globals();

        EXPECTS(keyboard_id < globals.pointers.size());
        auto& [_, state] = globals.keyboards[keyboard_id];

        return state.repeat.enabled;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::show_virtual_keyboard(bool) noexcept -> void {
        elog("wayland::WindowImpl::show_virtual_keyboard isn't yet implemented");
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::set_mouse_position(const math::vec2i& position, u32 mouse_id) noexcept
      -> void {
        if (not m_open) return;

        auto& globals = wl::get_globals();

        EXPECTS(mouse_id < globals.pointers.size());
        auto& [pointer, state] = globals.pointers[mouse_id];

        if (not state.serial) return;

        if (check_flag_bit(state.flags, wl::PointerState::Flag::LOCKED))
            zwp_locked_pointer_v1_set_cursor_position_hint(state.locked_pointer,
                                                           wl_fixed_to_int(position.x),
                                                           wl_fixed_to_int(position.y));
        else if (globals.pointer_warp)
            wp_pointer_warp_v1_warp_pointer(globals.pointer_warp,
                                            m_surface,
                                            pointer,
                                            wl_fixed_from_int(position.x),
                                            wl_fixed_from_int(position.y),
                                            state.serial.value());
        else
            elog("{} protocol is not supported by this DE, can't warp mouse.",
                 wp_pointer_warp_v1_interface.name);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::set_mouse_position_on_desktop(const math::vec2u&, u32) noexcept -> void {
        elog("wayland::WindowImpl::set_mouse_position_on_desktop isn't yet implemented");
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::get_monitor_settings() -> std::vector<Monitor> {
        if (not wl::init()) return {};
        auto& globals = wl::get_globals();
        return globals.monitors
               | stdv::transform([](const wl::WaylandMonitor& pair) static noexcept {
                     return pair.monitor;
                 })
               | stdr::to<std::vector>();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::native_handle() const noexcept -> NativeHandle {
        return std::bit_cast<NativeHandle>(&m_handles);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::handle_xdg_surface_configure(u32 serial) noexcept -> void {
        auto& globals = wl::get_globals();

        xdg_surface_ack_configure(m_xdg_surface, serial);

        wl_surface_commit(m_surface);
        wl_display_flush(globals.display);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::handle_xdg_surface_close() noexcept -> void {
        WindowImplBase::close_event();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::handle_xdg_top_level_configure(u32 width,
                                                    u32 height,
                                                    std::span<const xdg_toplevel_state>
                                                      states) noexcept -> void {
        auto& globals = wl::get_globals();

        m_open = true;

        auto activated  = false;
        auto suspended  = false;
        auto fullscreen = false;
        auto maximized  = false;
        auto resized    = false;

        for (const auto& state : states) {
            switch (state) {
                case XDG_TOPLEVEL_STATE_ACTIVATED: activated = true; break;
                case XDG_TOPLEVEL_STATE_SUSPENDED: suspended = true; break;
                case XDG_TOPLEVEL_STATE_FULLSCREEN: fullscreen = true; break;
                case XDG_TOPLEVEL_STATE_MAXIMIZED: maximized = true; break;
                case XDG_TOPLEVEL_STATE_RESIZING: resized = true; break;
                default: break;
            }
        }

        if (resized) {
            m_state.extent.width  = width;
            m_state.extent.height = height;
            WindowImplBase::resize_event(m_state.extent.width, m_state.extent.height);

            if (globals.viewporter) {
                wp_viewport_set_destination(m_viewport,
                                            m_state.extent.width,
                                            m_state.extent.height);

                if (not m_scale_content) reallocate_pixel_buffer();
            }
        }

        if (activated and not fullscreen) {
            if (m_state.minimized) {
                m_state.minimized = false;
                WindowImpl::restored_event();
            }

            if (maximized) {
                m_state.maximized = true;
                WindowImplBase::maximize_event();
            }
        }

        if (suspended) m_state.visible = false;

        m_state.fullscreen = m_pending_state.fullscreen.value_or(m_state.fullscreen);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::handle_surface_enter(wl_surface*, wl_output* output) noexcept -> void {
        m_current_output = output;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::handle_keyboard_key(Key key, char character, bool down) noexcept -> void {
        if (down) WindowImplBase::key_down_event(key, character);
        else
            WindowImplBase::key_up_event(key, character);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::handle_pointer_enter(wl_pointer* pointer, wl::PointerState& state) noexcept
      -> void {
        if (check_flag_bit(state.flags, wl::PointerState::Flag::HIDDEN))
            hide_mouse(true, pointer, state);

        WindowImplBase::mouse_entered_event();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::handle_pointer_leave() noexcept -> void {
        WindowImplBase::mouse_exited_event();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::handle_pointer_motion(wl_fixed_t surface_x, wl_fixed_t surface_y) noexcept
      -> void {
        WindowImplBase::mouse_move_event(wl_fixed_to_int(surface_x), wl_fixed_to_int(surface_y));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::handle_pointer_button(u32        button,
                                           u32        bstate,
                                           wl_fixed_t x,
                                           wl_fixed_t y) noexcept -> void {
        const auto down = !!bstate;

        const auto _x = wl_fixed_to_int(x);
        const auto _y = wl_fixed_to_int(y);

        const auto _button = [](auto button) static noexcept {
            switch (button) {
                case BTN_LEFT: return MouseButton::LEFT;
                case BTN_RIGHT: return MouseButton::RIGHT;
                case BTN_MIDDLE: return MouseButton::MIDDLE;
                case BTN_FORWARD: return MouseButton::BUTTON_1;
                case BTN_BACK: return MouseButton::BUTTON_2;
                default: return MouseButton::UNKNOWN;
            }

            std::unreachable();
        }(button);

        if (down) WindowImplBase::mouse_down_event(_button, _x, _y);
        else
            WindowImplBase::mouse_up_event(_button, _x, _y);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::reallocate_pixel_buffer() noexcept -> void {
        auto& globals = wl::get_globals();

        const auto stride = usize { m_state.extent.width * sizeof(u32) };
        const auto size   = usize { stride * m_state.extent.height };

        auto old_shm_buffer = std::move(m_shm_buffer);
        auto old_shm_pool   = std::move(m_shm_pool);

        // only scale up the buffer
        if (not m_shm_buffer or size > m_shm_buffer->size()) {
            SHMBuffer::create(size, std::format("StormKit::{}::PixelBuffer", m_title))
              .transform(bind_front(&DeferInit<SHMBuffer>::construct<SHMBuffer&&>, &m_shm_buffer))
              .transform_error(monadic::assert());

            m_shm_pool = wl::ShmPool {
                std::in_place,
                globals.shm,
                narrow<i32>(std::bit_cast<uptr>(m_shm_buffer->native_handle())),
                narrow<i32>(size)
            };
        }

        if (old_shm_buffer) {
            if (size > old_shm_buffer->size())
                stdr::copy(old_shm_buffer.get(), stdr::begin(m_shm_buffer.get()));
            else
                stdr::copy(std::span<const Byte> { old_shm_buffer.get().begin(), size },
                           stdr::begin(m_shm_buffer.get()));
        }

        m_pixel_buffer = wl::Buffer {
            wl_shm_pool_create_buffer(m_shm_pool,
                                      0,
                                      m_state.extent.width,
                                      m_state.extent.height,
                                      narrow<i32>(stride),
                                      WL_SHM_FORMAT_XRGB8888)
        };

        if (globals.viewporter)
            wp_viewport_set_source(m_viewport, 0, 0, m_state.extent.width, m_state.extent.height);
        wl_buffer_add_listener(m_pixel_buffer, &wl::g_buffer_listener, &m_pixel_buffer);

        wl_surface_commit(m_surface);
        wl_surface_attach(m_surface, m_pixel_buffer, 0, 0);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::hide_mouse(bool hidden, wl_pointer* pointer, wl::PointerState& state) noexcept
      -> void {
        if (not m_open or not state.serial) return;

        if (hidden) {
            state.cursor.name = "";
            std::println("{}", state.serial.value());
            wl_pointer_set_cursor(pointer, state.serial.value(), state.cursor.surface, 0, 0);

            state.flags |= wl::PointerState::Flag::HIDDEN;
        } else {
            state.cursor.name = "left_ptr";
            if (state.cursor.shape_device) {
                wp_cursor_shape_device_v1_set_shape(state.cursor.shape_device,
                                                    state.serial.value(),
                                                    1);
            } else
                set_cursor(state.cursor.name, pointer, state);

            state.flags &= ~wl::PointerState::Flag::HIDDEN;
        }

        auto& globals = wl::get_globals();
        wl_display_flush(globals.display);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::set_cursor(std::string_view  name,
                                wl_pointer*       pointer,
                                wl::PointerState& state) noexcept -> void {
        auto& globals = wl::get_globals();

        auto cursor_theme = globals.cursor_theme.handle();
        if (globals.cursor_theme_high_dpi) cursor_theme = globals.cursor_theme_high_dpi.handle();

        auto cursor = wl_cursor_theme_get_cursor(cursor_theme, stdr::data(name));

        const auto cursor_image = cursor->images[0];

        auto cursor_buffer = wl_cursor_image_get_buffer(cursor_image);

        wl_pointer_set_cursor(pointer,
                              state.serial.value(),
                              state.cursor.surface,
                              cursor_image->hotspot_x / m_scale,
                              cursor_image->hotspot_y / m_scale);

        wl_surface_set_buffer_scale(state.cursor.surface, m_scale);
        wl_surface_attach(state.cursor.surface, cursor_buffer, 0, 0);
        wl_surface_damage(m_surface, 0, 0, cursor_image->width, cursor_image->height);
        wl_surface_commit(state.cursor.surface);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto WindowImpl::handle_key_repeat() noexcept -> void {
        auto& globals = wl::get_globals();

        for (auto& [keyboard, state] : globals.keyboards) {
            if (state.focused_window != this or not state.repeat.enabled) continue;

            auto repeats = u64 { 0u };
            if (read(state.repeat.timer_fd, &repeats, sizeof(repeats)) == sizeof(repeats))
                for (auto _ : range(repeats))
                    WindowImplBase::key_down_event(state.repeat.key, state.repeat.c);
        }
    }

    namespace wl {
        /////////////////////////////////////
        /////////////////////////////////////
        auto xdg_surface_close_handler(void* data, xdg_surface*, u32) noexcept -> void {
            auto& window = *std::bit_cast<WindowImpl*>(data);
            window.handle_xdg_surface_close();
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto xdg_surface_configure_handler(void* data, xdg_surface*, u32 serial) noexcept -> void {
            auto& window = *std::bit_cast<WindowImpl*>(data);
            window.handle_xdg_surface_configure(serial);
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto xdg_top_level_configure_bounds_handler(void*, xdg_toplevel*, i32, i32) noexcept
          -> void {
            // nothing
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto xdg_top_level_configure_handler(void* data,
                                             xdg_toplevel*,
                                             i32       width,
                                             i32       height,
                                             wl_array* state) noexcept -> void {
            auto& window = *std::bit_cast<WindowImpl*>(data);
            window
              .handle_xdg_top_level_configure(as<u32>(width),
                                              as<u32>(height),
                                              { std::bit_cast<const xdg_toplevel_state*>(state
                                                                                           ->data),
                                                state->size });
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto xdg_top_level_close_handler(void*, xdg_toplevel*) noexcept -> void {
            // nothing
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto buffer_release_handler(void* data, wl_buffer*) noexcept -> void {
            auto& buffer = *std::bit_cast<wl::Buffer*>(data);
            buffer.reset();
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto xdg_top_level_decoration_configure_handler(void*,
                                                        zxdg_toplevel_decoration_v1*,
                                                        u32) noexcept -> void {
            // nothing
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto surface_enter_handler(void* data, wl_surface* surface, wl_output* output) noexcept
          -> void {
            auto* window = std::bit_cast<WindowImpl*>(data);
            window->handle_surface_enter(surface, output);
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto surface_leave_handler(void*, wl_surface*, wl_output*) noexcept -> void {
            // nothing
        }
    } // namespace wl
} // namespace stormkit::wsi::linux::wayland
