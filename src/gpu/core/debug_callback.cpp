// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/try_expected.hpp>

#include <stormkit/gpu/api.hpp>
#include <stormkit/gpu/vulkan.hpp>

module stormkit.gpu.core;

import std;

import stormkit.core;
import stormkit.log;

namespace stdr = std::ranges;
namespace stdv = std::views;

namespace stormkit::gpu {
    template class DebugCallbackInterface<DebugCallbackImplementation>;
    template class DebugCallbackInterface<view::DebugCallbackImplementation>;

    /////////////////////////////////////
    /////////////////////////////////////
    auto DebugCallbackImplementation::do_init(PrivateTag, const CreateInfo& create_info) noexcept -> Expected<void> {
        constexpr auto severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                  | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                  | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        constexpr auto type = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                              | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                              | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        const auto vk_create_info = VkDebugUtilsMessengerCreateInfoEXT {
            .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext           = nullptr,
            .flags           = 0,
            .messageSeverity = severity,
            .messageType     = type,
            .pfnUserCallback = create_info.messenger_closure,
            .pUserData       = create_info.user_data,
        };

        m_vk_handle = Try(vk::call_checked<
                          VkDebugUtilsMessengerEXT>(vkCreateDebugUtilsMessengerEXT, owner(), &vk_create_info, nullptr));
        Return {};
    }
} // namespace stormkit::gpu
