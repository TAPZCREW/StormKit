// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module stormkit.log;

import std;

import stormkit.core;

using namespace std::literals;

namespace stdfs = std::filesystem;

namespace {
    constexpr auto log_filename = "log.txt";
}

namespace stormkit::log {
    ////////////////////////////////////////
    ////////////////////////////////////////
    file_logger::file_logger(clock_type::time_point start, stdfs::path path) noexcept
        : logger { std::move(start) }, base_path_ { std::move(path) } {
        if (not stdfs::exists(base_path_)) stdfs::create_directory(base_path_);

        expects(stdfs::is_directory(base_path_), "path need to be a directory");

        auto filepath               = base_path_ / to_native_encoding(log_filename);
        streams_[filepath.string()] = std::ofstream { filepath.string() };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    file_logger::file_logger(clock_type::time_point start, stdfs::path path, severity log_level) noexcept
        : logger { std::move(start), log_level }, base_path_ { std::move(path) } {
        if (not stdfs::exists(base_path_)) stdfs::create_directory(base_path_);

        expects(stdfs::is_directory(base_path_), "path need to be a directory");

        auto filepath               = base_path_ / to_native_encoding(log_filename);
        streams_[filepath.string()] = std::ofstream { filepath.string() };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto file_logger::flush() noexcept -> void {
        for (auto& [path, stream] : streams_) stream.flush();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto file_logger::write(severity severity, const module& m, std::string_view str) noexcept -> void {
        const auto now  = clock_type::now();
        const auto time = std::chrono::duration_cast<std::chrono::seconds>(now - start_time_).count();

        auto filepath = base_path_ / stdfs::path { to_native_encoding(log_filename) };
        if (not std::empty(m.name)) {
            filepath = base_path_ / to_native_encoding(m.name);
            filepath += to_native_encoding("-") + to_native_encoding(log_filename);

            if (streams_.find(filepath.string()) == streams_.cend())
                streams_[filepath.string()] = std::ofstream { filepath.string() };
        }

        static constexpr auto format_string             = "[{}, {}] {}\n"sv;
        static constexpr auto format_string_with_module = "[{}, {}, {}] {}\n"sv;

        auto       final_string = string {};
        const auto severity_str = replace(as<string_view>(severity), "severity::", "");
        if (std::empty(m.name)) final_string = std::format(format_string, severity_str, time, str);
        else
            final_string = std::format(format_string_with_module, severity_str, time, m.name, str);

        streams_.at(filepath.string()) << final_string << std::flush;
    }
} // namespace stormkit::log
