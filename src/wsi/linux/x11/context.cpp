// Copyright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <xkbcommon/xkbcommon.h>

#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_errors.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xinput.h>

#include <stormkit/core/contract_macro.hpp>

module stormkit.wsi;

import std;

import stormkit.core;

import :linux.x11.log;
import :linux.x11.xcb;
import :linux.x11.context;

namespace stdr = std::ranges;

namespace stormkit::wsi::linux::x11::xcb {
    namespace {
        thread_local constinit auto initialized = false;
        thread_local constinit auto globals     = Globals {};
        thread_local auto           atoms       = stormkit::StringHashMap<xcb_atom_t> {};
    } // namespace

    /////////////////////////////////////
    /////////////////////////////////////
    auto init() noexcept -> bool {
        if (initialized) return true;

        globals.connection = xcb::Connection { std::in_place, nullptr, nullptr };
        if (not globals.connection or xcb_connection_has_error(globals.connection)) {
            elog("Failed to connect to X11");
            return false;
        }

        globals.error_context = xcb::ErrorContext { std::in_place, globals.connection };
        if (not globals.error_context) elog("Failed to setup X11 error context");

        globals.xkb_context = common::xkb::Context { std::in_place, XKB_CONTEXT_NO_FLAGS };

        initialized = true;
        dlog("Successfully connected to X11");
        return true;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto get_globals() noexcept -> Globals& {
        if (not initialized) initialized = init();

        EXPECTS(initialized);

        return globals;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto get_atom(std::string_view name, bool only_if_exists) noexcept
      -> std::expected<xcb_atom_t, Error> {
        auto& globals = get_globals();

        auto out = std::expected<xcb_atom_t, Error> {};

        auto it = atoms.find(name);
        if (it != stdr::end(atoms)) out = it->second;
        else {
            const auto cookie = xcb_intern_atom(globals.connection,
                                                (only_if_exists) ? 1 : 0,
                                                as<u16>(stdr::size(name)),
                                                stdr::data(name));
            auto       error  = xcb::GenericError {};
            const auto reply  = xcb::InternAtomReply { std::in_place,
                                                      globals.connection,
                                                      cookie,
                                                      &error.handle() };

            if (error or not reply.handle())
                out = std::unexpected<Error> { std::in_place, get_error(as_ref_mut(*error)) };
            else {
                auto atom = reply.handle()->atom;
                atoms.emplace(name, atom);

                out = atom;
            }
        }

        return out;
    }

    auto get_atom_name(xcb_atom_t atom) -> std::expected<std::string, Error> {
        auto out = std::expected<std::string, Error> {};

        const auto cookie = xcb_get_atom_name(globals.connection, atom);

        auto error = xcb::GenericError {};
        const auto
          reply = xcb::AtomNameReply { std::in_place, globals.connection, cookie, &error.handle() };
        if (error) out = std::unexpected<Error> { std::in_place, get_error(as_ref_mut(*error)) };
        else
            out = std::string { xcb_get_atom_name_name(reply),
                                as<usize>(xcb_get_atom_name_name_length(reply)) };

        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto get_error(Ref<xcb_generic_error_t> error) -> std::string {
        auto guard = xcb::GenericError { error };

        const auto major = xcb_errors_get_name_for_major_code(globals.error_context,
                                                              error->major_code);
        const auto minor = xcb_errors_get_name_for_minor_code(globals.error_context,
                                                              error->major_code,
                                                              error->minor_code);

        const auto* extension = CZString { nullptr };
        const auto  str_error = xcb_errors_get_name_for_error(globals.error_context,
                                                             error->major_code,
                                                             &extension);

        return std::format("{} extension: {} major: {} minor: {}\n",
                           str_error,
                           (extension) ? extension : "N/A",
                           major,
                           (minor) ? minor : "N/A");
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto get_xi_device_info(xcb_input_device_id_t device_id)
      -> std::expected<Ref<xcb_input_xi_device_info_t>, Error> {
        auto out = std::expected<Ref<xcb_input_xi_device_info_t>, Error> { std::unexpect };

        const auto cookie = xcb_input_xi_query_device(globals.connection, device_id);
        auto       error  = xcb::GenericError {};
        const auto reply  = xcb::InputXIQueryDeviceReply {
            std::in_place,
            globals.connection,
            cookie,
            &error.handle()
        };

        if (error) out = std::unexpected<Error> { std::in_place, get_error(as_ref_mut(*error)) };
        else {
            auto device_info_it = xcb_input_xi_query_device_infos_iterator(reply);
            for (; device_info_it.rem != 0; xcb_input_xi_device_info_next(&device_info_it)) {
                auto info = device_info_it.data;

                if (info->deviceid == device_id) out = as_ref_mut(info);
            }

            ensures(out.has_value());
        }

        return out;
    }
} // namespace stormkit::wsi::linux::x11::xcb
