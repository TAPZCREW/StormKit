// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <cstdio>

module stormkit.log;

import std;
import frozen;

import stormkit.core;

using namespace std::literals;

namespace stormkit::log {
    namespace {
        constexpr auto StyleMap = frozen::make_unordered_map<Severity, ConsoleStyle>({
          { Severity::INFO,
           ConsoleStyle { .fg = ConsoleColor::GREEN, .modifiers = StyleModifier::INVERSE }   },
          { Severity::WARNING,
           ConsoleStyle { .fg = ConsoleColor::MAGENTA, .modifiers = StyleModifier::INVERSE } },
          { Severity::ERROR,
           ConsoleStyle { .fg = ConsoleColor::YELLOW, .modifiers = StyleModifier::INVERSE }  },
          { Severity::FATAL,
           ConsoleStyle { .fg = ConsoleColor::RED, .modifiers = StyleModifier::INVERSE }     },
          { Severity::DEBUG,
           ConsoleStyle { .fg = ConsoleColor::CYAN, .modifiers = StyleModifier::INVERSE }    },
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
    auto ConsoleLogger::write(Severity severity, const Module& m, CZString string) noexcept
      -> void {
        const auto now  = LogClock::now();
        const auto time = std::chrono::duration_cast<std::chrono::seconds>(now - m_start_time);

        const auto str = [&]() {
            if (std::empty(m.name)) return std::format("[{}, {:%S}]", as_string(severity), time);
            else
                return std::format("[{}, {:%S}, {}]", as_string(severity), time, m.name);
        }();

        const auto is_error = severity == Severity::ERROR or severity == Severity::FATAL;
        const auto output   = (is_error) ? get_stderr() : get_stdout();

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
        std::fflush(get_stdout());
        std::fflush(get_stderr());
    }
} // namespace stormkit::log
