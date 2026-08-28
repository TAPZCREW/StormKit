// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>

#include <stormkit/log/api.hpp>

export module stormkit.log;

import std;

import stormkit.core;

namespace stdr  = std::ranges;
namespace stdfs = std::filesystem;

export {
    namespace stormkit::log {
        struct module;
        enum class severity : u8 {
            INFO    = 1 << 0,
            WARNING = 1 << 1,
            ERROR   = 1 << 2,
            FATAL   = 1 << 3,
            DEBUG   = 1 << 4,
        };

        [[nodiscard]]
        constexpr auto tag_invoke(as_fn<string_view>,
                                  severity value,
                                  source_location_arg = std::source_location::current()) noexcept -> string_view;
    } // namespace stormkit::log

    template<>
    inline constexpr auto stormkit::core::meta::FLAG_TRAIT<stormkit::log::severity> = true;

    template<>
    inline constexpr auto stormkit::core::meta::ENABLE_AS_STRING_AS_FORMATTER<stormkit::log::severity> = true;

    namespace stormkit::log {
        STORMKIT_LOG_API
        auto parse_args(array_view<const string_view> args) noexcept -> void;

        class STORMKIT_LOG_API logger {
          public:
            using clock_type = std::chrono::high_resolution_clock;

            explicit logger(clock_type::time_point start) noexcept;
            logger(clock_type::time_point start, severity mask) noexcept;
            virtual ~logger() noexcept;

            virtual auto write(severity severity, const module& module, string_view string) noexcept -> void = 0;
            virtual auto flush() noexcept -> void                                                            = 0;

            auto set_severity_mask(severity mask) noexcept -> void;

            [[nodiscard]]
            auto start_time() const noexcept -> const clock_type::time_point&;
            [[nodiscard]]
            auto severity_mask() const noexcept -> severity;

            [[nodiscard]]
            auto mutex() noexcept -> std::mutex&;

            template<typename T, typename... Ts>
            [[nodiscard]]
            static auto create_logger_instance(Ts&&... param_args) noexcept -> T;

            template<typename T, typename... Ts>
            [[nodiscard]]
            static auto allocate_logger_instance(Ts&&... param_args) noexcept -> heap_ptr<T>;

            template<class... Ts>
            static auto log(severity                  severity,
                            const module&             module,
                            std::format_string<Ts...> format_string,
                            Ts&&... args) noexcept -> void;

            template<class... Ts>
            static auto log(severity severity, std::format_string<Ts...> format_string, Ts&&... args) noexcept -> void;

            template<class... Ts>
            static auto log_runtime(severity severity, const module& module, string_view format_string, Ts&&... args) noexcept
              -> void;

            template<class... Ts>
            static auto log_runtime(severity severity, string_view format_string, Ts&&... args) noexcept -> void;

            template<class... Ts>
            static auto dlog(std::format_string<Ts...> format_string, Ts&&... args) noexcept -> void;
            template<class... Ts>
            static auto ilog(std::format_string<Ts...> format_string, Ts&&... args) noexcept -> void;
            template<class... Ts>
            static auto wlog(std::format_string<Ts...> format_string, Ts&&... args) noexcept -> void;
            template<class... Ts>
            static auto elog(std::format_string<Ts...> format_string, Ts&&... args) noexcept -> void;
            template<class... Ts>
            static auto flog(std::format_string<Ts...> format_string, Ts&&... args) noexcept -> void;

            template<class... Ts>
            static auto dlog(const module& module, std::format_string<Ts...> format_string, Ts&&... args) noexcept -> void;
            template<class... Ts>
            static auto ilog(const module& module, std::format_string<Ts...> format_string, Ts&&... args) noexcept -> void;
            template<class... Ts>
            static auto wlog(const module& module, std::format_string<Ts...> format_string, Ts&&... args) noexcept -> void;
            template<class... Ts>
            static auto elog(const module& module, std::format_string<Ts...> format_string, Ts&&... args) noexcept -> void;
            template<class... Ts>
            static auto flog(const module& module, std::format_string<Ts...> format_string, Ts&&... args) noexcept -> void;

            template<class... Ts>
            static auto dlog_runtime(string_view format_string, Ts&&... args) noexcept -> void;
            template<class... Ts>
            static auto ilog_runtime(string_view format_string, Ts&&... args) noexcept -> void;
            template<class... Ts>
            static auto wlog_runtime(string_view format_string, Ts&&... args) noexcept -> void;
            template<class... Ts>
            static auto elog_runtime(string_view format_string, Ts&&... args) noexcept -> void;
            template<class... Ts>
            static auto flog_runtime(string_view format_string, Ts&&... args) noexcept -> void;

            template<class... Ts>
            static auto dlog_runtime(const module& module, string_view format_string, Ts&&... args) noexcept -> void;
            template<class... Ts>
            static auto ilog_runtime(const module& module, string_view format_string, Ts&&... args) noexcept -> void;
            template<class... Ts>
            static auto wlog_runtime(const module& module, string_view format_string, Ts&&... args) noexcept -> void;
            template<class... Ts>
            static auto elog_runtime(const module& module, string_view format_string, Ts&&... args) noexcept -> void;
            template<class... Ts>
            static auto flog_runtime(const module& module, string_view format_string, Ts&&... args) noexcept -> void;

            [[nodiscard]]
            static auto has_logger() noexcept -> bool;
            [[nodiscard]]
            static auto instance() noexcept -> logger&;

          protected:
            clock_type::time_point m_start_time;
            severity               m_severity_mask;

            std::mutex m_mutex;
        };

        struct module {
            template<class... Ts>
            auto dlog(std::format_string<Ts...> format_string, Ts&&... args) const noexcept -> void;
            template<class... Ts>
            auto ilog(std::format_string<Ts...> format_string, Ts&&... args) const noexcept -> void;
            template<class... Ts>
            auto wlog(std::format_string<Ts...> format_string, Ts&&... args) const noexcept -> void;
            template<class... Ts>
            auto elog(std::format_string<Ts...> format_string, Ts&&... args) const noexcept -> void;
            template<class... Ts>
            auto flog(std::format_string<Ts...> format_string, Ts&&... args) const noexcept -> void;

            template<class... Ts>
            auto dlog_runtime(string_view format_string, Ts&&... args) const noexcept -> void;
            template<class... Ts>
            auto ilog_runtime(string_view format_string, Ts&&... args) const noexcept -> void;
            template<class... Ts>
            auto wlog_runtime(string_view format_string, Ts&&... args) const noexcept -> void;
            template<class... Ts>
            auto elog_runtime(string_view format_string, Ts&&... args) const noexcept -> void;
            template<class... Ts>
            auto flog_runtime(string_view format_string, Ts&&... args) const noexcept -> void;

            auto flush() const noexcept -> void;

            string_view name = "";
        };

        template<static_string str>
        [[nodiscard]]
        constexpr auto operator""_module() noexcept -> stormkit::log::module;

        class STORMKIT_LOG_API file_logger final: public logger {
          public:
            file_logger(clock_type::time_point start, stdfs::path path) noexcept;
            file_logger(clock_type::time_point start, stdfs::path path, severity mask) noexcept;
            ~file_logger() noexcept override;

            file_logger(const file_logger&) noexcept                    = delete;
            auto operator=(const file_logger&) noexcept -> file_logger& = delete;

            file_logger(file_logger&&) noexcept                    = delete;
            auto operator=(file_logger&&) noexcept -> file_logger& = delete;

            auto write(severity severity, const module& module, string_view string) noexcept -> void override;
            auto flush() noexcept -> void override;

          private:
            string_hash_map<std::ofstream> m_streams;

            stdfs::path m_base_path;
        };

        class STORMKIT_LOG_API console_logger final: public logger {
          public:
            explicit console_logger(clock_type::time_point start) noexcept;
            console_logger(clock_type::time_point start, severity mask) noexcept;

            console_logger(const console_logger&) noexcept                    = delete;
            auto operator=(const console_logger&) noexcept -> console_logger& = delete;

            console_logger(console_logger&&) noexcept                    = delete;
            auto operator=(console_logger&&) noexcept -> console_logger& = delete;

            ~console_logger() noexcept override;

            auto write(severity severity, const module& module, string_view string) noexcept -> void override;
            auto flush() noexcept -> void override;
        };
    } // namespace stormkit::log
}

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

using namespace std::literals;

namespace stormkit::log {
    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto tag_invoke(as_fn<string_view>, severity severity, source_location_arg) noexcept -> string_view {
        switch (severity) {
            case severity::INFO: return "INFO";
            case severity::WARNING: return "WARNING";
            case severity::ERROR: return "ERROR";
            case severity::FATAL: return "FATAL";
            case severity::DEBUG: return "DEBUG";
            default: break;
        }

        std::unreachable();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto logger::set_severity_mask(severity mask) noexcept -> void {
        m_severity_mask = mask;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto logger::start_time() const noexcept -> const clock_type::time_point& {
        return m_start_time;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto logger::severity_mask() const noexcept -> severity {
        return m_severity_mask;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto logger::mutex() noexcept -> std::mutex& {
        return m_mutex;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, typename... Ts>
    inline auto logger::create_logger_instance(Ts&&... param_args) noexcept -> T {
        static_assert(std::is_base_of<logger, T>::value, "T must inherit logger");

        auto time_point = clock_type::now();

        return T { std::move(time_point), std::forward<Ts>(param_args)... };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T, typename... Ts>
    inline auto logger::allocate_logger_instance(Ts&&... param_args) noexcept -> heap_ptr<T> {
        static_assert(std::is_base_of<logger, T>::value, "T must inherit logger");

        auto time_point = clock_type::now();

        return *allocate<T>(std::move(time_point), std::forward<Ts>(param_args)...)
                  .transform_error(core::monadic::assert("Failed to allocate logger instance"));
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    inline auto logger::log(severity                  severity,
                            const module&             m,
                            std::format_string<Ts...> format_string,
                            Ts&&... param_args) noexcept -> void {
        EXPECTS(has_logger());

        const auto mask = instance().severity_mask();
        if (not has_flag_bit(mask, severity)) return;

        auto size = std::formatted_size(format_string, std::forward<Ts>(param_args)...);

        if (size <= 64) {
            thread_local auto memory_buffer = array<char, 64> {};
            const auto        end_it        = std::format_to(stdr::begin(memory_buffer),
                                                             std::move(format_string),
                                                             std::forward<Ts>(param_args)...);

            const auto _ = std::unique_lock(instance().mutex());
            instance().write(severity, m, string_view { stdr::begin(memory_buffer), end_it });
        } else {
            auto memory_buffer = dynarray<char> {};
            memory_buffer.resize(size);
            const auto end_it = std::format_to(stdr::begin(memory_buffer),
                                               std::move(format_string),
                                               std::forward<Ts>(param_args)...);

            const auto _ = std::unique_lock(instance().mutex());
            instance().write(severity, m, string_view { stdr::begin(memory_buffer), end_it });
        }
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto logger::log(severity severity, std::format_string<Ts...> format_string, Ts&&... param_args) noexcept -> void {
        log(severity, module {}, std::move(format_string), std::forward<Ts>(param_args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    inline auto logger::log_runtime(severity severity, const module& m, string_view format_string, Ts&&... param_args) noexcept
      -> void {
        EXPECTS(has_logger());

        struct counter {
            usize n { 0 };
            using difference_type = long;

            auto operator*() const noexcept { return std::ignore; }

            auto operator++() noexcept -> counter& {
                ++n;
                return *this;
            }

            auto operator++(int) noexcept -> counter { return counter { n++ }; }
        };

        const auto mask = instance().severity_mask();
        if (not has_flag_bit(mask, severity)) return;

        auto args = std::format_args { std::forward<Ts>(param_args)... };

        auto c = counter {};
        c      = std::vformat_to(c, format_string, args);

        if (c.n <= 64) {
            thread_local auto memory_buffer = array<char, 64> {};
            const auto        end_it        = std::format_to(stdr::begin(memory_buffer),
                                                             std::move(format_string),
                                                             std::forward<Ts>(param_args)...);

            const auto _ = std::unique_lock(instance().mutex());
            instance().write(severity, m, string_view { stdr::begin(memory_buffer), end_it });
        } else {
            auto memory_buffer = dynarray<char> {};
            memory_buffer.resize(c.n);
            const auto end_it = std::vformat_to(stdr::begin(memory_buffer), std::move(format_string), args);

            const auto _ = std::unique_lock(instance().mutex());
            instance().write(severity, m, string_view { stdr::begin(memory_buffer), end_it });
        }
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto logger::log_runtime(severity severity, string_view format_string, Ts&&... param_args) noexcept -> void {
        log(severity, module {}, std::move(format_string), std::forward<Ts>(param_args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline console_logger::~console_logger() noexcept = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline file_logger::~file_logger() noexcept = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto logger::dlog(std::format_string<Ts...> format_string, Ts&&... args) noexcept -> void {
        log(severity::DEBUG, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto logger::ilog(std::format_string<Ts...> format_string, Ts&&... args) noexcept -> void {
        log(severity::INFO, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto logger::wlog(std::format_string<Ts...> format_string, Ts&&... args) noexcept -> void {
        log(severity::WARNING, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto logger::elog(std::format_string<Ts...> format_string, Ts&&... args) noexcept -> void {
        log(severity::ERROR, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto logger::flog(std::format_string<Ts...> format_string, Ts&&... args) noexcept -> void {
        log(severity::FATAL, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto logger::dlog(const module& module, std::format_string<Ts...> format_string, Ts&&... args) noexcept -> void {
        log(severity::DEBUG, module, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto logger::ilog(const module& module, std::format_string<Ts...> format_string, Ts&&... args) noexcept -> void {
        log(severity::INFO, module, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto logger::wlog(const module& module, std::format_string<Ts...> format_string, Ts&&... args) noexcept -> void {
        log(severity::WARNING, module, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto logger::elog(const module& module, std::format_string<Ts...> format_string, Ts&&... args) noexcept -> void {
        log(severity::ERROR, module, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto logger::flog(const module& module, std::format_string<Ts...> format_string, Ts&&... args) noexcept -> void {
        log(severity::FATAL, module, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto logger::dlog_runtime(string_view format_string, Ts&&... args) noexcept -> void {
        log_runtime(severity::DEBUG, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto logger::ilog_runtime(string_view format_string, Ts&&... args) noexcept -> void {
        log_runtime(severity::INFO, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto logger::wlog_runtime(string_view format_string, Ts&&... args) noexcept -> void {
        log_runtime(severity::WARNING, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto logger::elog_runtime(string_view format_string, Ts&&... args) noexcept -> void {
        log_runtime(severity::ERROR, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto logger::flog_runtime(string_view format_string, Ts&&... args) noexcept -> void {
        log_runtime(severity::FATAL, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto logger::dlog_runtime(const module& module, string_view format_string, Ts&&... args) noexcept -> void {
        log_runtime(severity::DEBUG, module, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto logger::ilog_runtime(const module& module, string_view format_string, Ts&&... args) noexcept -> void {
        log_runtime(severity::INFO, module, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto logger::wlog_runtime(const module& module, string_view format_string, Ts&&... args) noexcept -> void {
        log_runtime(severity::WARNING, module, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto logger::elog_runtime(const module& module, string_view format_string, Ts&&... args) noexcept -> void {
        log_runtime(severity::ERROR, module, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto logger::flog_runtime(const module& module, string_view format_string, Ts&&... args) noexcept -> void {
        log_runtime(severity::FATAL, module, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto module::dlog(std::format_string<Ts...> format_string, Ts&&... args) const noexcept -> void {
        logger::dlog(*this, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto module::ilog(std::format_string<Ts...> format_string, Ts&&... args) const noexcept -> void {
        logger::ilog(*this, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto module::wlog(std::format_string<Ts...> format_string, Ts&&... args) const noexcept -> void {
        logger::wlog(*this, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto module::elog(std::format_string<Ts...> format_string, Ts&&... args) const noexcept -> void {
        logger::elog(*this, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto module::flog(std::format_string<Ts...> format_string, Ts&&... args) const noexcept -> void {
        logger::flog(*this, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto module::dlog_runtime(string_view format_string, Ts&&... args) const noexcept -> void {
        logger::dlog_runtime(*this, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto module::ilog_runtime(string_view format_string, Ts&&... args) const noexcept -> void {
        logger::ilog_runtime(*this, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto module::wlog_runtime(string_view format_string, Ts&&... args) const noexcept -> void {
        logger::wlog_runtime(*this, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto module::elog_runtime(string_view format_string, Ts&&... args) const noexcept -> void {
        logger::elog_runtime(*this, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<class... Ts>
    STORMKIT_FORCE_INLINE
    inline auto module::flog_runtime(string_view format_string, Ts&&... args) const noexcept -> void {
        logger::flog_runtime(*this, std::move(format_string), std::forward<Ts>(args)...);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto module::flush() const noexcept -> void {
        logger::instance().flush();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<static_string str>
    STORMKIT_FORCE_INLINE
    constexpr auto operator""_module() noexcept -> stormkit::log::module {
        return module { str };
    }

    static_assert(meta::has_as_string<stormkit::log::severity> and meta::ENABLE_AS_STRING_AS_FORMATTER<stormkit::log::severity>);
} // namespace stormkit::log
