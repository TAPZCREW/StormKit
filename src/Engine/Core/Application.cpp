// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module stormkit.Engine;

import std;

import :Core;

import stormkit.core;
import stormkit.wsi;
import stormkit.entities;

namespace stormkit::engine {
    Application::Application(std::string_view     application_name,
                             const math::ExtentU& window_extent,
                             std::string          window_title,
                             PrivateTag) {
        m_window = wsi::Window { std::move(window_title), window_extent, wsi::WindowStyle::CLOSE };
        m_event_handler = wsi::EventHandler {};

        m_renderer = Renderer::create(application_name, as_ref(m_window))
                         .transform_error(monadic::assert("Failed to initialize renderer"))
                         .value();

        m_world = entities::EntityManager {};
    }

    auto Application::run() -> void {
        auto framegraph_mutex = std::mutex {};
        auto rebuild_graph    = std::atomic_bool { true };

        m_renderer->startRendering(framegraph_mutex, rebuild_graph);
        while (m_window->is_open()) {
            m_renderer->updateFrameGraph(framegraph_mutex, rebuild_graph, m_update_framegraph);
            m_event_handler->update(m_window);
            m_world->step(Secondf { 0 });
            // if (m_surf0ace->needRecreate()) {
            // m_surface->recreate();
            // do_initPerFrameObjects();
            //}
        }
    }
} // namespace stormkit::engine
