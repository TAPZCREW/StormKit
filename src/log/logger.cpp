// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>

module stormkit.log;

import std;

import stormkit.core;

namespace stdr = std::ranges;

namespace stormkit::log {
    namespace {
        constexpr auto default_log_mask = severity::info | severity::error | severity::fatal | severity::warning;

        constinit logger* logger_instance = nullptr;

        constinit auto debug_enabled = false;
    } // namespace

    /////////////////////////////////////
    /////////////////////////////////////
    auto parse_args(array_view<const string_view> args) noexcept -> void {
        debug_enabled = stdr::find_if(args, [](auto&& v) { return v == "--debug" or v == "-d"; }) != stdr::cend(args);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    logger::logger(clock_type::time_point start_time) noexcept : logger { std::move(start_time), default_log_mask } {
        EXPECTS(not logger_instance);

        if (debug_enabled) severity_mask_ |= severity::debug;

        logger_instance = this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    logger::logger(clock_type::time_point start_time, severity log_level) noexcept
        : start_time_ { std::move(start_time) }, severity_mask_ { log_level } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    logger::~logger() noexcept {
        logger_instance = nullptr;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto logger::has_logger() noexcept -> bool {
        if (logger_instance) [[likely]]
            return true;
        return false;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto logger::instance() noexcept -> logger& {
        EXPECTS(logger_instance != nullptr);

        return *logger_instance;
    }
} // namespace stormkit::log
