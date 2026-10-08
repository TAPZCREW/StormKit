// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module stormkit.wsi:linux.monitor;

import std;

import :core;
import :monitor;

import :linux.wayland.monitor;
import :linux.x11.monitor;

namespace stormkit::wsi::linux {
    /////////////////////////////////////
    /////////////////////////////////////
    auto get_monitors(window_manager wm, bool update = false) noexcept -> array_view<const monitor> {
        switch (wm) {
            case window_manager::x11: return x11::get_monitors(wm, update);
            case window_manager::wayland: return wayland::get_monitors(wm, update);
            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto get_primary_monitor(window_manager wm) noexcept -> const monitor& {
        const auto monitors = get_monitors(wm);
        auto       it       = stdr::find_if(monitors, [](const auto& monitor) static noexcept {
            return has_flag_bit(monitor.flags, monitor::flags::primary);
        });
        return *it;
    }
} // namespace stormkit::wsi::linux
