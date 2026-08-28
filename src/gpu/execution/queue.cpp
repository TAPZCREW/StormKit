// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/try_expected.hpp>

#include <stormkit/gpu/vulkan.hpp>

module stormkit.gpu.execution;

import std;

import stormkit.core;

import stormkit.gpu.core;

using namespace std::literals;

namespace stdr = std::ranges;
namespace stdv = std::views;
namespace stdp = std::pmr;

namespace stormkit::gpu {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto QueueInterface<Base>::wait_idle() const noexcept -> Expected<void> {
        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        return vk::call_checked(device_table.vkQueueWaitIdle, *this);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto QueueInterface<Base>::submit(array_view<const SubmitInfo> submit_infos, std::optional<view::Fence> fence) const noexcept
      -> Expected<void> {
        struct SubmitInfoRange {
            array_view<const VkSemaphore>          wait_semaphores;
            array_view<const VkPipelineStageFlags> wait_dst_stages;
            array_view<const VkCommandBuffer>      command_buffers;
            array_view<const VkSemaphore>          signal_semaphores;
        };

        const auto bytes_count = [&submit_infos] noexcept {
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
            return _wait_semaphores_count * sizeof(VkSemaphore)
                   + _wait_dst_stages_count * sizeof(VkPipelineStageFlags)
                   + _command_buffers_count * sizeof(VkCommandBuffer)
                   + _signal_semaphores_count * sizeof(VkSemaphore)
                   + stdr::size(submit_infos) * sizeof(SubmitInfoRange);
        }();

        auto memory_resource = stdp::monotonic_buffer_resource { bytes_count };

        auto wait_semaphores_buf = pmr::dyn_array<pmr::dyn_array<VkSemaphore>> { &memory_resource };
        wait_semaphores_buf.reserve(stdr::size(submit_infos));
        auto wait_dst_stages_buf = pmr::dyn_array<pmr::dyn_array<VkPipelineStageFlags>> { &memory_resource };
        wait_dst_stages_buf.reserve(stdr::size(submit_infos));
        auto command_buffers_buf = pmr::dyn_array<pmr::dyn_array<VkCommandBuffer>> { &memory_resource };
        command_buffers_buf.reserve(stdr::size(submit_infos));
        auto signal_semaphores_buf = pmr::dyn_array<pmr::dyn_array<VkSemaphore>> { &memory_resource };
        signal_semaphores_buf.reserve(stdr::size(submit_infos));

        const auto submit_ranges = [&] noexcept {
            auto vec = pmr::dyn_array<SubmitInfoRange> { &memory_resource };
            vec.reserve(stdr::size(submit_infos));
            for (auto&& submit_info : submit_infos) {
                auto& wait_semaphores = wait_semaphores_buf.emplace_back(std::from_range,
                                                                         submit_info.wait_semaphores
                                                                           | stdv::transform(vk::monadic::to_vk()));

                auto& wait_dst_stages = wait_dst_stages_buf.emplace_back(std::from_range,
                                                                         submit_info.wait_dst_stages
                                                                           | stdv::transform(vk::monadic::to_vk<
                                                                                             VkPipelineStageFlagBits>()));

                auto& command_buffers = command_buffers_buf.emplace_back(std::from_range,
                                                                         submit_info.command_buffers
                                                                           | stdv::transform(vk::monadic::to_vk()));

                auto& signal_semaphores = signal_semaphores_buf.emplace_back(std::from_range,
                                                                             submit_info.signal_semaphores
                                                                               | stdv::transform(vk::monadic::to_vk()));

                vec.emplace_back(SubmitInfoRange {
                  .wait_semaphores   = wait_semaphores,
                  .wait_dst_stages   = wait_dst_stages,
                  .command_buffers   = command_buffers,
                  .signal_semaphores = signal_semaphores,
                });
            }
            return vec;
        }();

        const auto vk_submit_infos = submit_ranges
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
                                     | stdr::to<dyn_array<VkSubmitInfo>>();

        const auto vk_fence = either(fence, vk::monadic::to_vk(), core::monadic::init<VkFence>(VK_NULL_HANDLE));

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();
        return vk::call_checked(device_table.vkQueueSubmit,
                                *this,
                                stdr::size(vk_submit_infos),
                                stdr::data(vk_submit_infos),
                                vk_fence);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto QueueInterface<Base>::present(array_view<const view::SwapChain> swapchains,
                                       array_view<const view::Semaphore> wait_semaphores,
                                       array_view<const u32>             image_indices) const noexcept -> Expected<Result> {
        EXPECTS(stdr::size(wait_semaphores) >= 1);
        EXPECTS(stdr::size(image_indices) >= 1);

        const auto swapchains_count      = stdr::size(swapchains);
        const auto wait_semaphores_count = stdr::size(wait_semaphores);

        const auto bytes_count     = swapchains_count * sizeof(VkSwapchainKHR) + wait_semaphores_count * sizeof(VkSemaphore);
        auto       memory_resource = stdp::monotonic_buffer_resource { bytes_count };

        const auto vk_swapchains = pmr::dyn_array<VkSwapchainKHR> {
            std::from_range,
            swapchains | stdv::transform(vk::monadic::to_vk()),
            &memory_resource
        };
        const auto vk_semaphores = pmr::dyn_array<VkSemaphore> {
            std::from_range,
            wait_semaphores | stdv::transform(vk::monadic::to_vk()),
            &memory_resource
        };

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

        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();
        const auto
          result = Try((vk::call_checked<VkResult, VK_ERROR_OUT_OF_DATE_KHR, VK_SUBOPTIMAL_KHR>(device_table.vkQueuePresentKHR,
                                                                                                *this,
                                                                                                &present_info)));
        Return vk::from_vk<Result>(result);
    }

    template class QueueInterface<QueueImplementation>;
    template class QueueInterface<view::QueueImplementation>;

    /////////////////////////////////////
    /////////////////////////////////////
    auto QueueImplementation::do_init(PrivateTag, const QueueEntry& entry) -> void {
        m_entry = entry;

        const auto& device       = owner();
        const auto& device_table = device.device_table();
        m_vk_handle              = vk::call<VkQueue>(device_table.vkGetDeviceQueue, device, m_entry.id, 0);
    }
} // namespace stormkit::gpu
