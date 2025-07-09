// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>

module stormkit.log;

import std;

import stormkit.core;

namespace stormkit::log {
    namespace {
#ifdef STORMKIT_BUILD_DEBUG
        constexpr auto DEFAULT_SEVERITY = Severity::INFO
                                          | Severity::DEBUG
                                          | Severity::ERROR
                                          | Severity::FATAL
                                          | Severity::WARNING;
#else
        constexpr auto DEFAULT_SEVERITY = Severity::INFO | Severity::ERROR | Severity::FATAL;
#endif
        constinit Logger* logger = nullptr;
    } // namespace

    /////////////////////////////////////
    /////////////////////////////////////
    Logger::Logger(LogClock::time_point start_time) noexcept
        : Logger { std::move(start_time), DEFAULT_SEVERITY } {
        EXPECTS(not logger);

        logger = this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    Logger::Logger(LogClock::time_point start_time, Severity log_level) noexcept
        : m_start_time { std::move(start_time) }, m_log_level { log_level } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    Logger::~Logger() noexcept {
        logger = nullptr;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Logger::has_logger() noexcept -> bool {
        if (logger) [[likely]]
            return true;
        return false;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Logger::instance() noexcept -> Logger& {
        EXPECTS(logger);

        return *logger;
    }
} // namespace stormkit::log
