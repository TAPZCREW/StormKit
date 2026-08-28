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
    template class SemaphoreInterface<SemaphoreImplementation>;
    template class SemaphoreInterface<view::SemaphoreImplementation>;

    /////////////////////////////////////
    /////////////////////////////////////
    auto SemaphoreImplementation::do_init(PrivateTag) noexcept -> Expected<void> {
        const auto create_info = VkSemaphoreCreateInfo {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
        };

        const auto& device       = owner();
        const auto& device_table = device.device_table();
        m_vk_handle = Try(vk::call_checked<VkSemaphore>(device_table.vkCreateSemaphore, device, &create_info, nullptr));

        Return {};
    }
} // namespace stormkit::gpu
