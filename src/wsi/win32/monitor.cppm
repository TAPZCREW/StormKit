// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

export module stormkit.wsi:win32.monitor;

import std;

import stormkit.core.win32;

import :core;
import :monitor;

namespace stdr = std::ranges;

export namespace stormkit::wsi::win32 {
    /////////////////////////////////////
    /////////////////////////////////////
    auto load_monitor(::win32::HMONITOR native) noexcept -> monitor {
        auto monitor_info   = ::win32::MONITORINFOEX {};
        monitor_info.cbSize = sizeof(::win32::MONITORINFOEX);

        ::win32::GetMonitorInfoA(native, &monitor_info);

        auto monitor          = wsi::monitor {};
        monitor.native_handle = native;
        if (has_flag_bit<::win32::DWORD>(monitor_info.dwFlags, ::win32::MONITORINFOF_PRIMARY))
            monitor.flags = wsi::monitor::flag::primary;
        // if ((monitor_info.dwFlags & MONITORINFOF_PRIMARY) == MONITORINFOF_PRIMARY) monitor.flags = wsi::monitor::flag::primary;

        monitor.name = string { monitor_info.szDevice };

        auto dm = ::win32::DEVMODE {};

        for (auto i = 0; ::win32::EnumDisplaySettingsA(monitor_info.szDevice, i, &dm) != 0; ++i) {
            monitor.extents.emplace_back(as<u32>(dm.dmPelsWidth), as<u32>(dm.dmPelsHeight));
        }

        // currently do ICE on clang
        // monitor.extents.erase(std::unique(std::begin(monitor.extents),
        //                                   std::end(monitor.extents)),
        //                       std::end(monitor.extents));
        // stdr::sort(monitor.extents);

        return monitor;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto load_monitors(::win32::HMONITOR native, ::win32::HDC, ::win32::LPRECT, ::win32::LPARAM data) noexcept -> ::win32::BOOL {
        if (native == nullptr) return ::win32::TRUE;

        auto& monitors = *reinterpret_cast<dynarray<monitor>*>(data);
        monitors.emplace_back(load_monitor(native));

        return ::win32::TRUE;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto get_monitors(window_manager, bool update = false) noexcept -> array_view<const monitor> {
        thread_local auto monitors = dynarray<monitor> {};

        if (update or stdr::empty(monitors))
            ::win32::EnumDisplayMonitors(nullptr, nullptr, load_monitors, reinterpret_cast<::win32::LPARAM>(&monitors));

        return monitors;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto get_primary_monitor(window_manager wm) noexcept -> const monitor& {
        const auto monitors = get_monitors(wm);
        auto       it       = stdr::find_if(monitors, [](const auto& monitor) static noexcept {
            return has_flag_bit(monitor.flags, monitor::flag::primary);
        });
        return *it;
    }
} // namespace stormkit::wsi::win32
