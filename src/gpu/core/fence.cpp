// Copyright (C) 2023 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>
#include <stormkit/core/try_expected.hpp>

#include <stormkit/gpu/api.hpp>
#include <stormkit/gpu/vulkan.hpp>

module stormkit.gpu.core;

import std;

import stormkit.core;

import :vulkan;

namespace stormkit::gpu {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto FenceInterface<Base>::status() const noexcept -> Expected<Status> {
        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        const auto result = Try((vk::call_checked<VkResult, VK_NOT_READY>(device_table.vkGetFenceStatus, device, *this)));
        if (result == VK_NOT_READY) Return Fence::Status::UNSIGNALED;
        Return Fence::Status::SIGNALED;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto FenceInterface<Base>::wait(const std::chrono::milliseconds& wait_for) const noexcept -> Expected<Result> {
        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();
        const auto  handle       = Base::native_handle();

        const auto
          result = Try((vk::call_checked<VkResult,
                                         VK_NOT_READY>(device_table.vkWaitForFences,
                                                       device,
                                                       1u,
                                                       &handle,
                                                       true,
                                                       std::chrono::duration_cast<std::chrono::nanoseconds>(wait_for).count())));

        Return vk::from_vk<Result>(result);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto FenceInterface<Base>::reset() const noexcept -> Expected<void> {
        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();
        const auto  handle       = Base::native_handle();

        Try(vk::call_checked(device_table.vkResetFences, device, 1u, &handle));

        Return {};
    }

    template class FenceInterface<FenceImplementation>;
    template class FenceInterface<view::FenceImplementation>;

    /////////////////////////////////////
    /////////////////////////////////////
    auto FenceImplementation::do_init(PrivateTag, const CreateInfo& create_info) noexcept -> Expected<void> {
        const auto flags = (create_info.signaled) ? VkFenceCreateFlags { VK_FENCE_CREATE_SIGNALED_BIT } : VkFenceCreateFlags {};

        const auto vk_create_info = VkFenceCreateInfo {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags
        };

        const auto& device       = owner();
        const auto& device_table = device.device_table();

        m_vk_handle = Try(vk::call_checked<VkFence>(device_table.vkCreateFence, device, &vk_create_info, nullptr));

        Return {};
    }
} // namespace stormkit::gpu
