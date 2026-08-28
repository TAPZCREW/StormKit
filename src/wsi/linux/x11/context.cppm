// Copyright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_errors.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xinput.h>

export module stormkit.wsi:linux.x11.context;

import std;

import stormkit.core;

import :linux.common.xkb;

import :linux.x11.xcb;

export namespace stormkit::wsi::linux::x11::xcb {
    struct Globals {
        Connection   connection    = Connection::empty();
        ErrorContext error_context = ErrorContext::empty();

        common::xkb::Context xkb_context = common::xkb::Context::empty();
    };

    auto init() noexcept -> bool;
    auto get_globals() noexcept -> Globals&;

    auto get_atom(string_view name, bool only_if_exists) noexcept -> std::expected<xcb_atom_t, Error>;
    auto get_atom_name(xcb_atom_t atom) -> std::expected<string, Error>;

    auto get_error(ref<xcb_generic_error_t> error) -> string;

    auto get_xi_device_info(xcb_input_device_id_t device_id) -> std::expected<ref<xcb_input_xi_device_info_t>, Error>;

    // template<typename T>
    // auto get_xft_value(string_view name) -> std::optional<T>;
} // namespace stormkit::wsi::linux::x11::xcb

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

export namespace stormkit::wsi::linux::x11::xcb {
    namespace stdr = std::ranges;

    // /////////////////////////////////////
    // /////////////////////////////////////
    // template<typename T>
    // auto get_xft_value(string_view name) -> std::optional<T> {
    //     using XcbXrmDatabase = RAIICapsule<xcb_xrm_database_t*,
    //                                        xcb_xrm_database_from_default,
    //                                        xcb_xrm_database_free,
    //                                        struct XRMDbTag,
    //                                        nullptr>;

    //    auto db  = XcbXrmDatabase::create(xcb::get_globals().connection);
    //    auto val = long { 0 };
    //    auto res = xcb_xrm_resource_get_long(db, stdr::data(name), nullptr, &val);
    //    if (res < 0) {
    //        elog("Could not fetch value of {}", name);
    //        return std::nullopt;
    //    }
    //    return as<T>(val);
    // } // namespace stormkit::wsi::linux::x11
} // namespace stormkit::wsi::linux::x11::xcb
