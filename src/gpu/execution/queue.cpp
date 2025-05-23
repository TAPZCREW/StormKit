// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>

#include <volk.h>

module stormkit.gpu.execution;

import std;

import stormkit.core;

import stormkit.gpu.core;

using namespace std::literals;

namespace stdr = std::ranges;
namespace stdv = std::views;

namespace stormkit::gpu {

    /////////////////////////////////////
    /////////////////////////////////////
    auto Queue::submit(std::span<const SubmitInfo> submit_infos,
                       OptionalRef<const Fence>    fence) const noexcept -> Expected<void> {
        struct SubmitInfoRange {
            std::span<const VkSemaphore>          wait_semaphores;
            std::span<const VkPipelineStageFlags> wait_dst_stages;
            std::span<const VkCommandBuffer>      command_buffers;
            std::span<const VkSemaphore>          signal_semaphores;
        };

        const auto [wait_semaphores_count,
                    wait_dst_stages_count,
                    command_buffers_count,
                    signal_semaphores_count]
          = [&submit_infos] noexcept {
                auto _wait_semaphores_count   = 0uz;
                auto _wait_dst_stages_count   = 0uz;
                auto _command_buffers_count   = 0uz;
                auto _signal_semaphores_count = 0uz;

                for (auto&& submit_info : submit_infos) {
                    _wait_semaphores_count   = stdr::size(submit_info.wait_semaphores);
                    _wait_dst_stages_count   = stdr::size(submit_info.wait_dst_stages);
                    _command_buffers_count   = stdr::size(submit_info.command_buffers);
                    _signal_semaphores_count = stdr::size(submit_info.signal_semaphores);
                }
                return std::make_tuple(_wait_semaphores_count,
                                       _wait_dst_stages_count,
                                       _command_buffers_count,
                                       _signal_semaphores_count);
            }();

        // replace with strong type
        struct VkWaitSemaphore {
            VkSemaphore* _;
        };

        struct VkSignalSemaphore {
            VkSemaphore* _;
        };

        auto payload = MultiBuffer<VkWaitSemaphore,
                                   VkPipelineStageFlags,
                                   VkCommandBuffer,
                                   VkSignalSemaphore> {
            { wait_semaphores_count * sizeof(vkWaitSemaphores),
             wait_dst_stages_count * sizeof(VkPipelineStageFlags),
             command_buffers_count * sizeof(VkCommandBuffer),
             signal_semaphores_count * sizeof(VkSignalSemaphore) }
        };

        const auto submit_ranges
          = submit_infos
            | stdv::transform([&payload](auto&& submit_info) mutable noexcept {
                  std::println("VVVVVVVVVVVVVVV");
                  const auto wait_semaphores = payload.init_range<
                    0>(submit_info.wait_semaphores | stdv::transform(monadic::to_vk()));

                  const auto wait_dst_stages = payload.init_range<
                    1>(submit_info.wait_dst_stages
                       | stdv::transform(monadic::to_vk<VkPipelineStageFlagBits>()));

                  const auto command_buffers = payload.init_range<
                    VkCommandBuffer>(submit_info.command_buffers
                                     | stdv::transform(monadic::to_vk()));

                  const auto signal_semaphores = payload.init_range<
                    3>(submit_info.signal_semaphores | stdv::transform(monadic::to_vk()));

                  return SubmitInfoRange {
                      .wait_semaphores   = { std::bit_cast<
                                               VkSemaphore*>(stdr::begin(wait_semaphores)),
                                            stdr::size(wait_semaphores)   },
                      .wait_dst_stages   = wait_dst_stages,
                      .command_buffers   = command_buffers,
                      .signal_semaphores = { std::bit_cast<
                                               VkSemaphore*>(stdr::begin(signal_semaphores)),
                                            stdr::size(signal_semaphores) },
                  };
              })
            | stdr::to<std::vector>();

        const auto vk_submit_infos
          = submit_ranges
            | stdv::transform([](auto&& submit_range) noexcept {
                  EXPECTS(stdr::size(submit_range.wait_semaphores)
                          == stdr::size(submit_range.wait_dst_stages));

                  return VkSubmitInfo {
                      .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                      .pNext                = nullptr,
                      .waitSemaphoreCount   = as<u32>(stdr::size(submit_range.wait_semaphores)),
                      .pWaitSemaphores      = stdr::data(submit_range.wait_semaphores),
                      .pWaitDstStageMask    = stdr::data(submit_range.wait_dst_stages),
                      .commandBufferCount   = as<u32>(stdr::size(submit_range.command_buffers)),
                      .pCommandBuffers      = stdr::data(submit_range.command_buffers),
                      .signalSemaphoreCount = as<u32>(stdr::size(submit_range.signal_semaphores)),
                      .pSignalSemaphores    = stdr::data(submit_range.signal_semaphores),
                  };
              })
            | stdr::to<std::vector>();

        const auto vk_fence = core::either(fence,
                                           monadic::to_vk(),
                                           core::monadic::init<VkFence>(nullptr));

        return vk_call(m_vk_device_table->vkQueueSubmit,
                       m_vk_handle,
                       stdr::size(vk_submit_infos),
                       stdr::data(vk_submit_infos),
                       vk_fence)
          .transform_error(monadic::from_vk<Result>());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Queue::present(std::span<const Ref<const SwapChain>> swapchains,
                        std::span<const Ref<const Semaphore>> wait_semaphores,
                        std::span<const u32> image_indices) const noexcept -> Expected<Result> {
        EXPECTS(stdr::size(wait_semaphores) >= 1);
        EXPECTS(stdr::size(image_indices) >= 1);

        const auto swapchains_count      = stdr::size(swapchains);
        const auto wait_semaphores_count = stdr::size(wait_semaphores);

        auto payload = MultiBuffer<VkSwapchainKHR, VkSemaphore> {
            { swapchains_count, wait_semaphores_count }
        };
        const auto vk_swapchains = payload.init_range<
          VkSwapchainKHR>(swapchains | stdv::transform(monadic::to_vk()));
        const auto vk_semaphores = payload
                                     .init_range<VkSemaphore>(wait_semaphores
                                                              | stdv::transform(monadic::to_vk()));

        const auto present_info = VkPresentInfoKHR {
            .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext              = nullptr,
            .waitSemaphoreCount = as<u32>(stdr::size(vk_semaphores)),
            .pWaitSemaphores    = stdr::data(vk_semaphores),
            .swapchainCount     = as<u32>(stdr::size(vk_swapchains)),
            .pSwapchains        = stdr::data(vk_swapchains),
            .pImageIndices      = stdr::data(image_indices),
            .pResults           = nullptr,
        };

        const auto possible_results = into_array_of<VkResult>(VK_SUCCESS,
                                                              VK_ERROR_OUT_OF_DATE_KHR,
                                                              VK_SUBOPTIMAL_KHR);
        return vk_call<VkResult>(m_vk_device_table->vkQueuePresentKHR,
                                 as_view(possible_results),
                                 m_vk_handle,
                                 &present_info)
          .transform(monadic::from_vk<Result>())
          .transform_error(monadic::from_vk<Result>());
    }
} // namespace stormkit::gpu
