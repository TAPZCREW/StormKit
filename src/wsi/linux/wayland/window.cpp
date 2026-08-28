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

import :linux.wayland.window;
import :linux.wayland;
import :linux.wayland.context;
import :linux.wayland.input;
import :linux.wayland.log;

namespace stdr = std::ranges;
namespace stdv = std::views;

namespace stormkit::wsi::linux::wayland {
    namespace wl {
        auto xdg_surface_configure_handler(void*, xdg_surface*, u32) noexcept -> void;

        auto xdg_top_level_configure_bounds_handler(void*, xdg_toplevel*, i32, i32) noexcept -> void;
        auto xdg_top_level_configure_handler(void*, xdg_toplevel*, i32, i32, wl_array*) noexcept -> void;
        auto xdg_top_level_close_handler(void*, xdg_toplevel*) noexcept -> void;

        auto xdg_top_level_decoration_configure_handler(void*, zxdg_toplevel_decoration_v1*, u32) noexcept -> void;

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

            constexpr auto g_xdg_top_level_decoration_listener = zxdg_toplevel_decoration_v1_listener {
                .configure = xdg_top_level_decoration_configure_handler,
            };

            constexpr auto g_buffer_listener = wl_buffer_listener { .release = buffer_release_handler };

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
    Window::Window() noexcept {
        wl::init();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    Window::~Window() noexcept {
        auto& globals = wl::get_globals();
        if (globals.display) wl_display_flush(globals.display);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    Window::Window(Window&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::operator=(Window&&) noexcept -> Window& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::open(string title, const math::uextent2& extent, WindowFlag flags) noexcept -> void {
        auto& globals = wl::get_globals();

        m_surface = wl::Surface::create(globals.compositor);
        wl_surface_add_listener(m_surface, &wl::g_surface_listener, this);

        m_xdg_surface = wl::XDGSurface::create(globals.xdg_wm_base, m_surface);
        xdg_surface_add_listener(m_xdg_surface, &wl::g_xdg_surface_listener, this);

        m_xdg_top_level = wl::XDGTopLevel::create(m_xdg_surface);
        xdg_toplevel_add_listener(m_xdg_top_level, &wl::g_xdg_top_level_listener, this);

        const auto app_id = std::format("StormKit/{}", title);
        xdg_toplevel_set_app_id(m_xdg_top_level, stdr::data(app_id));

        if (globals.decoration_manager) {
            m_xdg_top_level_decoration = wl::XDGTopLevelDecoration::create(globals.decoration_manager, m_xdg_top_level);
            zxdg_toplevel_decoration_v1_set_mode(m_xdg_top_level_decoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
            zxdg_toplevel_decoration_v1_add_listener(m_xdg_top_level_decoration,
                                                     &wl::g_xdg_top_level_decoration_listener,
                                                     nullptr);
        }

        if (globals.content_type_manager) {
            m_content_type = wl::ContentType::create(globals.content_type_manager, m_surface);
            // game
            wp_content_type_v1_set_content_type(m_content_type, 3);
        }

        globals.windows.emplace_back(m_surface, this);

        m_title           = std::move(title);
        m_state.extent    = extent;
        m_state.visible   = true;
        m_flags           = flags;
        m_state.open      = true;
        m_handles.display = globals.display;
        m_handles.surface = m_surface;

        if (globals.viewporter) {
            m_viewport         = wl::Viewport::create(globals.viewporter, m_surface);
            const auto _extent = m_state.extent.to<i32>();
            wp_viewport_set_destination(m_viewport, _extent.width, _extent.height);
        }

        if (not has_flag_bit(m_flags, WindowFlag::EXTERNAL_CONTEXT)) reallocate_pixel_buffer();

        wl_surface_commit(m_surface);
        wl_display_roundtrip(globals.display);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::close() noexcept -> void {
        auto& globals = wl::get_globals();
        wl_display_flush(globals.display);

        m_state = {};
        m_title.clear();
        m_flags      = {};
        m_state.open = false;
        m_configured = false;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::handle_events() noexcept -> void {
        auto& globals = wl::get_globals();

        while (wl_display_prepare_read(globals.display) != 0) wl_display_dispatch_pending(globals.display);

        wl_display_flush(globals.display);
        wl_display_read_events(globals.display);
        wl_display_dispatch_pending(globals.display);

        handle_key_repeat();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::clear(const ucolor_rgb& color) noexcept -> void {
        const auto value = (255 << 24) + (color.r << 16) + (color.g << 8) + (color.b);

        auto view = array_view<i32> { std::bit_cast<i32*>(m_shm_buffer.value().begin()), m_shm_buffer->size() / sizeof(i32) };
        stdr::fill(view, value);

        const auto [width, height] = extent().to<i32>();

        wl_surface_damage(m_surface, 0, 0, width, height);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::fill_framebuffer(array_view<const ucolor_rgb> colors) noexcept -> void {
        auto view = array_view<i32> { std::bit_cast<i32*>(m_shm_buffer.value().begin()), m_shm_buffer->size() / sizeof(i32) };
        stdr::copy(colors | stdv::transform([](const auto& color) static noexcept {
                       return (255 << 24) + (color.r << 16) + (color.g << 8) + (color.b);
                   }),
                   stdr::begin(view));

        const auto [width, height] = extent().to<i32>();

        wl_surface_damage(m_surface, 0, 0, width, height);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_title(string title) noexcept -> void {
        if (!m_state.open) return;
        m_title = std::move(title);

        xdg_toplevel_set_title(m_xdg_top_level, stdr::data(m_title));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_extent(const math::uextent2&) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_fullscreen(bool enabled) noexcept -> void {
        if (not m_state.open or not m_current_output) return;

        if (enabled) xdg_toplevel_set_fullscreen(m_xdg_top_level, m_current_output);
        else
            xdg_toplevel_unset_fullscreen(m_xdg_top_level);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::confine_mouse(bool confined, u8 mouse_id) noexcept -> void {
        if (not m_state.open) return;

        auto& globals = wl::get_globals();
        if (not globals.pointer_constraints) {
            elog("{} protocol is not supported by this DE, can't confine mouse.", zwp_pointer_constraints_v1_interface.name);
            return;
        }

        EXPECTS(mouse_id < globals.pointers.size());
        auto& [pointer, state] = globals.pointers[mouse_id];

        if (not state.serial or has_flag_bit(state.flags, wl::PointerState::Flag::LOCKED)) return;

        if (confined) {
            if (not has_flag_bit(state.flags, wl::PointerState::Flag::CONFINED)) {
                state.confined_pointer = wl::ConfinedPointer ::
                  create(globals.pointer_constraints, m_surface, pointer, nullptr, ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_ONESHOT);

                zwp_confined_pointer_v1_add_listener(state.confined_pointer, &wl::g_confined_pointer_listener, &state);
                state.flags |= wl::PointerState::Flag::CONFINED;
            }
        } else {
            state.confined_pointer.reset();

            state.flags &= ~wl::PointerState::Flag::CONFINED;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_PURE
    auto Window::is_mouse_confined(u8 mouse_id) const noexcept -> bool {
        auto& globals = wl::get_globals();

        EXPECTS(mouse_id < globals.pointers.size());
        const auto& [_, state] = globals.pointers[mouse_id];

        return has_flag_bit(state.flags, wl::PointerState::Flag::CONFINED);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::lock_mouse(bool locked, u8 mouse_id) noexcept -> void {
        if (not m_state.open) return;

        auto& globals = wl::get_globals();
        if (not globals.pointer_constraints) {
            elog("{} protocol is not supported by this DE, can't lock mouse.", zwp_pointer_constraints_v1_interface.name);
            return;
        }

        EXPECTS(mouse_id < globals.pointers.size());
        auto& [pointer, state] = globals.pointers[mouse_id];

        if (not state.serial or has_flag_bit(state.flags, wl::PointerState::Flag::CONFINED)) return;

        if (locked) {
            if (not has_flag_bit(state.flags, wl::PointerState::Flag::LOCKED)) {
                state.locked_pointer = wl::LockedPointer::create(globals.pointer_constraints,
                                                                 m_surface,
                                                                 pointer,
                                                                 nullptr,
                                                                 ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);

                zwp_locked_pointer_v1_add_listener(state.locked_pointer, &wl::g_locked_pointer_listener, &state);

                state.flags |= wl::PointerState::Flag::LOCKED;
            }
        } else {
            state.locked_pointer.reset();

            state.flags &= ~wl::PointerState::Flag::LOCKED;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_PURE
    auto Window::is_mouse_locked(u8 mouse_id) const noexcept -> bool {
        auto& globals = wl::get_globals();

        EXPECTS(mouse_id < globals.pointers.size());
        const auto& [_, state] = globals.pointers[mouse_id];

        return has_flag_bit(state.flags, wl::PointerState::Flag::LOCKED);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::hide_mouse(bool hidden, u8 mouse_id) noexcept -> void {
        auto& globals = wl::get_globals();

        EXPECTS(mouse_id < globals.pointers.size());
        auto& [pointer, state] = globals.pointers[mouse_id];

        hide_mouse(hidden, pointer, state);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_PURE
    auto Window::is_mouse_hidden(u8 mouse_id) const noexcept -> bool {
        auto& globals = wl::get_globals();

        EXPECTS(mouse_id < globals.pointers.size());
        auto& [_, state] = globals.pointers[mouse_id];

        return has_flag_bit(state.flags, wl::PointerState::Flag::HIDDEN);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_relative_mouse(bool enabled, u8 mouse_id) noexcept -> void {
        if (not m_state.open) return;

        auto& globals = wl::get_globals();
        if (not globals.relative_pointer_manager) {
            elog("{} protocol is not supported by this DE, can't enable relative mouse.",
                 zwp_relative_pointer_manager_v1_interface.name);
            return;
        }

        EXPECTS(mouse_id < globals.pointers.size());
        auto& [pointer, state] = globals.pointers[mouse_id];

        if (enabled) {
            if (not has_flag_bit(state.flags, wl::PointerState::Flag::RELATIVE)) {
                state.relative_pointer = wl::RelativePointer::create(globals.relative_pointer_manager, pointer);
                zwp_relative_pointer_v1_add_listener(state.relative_pointer, &wl::g_relative_pointer_listener, &state);

                state.flags |= wl::PointerState::Flag::RELATIVE;
            }
        } else {
            state.relative_pointer.reset();
            state.flags &= ~wl::PointerState::Flag::RELATIVE;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_PURE
    auto Window::is_mouse_relative(u8 mouse_id) const noexcept -> bool {
        auto& globals = wl::get_globals();

        EXPECTS(mouse_id < globals.pointers.size());
        auto& [_, state] = globals.pointers[mouse_id];

        return has_flag_bit(state.flags, wl::PointerState::Flag::RELATIVE);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_key_repeat(bool enabled, u8 keyboard_id) noexcept -> void {
        auto& globals = wl::get_globals();

        EXPECTS(keyboard_id < globals.pointers.size());
        auto& [_, state] = globals.keyboards[keyboard_id];

        state.repeat.enabled = enabled;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_PURE
    auto Window::is_key_repeat_enabled(u8 keyboard_id) const noexcept -> bool {
        auto& globals = wl::get_globals();

        EXPECTS(keyboard_id < globals.pointers.size());
        auto& [_, state] = globals.keyboards[keyboard_id];

        return state.repeat.enabled;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::show_virtual_keyboard(bool) noexcept -> void {
        elog("wayland::Window::show_virtual_keyboard isn't yet implemented");
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_mouse_position(const math::ivec2& position, u8 mouse_id) noexcept -> void {
        if (not m_state.open) return;

        auto& globals = wl::get_globals();

        EXPECTS(mouse_id < globals.pointers.size());
        auto& [pointer, state] = globals.pointers[mouse_id];

        if (not state.serial) return;

        if (has_flag_bit(state.flags, wl::PointerState::Flag::LOCKED))
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
            elog("{} protocol is not supported by this DE, can't warp mouse.", wp_pointer_warp_v1_interface.name);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::native_handle() const noexcept -> NativeHandle {
        return std::bit_cast<NativeHandle>(&m_handles);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::handle_xdg_surface_configure(u32 serial) noexcept -> void {
        auto& globals = wl::get_globals();

        if (m_pending_state.resizing) {
            m_state.extent = m_pending_state.resizing.value();

            if (not has_flag_bit(m_flags, WindowFlag::EXTERNAL_CONTEXT)) reallocate_pixel_buffer();

            if (m_viewport) {
                const auto _extent = m_state.extent.to<i32>();
                wp_viewport_set_destination(m_viewport, _extent.width, _extent.height);
            }

            resized_event(m_state.extent);
        } else if (m_pending_state.restored)
            restored_event();
        else if (m_pending_state.activated)
            activate_event();
        else if (m_pending_state.suspended)
            deactivate_event();
        else if (m_pending_state.fullscreen)
            m_state.fullscreen = true;

        m_pending_state = {};

        xdg_surface_ack_configure(m_xdg_surface, serial);

        wl_surface_commit(m_surface);
        wl_display_flush(globals.display);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::handle_xdg_surface_close() noexcept -> void {
        WindowBase::closed_event();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::handle_xdg_top_level_configure(u32 width, u32 height, array_view<const xdg_toplevel_state> states) noexcept
      -> void {
        m_state.open = true;

        if (not has_flag_bit(m_flags, wsi::WindowFlag::RESIZEABLE)) {
            xdg_toplevel_set_min_size(m_xdg_top_level, as<i32>(width), as<i32>(height));
            xdg_toplevel_set_max_size(m_xdg_top_level, as<i32>(width), as<i32>(height));
        }

        for (const auto& state : states) {
            switch (state) {
                case XDG_TOPLEVEL_STATE_ACTIVATED:
                    if (m_state.minimized) m_pending_state.restored = true;
                    else
                        m_pending_state.activated = true;
                    break;
                case XDG_TOPLEVEL_STATE_SUSPENDED: m_pending_state.suspended = true; break;
                case XDG_TOPLEVEL_STATE_FULLSCREEN: m_pending_state.fullscreen = true; break;
                case XDG_TOPLEVEL_STATE_RESIZING: m_pending_state.resizing = { width, height }; break;
                default: break;
            }
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::handle_surface_enter(wl_surface*, wl_output* output) noexcept -> void {
        m_current_output = output;

        const auto& monitor = wl::get_monitor(wl::get_globals(), output);
        if (as<f32>(monitor.scale_factor) != m_state.dpi) {
            m_state.dpi = as<f32>(monitor.scale_factor);
            if (not has_flag_bit(m_flags, WindowFlag::EXTERNAL_CONTEXT)) reallocate_pixel_buffer();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::handle_keyboard_key(Key key, char character, bool down) noexcept -> void {
        if (down) key_down_event(GLOBAL_KEYBOARD_ID, key, character);
        else
            key_up_event(GLOBAL_KEYBOARD_ID, key, character);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::handle_pointer_enter(wl_pointer* pointer, wl::PointerState& state) noexcept -> void {
        if (has_flag_bit(state.flags, wl::PointerState::Flag::HIDDEN)) hide_mouse(true, pointer, state);

        // mouse_entered_event(GLOBAL_MOUSE_ID);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::handle_pointer_leave() noexcept -> void {
        // mouse_exited_event(GLOBAL_MOUSE_ID);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::handle_pointer_motion(wl_fixed_t surface_x, wl_fixed_t surface_y) noexcept -> void {
        mouse_moved_event(GLOBAL_MOUSE_ID, math::vec2 { wl_fixed_to_int(surface_x), wl_fixed_to_int(surface_y) });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::handle_pointer_button(u32 button, u32 bstate, wl_fixed_t x, wl_fixed_t y) noexcept -> void {
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
                default: break;
            }

            std::unreachable();
        }(button);

        if (down) mouse_button_down_event(GLOBAL_MOUSE_ID, _button, math::vec2 { _x, _y });
        else
            mouse_button_up_event(GLOBAL_MOUSE_ID, _button, math::vec2 { _x, _y });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::reallocate_pixel_buffer() noexcept -> void {
        auto& globals = wl::get_globals();

        const auto& extent = m_state.extent;
        const auto  stride = as<usize>(extent.width * sizeof(u32));
        const auto  size   = as<usize>(stride * extent.height);

        auto old_shm_buffer   = defer_init<shm_buffer> {};
        auto old_shm_pool     = defer_init<wl::ShmPool> {};
        auto old_pixel_buffer = defer_init<wl::Buffer> {};

        const auto [width, height] = extent.to<i32>();
        if (not m_shm_buffer or stdr::size(m_shm_buffer.value()) < size) {
            old_shm_buffer   = std::move(m_shm_buffer);
            old_shm_pool     = std::move(m_shm_pool);
            old_pixel_buffer = std::move(m_pixel_buffer);

            auto _ = shm_buffer::create(size, std::format("StormKit::{}::PixelBuffer", m_title))
                       .transform(bind_front(&defer_init<shm_buffer>::construct<shm_buffer&&>, &m_shm_buffer))
                       .transform_error(monadic::assert());

            m_shm_pool = wl::ShmPool::create(globals.shm,
                                             unchecked_narrow<i32>(std::bit_cast<uptr>(m_shm_buffer->native_handle())),
                                             unchecked_narrow<i32>(size));

            m_pixel_buffer = wl::Buffer::
              take(wl_shm_pool_create_buffer(m_shm_pool, 0, width, height, unchecked_narrow<i32>(stride), WL_SHM_FORMAT_XRGB8888));

            wl_buffer_add_listener(m_pixel_buffer, &wl::g_buffer_listener, &m_pixel_buffer);
        }

        if (m_viewport) {
            wp_viewport_set_source(m_viewport,
                                   wl_fixed_from_int(0),
                                   wl_fixed_from_int(0),
                                   wl_fixed_from_int(width),
                                   wl_fixed_from_int(height));
        }

        wl_surface_commit(m_surface);
        wl_surface_attach(m_surface, m_pixel_buffer, 0, 0);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::hide_mouse(bool hidden, wl_pointer* pointer, wl::PointerState& state) noexcept -> void {
        if (not m_state.open or not state.serial) return;

        if (hidden) {
            state.cursor.name = "";
            wl_pointer_set_cursor(pointer, state.serial.value(), state.cursor.surface, 0, 0);

            state.flags |= wl::PointerState::Flag::HIDDEN;
        } else {
            state.cursor.name = "left_ptr";
            if (state.cursor.shape_device) {
                wp_cursor_shape_device_v1_set_shape(state.cursor.shape_device, state.serial.value(), 1);
            } else
                set_cursor(state.cursor.name, pointer, state);

            state.flags &= ~wl::PointerState::Flag::HIDDEN;
        }

        auto& globals = wl::get_globals();
        wl_display_flush(globals.display);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_cursor(string_view name, wl_pointer* pointer, wl::PointerState& state) noexcept -> void {
        auto& globals = wl::get_globals();

        auto cursor_theme = globals.cursor_theme.handle();
        if (globals.cursor_theme_high_dpi) cursor_theme = globals.cursor_theme_high_dpi.handle();

        auto cursor = wl_cursor_theme_get_cursor(cursor_theme, stdr::data(name));

        const auto cursor_image = cursor->images[0];

        auto cursor_buffer = wl_cursor_image_get_buffer(cursor_image);

        wl_pointer_set_cursor(pointer,
                              state.serial.value(),
                              state.cursor.surface,
                              as<i32>(cursor_image->hotspot_x / m_scale),
                              as<i32>(cursor_image->hotspot_y / m_scale));

        wl_surface_set_buffer_scale(state.cursor.surface, m_scale);
        wl_surface_attach(state.cursor.surface, cursor_buffer, 0, 0);
        wl_surface_damage(m_surface, 0, 0, as<i32>(cursor_image->width), as<i32>(cursor_image->height));
        wl_surface_commit(state.cursor.surface);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::handle_key_repeat() noexcept -> void {
        auto& globals = wl::get_globals();

        for (auto& [keyboard, state] : globals.keyboards) {
            if (state.focused_window != this or not state.repeat.enabled) continue;

            auto repeats = u64 { 0u };
            if (read(state.repeat.timer_fd, &repeats, sizeof(repeats)) == sizeof(repeats))
                for (auto _ : range(repeats)) key_down_event(GLOBAL_KEYBOARD_ID, state.repeat.key, state.repeat.c);
        }
    }

    namespace wl {
        /////////////////////////////////////
        /////////////////////////////////////
        auto xdg_surface_close_handler(void* data, xdg_surface*, u32) noexcept -> void {
            auto& window = *std::bit_cast<Window*>(data);
            window.handle_xdg_surface_close();
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto xdg_surface_configure_handler(void* data, xdg_surface*, u32 serial) noexcept -> void {
            auto& window = *std::bit_cast<Window*>(data);
            window.handle_xdg_surface_configure(serial);
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto xdg_top_level_configure_bounds_handler(void*, xdg_toplevel*, i32, i32) noexcept -> void {
            // nothing
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto xdg_top_level_configure_handler(void* data, xdg_toplevel*, i32 width, i32 height, wl_array* state) noexcept -> void {
            auto& window = *std::bit_cast<Window*>(data);
            window.handle_xdg_top_level_configure(as<u32>(width),
                                                  as<u32>(height),
                                                  { std::bit_cast<const xdg_toplevel_state*>(state->data), state->size });
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
        auto xdg_top_level_decoration_configure_handler(void*, zxdg_toplevel_decoration_v1*, u32) noexcept -> void {
            // nothing
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto surface_enter_handler(void* data, wl_surface* surface, wl_output* output) noexcept -> void {
            auto* window = std::bit_cast<Window*>(data);
            window->handle_surface_enter(surface, output);
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto surface_leave_handler(void*, wl_surface*, wl_output*) noexcept -> void {
            // nothing
        }
    } // namespace wl
} // namespace stormkit::wsi::linux::wayland
