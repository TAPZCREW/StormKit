// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

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
// #include <xdg-shell-client-protocol.h>
//
#include <stormkit/core/try_expected.hpp>

module stormkit.wsi;

import std;
import frozen;

import stormkit.core;

import :linux.wayland;
import :linux.wayland.context;
import :linux.wayland.input;
import :linux.wayland.window;
import :linux.wayland.log;

namespace stdr = std::ranges;

namespace stormkit::wsi::linux::wayland::wl {
    auto registry_handler(void*, wl_registry*, u32, const char*, u32) noexcept -> void;
    auto registry_remove_handler(void*, wl_registry*, u32) noexcept -> void;

    auto output_geometry_handler(void*, wl_output*, i32, i32, i32, i32, i32, const char*, const char*, i32) noexcept -> void;
    auto output_mode_handler(void*, wl_output*, u32, i32, i32, i32) noexcept -> void;
    auto output_done_handler(void*, wl_output*) noexcept -> void;
    auto output_scale_handler(void*, wl_output*, i32) noexcept -> void;
    auto output_name_handler(void*, wl_output*, const char*) noexcept -> void;
    auto output_description_handler(void*, wl_output*, const char*) noexcept -> void;

    auto wm_base_ping_handler(void*, ::xdg_wm_base*, u32) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto get_monitor(wl_globals& globals, void* output) noexcept -> monitor& {
        const auto output_id = std::bit_cast<uptr>(output);
        const auto is_output = [&output_id](const auto& pair) noexcept { return pair.id == output_id; };

        if (auto it = stdr::find_if(globals.monitors, is_output); it != stdr::end(globals.monitors)) return it->monitor;

        globals.monitors.push_back(wayland_monitor { .id = output_id, .monitor = {} });
        return globals.monitors.back().monitor;
    }

    namespace {
        thread_local constinit auto globals = wl_globals {};

        constexpr auto g_registry_listener = wl_registry_listener {
            .global        = registry_handler,
            .global_remove = registry_remove_handler,
        };

        constexpr auto g_output_listener = wl_output_listener {
            .geometry    = output_geometry_handler,
            .mode        = output_mode_handler,
            .done        = output_done_handler,
            .scale       = output_scale_handler,
            .name        = output_name_handler,
            .description = output_description_handler,
        };

        constexpr auto g_wm_base_listener = xdg_wm_base_listener {
            .ping = wm_base_ping_handler,
        };

        constexpr const auto g_seat_listener = wl_seat_listener {
            .capabilities = seat_capabilities_handler,
            .name         = seat_name_handler,
        };

        struct registry_binder {
            const wl_interface*                           interface;
            std23::function_ref<void(wl_globals&, void*)> bind;
            u32                                           version    = 1;
            std23::function_ref<void(wl_globals&, void*)> after_bind = monadic::noop();
        };

        /////////////////////////////////////
        /////////////////////////////////////
        template<auto member>
        constexpr auto make_binder() noexcept -> decltype(auto) {
            return [](wl_globals& globals, void* ptr) static noexcept {
                using U           = meta::to_plain_type<decltype(globals.*member)>;
                (globals.*member) = U::take(std::bit_cast<meta::value_type<U>>(ptr));
            };
        }

        /////////////////////////////////////
        /////////////////////////////////////
        template<auto member>
        constexpr auto make_binder_to_array() noexcept -> decltype(auto) {
            return [](wl_globals& globals, void* ptr) static noexcept {
                using Vec = meta::to_plain_type<decltype(globals.*member)>;
                using U   = meta::value_type<Vec>;
                (globals.*member).push_back(U::take(std::bit_cast<meta::value_type<U>>(ptr)));
            };
        }

        const auto INTERFACE_MAP = make_static_hash_map<frozen::string, registry_binder>({
          { frozen::string { wl_compositor_interface.name },
           { &wl_compositor_interface, make_binder<&wl_globals::compositor>(), 4 } },
          {
           frozen::string { wl_output_interface.name },
           { &wl_output_interface,
              make_binder_to_array<&wl_globals::outputs>(),
              4,
              [](wl_globals& globals, void* output) static noexcept {
                  wl_output_add_listener(reinterpret_cast<wl_output*>(output), &g_output_listener, &globals);
              } },
           },
          {
           frozen::string { xdg_wm_base_interface.name },
           { &xdg_wm_base_interface,
              make_binder<&wl_globals::xdg_wm_base>(),
              3,
              [](wl_globals& globals, void* output) static noexcept {
                  xdg_wm_base_add_listener(reinterpret_cast<::xdg_wm_base*>(output), &g_wm_base_listener, &globals);
              } },
           },
          { frozen::string { wl_shm_interface.name }, { &wl_shm_interface, make_binder<&wl_globals::shm>(), 1 } },
          { frozen::string { zxdg_decoration_manager_v1_interface.name },
           { &zxdg_decoration_manager_v1_interface, make_binder<&wl_globals::decoration_manager>(), 1 } },
          { frozen::string { wl_seat_interface.name },
           { &wl_seat_interface,
              make_binder<&wl_globals::seat>(),
              8,
              [](wl_globals& globals, void* output) static noexcept {
                  wl_seat_add_listener(reinterpret_cast<wl_seat*>(output), &g_seat_listener, &globals);
              } } },
          { frozen::string { wp_pointer_warp_v1_interface.name },
           { &wp_pointer_warp_v1_interface, make_binder<&wl_globals::pointer_warp>(), 1 } },
          { frozen::string { zwp_pointer_constraints_v1_interface.name },
           { &zwp_pointer_constraints_v1_interface, make_binder<&wl_globals::pointer_constraints>(), 1 } },
          { frozen::string { wp_cursor_shape_manager_v1_interface.name },
           { &wp_cursor_shape_manager_v1_interface, make_binder<&wl_globals::cursor_shape_manager>(), 1 } },
          { frozen::string { zwp_relative_pointer_manager_v1_interface.name },
           { &zwp_relative_pointer_manager_v1_interface, make_binder<&wl_globals::relative_pointer_manager>(), 1 } },
          { frozen::string { wp_single_pixel_buffer_manager_v1_interface.name },
           { &wp_single_pixel_buffer_manager_v1_interface, make_binder<&wl_globals::single_pixel_buffer_manager>(), 1 } },
          { frozen::string { wp_viewporter_interface.name },
           { &wp_viewporter_interface, make_binder<&wl_globals::viewporter>(), 1 } },
          { frozen::string { wp_content_type_manager_v1_interface.name },
           { &wp_content_type_manager_v1_interface, make_binder<&wl_globals::content_type_manager>(), 1 } },
        });
    } // namespace

    /////////////////////////////////////
    /////////////////////////////////////
    auto init() noexcept -> bool {
        if (globals.initialized) return true;

        auto globals_ = wl_globals {};

        globals_.display = wl::display::create(nullptr);
        if (not globals_.display) {
            elog("Failed to initialize Wayland display");
            return false;
        }

        globals_.registry = wl::registry::create(globals_.display);
        if (not globals_.registry) {
            elog("Failed to initialize Wayland display");
            return false;
        }

        wl_registry_add_listener(globals_.registry, &g_registry_listener, &globals_);

        wl_display_roundtrip(globals_.display);
        wl_display_dispatch(globals_.display);

        if (not globals_.compositor) {
            elog("Failed to find compositor interface");
            return false;
        }

        if (not globals_.decoration_manager) {
            elog("{} protocol is not supported by this DE, can't enable server side decoration.",
                 zxdg_decoration_manager_v1_interface.name);
            return false;
        }

        if (not globals_.cursor_shape_manager) {
            auto cursor_size = 16;

            const auto size_str = std::getenv("XCURSOR_SIZE");
            if (size_str) {
                TryToOr(size, (to<i32>(size_str, 10)), ([](auto&&) static noexcept { return 16; }));
                cursor_size = size;
            }

            const auto theme = std::getenv("XCURSOR_THEME");

            globals_.cursor_theme          = wl::cursor_theme::create(theme, cursor_size, globals_.shm);
            globals_.cursor_theme_high_dpi = wl::cursor_theme::create(theme, cursor_size * 2, globals_.shm);
        }

        globals_.initialized = true;
        globals              = std::move(globals_);

        dlog("Wayland backend successfully initialiazed");
        return true;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto get_globals() noexcept -> wl_globals& {
        return globals;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto registry_handler(void* data, wl_registry* registry, u32 id, const char* interface, u32 version) noexcept -> void {
        dlog("registry found interface {} (id: {}, version: {})", interface, id, version);

        auto& globals = *reinterpret_cast<wl_globals*>(data);

        const auto interface_name = string_view { interface, std::char_traits<char>::length(interface) };

        const auto it = INTERFACE_MAP.find(interface_name);
        if (it == stdr::cend(INTERFACE_MAP)) return;

        const auto& [_, binder] = *it;
        if (version < binder.version) {
            elog("Requested version {} for interface {} is not supported (found {})", binder.version, interface_name, version);
            return;
        }

        const auto ptr = wl_registry_bind(registry, id, binder.interface, binder.version);
        binder.bind(globals, ptr);
        binder.after_bind(globals, ptr);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto registry_remove_handler(void*, wl_registry*, u32) noexcept -> void {
        // nothing
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto output_geometry_handler(void* data, wl_output* output, i32, i32, i32, i32, i32, const char*, const char*, i32) noexcept
      -> void {
        auto&       globals = *reinterpret_cast<wl_globals*>(data);
        const auto& _       = get_monitor(globals, output);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto output_mode_handler(void* data, wl_output* output, u32, i32 width, i32 height, i32) noexcept -> void {
        auto& globals = *reinterpret_cast<wl_globals*>(data);
        auto& monitor = get_monitor(globals, output);

        monitor.extents.emplace_back(as<u32>(width), as<u32>(height));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto output_done_handler(void* data, wl_output* output) noexcept -> void {
        auto& globals = *reinterpret_cast<wl_globals*>(data);
        auto& monitor = get_monitor(globals, output);

        if (&monitor == &globals.monitors.front().monitor) monitor.flags = monitor::flag::primary;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto output_scale_handler(void* data, wl_output* output, i32 scale_factor) noexcept -> void {
        auto& globals        = *reinterpret_cast<wl_globals*>(data);
        auto& monitor        = get_monitor(globals, output);
        monitor.scale_factor = as<u32>(scale_factor);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto output_name_handler(void* data, wl_output* output, const char* name) noexcept -> void {
        auto& globals = *reinterpret_cast<wl_globals*>(data);
        auto& monitor = get_monitor(globals, output);
        monitor.name  = name;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto output_description_handler(void* data, wl_output* output, const char* description) noexcept -> void {
        auto& globals = *reinterpret_cast<wl_globals*>(data);
        auto& monitor = get_monitor(globals, output);
        monitor.name  = std::format("{} ({})", monitor.name, description);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto wm_base_ping_handler(void*, ::xdg_wm_base* xdg_shell, u32 serial) noexcept -> void {
        dlog("Ping received from xdg shell");

        xdg_wm_base_pong(xdg_shell, serial);
    }
} // namespace stormkit::wsi::linux::wayland::wl
