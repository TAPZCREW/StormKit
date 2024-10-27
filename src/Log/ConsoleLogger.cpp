// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <cstdio>

module stormkit.Log;

import std;

import stormkit.Core;

using namespace std::literals;

namespace stormkit::log {
    auto makeStyle(Severity severity) noexcept -> ConsoleStyle {
        auto bg = ConsoleStyle::White;
        auto fg = ConsoleStyle::Black;
        switch (severity) {
            case Severity::Info: bg = ConsoleStyle::Green; break;
            case Severity::Warning: bg = ConsoleStyle::Magenta; break;
            case Severity::Error: bg = ConsoleStyle::Yellow; break;
            case Severity::Fatal: bg = ConsoleStyle::Red; break;
            case Severity::Debug: bg = ConsoleStyle::Cyan; break;
            default: std::unreachable();
        }
        return {
            .fg = fg,
            .bg = bg,
        };
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
        auto&      output   = (is_error) ? std::cerr : std::cout;
        const auto style    = makeStyle(severity);

        // not yet
        /*
        auto state          = std::mbstate_t {};
        std::string out_str = std::string { MB_LEN_MAX };
        for (const auto &c : str) std::c8rtomb(std::data(out_str), c, &state);

        state                  = std::mbstate_t {};
        std::string out_string = std::string { MB_LEN_MAX };
        for (const auto &c : string) { std::c8rtomb(std::data(out_string), c, &state); }*/

        std::print(output, "{}", style | str);
        std::println(output, " {}", string);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto ConsoleLogger::flush() noexcept -> void {
        std::fflush(std::cout);
    }
} // namespace stormkit::log
