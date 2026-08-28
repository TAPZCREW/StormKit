// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

module stormkit.wsi:macos.monitor;

import std;

import :core;
import :monitor;

namespace stdr = std::ranges;

namespace stormkit::wsi::macos {
    /////////////////////////////////////
    /////////////////////////////////////
    auto get_monitors(WM, [[maybe_unused]] bool update = false) noexcept
      -> array_view<const Monitor> {
        return {};
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
} // namespace stormkit::wsi::macos
