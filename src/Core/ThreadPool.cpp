// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module stormkit.Core;

import std;

namespace stormkit {
    /////////////////////////////////////
    /////////////////////////////////////
    ThreadPool::ThreadPool(ThreadPool&& other) noexcept {
        auto lock = std::scoped_lock { other.m_mutex };

        m_worker_count = std::exchange(other.m_worker_count, 0u);
        m_tasks        = std::move(other.m_tasks);

        m_workers.reserve(m_worker_count);
        for (const auto i : range(m_worker_count)) {
            auto& thread = m_workers.emplace_back([this] { workerMain(); });
            setThreadName(thread, std::format("StormKit:WorkerThread:{}", i));
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto ThreadPool::operator=(ThreadPool&& other) noexcept -> ThreadPool& {
        if (&other == this) [[unlikely]]
            return *this;

        auto lock1 = std::unique_lock { other.m_mutex, std::defer_lock };
        auto lock2 = std::unique_lock { other.m_mutex, std::defer_lock };
        std::lock(lock1, lock2);

        joinAll();

        m_worker_count = std::exchange(other.m_worker_count, 0u);
        m_tasks        = std::move(other.m_tasks);

        m_workers.reserve(m_worker_count);
        for (const auto i : range(m_worker_count)) {
            auto& thread = m_workers.emplace_back([this] { workerMain(); });
            setThreadName(thread, std::format("StormKit:WorkerThread:{}", i));
        }

        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto ThreadPool::joinAll() -> void {
        for ([[maybe_unused]] const auto i : range(m_worker_count))
            postTask<void>(Task::Type::Terminate, [] {}, ThreadPool::NoFuture);

        for (auto& thread : m_workers)
            if (thread.joinable()) thread.join();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto ThreadPool::workerMain() noexcept -> void {
        for (;;) {
            auto task = Task {};

            {
                auto lock = std::unique_lock { m_mutex };
                if (std::empty(m_tasks))
                    m_work_signal.wait(lock, [this] { return not std::empty(m_tasks); });

                task = std::move(m_tasks.front());
                m_tasks.pop();
            }

            task.work();

            if (task.type == Task::Type::Terminate) return;
        }
    }
} // namespace stormkit
