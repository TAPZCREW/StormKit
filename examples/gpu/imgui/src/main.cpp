// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit;
import gpu_app;

#include <stormkit/core/try_expected.hpp>
#include <stormkit/gpu/vulkan.hpp>
#include <stormkit/log/log_macro.hpp>
#include <stormkit/main/main_macro.hpp>

#define IMGUI_IMPL_VULKAN_NO_PROTOTYPES
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#undef assert

namespace stdr  = std::ranges;
namespace stdfs = std::filesystem;

using namespace std::literals;
using namespace stormkit;

struct SubmissionResource {
    gpu::Fence         in_flight;
    gpu::Semaphore     image_available;
    gpu::CommandBuffer render_cmb;
};

struct SwapchainImageResource {
    gpu::view::Image image;
    gpu::ImageView   view;
    gpu::Semaphore   render_finished;
};

namespace {
    constexpr auto BUFFERING_COUNT = 2_u32;
    constexpr auto POOL_SIZES      = array {
        gpu::DescriptorPool::Size { .type = gpu::DescriptorType::COMBINED_IMAGE_SAMPLER, .descriptor_count = BUFFERING_COUNT }
    };
} // namespace

class Application: public base::Application {
  public:
    auto init_example() {
        init_resources();
        init_imgui();
    }

    auto init_resources() -> void {
        // initialilze descriptor pool
        m_descriptor_pool = TryAssert(gpu::DescriptorPool::create(m_device, POOL_SIZES, BUFFERING_COUNT),
                                      "Failed to create descriptor pool!");

        // create present engine resources
        m_submission_resources = init_by<dynarray<SubmissionResource>>([&](auto& out) noexcept {
            out.reserve(BUFFERING_COUNT);
            for (auto _ : range(BUFFERING_COUNT)) {
                out.push_back({
                  .in_flight       = TryAssert(gpu::Fence::create_signaled(m_device),
                                               "Failed to create swapchain image "
                                               "in flight fence!"),
                  .image_available = TryAssert(gpu::Semaphore::create(m_device),
                                               "Failed to create "
                                               "present wait semaphore!"),
                  .render_cmb      = TryAssert(m_command_pool->create_command_buffer(),
                                               "Failed to create transition "
                                               "command buffers!"),
                });
            }
        });

        // transition swapchain image to present image
        const auto& images = m_swapchain->images();

        const auto image_count     = stdr::size(images);
        auto       transition_cmbs = TryAssert(m_command_pool->create_command_buffers(image_count),
                                               "Failed to create transition command buffers!");
        m_image_resources.reserve(stdr::size(images));

        auto image_index = 0u;
        for (const auto& swap_image : images) {
            auto view = TryAssert(gpu::ImageView::create(m_device, { swap_image }), "Failed to create swapchain image view!");

            m_image_resources.push_back({ .image           = swap_image,
                                          .view            = std::move(view),
                                          .render_finished = TryAssert(gpu::Semaphore::create(m_device),
                                                                       "Failed to create render "
                                                                       "signal semaphore!") });

            auto& transition_cmb = transition_cmbs[image_index];
            DiscardTryAssert((transition_cmb.record([&](auto cmb) noexcept {
                                 cmb.begin_debug_region(std::format("Transition image {}", image_index))
                                   .transition_image_layout(swap_image,
                                                            gpu::ImageLayout::UNDEFINED,
                                                            gpu::ImageLayout::PRESENT_SRC)
                                   .end_debug_region();
                             })),
                             std::format("Failed to record transition cmb {}!", image_index));

            ++image_index;
        }

        const auto fence = TryAssert(gpu::Fence::create(m_device), "Failed to create transition fence!");

        const auto cmbs = to_views(transition_cmbs);

        TryAssert(m_raster_queue->submit({ .command_buffers = cmbs }, fence),
                  "Failed to submit texture transition command buffers!");

        // wait for transition to be done
        TryAssert(fence.wait(), "");
    }

    auto init_imgui() -> void {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io      = ImGui::GetIO();
        io.DisplaySize.x = m_window->extent().to<f32>().width;
        io.DisplaySize.y = m_window->extent().to<f32>().height;

        const auto format = gpu::vk::to_vk<VkFormat>(m_swapchain->pixel_format());
        /*const*/ auto init_info = ImGui_ImplVulkan_InitInfo {
            .ApiVersion                  = VK_API_VERSION_1_3,
            .Instance                    = m_instance->native_handle(),
            .PhysicalDevice              = m_physical_device->native_handle(),
            .Device                      = m_device->native_handle(),
            .QueueFamily                 = 0,
            .Queue                       = m_raster_queue->native_handle(),
            .DescriptorPool              = m_descriptor_pool->native_handle(),
            .DescriptorPoolSize          = 0,
            .MinImageCount               = BUFFERING_COUNT,
            .ImageCount                  = BUFFERING_COUNT,
            .PipelineCache               = nullptr,
            .PipelineInfoMain = {
            .RenderPass                  = nullptr,
            .Subpass                     = {},
            .MSAASamples                 = {},
            .PipelineRenderingCreateInfo = {
                .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                .pNext                   = nullptr,
                .viewMask                = 0,
                .colorAttachmentCount    = 1,
                .pColorAttachmentFormats = &format,
                .depthAttachmentFormat   = {},
                .stencilAttachmentFormat = {}
            
        },
          },
            .UseDynamicRendering         = true,
            .Allocator                   = nullptr,
            .CheckVkResultFn =
              [](auto result) static noexcept {
                  if (result != VK_SUCCESS) elog("{}", gpu::vk::from_vk<gpu::Result>(result));
              },
            .MinAllocationSize = 1024 * 1024,
            .CustomShaderVertCreateInfo = {},
            .CustomShaderFragCreateInfo = {},
        };
        ImGui_ImplVulkan_LoadFunctions(VK_API_VERSION_1_1, gpu::vk::imgui_vk_loader, &*m_device);
        ImGui_ImplVulkan_Init(&init_info);

        m_window->on(wsi::KeyDownEventFunc { [this, &io](u8 /*id*/, wsi::Key key, char c) mutable noexcept {
                         if (key == wsi::Key::ESCAPE) m_window->close();
                         io.AddInputCharactersUTF8(&c);
                     } },
                     wsi::MouseMovedEventFunc { [&io](u8 /*id*/, const math::ivec2& position) mutable noexcept {
                         const auto _position = position.to<f32>();

                         io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
                         io.AddMousePosEvent(_position.x, _position.y);
                     } },
                     wsi::MouseButtonDownEventFunc {
                       [&io](u8 /*id*/, wsi::MouseButton button, const math::ivec2&) mutable noexcept {
                           auto mouse_button = -1;
                           if (button == wsi::MouseButton::LEFT) mouse_button = 0;
                           if (button == wsi::MouseButton::RIGHT) mouse_button = 1;
                           if (button == wsi::MouseButton::MIDDLE) mouse_button = 2;
                           if (button == wsi::MouseButton::BUTTON_1) mouse_button = 3;
                           if (button == wsi::MouseButton::BUTTON_2) mouse_button = 4;
                           if (mouse_button == -1) return;
                           io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
                           io.AddMouseButtonEvent(mouse_button, true);
                       } },
                     wsi::MouseButtonUpEventFunc {
                       [&io](u8 /*id*/, wsi::MouseButton button, const math::ivec2&) mutable noexcept {
                           auto mouse_button = -1;
                           if (button == wsi::MouseButton::LEFT) mouse_button = 0;
                           if (button == wsi::MouseButton::RIGHT) mouse_button = 1;
                           if (button == wsi::MouseButton::MIDDLE) mouse_button = 2;
                           if (button == wsi::MouseButton::BUTTON_1) mouse_button = 3;
                           if (button == wsi::MouseButton::BUTTON_2) mouse_button = 4;
                           if (mouse_button == -1) return;
                           io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
                           io.AddMouseButtonEvent(mouse_button, false);
                       } });
    }

    auto run_example() {
        LOG_MODULE.flush();

        ImGui_ImplVulkan_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        ImGui::Render();

        // get next swapchain image
        auto& submission_resource = m_submission_resources[m_current_frame];

        const auto& wait      = submission_resource.image_available;
        auto&       in_flight = submission_resource.in_flight;

        TryAssert(in_flight.wait(), "Failed to wait in_flight fence!");
        TryAssert(in_flight.reset(), "Failed to reset in_flight fence!");

        const auto&& [_, image_index] = TryAssert(m_swapchain->acquire_next_image(100ms, wait),
                                                  "Failed to acquire next swapchain image!");

        const auto& swapchain_image_resource = m_image_resources[image_index];
        const auto& signal                   = swapchain_image_resource.render_finished;

        static constexpr auto PIPELINE_FLAGS = array { gpu::PipelineStageFlag::COLOR_ATTACHMENT_OUTPUT };

        const auto window_extent  = m_window->extent().to<i32>();
        const auto rendering_info = gpu::RenderingInfo {
            .render_area       = { .x = 0, .y = 0, .width = window_extent.width, .height = window_extent.height },
            .color_attachments = { { .image_view  = swapchain_image_resource.view,
                                     .layout      = gpu::ImageLayout::ATTACHMENT_OPTIMAL,
                                     .clear_value = gpu::ClearColor { .color = colors::SILVER<f32> } } }
        };

        // render in it
        auto& render_cmb = submission_resource.render_cmb;
        TryAssert(render_cmb.reset(), std::format("Failed to reset render cmb {}!", image_index));
        DiscardTryAssert((render_cmb.record([&](auto cmb) noexcept {
                             cmb
                               .transition_image_layout(swapchain_image_resource.image,
                                                        gpu::ImageLayout::PRESENT_SRC,
                                                        gpu::ImageLayout::ATTACHMENT_OPTIMAL)
                               .begin_debug_region("Render imgui")
                               .begin_rendering(rendering_info);

                             ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmb);

                             cmb
                               .end_rendering() //
                               .end_debug_region()
                               .transition_image_layout(swapchain_image_resource.image,
                                                        gpu::ImageLayout::ATTACHMENT_OPTIMAL,
                                                        gpu::ImageLayout::PRESENT_SRC);
                         })),
                         std::format("Failed to record render cmb {}!", image_index));

        DiscardTryAssert(render_cmb.submit(m_raster_queue, gpu::as_views(wait), PIPELINE_FLAGS, gpu::as_views(signal), in_flight),
                         "Failed to submit render command buffer");

        // present it
        DiscardTryAssert(m_raster_queue->present(gpu::as_views(m_swapchain), gpu::as_views(signal), as_view(image_index)),
                         "Failed to present swapchain image");

        if (++m_current_frame >= BUFFERING_COUNT) m_current_frame = 0;
    }

    auto deinit() {
        ImGui_ImplVulkan_Shutdown();
        ImGui::DestroyContext();
    }

    constexpr auto example_name() const noexcept -> string_view { return "Imgui"; }

  private:
    DeferInit<gpu::DescriptorPool>    m_descriptor_pool;
    dynarray<SubmissionResource>     m_submission_resources;
    dynarray<SwapchainImageResource> m_image_resources;
    usize                             m_current_frame = 0_usize;
};

auto main(array_view<const string_view> args) -> int {
    auto app = Application {};
    app.run(args);
    return 0;
}
