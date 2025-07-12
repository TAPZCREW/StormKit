module;

#include <stormkit/log/log_macro.hpp>

module stormkit.Engine;

import std;

import stormkit.core;
import stormkit.log;
import stormkit.wsi;
import stormkit.gpu;

import :Renderer;
import :Renderer.FrameGraph;

using namespace std::literals;

namespace stormkit::engine {
    LOGGER("stormkit.Renderer")

    namespace {
        constexpr auto RAYTRACING_EXTENSIONS
            = std::array { "VK_KHR_ray_tracing_pipeline"sv,  "VK_KHR_acceleration_structure"sv,
                           "VK_KHR_buffer_device_address"sv, "VK_KHR_deferred_host_operations"sv,
                           "VK_EXT_descriptor_indexing"sv,   "VK_KHR_spirv_1_4"sv,
                           "VK_KHR_shader_float_controls"sv };

        constexpr auto BASE_EXTENSIONS = std::array { "VK_KHR_maintenance3"sv };

        constexpr auto SWAPCHAIN_EXTENSIONS = std::array { "VK_KHR_swapchain"sv };

        /////////////////////////////////////
        /////////////////////////////////////
        auto scorePhysicalDevice(const gpu::PhysicalDevice& physical_device) -> u64 {
            const auto support_raytracing
                = physical_device.check_extension_support(RAYTRACING_EXTENSIONS);

            auto score = u64 { 0u };

            const auto& info         = physical_device.info();
            const auto& capabilities = physical_device.capabilities();

            if (info.type == gpu::PhysicalDeviceType::Discrete_GPU) score += 10000000u;
            else if (info.type == gpu::PhysicalDeviceType::Virtual_GPU)
                score += 5000000u;
            else if (info.type == gpu::PhysicalDeviceType::Integrated_GPU)
                score += 250000u;

            score += capabilities.limits.max_image_dimension_1D;
            score += capabilities.limits.max_image_dimension_2D;
            score += capabilities.limits.max_image_dimension_3D;
            score += capabilities.limits.max_image_dimension_cube;
            score += capabilities.limits.max_uniform_buffer_range;
            score += info.api_major_version * 10000000u;
            score += info.api_minor_version * 10000u;
            score += info.api_patch_version * 100u;

            if (support_raytracing) score += 10000000u;

            return score;
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto pickPhysicalDevice(std::span<const gpu::PhysicalDevice> physical_devices) noexcept
            -> std::optional<Ref<const gpu::PhysicalDevice>> {
            auto ranked_devices = std::multimap<u64, Ref<const gpu::PhysicalDevice>> {};

            for (const auto& physical_device : physical_devices) {
                if (not physical_device.check_extension_support(BASE_EXTENSIONS)) {
                    dlog("Base required extensions not supported for GPU {}", physical_device);
                    continue;
                }
                if (not physical_device.check_extension_support(SWAPCHAIN_EXTENSIONS)) {
                    dlog("Swapchain required extensions not supported for GPU {}", physical_device);
                    continue;
                }

                const auto& info = physical_device.info();

                dlog("Scoring for {}\n"
                     "    device id:      {:#06x}\n"
                     "    vendor name:    {}\n"
                     "    vendor id:      {}\n"
                     "    api version:    {}.{}.{}\n"
                     "    driver version: {}.{}.{}\n"
                     "    type:           {}",
                     info.device_name,
                     info.device_id,
                     info.vendor_name,
                     info.vendor_id,
                     info.api_major_version,
                     info.api_minor_version,
                     info.api_patch_version,
                     info.driver_major_version,
                     info.driver_minor_version,
                     info.driver_patch_version,
                     info.type);

                const auto score = scorePhysicalDevice(physical_device);

                dlog("Score is {}", score);

                ranked_devices.emplace(score, physical_device);
            }

            if (std::ranges::empty(ranked_devices)) return std::nullopt;

            return ranked_devices.rbegin()->second;
        }

        /////////////////////////////////////
        /////////////////////////////////////
        // auto
        //     chooseSwapExtent(const math::ExtentU&      extent,
        //                      const gpu::SurfaceCapabilities& capabilities) noexcept ->
        //                      gpu::ExtentU {
        //     constexpr static auto int_max = std::numeric_limits<u32>::max();
        //
        //     if (capabilities.currentExtent.width != int_max &&
        //         capabilities.currentExtent.height != int_max)
        //         return capabilities.currentExtent;
        //
        //     auto actual_extent   = extent;
        //     actual_extent.width  = math::clamp(actual_extent.width,
        //                                             capabilities.minImageExtent.width,
        //                                             capabilities.maxImageExtent.width);
        //     actual_extent.height = math::clamp(actual_extent.height,
        //                                              capabilities.minImageExtent.height,
        //                                              capabilities.maxImageExtent.height);
        //
        //     return actual_extent;
        // }
        //
        // /////////////////////////////////////
        // /////////////////////////////////////
        // auto chooseImageCount(const gpu::SurfaceCapabilities& capabilities) noexcept
        //     -> u32 {
        //     auto image_count = capabilities.minImageCount + 1;
        //     return math::clamp(image_count,
        //                              capabilities.minImageCount,
        //                              capabilities.maxImageCount);
        // }
    } // namespace

    /////////////////////////////////////
    /////////////////////////////////////
    auto Renderer::do_init(std::string_view                      application_name,
                          std::optional<Ref<const wsi::Window>> window) noexcept
        -> gpu::Expected<void> {
        ilog("Initializing Renderer");
        return do_init_instance(application_name)
            .and_then(bind_front(&Renderer::do_initDevice, this))
            .and_then(
                bind_front(gpu::Queue::create, std::cref(*m_device), m_device->raster_queue_entry()))
            .transform(monadic::set(m_raster_queue))
            .and_then(bind_front(gpu::CommandPool::create,
                                std::cref(*m_device),
                                std::cref(*m_raster_queue)))
            .transform(monadic::set(m_main_command_pool))
            .and_then(bind_front(&Renderer::do_initRenderSurface, this, std::move(window)));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Renderer::do_init_instance(std::string_view application_name) noexcept
        -> gpu::Expected<void> {
        return gpu::Instance::create(std::string { application_name })
            .transform(monadic::set(m_instance));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Renderer::do_initDevice() noexcept -> gpu::Expected<void> {
        const auto& physical_devices = m_instance->physical_devices();
        auto        physical_device  = pickPhysicalDevice(physical_devices)
                                   .or_else(expectsWithMessage<Ref<const gpu::PhysicalDevice>>(
                                       "No suitable GPU found !"))
                                   .value();

        ilog("Using physical device {}", *physical_device);

        return gpu::Device::create(*physical_device, *m_instance).transform(monadic::set(m_device));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Renderer::do_initRenderSurface(std::optional<Ref<const wsi::Window>> window) noexcept
        -> gpu::Expected<void> {
        if (window)
            return RenderSurface::create_from_window(*m_instance,
                                                   *m_device,
                                                   *m_raster_queue,
                                                   *(window.value()))
                .transform(monadic::set(m_surface));

        ensures(not window, "Offscreen rendering not yet implemented");

        return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Renderer::threadLoop(std::mutex&       framegraph_mutex,
                              std::atomic_bool& rebuild_graph,
                              std::stop_token   token) noexcept -> void {
        set_current_thread_name("StormKit:RenderThread");

        m_command_buffers
            = m_main_command_pool->create_command_buffers(m_device, m_surface->bufferingCount());

        m_framegraphs.resize(m_surface->bufferingCount());

        for (;;) {
            if (token.stop_requested()) return;

            m_surface->beginFrame(m_device)
                .and_then(bind_front(&Renderer::doRender,
                                    this,
                                    std::ref(framegraph_mutex),
                                    std::ref(rebuild_graph)))
                .and_then(bind_front(&RenderSurface::presentFrame,
                                    &m_surface.get(),
                                    std::cref(*m_raster_queue)))
                .transform_error(assert("Failed to render frame"));
        }

        m_device->wait_idle();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Renderer::doRender(std::mutex&            framegraph_mutex,
                            std::atomic_bool&      rebuild_graph,
                            RenderSurface::Frame&& frame) noexcept
        -> gpu::Expected<RenderSurface::Frame> {
        if (rebuild_graph) {
            auto _ = std::unique_lock { framegraph_mutex };
            m_graph_builder.bake();

            for (auto&& framegraph : m_framegraphs)
                framegraph = m_graph_builder.createFrameGraph(m_device, m_main_command_pool);

            rebuild_graph = false;
        }

        auto&& framegraph = m_framegraphs[frame.current_frame];

        auto&& present_image = m_surface->images()[frame.current_frame];
        auto&& blit_cmb      = m_command_buffers[frame.current_frame];

        blit_cmb.reset();
        blit_cmb.begin(true);
        auto&& result = framegraph->execute(m_raster_queue);
        if (not result) return std::unexpected { result.error() };

        auto&& semaphore  = *result;
        auto&& backbuffer = framegraph->backbuffer();
        blit_cmb.transition_image_layout(backbuffer,
                                       gpu::ImageLayout::Color_Attachment_Optimal,
                                       gpu::ImageLayout::Transfer_Src_Optimal);
        blit_cmb.transition_image_layout(present_image,
                                       gpu::ImageLayout::Present_Src,
                                       gpu::ImageLayout::Transfer_Dst_Optimal);
        blit_cmb.blit_image(backbuffer,
                           present_image,
                           gpu::ImageLayout::Transfer_Src_Optimal,
                           gpu::ImageLayout::Transfer_Dst_Optimal,
                           std::array {
                               gpu::BlitRegion {
                                                .src        = {},
                                                .dst        = {},
                                                .src_offset = { math::vec3f { 0.f, 0.f, 0.f },
                                                   as<math::vec3f>(backbuffer.extent()) },
                                                .dst_offset = { math::vec3f { 0.f, 0.f, 0.f },
                                                   as<math::vec3f>(present_image.extent()) } }
        },
                           gpu::Filter::Linear);
        blit_cmb.transition_image_layout(backbuffer,
                                       gpu::ImageLayout::Transfer_Src_Optimal,
                                       gpu::ImageLayout::Color_Attachment_Optimal);
        blit_cmb.transition_image_layout(present_image,
                                       gpu::ImageLayout::Transfer_Dst_Optimal,
                                       gpu::ImageLayout::Present_Src);
        blit_cmb.end();

        auto wait       = as_refs<std::array>(semaphore, frame.image_available);
        auto stage_mask = std::array { gpu::PipelineStageFlag::Color_Attachment_Output,
                                       gpu::PipelineStageFlag::Transfer };
        auto signal     = as_refs<std::array>(frame.render_finished);

        blit_cmb.submit(m_raster_queue, wait, stage_mask, signal, frame.in_flight);
        return frame;
    }
} // namespace stormkit::engine
