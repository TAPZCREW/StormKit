// Copyright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#ifdef STORMKIT_OS_WINDOWS
    #include <stormkit/core/platform/windows.hpp>
#endif

export module stormkit.test;

import stormkit.core;
import std;

using namespace stormkit;

export namespace test {
    struct test_func {
        string                name;
        std::function<void()> func;
    };

    struct test_suite_holder {
        auto                 has_test(string_view name) noexcept;
        auto                 run_test(string_view name) noexcept;
        auto                 run_tests() noexcept;
        string               name;
        dynarray<test_func>  tests;
        std::source_location location;
    };

    struct test_suite {
        test_suite(string&&              name,
                   dynarray<test_func>&& tests,
                   source_location_arg   location = std::source_location::current()) noexcept;
    };

    auto expects(bool cond, string_view message, source_location_arg location = std::source_location::current()) noexcept;

    auto parse_args(array_view<const string_view> args) noexcept -> void;
    auto run_tests() noexcept -> int;
} // namespace test

// module :private;

using namespace std::literals;

namespace test {
    enum class status {
        PASSED,
        NOT_PASSED,
        CHECK_MARK,
        CROSS_MARK,
    };

    struct test_state {
        dynarray<std::unique_ptr<test_suite_holder>> test_suites;
        bool                                         verbose        = false;
        bool                                         failed         = false;
        bool                                         plain          = false;
        std::optional<string>                        requested_test = std::nullopt;
    };

    namespace {
        constexpr auto STYLE_MAP = make_static_hash_map<status, console_style>({
          { status::PASSED,     console_style { .fg = console_color::BLACK, .bg = console_color::GREEN } },
          { status::NOT_PASSED, console_style { .fg = console_color::BLACK, .bg = console_color::RED }   },
          { status::CHECK_MARK,
           console_style {
              .fg = console_color::GREEN,
            }                                                                                            },
          { status::CROSS_MARK,
           console_style {
              .fg = console_color::RED,
            }                                                                                            },
        });

        constexpr auto PASSED     = STYLE_MAP.at(status::PASSED) | "Passed"sv;
        constexpr auto NOT_PASSED = STYLE_MAP.at(status::NOT_PASSED) | "Not Passed"sv;
        constexpr auto CHECK_MARK = STYLE_MAP.at(status::CHECK_MARK) | "✔ "sv;
        constexpr auto CROSS_MARK = STYLE_MAP.at(status::CROSS_MARK) | "❌"sv;
    } // namespace

    auto state = test_state {};

    auto test_suite_holder::has_test(string_view _name) noexcept {
        for (const auto& test : tests) {
            if (test.name == _name) return true;
        }

        return false;
    }

    auto test_suite_holder::run_test(string_view _name) noexcept {
        for (const auto& test : tests) {
            if (test.name == _name) {
                if (state.verbose) std::println("     running test {}", test.name);
                test.func();
                std::println("     {}", (state.failed) ? NOT_PASSED : PASSED);
                return not state.failed;
            }
        }
        return true;
    }

    auto test_suite_holder::run_tests() noexcept {
        if (state.verbose) { std::println("   > file: {}", location.file_name()); }

        auto passed_tests = 0;
        auto failed_tests = 0;

        for (const auto& test : tests) {
            state.failed = false;
            if (state.verbose) std::println("     running test {}", test.name);
            test.func();

            if (state.verbose) std::println("     {}", (state.failed) ? NOT_PASSED : PASSED);
            if (not state.failed) ++passed_tests;
            else
                ++failed_tests;
        }

        if (not state.plain) std::print("{}", CHECK_MARK);
        std::print("{} test PASSED", passed_tests);
        if (failed_tests > 0) {
            std::print(", ");
            if (not state.plain) std::print("{}", CROSS_MARK);
            std::print("{} test state.failed", failed_tests);
        }
        std::println("");

        return failed_tests == 0;
    }

    test_suite::test_suite(string&& _name, dynarray<test_func>&& tests, source_location_arg location) noexcept {
        state.test_suites.emplace_back(std::make_unique<test_suite_holder>(std::move(_name), std::move(tests), location));
    }

    auto expects(bool cond, string_view message, source_location_arg location) noexcept {
        if (not cond) [[unlikely]] {
            state.failed = true;
            if (state.verbose) {
                std::println("{} on expression \"{}\" \n"
                             "          > line: {}",
                             STYLE_MAP.at(status::NOT_PASSED) | "FAILURE",
                             message,
                             location.line());
            }
        }
    }

    auto split(string_view str, char delim) noexcept -> dynarray<string> {
        auto output = dynarray<string> {};
        auto first  = std::size_t { 0u };

        while (first < str.size()) {
            const auto second = str.find_first_of(delim, first);

            if (first != second) output.emplace_back(str.substr(first, second - first));

            if (second == string_view::npos) break;

            first = second + 1;
        }

        return output;
    }

    auto parse_args(array_view<const string_view> args) noexcept -> void {
        for (const auto& arg : args) {
            if (arg == "--verbose" or arg == "-v") state.verbose = true;
            else if (arg == "--plain" or arg == "-p")
                state.plain = true;
            else if (arg.starts_with("--test_name=")) { state.requested_test = split(arg, '=')[1]; }
        }
    }

    auto run_tests() noexcept -> int {
        auto return_code = 0;

        if (state.requested_test) {
            for (const auto& suite : state.test_suites) {
                if (suite->has_test(*state.requested_test)) {
                    if (not suite->run_test(*state.requested_test)) return_code = -1;
                    break;
                }
            }
        } else
            for (const auto& suite : state.test_suites) {
                std::println("Running test suite {} ({} tests)", suite->name, std::size(suite->tests));
                if (not suite->run_tests()) return_code = -1;
            }

        return return_code;
    }
} // namespace test
