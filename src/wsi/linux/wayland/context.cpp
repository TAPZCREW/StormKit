// Copyright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
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

module stormkit.wsi;

import std;

import stormkit.core;

import :linux.wayland;
import :linux.wayland.context;
import :linux.wayland.input;
import :linux.wayland.window_impl;
import :linux.wayland.log;

namespace stdr = std::ranges;

namespace stormkit::wsi::linux::wayland::wl {
    auto registry_handler(void*, wl_registry*, u32, const char*, u32) noexcept -> void;
    auto registry_remove_handler(void*, wl_registry*, u32) noexcept -> void;

    auto output_geometry_handler(void*,
                                 wl_output*,
                                 i32,
                                 i32,
                                 i32,
                                 i32,
                                 i32,
                                 const char*,
                                 const char*,
                                 i32) noexcept -> void;
    auto output_mode_handler(void*, wl_output*, u32, i32, i32, i32) noexcept -> void;
    auto output_done_handler(void*, wl_output*) noexcept -> void;
    auto output_scale_handler(void*, wl_output*, i32) noexcept -> void;
    auto output_name_handler(void*, wl_output*, const char*) noexcept -> void;
    auto output_description_handler(void*, wl_output*, const char*) noexcept -> void;

    auto wm_base_ping_handler(void*, xdg_wm_base*, u32) noexcept -> void;

    /////////////////////////////////////
    /////////////////////////////////////
    auto get_monitor(Globals& _globals, void* output) noexcept -> Monitor& {
        const auto output_id = std::bit_cast<uptr>(output);
        const auto is_output = [&output_id](const auto& pair) noexcept {
            return pair.id == output_id;
        };

        if (auto it = stdr::find_if(_globals.monitors, is_output);
            it != stdr::end(_globals.monitors))
            return it->monitor;

        _globals.monitors.push_back(WaylandMonitor { .id = output_id, .monitor = {} });
        return _globals.monitors.back().monitor;
    }

    namespace {
        thread_local constinit auto globals = Globals {};

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

        struct RegistryBinder {
            const wl_interface*                interface;
            FunctionRef<void(Globals&, void*)> bind;
            u32                                version    = 1;
            FunctionRef<void(Globals&, void*)> after_bind = monadic::noop();
        };

        /////////////////////////////////////
        /////////////////////////////////////
        template<auto member>
        constexpr auto make_binder() noexcept -> decltype(auto) {
            return [](Globals& _globals, void* ptr) static noexcept {
                using U = meta::CanonicalType<decltype(globals.*member)>;
                (_globals.*member).reset(std::bit_cast<typename U::value_type>(ptr));
            };
        }

        /////////////////////////////////////
        /////////////////////////////////////
        template<auto member>
        constexpr auto make_binder_to_array() noexcept -> decltype(auto) {
            return [](Globals& _globals, void* ptr) static noexcept {
                using Vec = meta::CanonicalType<decltype(globals.*member)>;
                using U   = meta::CanonicalType<typename Vec::value_type>;
                auto& ret = (_globals.*member).emplace_back();
                ret.reset(std::bit_cast<typename U::value_type>(ptr));
            };
        }

        const auto INTERFACE_MAP = frozen::make_unordered_map<frozen::string, RegistryBinder>({
          { frozen::string { wl_compositor_interface.name },
           { &wl_compositor_interface, make_binder<&Globals::compositor>(), 4 } },
          {
           frozen::string { wl_output_interface.name },
           { &wl_output_interface,
              make_binder_to_array<&Globals::outputs>(),
              4,
              [](Globals& _globals, void* output) static noexcept {
                  wl_output_add_listener(std::bit_cast<wl_output*>(output),
                                         &g_output_listener,
                                         &_globals);
              } },
           },
          {
           frozen::string { xdg_wm_base_interface.name },
           { &xdg_wm_base_interface,
              make_binder<&Globals::xdg_wm_base>(),
              3,
              [](Globals& _globals, void* output) static noexcept {
                  xdg_wm_base_add_listener(std::bit_cast<xdg_wm_base*>(output),
                                           &g_wm_base_listener,
                                           &_globals);
              } },
           },
          { frozen::string { wl_shm_interface.name },
           { &wl_shm_interface, make_binder<&Globals::shm>(), 1 } },
          { frozen::string { zxdg_decoration_manager_v1_interface.name },
           { &zxdg_decoration_manager_v1_interface,
              make_binder<&Globals::decoration_manager>(),
              1 } },
          { frozen::string { wl_seat_interface.name },
           { &wl_seat_interface,
              make_binder<&Globals::seat>(),
              8,
              [](Globals& _globals, void* output) static noexcept {
                  wl_seat_add_listener(std::bit_cast<wl_seat*>(output),
                                       &g_seat_listener,
                                       &_globals);
              } } },
          { frozen::string { wp_pointer_warp_v1_interface.name },
           { &wp_pointer_warp_v1_interface, make_binder<&Globals::pointer_warp>(), 1 } },
          { frozen::string { zwp_pointer_constraints_v1_interface.name },
           { &zwp_pointer_constraints_v1_interface,
              make_binder<&Globals::pointer_constraints>(),
              1 } },
          { frozen::string { wp_cursor_shape_manager_v1_interface.name },
           { &wp_cursor_shape_manager_v1_interface,
              make_binder<&Globals::cursor_shape_manager>(),
              1 } },
          { frozen::string { zwp_relative_pointer_manager_v1_interface.name },
           { &zwp_relative_pointer_manager_v1_interface,
              make_binder<&Globals::relative_pointer_manager>(),
              1 } },
          { frozen::string { wp_single_pixel_buffer_manager_v1_interface.name },
           { &wp_single_pixel_buffer_manager_v1_interface,
              make_binder<&Globals::single_pixel_buffer_manager>(),
              1 } },
          { frozen::string { wp_viewporter_interface.name },
           { &wp_viewporter_interface, make_binder<&Globals::viewporter>(), 1 } },
          { frozen::string { wp_content_type_manager_v1_interface.name },
           { &wp_content_type_manager_v1_interface,
              make_binder<&Globals::content_type_manager>(),
              1 } },
        });
    } // namespace

    /////////////////////////////////////
    /////////////////////////////////////
    auto init() noexcept -> bool {
        if (globals.initialized) return true;

        auto _globals = Globals {};

        _globals.display = wl::Display { std::in_place, nullptr };
        if (not _globals.display) {
            elog("Failed to initialize Wayland display");
            return false;
        }

        _globals.registry = wl::Registry { std::in_place, _globals.display };
        if (not _globals.registry) {
            elog("Failed to initialize Wayland display");
            return false;
        }

        wl_registry_add_listener(_globals.registry, &g_registry_listener, &_globals);

        wl_display_roundtrip(_globals.display);
        wl_display_dispatch(_globals.display);

        if (not _globals.compositor) {
            elog("Failed to find compositor interface");
            return false;
        }

        if (not _globals.decoration_manager) {
            elog("{} protocol is not supported by this DE, can't enable server side decoration.",
                 zxdg_decoration_manager_v1_interface.name);
            return false;
        }

        if (not _globals.cursor_shape_manager) {
            auto cursor_size = 16;

            const auto size_str = std::getenv("XCURSOR_SIZE");
            if (size_str) cursor_size = *from_string<i32>(size_str, 10);

            const auto theme = std::getenv("XCURSOR_THEME");

            _globals
              .cursor_theme = wl::CursorTheme { std::in_place, theme, cursor_size, _globals.shm };
            _globals.cursor_theme_high_dpi = wl::CursorTheme {
                std::in_place,
                theme,
                cursor_size * 2,
                _globals.shm
            };
        }

        _globals.initialized = true;
        globals              = std::move(_globals);

        dlog("Wayland backend successfully initialiazed");
        return true;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto get_globals() noexcept -> Globals& {
        return globals;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto registry_handler(void*        data,
                          wl_registry* registry,
                          u32          id,
                          const char*  interface,
                          u32          version) noexcept -> void {
        dlog("Registry found interface {} (id: {}, version: {})", interface, id, version);

        auto& _globals = *std::bit_cast<Globals*>(data);

        const auto interface_name = std::string_view { interface,
                                                       std::char_traits<char>::length(interface) };

        const auto it = INTERFACE_MAP.find(interface_name);
        if (it == stdr::cend(INTERFACE_MAP)) return;

        const auto& [_, binder] = *it;
        if (version < binder.version) {
            elog("Requested version {} for interface {} is not supported (found {})",
                 binder.version,
                 interface_name,
                 version);
            return;
        }

        const auto ptr = wl_registry_bind(registry, id, binder.interface, binder.version);
        binder.bind(_globals, ptr);
        binder.after_bind(_globals, ptr);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto registry_remove_handler(void*, wl_registry*, u32) noexcept -> void {
        // nothing
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto output_geometry_handler(void*      data,
                                 wl_output* output,
                                 i32,
                                 i32,
                                 i32,
                                 i32,
                                 i32,
                                 const char*,
                                 const char*,
                                 i32) noexcept -> void {
        auto&       _globals = *std::bit_cast<Globals*>(data);
        const auto& _        = get_monitor(_globals, output);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto output_mode_handler(void*      data,
                             wl_output* output,
                             u32,
                             i32 width,
                             i32 height,
                             i32) noexcept -> void {
        auto& _globals = *std::bit_cast<Globals*>(data);
        auto& monitor  = get_monitor(_globals, output);

        monitor.extents.emplace_back(as<u32>(width), as<u32>(height));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto output_done_handler(void* data, wl_output* output) noexcept -> void {
        auto& _globals = *std::bit_cast<Globals*>(data);
        auto& monitor  = get_monitor(_globals, output);

        if (&monitor == &_globals.monitors.front().monitor) monitor.flags = Monitor::Flags::PRIMARY;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto output_scale_handler(void* data, wl_output* output, i32 scale_factor) noexcept -> void {
        auto& _globals       = *std::bit_cast<Globals*>(data);
        auto& monitor        = get_monitor(_globals, output);
        monitor.scale_factor = scale_factor;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto output_name_handler(void* data, wl_output* output, const char* name) noexcept -> void {
        auto& _globals = *std::bit_cast<Globals*>(data);
        auto& monitor  = get_monitor(_globals, output);
        monitor.name   = name;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto output_description_handler(void* data, wl_output* output, const char* description) noexcept
      -> void {
        auto& _globals = *std::bit_cast<Globals*>(data);
        auto& monitor  = get_monitor(_globals, output);
        monitor.name   = std::format("{} ({})", monitor.name, description);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto wm_base_ping_handler(void*, xdg_wm_base* xdg_shell, u32 serial) noexcept -> void {
        dlog("Ping received from xdg shell");

        xdg_wm_base_pong(xdg_shell, serial);
    }
} // namespace stormkit::wsi::linux::wayland::wl
