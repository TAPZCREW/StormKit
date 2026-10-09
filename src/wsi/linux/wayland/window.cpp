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
        auto xdg_surface_configure_handler(void*, ::xdg_surface*, u32) noexcept -> void;

        auto xdg_top_level_configure_bounds_handler(void*, ::xdg_toplevel*, i32, i32) noexcept -> void;
        auto xdg_top_level_configure_handler(void*, ::xdg_toplevel*, i32, i32, wl_array*) noexcept -> void;
        auto xdg_top_level_close_handler(void*, ::xdg_toplevel*) noexcept -> void;

        auto xdg_top_level_decoration_configure_handler(void*, ::zxdg_toplevel_decoration_v1*, u32) noexcept -> void;

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
    window::window() noexcept {
        wl::init();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    window::~window() noexcept {
        auto& globals = wl::get_globals();
        if (globals.display) wl_display_flush(globals.display);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    window::window(window&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::operator=(window&&) noexcept -> window& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::open(string title, const math::uextent2& extent, window_flag flags) noexcept -> void {
        auto& globals = wl::get_globals();

        surface_ = wl::surface::create(globals.compositor);
        wl_surface_add_listener(surface_, &wl::g_surface_listener, this);

        xdg_surface_ = wl::xdg_surface::create(globals.xdg_wm_base, surface_);
        xdg_surface_add_listener(xdg_surface_, &wl::g_xdg_surface_listener, this);

        xdg_top_level_ = wl::xdg_top_level::create(xdg_surface_);
        xdg_toplevel_add_listener(xdg_top_level_, &wl::g_xdg_top_level_listener, this);

        const auto app_id = std::format("StormKit/{}", title);
        xdg_toplevel_set_app_id(xdg_top_level_, stdr::data(app_id));

        if (globals.decoration_manager) {
            xdg_top_level_decoration_ = wl::xdg_top_level_decoration::create(globals.decoration_manager, xdg_top_level_);
            zxdg_toplevel_decoration_v1_set_mode(xdg_top_level_decoration_, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
            zxdg_toplevel_decoration_v1_add_listener(xdg_top_level_decoration_,
                                                     &wl::g_xdg_top_level_decoration_listener,
                                                     nullptr);
        }

        if (globals.content_type_manager) {
            content_type_ = wl::content_type::create(globals.content_type_manager, surface_);
            // game
            wp_content_type_v1_set_content_type(content_type_, 3);
        }

        globals.windows.emplace_back(surface_, this);

        title_           = std::move(title);
        state_.extent    = extent;
        state_.visible   = true;
        flags_           = flags;
        state_.open      = true;
        handles_.display = globals.display;
        handles_.surface = surface_;

        if (globals.viewporter) {
            viewport_         = wl::viewport::create(globals.viewporter, surface_);
            const auto extent = as<math::iextent2>(state_.extent);
            wp_viewport_set_destination(viewport_, extent.width, extent.height);
        }

        if (not has_flag_bit(flags_, window_flag::external_context)) reallocate_pixel_buffer();

        wl_surface_commit(surface_);
        wl_display_roundtrip(globals.display);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::close() noexcept -> void {
        auto& globals = wl::get_globals();
        wl_display_flush(globals.display);

        state_ = {};
        title_.clear();
        flags_      = {};
        state_.open = false;
        configured_ = false;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::handle_events() noexcept -> void {
        auto& globals = wl::get_globals();

        while (wl_display_prepare_read(globals.display) != 0) wl_display_dispatch_pending(globals.display);

        wl_display_flush(globals.display);
        wl_display_read_events(globals.display);
        wl_display_dispatch_pending(globals.display);

        handle_key_repeat();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::clear(const ucolor_rgb& color) noexcept -> void {
        const auto value = (255 << 24) + (color.r << 16) + (color.g << 8) + (color.b);

        auto view = array_view<i32> { std::bit_cast<i32*>(shm_buffer_.value().begin()), shm_buffer_->size() / sizeof(i32) };
        stdr::fill(view, value);

        const auto [width, height] = as<math::iextent2>(extent());

        wl_surface_damage(surface_, 0, 0, width, height);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::fill_framebuffer(array_view<const ucolor_rgb> colors) noexcept -> void {
        auto view = array_view<i32> { std::bit_cast<i32*>(shm_buffer_.value().begin()), shm_buffer_->size() / sizeof(i32) };
        stdr::copy(colors | stdv::transform([](const auto& color) static noexcept {
                       return (255 << 24) + (color.r << 16) + (color.g << 8) + (color.b);
                   }),
                   stdr::begin(view));

        const auto [width, height] = as<math::iextent2>(extent());

        wl_surface_damage(surface_, 0, 0, width, height);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::set_title(string&& title) noexcept -> void {
        window_base::set_title(std::move(title));

        xdg_toplevel_set_title(xdg_top_level_, stdr::data(title_));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::set_extent(const math::uextent2&) noexcept -> void {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::set_fullscreen(bool enabled) noexcept -> void {
        if (not state_.open or not current_output_) return;

        if (enabled) xdg_toplevel_set_fullscreen(xdg_top_level_, current_output_);
        else
            xdg_toplevel_unset_fullscreen(xdg_top_level_);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::confine_mouse(bool confined, u8 mouse_id) noexcept -> void {
        if (not state_.open) return;

        auto& globals = wl::get_globals();
        if (not globals.pointer_constraints) {
            elog("{} protocol is not supported by this DE, can't confine mouse.", zwp_pointer_constraints_v1_interface.name);
            return;
        }

        EXPECTS(mouse_id < globals.pointers.size());
        auto& [pointer, state] = globals.pointers[mouse_id];

        if (not state.serial or has_flag_bit(state.flags, wl::pointer_state::flag::locked)) return;

        if (confined) {
            if (not has_flag_bit(state.flags, wl::pointer_state::flag::confined)) {
                state.confined_pointer = wl::confined_pointer::
                  create(globals.pointer_constraints, surface_, pointer, nullptr, ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_ONESHOT);

                zwp_confined_pointer_v1_add_listener(state.confined_pointer, &wl::g_confined_pointer_listener, &state);
                state.flags |= wl::pointer_state::flag::confined;
            }
        } else {
            state.confined_pointer.reset();

            state.flags &= ~wl::pointer_state::flag::confined;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_PURE
    auto window::is_mouse_confined(u8 mouse_id) const noexcept -> bool {
        auto& globals = wl::get_globals();

        EXPECTS(mouse_id < globals.pointers.size());
        const auto& [_, state] = globals.pointers[mouse_id];

        return has_flag_bit(state.flags, wl::pointer_state::flag::confined);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::lock_mouse(bool locked, u8 mouse_id) noexcept -> void {
        if (not state_.open) return;

        auto& globals = wl::get_globals();
        if (not globals.pointer_constraints) {
            elog("{} protocol is not supported by this DE, can't lock mouse.", zwp_pointer_constraints_v1_interface.name);
            return;
        }

        EXPECTS(mouse_id < globals.pointers.size());
        auto& [pointer, state] = globals.pointers[mouse_id];

        if (not state.serial or has_flag_bit(state.flags, wl::pointer_state::flag::confined)) return;

        if (locked) {
            if (not has_flag_bit(state.flags, wl::pointer_state::flag::locked)) {
                state.locked_pointer = wl::locked_pointer::
                  create(globals.pointer_constraints, surface_, pointer, nullptr, ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);

                zwp_locked_pointer_v1_add_listener(state.locked_pointer, &wl::g_locked_pointer_listener, &state);

                state.flags |= wl::pointer_state::flag::locked;
            }
        } else {
            state.locked_pointer.reset();

            state.flags &= ~wl::pointer_state::flag::locked;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_PURE
    auto window::is_mouse_locked(u8 mouse_id) const noexcept -> bool {
        auto& globals = wl::get_globals();

        EXPECTS(mouse_id < globals.pointers.size());
        const auto& [_, state] = globals.pointers[mouse_id];

        return has_flag_bit(state.flags, wl::pointer_state::flag::locked);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::hide_mouse(bool hidden, u8 mouse_id) noexcept -> void {
        auto& globals = wl::get_globals();

        EXPECTS(mouse_id < globals.pointers.size());
        auto& [pointer, state] = globals.pointers[mouse_id];

        hide_mouse(hidden, pointer, state);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_PURE
    auto window::is_mouse_hidden(u8 mouse_id) const noexcept -> bool {
        auto& globals = wl::get_globals();

        EXPECTS(mouse_id < globals.pointers.size());
        auto& [_, state] = globals.pointers[mouse_id];

        return has_flag_bit(state.flags, wl::pointer_state::flag::hidden);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::set_relative_mouse(bool enabled, u8 mouse_id) noexcept -> void {
        if (not state_.open) return;

        auto& globals = wl::get_globals();
        if (not globals.relative_pointer_manager) {
            elog("{} protocol is not supported by this DE, can't enable relative mouse.",
                 zwp_relative_pointer_manager_v1_interface.name);
            return;
        }

        EXPECTS(mouse_id < globals.pointers.size());
        auto& [pointer, state] = globals.pointers[mouse_id];

        if (enabled) {
            if (not has_flag_bit(state.flags, wl::pointer_state::flag::relative)) {
                state.relative_pointer = wl::relative_pointer::create(globals.relative_pointer_manager, pointer);
                zwp_relative_pointer_v1_add_listener(state.relative_pointer, &wl::g_relative_pointer_listener, &state);

                state.flags |= wl::pointer_state::flag::relative;
            }
        } else {
            state.relative_pointer.reset();
            state.flags &= ~wl::pointer_state::flag::relative;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_PURE
    auto window::is_mouse_relative(u8 mouse_id) const noexcept -> bool {
        auto& globals = wl::get_globals();

        EXPECTS(mouse_id < globals.pointers.size());
        auto& [_, state] = globals.pointers[mouse_id];

        return has_flag_bit(state.flags, wl::pointer_state::flag::relative);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::set_key_repeat(bool enabled, u8 keyboard_id) noexcept -> void {
        auto& globals = wl::get_globals();

        EXPECTS(keyboard_id < globals.pointers.size());
        auto& [_, state] = globals.keyboards[keyboard_id];

        state.repeat.enabled = enabled;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_PURE
    auto window::is_key_repeat_enabled(u8 keyboard_id) const noexcept -> bool {
        auto& globals = wl::get_globals();

        EXPECTS(keyboard_id < globals.pointers.size());
        auto& [_, state] = globals.keyboards[keyboard_id];

        return state.repeat.enabled;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::show_virtual_keyboard(bool) noexcept -> void {
        elog("wayland::window::show_virtual_keyboard isn't yet implemented");
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::set_mouse_position(const math::ivec2& position, u8 mouse_id) noexcept -> void {
        if (not state_.open) return;

        auto& globals = wl::get_globals();

        EXPECTS(mouse_id < globals.pointers.size());
        auto& [pointer, state] = globals.pointers[mouse_id];

        if (not state.serial) return;

        if (has_flag_bit(state.flags, wl::pointer_state::flag::locked))
            zwp_locked_pointer_v1_set_cursor_position_hint(state.locked_pointer,
                                                           wl_fixed_to_int(position.x()),
                                                           wl_fixed_to_int(position.y()));
        else if (globals.pointer_warp)
            wp_pointer_warp_v1_warp_pointer(globals.pointer_warp,
                                            surface_,
                                            pointer,
                                            wl_fixed_from_int(position.x()),
                                            wl_fixed_from_int(position.y()),
                                            state.serial.value());
        else
            elog("{} protocol is not supported by this DE, can't warp mouse.", wp_pointer_warp_v1_interface.name);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::native_handle() const noexcept -> native_handle_type {
        return std::bit_cast<native_handle_type>(&handles_);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::handle_xdg_surface_configure(u32 serial) noexcept -> void {
        auto& globals = wl::get_globals();

        if (pending_state_.resizing) {
            state_.extent = pending_state_.resizing.value();

            if (not has_flag_bit(flags_, window_flag::external_context)) reallocate_pixel_buffer();

            if (viewport_) {
                const auto extent = as<math::iextent2>(state_.extent);
                wp_viewport_set_destination(viewport_, extent.width, extent.height);
            }

            resized_event(state_.extent);
        } else if (pending_state_.restored)
            restored_event();
        else if (pending_state_.activated)
            activate_event();
        else if (pending_state_.suspended)
            deactivate_event();
        else if (pending_state_.fullscreen)
            state_.fullscreen = true;

        pending_state_ = {};

        xdg_surface_ack_configure(xdg_surface_, serial);

        wl_surface_commit(surface_);
        wl_display_flush(globals.display);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::handle_xdg_surface_close() noexcept -> void {
        window_base::closed_event();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::handle_xdg_top_level_configure(u32 width, u32 height, array_view<const xdg_toplevel_state> states) noexcept
      -> void {
        state_.open = true;

        if (not has_flag_bit(flags_, wsi::window_flag::resizeable)) {
            xdg_toplevel_set_min_size(xdg_top_level_, as<i32>(width), as<i32>(height));
            xdg_toplevel_set_max_size(xdg_top_level_, as<i32>(width), as<i32>(height));
        }

        for (const auto& state : states) {
            switch (state) {
                case XDG_TOPLEVEL_STATE_ACTIVATED:
                    if (state_.minimized) pending_state_.restored = true;
                    else
                        pending_state_.activated = true;
                    break;
                case XDG_TOPLEVEL_STATE_SUSPENDED: pending_state_.suspended = true; break;
                case XDG_TOPLEVEL_STATE_FULLSCREEN: pending_state_.fullscreen = true; break;
                case XDG_TOPLEVEL_STATE_RESIZING: pending_state_.resizing = { width, height }; break;
                default: break;
            }
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::handle_surface_enter(wl_surface*, wl_output* output) noexcept -> void {
        current_output_ = output;

        const auto& monitor = wl::get_monitor(wl::get_globals(), output);
        if (as<f32>(monitor.scale_factor) != state_.dpi) {
            state_.dpi = as<f32>(monitor.scale_factor);
            if (not has_flag_bit(flags_, window_flag::external_context)) reallocate_pixel_buffer();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::handle_keyboard_key(wsi::key key, char character, bool down) noexcept -> void {
        if (down) key_down_event(global_keyboard_id, key, character);
        else
            key_up_event(global_keyboard_id, key, character);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::handle_pointer_enter(wl_pointer* pointer, wl::pointer_state& state) noexcept -> void {
        if (has_flag_bit(state.flags, wl::pointer_state::flag::hidden)) hide_mouse(true, pointer, state);

        // mouse_entered_event(global_mouse_id);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::handle_pointer_leave() noexcept -> void {
        // mouse_exited_event(global_mouse_id);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::handle_pointer_motion(wl_fixed_t surface_x, wl_fixed_t surface_y) noexcept -> void {
        mouse_moved_event(global_mouse_id, math::ivec2 { wl_fixed_to_int(surface_x), wl_fixed_to_int(surface_y) });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::handle_pointer_button(u32 button, u32 bstate, wl_fixed_t x, wl_fixed_t y) noexcept -> void {
        const auto down = !!bstate;

        const auto _x = wl_fixed_to_int(x);
        const auto _y = wl_fixed_to_int(y);

        const auto _button = [](auto button) static noexcept {
            switch (button) {
                case BTN_LEFT: return mouse_button::left;
                case BTN_RIGHT: return mouse_button::right;
                case BTN_MIDDLE: return mouse_button::middle;
                case BTN_FORWARD: return mouse_button::button_1;
                case BTN_BACK: return mouse_button::button_2;
                default: break;
            }

            std::unreachable();
        }(button);

        if (down) mouse_button_down_event(global_mouse_id, _button, math::ivec2 { _x, _y });
        else
            mouse_button_up_event(global_mouse_id, _button, math::ivec2 { _x, _y });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::reallocate_pixel_buffer() noexcept -> void {
        auto& globals = wl::get_globals();

        const auto& extent = state_.extent;
        const auto  stride = as<usize>(extent.width * sizeof(u32));
        const auto  size   = as<usize>(stride * extent.height);

        auto old_shm_buffer   = defer_init<shm_buffer> {};
        auto old_shm_pool     = defer_init<wl::shm_pool> {};
        auto old_pixel_buffer = defer_init<wl::Buffer> {};

        const auto [width, height] = as<math::iextent2>(extent);
        if (not shm_buffer_ or stdr::size(shm_buffer_.value()) < size) {
            old_shm_buffer   = std::move(shm_buffer_);
            old_shm_pool     = std::move(shm_pool_);
            old_pixel_buffer = std::move(pixel_buffer_);

            auto _ = shm_buffer::create(size, std::format("StormKit::{}::PixelBuffer", title_))
                       .transform(bind_front(&defer_init<shm_buffer>::construct<shm_buffer&&>, &shm_buffer_))
                       .transform_error(monadic::assert());

            shm_pool_ = wl::shm_pool::create(globals.shm,
                                             unchecked_narrow<i32>(std::bit_cast<uptr>(shm_buffer_->native_handle())),
                                             unchecked_narrow<i32>(size));

            pixel_buffer_ = wl::Buffer::
              take(wl_shm_pool_create_buffer(shm_pool_, 0, width, height, unchecked_narrow<i32>(stride), WL_SHM_FORMAT_XRGB8888));

            wl_buffer_add_listener(pixel_buffer_, &wl::g_buffer_listener, &pixel_buffer_);
        }

        if (viewport_) {
            wp_viewport_set_source(viewport_,
                                   wl_fixed_from_int(0),
                                   wl_fixed_from_int(0),
                                   wl_fixed_from_int(width),
                                   wl_fixed_from_int(height));
        }

        wl_surface_commit(surface_);
        wl_surface_attach(surface_, pixel_buffer_, 0, 0);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::hide_mouse(bool hidden, wl_pointer* pointer, wl::pointer_state& state) noexcept -> void {
        if (not state_.open or not state.serial) return;

        if (hidden) {
            state.cursor.name = "";
            wl_pointer_set_cursor(pointer, state.serial.value(), state.cursor.surface, 0, 0);

            state.flags |= wl::pointer_state::flag::hidden;
        } else {
            state.cursor.name = "left_ptr";
            if (state.cursor.shape_device) {
                wp_cursor_shape_device_v1_set_shape(state.cursor.shape_device, state.serial.value(), 1);
            } else
                set_cursor(state.cursor.name, pointer, state);

            state.flags &= ~wl::pointer_state::flag::hidden;
        }

        auto& globals = wl::get_globals();
        wl_display_flush(globals.display);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::set_cursor(string_view name, wl_pointer* pointer, wl::pointer_state& state) noexcept -> void {
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
        wl_surface_damage(surface_, 0, 0, as<i32>(cursor_image->width), as<i32>(cursor_image->height));
        wl_surface_commit(state.cursor.surface);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto window::handle_key_repeat() noexcept -> void {
        auto& globals = wl::get_globals();

        for (auto& [keyboard, state] : globals.keyboards) {
            if (state.focused_window != this or not state.repeat.enabled) continue;

            auto repeats = u64 { 0u };
            if (read(state.repeat.timer_fd, &repeats, sizeof(repeats)) == sizeof(repeats))
                for (auto _ : range(repeats)) key_down_event(global_keyboard_id, state.repeat.key, state.repeat.c);
        }
    }

    namespace wl {
        /////////////////////////////////////
        /////////////////////////////////////
        auto xdg_surface_close_handler(void* data, ::xdg_surface*, u32) noexcept -> void {
            auto& window = *reinterpret_cast<wayland::window*>(data);
            window.handle_xdg_surface_close();
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto xdg_surface_configure_handler(void* data, ::xdg_surface*, u32 serial) noexcept -> void {
            auto& window = *reinterpret_cast<wayland::window*>(data);
            window.handle_xdg_surface_configure(serial);
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto xdg_top_level_configure_bounds_handler(void*, ::xdg_toplevel*, i32, i32) noexcept -> void {
            // nothing
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto xdg_top_level_configure_handler(void* data, ::xdg_toplevel*, i32 width, i32 height, wl_array* state) noexcept
          -> void {
            auto& window = *reinterpret_cast<wayland::window*>(data);
            window.handle_xdg_top_level_configure(as<u32>(width),
                                                  as<u32>(height),
                                                  { std::bit_cast<const xdg_toplevel_state*>(state->data), state->size });
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto xdg_top_level_close_handler(void*, ::xdg_toplevel*) noexcept -> void {
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
        auto xdg_top_level_decoration_configure_handler(void*, ::zxdg_toplevel_decoration_v1*, u32) noexcept -> void {
            // nothing
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto surface_enter_handler(void* data, wl_surface* surface, wl_output* output) noexcept -> void {
            auto* window = reinterpret_cast<wayland::window*>(data);
            window->handle_surface_enter(surface, output);
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto surface_leave_handler(void*, wl_surface*, wl_output*) noexcept -> void {
            // nothing
        }
    } // namespace wl
} // namespace stormkit::wsi::linux::wayland
