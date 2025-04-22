// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module stormkit.Log;

import std;

import stormkit.core;

using namespace std::literals;

namespace {
    constexpr auto LOG_FILE_NAME = "log.txt";
}

namespace stormkit::log {
    ////////////////////////////////////////
    ////////////////////////////////////////
    FileLogger::FileLogger(LogClock::time_point start, std::filesystem::path path)
        : Logger { std::move(start) }, m_base_path { std::move(path) } {
        if (not std::filesystem::exists(m_base_path))
            std::filesystem::create_directory(m_base_path);

        expects(std::filesystem::is_directory(m_base_path), "path need to be a directory");

        auto filepath                = m_base_path / to_native_encoding(LOG_FILE_NAME);
        m_streams[filepath.string()] = std::ofstream { filepath.string() };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    FileLogger::FileLogger(LogClock::time_point  start,
                           std::filesystem::path path,
                           Severity              log_level)
        : Logger { std::move(start), log_level }, m_base_path { std::move(path) } {
        if (not std::filesystem::exists(m_base_path))
            std::filesystem::create_directory(m_base_path);

        expects(std::filesystem::is_directory(m_base_path), "path need to be a directory");

        auto filepath                = m_base_path / to_native_encoding(LOG_FILE_NAME);
        m_streams[filepath.string()] = std::ofstream { filepath.string() };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    FileLogger::~FileLogger() {};

    ////////////////////////////////////////
    ////////////////////////////////////////
    FileLogger::FileLogger(FileLogger&&) = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto FileLogger::operator=(FileLogger&&) -> FileLogger& = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto FileLogger::flush() -> void {
        for (auto& [path, stream] : m_streams) stream.flush();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto FileLogger::write([[maybe_unused]] Severity      severity,
                           [[maybe_unused]] const Module& m,
                           [[maybe_unused]] const char*   string) -> void {
        const auto now = LogClock::now();
        const auto time
            = std::chrono::duration_cast<std::chrono::seconds>(now - m_start_time).count();

        auto filepath = m_base_path / std::filesystem::path { to_native_encoding(LOG_FILE_NAME) };
        if (not std::empty(m.name)) {
            filepath = m_base_path / to_native_encoding(m.name);
            filepath += to_native_encoding("-") + to_native_encoding(LOG_FILE_NAME);

            if (m_streams.find(filepath.string()) == m_streams.cend())
                m_streams[filepath.string()] = std::ofstream { filepath.string() };
        }

        static constexpr auto LOG_LINE        = "[{}, {}] {}\n"sv;
        static constexpr auto LOG_LINE_MODULE = "[{}, {}, {}] {}\n"sv;

        auto final_string = std::string {};
        if (std::empty(m.name))
            final_string = std::format(LOG_LINE, toString(severity), time, string);
        else
            final_string = std::format(LOG_LINE_MODULE, toString(severity), time, m.name, string);

        m_streams.at(filepath.string()) << final_string << std::flush;
    }
} // namespace stormkit::log
