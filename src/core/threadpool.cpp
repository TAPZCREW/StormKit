// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module stormkit.core.parallelism.threadpool;

import std;

namespace stdr = std::ranges;

namespace stormkit {
    /////////////////////////////////////
    /////////////////////////////////////
    thread_pool::thread_pool(thread_pool&& other) noexcept
        : worker_count_ { other.worker_count_ }, running_task_counter_ { worker_count_ } {
        wait_idle();
        other.wait_idle();

        auto lock = std::scoped_lock { other.mutex_ };

        tasks_ = std::move(other.tasks_);

        workers_.clear();
        spawn_workers();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto thread_pool::operator=(thread_pool&& other) noexcept -> thread_pool& {
        if (&other == this) [[unlikely]]
            return *this;

        wait_idle();
        other.wait_idle();

        auto lock = std::scoped_lock { mutex_, other.mutex_ };
        join_all();

        worker_count_ = other.worker_count_;
        tasks_        = std::move(other.tasks_);

        workers_.clear();
        spawn_workers();

        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto thread_pool::join_all() noexcept -> void {
        for (const auto _ : range(worker_count_)) post_task<void>(task_type::terminate, [] {}, no_future);

        for (auto& thread : workers_)
            if (thread.joinable()) thread.join();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto thread_pool::wait_idle(bool cancel_tasks) noexcept -> void {
        if (cancel_tasks) {
            auto _ = std::unique_lock { mutex_ };
            while (not stdr::empty(tasks_)) tasks_.pop();
        }

        for (;;) {
            {
                auto _ = std::unique_lock { mutex_ };
                if (stdr::empty(tasks_)) break;
            }
            std::this_thread::yield();
        }

        auto count = worker_count();
        for (;;) {
            while (running_task_counter_.try_acquire()) --count;

            if (count == 0) break;

            std::this_thread::yield();
        }

        for (auto _ : range(worker_count())) running_task_counter_.release();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto thread_pool::worker_main(i32 id) noexcept -> void {
        set_current_thread_name(std::format("stormkit:worker_thread:{}", id));
        for (;;) {
            auto task_ = task {};

            {
                auto lock = std::unique_lock { mutex_ };
                work_signal_.wait(lock, [this] { return not std::empty(tasks_); });
                task_ = std::move(tasks_.front());
                tasks_.pop();
            }

            running_task_counter_.acquire();
            task_.work();
            running_task_counter_.release();

            if (task_.type == task_type::terminate) return;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto thread_pool::spawn_workers() noexcept -> void {
        workers_.reserve(worker_count_);

        for (auto i : range(worker_count_)) workers_.emplace_back(bind_front(&thread_pool::worker_main, this, i));
    }
} // namespace stormkit
