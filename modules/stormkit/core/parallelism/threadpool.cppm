// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/api.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.parallelism.threadpool;

import std;

import stormkit.core.meta;
import stormkit.core.functional;
import stormkit.core.function_ref;
import stormkit.core.ranges;
import stormkit.core.types;
import stormkit.core.typesafe;
import stormkit.core.parallelism.threadutils;

export namespace stormkit { inline namespace core {
    class STORMKIT_CORE_API thread_pool {
      public:
        static constexpr struct no_future_type {
        } NO_FUTURE = {};

        template<typename T>
        using closure_type = std23::move_only_function<T()>;

        explicit thread_pool(u32 worker_count = std::thread::hardware_concurrency() / 2);
        ~thread_pool();

        thread_pool(const thread_pool&)                    = delete;
        auto operator=(const thread_pool&) -> thread_pool& = delete;

        thread_pool(thread_pool&&) noexcept;
        auto operator=(thread_pool&&) noexcept -> thread_pool&;

        [[nodiscard]]
        auto worker_count() const noexcept -> u32;

        template<typename T>
        [[nodiscard]]
        auto post_task(closure_type<T>&& task) noexcept -> decltype(auto);

        template<typename T>
        auto post_task(closure_type<T>&& task, no_future_type) noexcept -> void;

        auto join_all() noexcept -> void;

        auto wait_idle(bool cancel_tasks = false) noexcept -> void;

        auto set_name(string_view name) noexcept -> void;

      private:
        enum class task_type {
            STANDARD,
            TERMINATE,
        };

        struct task {
            task() = default;

            inline task(task_type _type, std23::move_only_function<void()>&& _work) : type { _type }, work { std::move(_work) } {}

            task(task&&) noexcept                    = default;
            auto operator=(task&&) noexcept -> task& = default;

            task_type                         type;
            std23::move_only_function<void()> work;
        };

        template<typename T>
        auto post_task(task_type type, closure_type<T>&& task) noexcept -> decltype(auto);

        template<typename T>
        auto post_task(task_type type, closure_type<T>&& task, no_future_type) noexcept -> void;

        auto worker_main(i32 id) noexcept -> void;

        auto spawn_workers() noexcept -> void;

        u32 m_worker_count = 0;

        dynarray<std::jthread> m_workers;

        mutable std::mutex          m_mutex;
        std::condition_variable     m_work_signal;
        std::counting_semaphore<64> m_running_task_counter;
        std::queue<task>            m_tasks;
    };

    template<meta::plain::apply_to<std::ranges::input_range>                     Range,
             std::invocable<meta::range_value_type<meta::to_plain_type<Range>>&> F>
    auto parallel_for(thread_pool& pool, Range&& range, F&& f) noexcept -> void;

    template<meta::plain::apply_to<std::ranges::input_range>                     Range,
             std::invocable<meta::range_value_type<meta::to_plain_type<Range>>&> F>
    auto parallel_for_async(thread_pool& pool, Range& range, F&& f) noexcept -> dynarray<std::future<void>>;
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stdr = std::ranges;
namespace stdv = std::views;

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline thread_pool::thread_pool(u32 worker_count)
        : m_worker_count { worker_count }, m_running_task_counter { worker_count } {
        spawn_workers();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline thread_pool::~thread_pool() {
        wait_idle();
        join_all();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto thread_pool::worker_count() const noexcept -> u32 {
        return m_worker_count;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto thread_pool::set_name(string_view name) noexcept -> void {
        // for (const auto& [i, worker] : stdv::enumerate(m_workers))
        for (auto i : range(m_worker_count)) set_thread_name(m_workers[i], std::format("{}:{}", name, i));
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline auto thread_pool::post_task(closure_type<T>&& task) noexcept -> decltype(auto) {
        return post_task<T>(task_type::STANDARD, std::move(task));
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline auto thread_pool::post_task(closure_type<T>&& task, no_future_type t) noexcept -> void {
        post_task<T>(task_type::STANDARD, std::move(task), t);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    inline auto thread_pool::post_task(task_type type, closure_type<T>&& closure) noexcept -> decltype(auto) {
        auto packaged_task = std::make_shared<std::packaged_task<T()>>(std::move(closure));

        auto future = packaged_task->get_future();

        auto task_ = task { type, [task = std::move(packaged_task)]() noexcept { (*task)(); } };

        {
            auto _ = std::unique_lock { m_mutex };

            m_tasks.emplace(std::move(task_));
        }

        m_work_signal.notify_one();

        return future;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    inline auto thread_pool::post_task(task_type type, closure_type<T>&& closure, no_future_type) noexcept -> void {
        auto task_ = task { type, [task = std::move(closure)]() mutable noexcept { task(); } };

        {
            auto lock = std::unique_lock { m_mutex };

            m_tasks.emplace(std::move(task_));
        }

        m_work_signal.notify_one();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::plain::apply_to<std::ranges::input_range>                     Range,
             std::invocable<meta::range_value_type<meta::to_plain_type<Range>>&> F>
    inline auto parallel_for(thread_pool& pool, Range&& range, F&& f) noexcept -> void {
        auto futures = parallel_for_async(pool, range, std::forward<F>(f));
        wait_all(futures);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::plain::apply_to<std::ranges::input_range>                     Range,
             std::invocable<meta::range_value_type<meta::to_plain_type<Range>>&> F>
    inline auto parallel_for_async(thread_pool& pool, Range& range, F&& f) noexcept -> dynarray<std::future<void>> {
        const auto size        = stdr::size(range);
        const auto chunk_size  = size / pool.worker_count();
        const auto chunk_count = size / chunk_size;

        auto out = dynarray<std::future<void>> {};
        out.reserve(chunk_count);

        for (auto chunk : stormkit::range(chunk_count)) {
            const auto start = chunk * chunk_size;
            auto       end   = (chunk + 1u) * chunk_size;
            if (end >= (chunk_count * size)) end = size;

            out.emplace_back(pool.post_task<void>([&f, &range, start, end] mutable noexcept {
                auto it = std::begin(range) + as<ioffset>(start);
                for (auto _ : stormkit::range(start, end)) {
                    f(*it);
                    ++it;
                }
            }));
        }

        return out;
    }
}} // namespace stormkit::core
