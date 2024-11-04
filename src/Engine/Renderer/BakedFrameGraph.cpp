// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module stormkit.Engine;

import std;

import stormkit.Core;
import stormkit.Gpu;

import :Renderer.FrameGraph;
import :Renderer.RenderSurface;

namespace stormkit::engine {
    /////////////////////////////////////
    /////////////////////////////////////
    BakedFrameGraph::BakedFrameGraph(const gpu::Image&                 backbuffer,
                                     Data&&                            data,
                                     [[maybe_unused]] BakedFrameGraph* old)
        : m_backbuffer { borrow(backbuffer) }, m_data { std::move(data) } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto BakedFrameGraph::execute(const gpu::Queue& queue) noexcept
        -> gpu::Expected<Ref<const gpu::Semaphore>> {
        return m_data.fence->wait().transform([&](auto&&) noexcept {
            m_data.fence->reset();

            auto signal = borrows<std::array>(std::as_const(m_data.semaphore));
            m_data.cmb->submit(queue, {}, {}, signal, borrow(m_data.fence));

            return borrow(std::as_const(*m_data.semaphore));
        });
    }
} // namespace stormkit::engine
