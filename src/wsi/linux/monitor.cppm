// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
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
    auto get_monitors(WM wm, bool update = false) noexcept -> array_view<const Monitor> {
        switch (wm) {
            case WM::X11: return x11::get_monitors(wm, update);
            case WM::WAYLAND: return wayland::get_monitors(wm, update);
            default: break;
        }

        std::unreachable();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto get_primary_monitor(WM wm) noexcept -> const Monitor& {
        const auto monitors = get_monitors(wm);
        auto       it       = stdr::find_if(monitors, [](const auto& monitor) static noexcept {
            return has_flag_bit(monitor.flags, Monitor::Flags::PRIMARY);
        });
        return *it;
    }
} // namespace stormkit::wsi::linux
