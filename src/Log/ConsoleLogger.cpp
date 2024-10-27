// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <cstdio>

module stormkit.Log;

import std;
import frozen;

import stormkit.Core;

using namespace std::literals;

namespace stormkit::log {
    namespace {
        constexpr auto StyleMap = frozen::make_unordered_map<Severity, ConsoleStyle>({
            { Severity::Info,    ConsoleStyle { .bg = ConsoleColor::Green }   },
            { Severity::Warning, ConsoleStyle { .bg = ConsoleColor::Magenta } },
            { Severity::Error,   ConsoleStyle { .bg = ConsoleColor::Yellow }  },
            { Severity::Fatal,   ConsoleStyle { .bg = ConsoleColor::Red }     },
            { Severity::Debug,   ConsoleStyle { .bg = ConsoleColor::Cyan }    },
        });
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    ConsoleLogger::ConsoleLogger(LogClock::time_point start) noexcept
        : Logger { std::move(start) } {
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    ConsoleLogger::ConsoleLogger(LogClock::time_point start, Severity log_level) noexcept
        : Logger { std::move(start), log_level } {
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    ConsoleLogger::~ConsoleLogger() = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto ConsoleLogger::write(Severity severity, const Module& m, const char* string) -> void {
        const auto now  = LogClock::now();
        const auto time = std::chrono::duration_cast<std::chrono::seconds>(now - m_start_time);

        const auto str = [&]() {
            if (std::empty(m.name)) return std::format("[{}, {:%S}]", severity, time);
            else
                return std::format("[{}, {:%S}, {}]", severity, time, m.name);
        }();

        const auto is_error = severity == Severity::Error or severity == Severity::Fatal;
        const auto output   = (is_error) ? getSTDErr() : getSTDOut();

        // not yet
        /*
        auto state          = std::mbstate_t {};
        std::string out_str = std::string { MB_LEN_MAX };
        for (const auto &c : str) std::c8rtomb(std::data(out_str), c, &state);

        state                  = std::mbstate_t {};
        std::string out_string = std::string { MB_LEN_MAX };
        for (const auto &c : string) { std::c8rtomb(std::data(out_string), c, &state); }*/

        std::println(output, "{} {}", StyleMap.at(severity) | str, string);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto ConsoleLogger::flush() noexcept -> void {
        std::fflush(getSTDOut());
        std::fflush(getSTDErr());
    }
} // namespace stormkit::log
