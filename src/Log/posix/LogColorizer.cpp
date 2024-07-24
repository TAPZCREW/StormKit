// Copyright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module stormkit.Log;

import std;

import stormkit.Core;

using std::literals::string_view_literals::operator""sv;

namespace stormkit::log::details {
    namespace {
        [[maybe_unused]] constexpr auto KNRM  = "\x1B[0m"sv;
        [[maybe_unused]] constexpr auto KRED  = "\x1B[31m"sv;
        [[maybe_unused]] constexpr auto KGRN  = "\x1B[32m"sv;
        [[maybe_unused]] constexpr auto KYEL  = "\x1B[33m"sv;
        [[maybe_unused]] constexpr auto KBLU  = "\x1B[34m"sv;
        [[maybe_unused]] constexpr auto KMAG  = "\x1B[35m"sv;
        [[maybe_unused]] constexpr auto KCYN  = "\x1B[36m"sv;
        [[maybe_unused]] constexpr auto KWHT  = "\x1B[37m"sv;
        [[maybe_unused]] constexpr auto KGRS  = "\033[1m"sv;
        [[maybe_unused]] constexpr auto KINV  = "\033[7m"sv;
        [[maybe_unused]] constexpr auto KBLCK = "\033[30m"sv;
    } // namespace

    /////////////////////////////////////
    /////////////////////////////////////
    auto colorifyBegin(Severity severity, bool to_stderr) noexcept -> void {
        auto output = (to_stderr) ? getSTDErr() : getSTDOut();
        switch (severity) {
            case Severity::Info: std::print(output, "{}{}{}", KBLCK, KINV, KGRN); break;
            case Severity::Warning: std::print(output, "{}{}{}", KBLCK, KINV, KMAG); break;
            case Severity::Error: std::print(output, "{}{}{}", KBLCK, KINV, KYEL); break;
            case Severity::Fatal: std::print(output, "{}{}{}", KBLCK, KINV, KRED); break;
            case Severity::Debug: std::print(output, "{}{}{}", KBLCK, KINV, KCYN); break;
            default: std::unreachable();
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto colorifyEnd(bool to_stderr) noexcept -> void {
        auto output = (to_stderr) ? getSTDErr() : getSTDOut();
        std::print(output, KNRM);
    }
} // namespace stormkit::log::details
