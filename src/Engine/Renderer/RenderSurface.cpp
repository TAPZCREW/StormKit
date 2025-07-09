module stormkit.Engine;

import std;

import stormkit.core;
import stormkit.log;
import stormkit.wsi;
import stormkit.gpu;

import :Renderer;

using namespace std::chrono_literals;

namespace stormkit::engine {
    /////////////////////////////////////
    /////////////////////////////////////
    RenderSurface::RenderSurface(const gpu::Instance& instance,
                                 const gpu::Device&   device,
                                 const gpu::Queue&    raster_queue,
                                 const wsi::Window&   window,
                                 Tag) {
        gpu::Surface::create_from_window(instance, window)
            .transform(monadic::set(m_surface))
            .and_then(bind_front(gpu::Swapchain::create,
                                std::cref(device),
                                std::cref(*m_surface),
                                std::cref(window.extent()),
                                std::nullopt))
            .transform(monadic::set(m_swapchain))
            .transform_error(monadic::throw_as_exception());

        for (auto _ : range(std::size(m_swapchain->images()))) {
            gpu::Semaphore::create(device)
                .transform(monadic::emplace_to(m_image_availables))
                .and_then(bind_front(gpu::Semaphore::create, std::cref(device)))
                .transform(monadic::emplace_to(m_render_finisheds))
                .and_then(bind_front(gpu::Fence::create_signaled, std::cref(device)))
                .transform(monadic::emplace_to(m_in_flight_fences))
                .transform_error(
                    monadic::map(monadic::narrow<gpu::Result>(), monadic::throw_as_exception()));
        }

        const auto command_pool
            = gpu::CommandPool::create(device)
                  .transform_error(monadic::assert("Failed to raster Queue command pool"))
                  .value();

        auto transition_command_buffers
            = command_pool.create_command_buffers(device, std::size(m_swapchain->images()));

        for (auto i : range(std::size(transition_command_buffers))) {
            auto&& image                     = m_swapchain->images()[i];
            auto&& transition_command_buffer = transition_command_buffers[i];

            transition_command_buffer.begin(true);
            transition_command_buffer.transition_image_layout(image,
                                                            gpu::ImageLayout::UNDEFINED,
                                                            gpu::ImageLayout::Present_Src);
            transition_command_buffer.end();
        }

        auto fence = gpu::Fence::create(device)
                         .transform_error(monadic::map(monadic::narrow<gpu::Result>(),
                                                       monadic::throw_as_exception()))
                         .value();

        auto cmbs = to_refs(transition_command_buffers);
        raster_queue.submit({ .command_buffers = cmbs }, fence);

        fence.wait().transform_error(
            monadic::map(monadic::narrow<gpu::Result>(), monadic::throw_as_exception()));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto RenderSurface::beginFrame(const gpu::Device& device) -> gpu::Expected<Frame> {
        expects(m_surface.initialized());
        expects(m_swapchain.initialized());

        const auto& image_available = m_image_availables[m_current_frame];
        const auto& render_finished = m_render_finisheds[m_current_frame];
        auto&       in_flight       = m_in_flight_fences[m_current_frame];

        return in_flight.wait()
            .transform([&in_flight](auto&& _) noexcept { in_flight.reset(); })
            .and_then(bind_front(&gpu::Swapchain::acquire_next_image,
                                &(m_swapchain.get()),
                                100ms,
                                std::cref(image_available)))
            .transform([&, this](auto&& _result) noexcept {
                auto&& [result, image_index] = _result; // TODO handle result
                return Frame { .current_frame   = as<u32>(m_current_frame),
                               .image_index     = image_index,
                               .image_available = image_available,
                               .render_finished = render_finished,
                               .in_flight       = in_flight };
            });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto RenderSurface::presentFrame(const gpu::Queue& queue, const Frame& frame)
        -> gpu::Expected<void> {
        const auto image_indices   = std::array { frame.image_index };
        const auto wait_semaphores = as_refs<std::array>(*frame.render_finished);
        const auto swapchains      = as_refs<std::array>(*m_swapchain);

        return queue.present(swapchains, wait_semaphores, image_indices)
            .transform([this](auto&& _) noexcept {
                if (++m_current_frame >= bufferingCount()) m_current_frame = 0;
            });
    }
} // namespace stormkit::engine
