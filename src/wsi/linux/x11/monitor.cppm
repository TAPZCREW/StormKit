// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <xcb/randr.h>
#include <xcb/xcb.h>

module stormkit.wsi:linux.x11.monitor;

import std;

import :core;
import :monitor;

import :linux.x11.context;

namespace stdr = std::ranges;

namespace stormkit::wsi::linux::x11 {
    /////////////////////////////////////
    /////////////////////////////////////
    auto get_monitors(WM, bool update) noexcept -> array_view<const Monitor> {
        thread_local auto monitors = dynarray<Monitor> {};

        if (stdr::empty(monitors) or update) {
            auto& globals = xcb::get_globals();

            using Monitors = RAIICapsule<xcb_randr_get_monitors_reply_t*,
                                         xcb_randr_get_monitors_reply,
                                         std::free,
                                         struct MonitorTag>;
            using Output   = RAIICapsule<xcb_randr_get_output_info_reply_t*,
                                         xcb_randr_get_output_info_reply,
                                         std::free,
                                         struct OutputTag>;
            using CRTC = RAIICapsule<xcb_randr_get_crtc_info_reply_t*, xcb_randr_get_crtc_info_reply, std::free, struct CRTCTag>;

            const auto root = xcb_setup_roots_iterator(xcb_get_setup(globals.connection)).data;

            auto xcb_monitors = Monitors::create(globals.connection,
                                                 xcb_randr_get_monitors(globals.connection, root->root, 0),
                                                 nullptr);

            auto xcb_monitor_iter = xcb_randr_get_monitors_monitors_iterator(xcb_monitors);
            for (auto i = 0; xcb_monitor_iter.rem; xcb_randr_monitor_info_next(&xcb_monitor_iter), ++i) {
                auto monitor_info = xcb_monitor_iter.data;

                xcb_randr_select_input(globals.connection, root->root, true);

                auto& monitor = monitors.emplace_back();
                if (monitor_info->primary) monitor.flags = Monitor::Flags::PRIMARY;

                auto name = xcb::get_atom_name(monitor_info->name);
                if (!name) monitor.name = std::format("Monitor {}", i);
                else
                    monitor.name = std::move(*name);

                auto len     = xcb_randr_monitor_info_outputs_length(monitor_info);
                auto outputs = xcb_randr_monitor_info_outputs(monitor_info);

                for (auto j : range(len)) {
                    auto
                      output_cookie = xcb_randr_get_output_info(globals.connection, outputs[j], xcb_monitors.handle()->timestamp);
                    auto output     = Output::create(globals.connection, output_cookie, nullptr);

                    if (!output) continue;
                    if (output.handle()->connection != XCB_RANDR_CONNECTION_CONNECTED) continue;
                    if (output.handle()->crtc != XCB_NONE) continue;

                    auto crtc_cookie = xcb_randr_get_crtc_info(globals.connection,
                                                               output.handle()->crtc,
                                                               output.handle()->timestamp);
                    auto crtc        = CRTC::create(globals.connection, crtc_cookie, nullptr);

                    if (crtc == nullptr) {}

                    monitor.extents.emplace_back(math::extent2 { as<u32>(crtc.handle()->width), as<u32>(crtc.handle()->height) });
                }

                if (stdr::empty(monitor.extents))
                    monitor.extents.emplace_back(math::extent2 { as<u32>(xcb_monitor_iter.data->width),
                                                                 as<u32>(xcb_monitor_iter.data->height) });
            }
        }
        return monitors;
    }
} // namespace stormkit::wsi::linux::x11
