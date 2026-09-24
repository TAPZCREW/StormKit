// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module stormkit.log;

import std;

import stormkit.core;

using namespace std::literals;

namespace stdfs = std::filesystem;

namespace {
    constexpr auto LOG_FILE_NAME = "log.txt";
}

namespace stormkit::log {
    ////////////////////////////////////////
    ////////////////////////////////////////
    file_logger::file_logger(clock_type::time_point start, stdfs::path path) noexcept
        : logger { std::move(start) }, m_base_path { std::move(path) } {
        if (not stdfs::exists(m_base_path)) stdfs::create_directory(m_base_path);

        expects(stdfs::is_directory(m_base_path), "path need to be a directory");

        auto filepath                = m_base_path / to_native_encoding(LOG_FILE_NAME);
        m_streams[filepath.string()] = std::ofstream { filepath.string() };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    file_logger::file_logger(clock_type::time_point start, stdfs::path path, severity log_level) noexcept
        : logger { std::move(start), log_level }, m_base_path { std::move(path) } {
        if (not stdfs::exists(m_base_path)) stdfs::create_directory(m_base_path);

        expects(stdfs::is_directory(m_base_path), "path need to be a directory");

        auto filepath                = m_base_path / to_native_encoding(LOG_FILE_NAME);
        m_streams[filepath.string()] = std::ofstream { filepath.string() };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto file_logger::flush() noexcept -> void {
        for (auto& [path, stream] : m_streams) stream.flush();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto file_logger::write(severity severity, const module& m, std::string_view str) noexcept -> void {
        const auto now  = clock_type::now();
        const auto time = std::chrono::duration_cast<std::chrono::seconds>(now - m_start_time).count();

        auto filepath = m_base_path / stdfs::path { to_native_encoding(LOG_FILE_NAME) };
        if (not std::empty(m.name)) {
            filepath = m_base_path / to_native_encoding(m.name);
            filepath += to_native_encoding("-") + to_native_encoding(LOG_FILE_NAME);

            if (m_streams.find(filepath.string()) == m_streams.cend())
                m_streams[filepath.string()] = std::ofstream { filepath.string() };
        }

        static constexpr auto LOG_LINE        = "[{}, {}] {}\n"sv;
        static constexpr auto LOG_LINE_MODULE = "[{}, {}, {}] {}\n"sv;

        auto       final_string = string {};
        const auto severity_str = replace(as<string_view>(severity), "severity::", "");
        if (std::empty(m.name)) final_string = std::format(LOG_LINE, severity_str, time, str);
        else
            final_string = std::format(LOG_LINE_MODULE, severity_str, time, m.name, str);

        m_streams.at(filepath.string()) << final_string << std::flush;
    }
} // namespace stormkit::log
