// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

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
    auto CommandPool::create_command_buffers(usize count, CommandBufferLevel level) const noexcept
      -> Expected<std::vector<CommandBuffer>> {
        return create_vk_command_buffers(count, level)
          .transform([this, &level](auto&& command_buffers) noexcept {
              return command_buffers
                     | stdv::as_rvalue
                     | stdv::transform([this,
                                        &level](VkCommandBuffer&& cmb) noexcept -> decltype(auto) {
                           return CommandBuffer::create(m_vk_device,
                                                        m_vk_handle,
                                                        m_vk_device_table,
                                                        level,
                                                        std::move(cmb),
                                                        CommandPool::delete_vk_command_buffers);
                       })
                     | stdr::to<std::vector>();
          })
          .transform_error(monadic::from_vk<Result>());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto CommandPool::allocate_command_buffers(usize count, CommandBufferLevel level) const noexcept
      -> Expected<std::vector<Heap<CommandBuffer>>> {
        return create_vk_command_buffers(count, level)
          .transform([this, &level](auto&& command_buffers) noexcept {
              return command_buffers
                     | stdv::as_rvalue
                     | stdv::transform([this,
                                        &level](VkCommandBuffer&& cmb) noexcept -> decltype(auto) {
                           return CommandBuffer::allocate(m_vk_device,
                                                          m_vk_handle,
                                                          m_vk_device_table,
                                                          level,
                                                          std::move(cmb),
                                                          CommandPool::delete_vk_command_buffers);
                       })
                     | stdr::to<std::vector>();
          })
          .transform_error(monadic::from_vk<Result>());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto CommandPool::create_vk_command_buffers(usize              count,
                                                CommandBufferLevel level) const noexcept
      -> VulkanExpected<std::vector<VkCommandBuffer>> {
        // auto out = std::vector<VkCommandBuffer> {};
        // const auto reuse_count  = stdr::empty(m_reusable_command_buffers)
        //                             ? 0
        //                             : math::abs(stdr::size(m_reusable_command_buffers) - count);
        // auto       create_count = count - reuse_count;

        {
            // auto lock = std::unique_lock { m_reuse_mutex };
            // auto erase_end   = std::ranges::end(m_reusable_command_buffers);
            // auto erase_begin = std::ranges::end(m_reusable_command_buffers) - reuse_count;
            //
            // std::ranges::for_each(m_reusable_command_buffers | std::views::reverse |
            //                           std::views::take(reuse_count),
            //                       [&out](VkCommandBuffer&& cmb) {
            //                           out.emplace_back(std::move(cmb));
            //                       });
            //
            // m_reusable_command_buffers.erase(erase_begin, erase_end);
        }
        // out.reserve(count);

        // if (create_count > 0) {
        const auto allocate_info = VkCommandBufferAllocateInfo {
            .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext              = nullptr,
            .commandPool        = m_vk_handle,
            .level              = to_vk<VkCommandBufferLevel>(level),
            .commandBufferCount = as<u32>(count)
        };

        return vk_allocate<VkCommandBuffer>(count,
                                            m_vk_device_table->vkAllocateCommandBuffers,
                                            m_vk_device,
                                            &allocate_info);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto CommandPool::delete_vk_command_buffers(VkDevice               device,
                                                VkCommandPool          command_pool,
                                                const VolkDeviceTable& device_table,
                                                VkCommandBuffer command_buffer) noexcept -> void {
        vk_call(device_table.vkFreeCommandBuffers, device, command_pool, 1, &command_buffer);
        // auto lock = std::unique_lock { m_reuse_mutex };
        // m_reusable_command_buffers.emplace_back(std::move(cmb));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto CommandPool::do_init() noexcept -> Expected<void> {
        const auto create_info = VkCommandPoolCreateInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
                     | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
            .queueFamilyIndex = 0,
        };

        return vk_call<VkCommandPool>(m_vk_device_table->vkCreateCommandPool,
                                      m_vk_device,
                                      &create_info,
                                      nullptr)
          .transform(core::monadic::set(m_vk_handle))
          .transform_error(monadic::from_vk<Result>());
    }
} // namespace stormkit::gpu
