// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <vulkan/vulkan_hpp_macros.hpp>

#include <stormkit/Gpu/Core/VulkanMacro.hpp>
#include <stormkit/log/log_macro.hpp>

module stormkit.Gpu;

import std;

import stormkit.core;
import stormkit.log;

import :Core.Device;
import :Core.Sync;
import stormkit.Gpu.Vulkan;

using namespace std::literals;

namespace {
    constexpr auto RAYTRACING_EXTENSIONS
        = std::array { "VK_KHR_ray_tracing_pipeline"sv,  "VK_KHR_acceleration_structure"sv,
                       "VK_KHR_buffer_device_address"sv, "VK_KHR_deferred_host_operations"sv,
                       "VK_EXT_descriptor_indexing"sv,   "VK_KHR_spirv_1_4"sv,
                       "VK_KHR_shader_float_controls"sv };

    constexpr auto BASE_EXTENSIONS = std::array { "VK_KHR_maintenance3"sv };

    constexpr auto SWAPCHAIN_EXTENSIONS = std::array { "VK_KHR_swapchain"sv };

} // namespace

namespace stormkit::gpu {
    NAMED_LOGGER(device_logger, "stormkit.Gpu:core.Device")

    template<QueueFlag flag, QueueFlag... no_flag>
    constexpr auto findQueue() {
        return [](const auto& family) {
            return core::checkFlag(family.flags, flag)
                   and (not core::checkFlag(family.flags, no_flag) and ...);
        };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    Device::Device(const PhysicalDevice& physical_device,
                   const Instance&       instance,
                   const Info&           info,
                   Tag)
        : m_physical_device { as_ref(physical_device) } {
        const auto& queue_families = m_physical_device->queueFamilies();

        struct Queue_ {
            std::optional<UInt32> id    = std::nullopt;
            UInt32                count = 0u;
            Byte                  _[3];
            QueueFlag             flags = QueueFlag {};
        };

        const auto raster_queue = [&queue_families]() -> Queue_ {
            const auto it = std::ranges::find_if(queue_families, findQueue<QueueFlag::Graphics>());
            if (it == std::ranges::cend(queue_families)) return {};

            return { .id    = as<UInt32>(std::distance(std::ranges::cbegin(queue_families), it)),
                     .count = it->count,
                     .flags = it->flags };
        }();

        const auto compute_queue = [&queue_families]() -> Queue_ {
            const auto it
                = std::ranges::find_if(queue_families,
                                       findQueue<QueueFlag::Transfer, QueueFlag::Graphics>());
            if (it == std::ranges::cend(queue_families)) return {};

            return { .id    = as<UInt32>(std::distance(std::ranges::cbegin(queue_families), it)),
                     .count = it->count,
                     .flags = it->flags };
        }();

        const auto transfert_queue = [&queue_families]() -> Queue_ {
            const auto it = std::ranges::find_if(
                queue_families,
                findQueue<QueueFlag::Compute, QueueFlag::Graphics, QueueFlag::Transfer>());
            if (it == std::ranges::cend(queue_families)) return {};

            return { .id    = as<UInt32>(std::distance(std::ranges::cbegin(queue_families), it)),
                     .count = it->count,
                     .flags = it->flags };
        }();

        const auto queues = [&] {
            auto q = std::vector<const Queue_*> {};
            q.reserve(3);

            if (raster_queue.id) q.push_back(&raster_queue);
            if (compute_queue.id) q.push_back(&compute_queue);
            if (transfert_queue.id) q.push_back(&transfert_queue);

            return q;
        }();

        auto priorities = std::vector<std::vector<float>> {};
        priorities.reserve(std::size(queues));

        const auto queue_create_infos = transform(queues, [&priorities](auto queue) {
            auto& priority = priorities.emplace_back();

            priority.resize(queue->count, 1.f);

            return vk::DeviceQueueCreateInfo {}
                .setQueueFamilyIndex(queue->id.value())
                .setQueuePriorities(priority);
        });

        const auto& capabilities = m_physical_device->capabilities();
        const auto  enabled_features
            = vk::PhysicalDeviceFeatures {}
                  .setSampleRateShading(capabilities.features.sampler_rate_shading)
                  .setMultiDrawIndirect(capabilities.features.multi_draw_indirect)
                  .setFillModeNonSolid(capabilities.features.fill_Mode_non_solid)
                  .setSamplerAnisotropy(capabilities.features.sampler_anisotropy);

        const auto device_extensions
            = m_physical_device->vkHandle().enumerateDeviceExtensionProperties();

        device_logger.dlog("Device extensions: {}",
                           device_extensions | std::views::transform([](auto&& ext) {
                               return std::string_view { ext.extensionName };
                           }));

        const auto swapchain_available = [&] {
            for (const auto& ext : SWAPCHAIN_EXTENSIONS) {
                if (std::ranges::find_if(device_extensions,
                                         [&](const auto& e) { return ext == e.extensionName; })
                    == std::ranges::cend(device_extensions)) {
                    return false;
                }
            }

            return true;
        }();

        const auto raytracing_available = [&] {
            for (const auto& ext : RAYTRACING_EXTENSIONS) {
                if (std::ranges::find_if(device_extensions,
                                         [&](const auto& e) { return ext == e.extensionName; })
                    == std::ranges::cend(device_extensions)) {
                    return false;
                }
            }

            return true;
        }();

        const auto extensions = [&] {
            constexpr auto as_czstring = [](const auto& v) { return std::data(v); };

            auto e = transform(BASE_EXTENSIONS, as_czstring);
            if (swapchain_available and info.enable_swapchain)
                merge(e, transform(SWAPCHAIN_EXTENSIONS, as_czstring));
            if (raytracing_available and info.enable_raytracing)
                merge(e, transform(RAYTRACING_EXTENSIONS, as_czstring));

            return e;
        }();

        const auto acceleration_feature = vk::PhysicalDeviceAccelerationStructureFeaturesKHR {};
        const auto rt_pipeline_feature
            = vk::PhysicalDeviceRayTracingPipelineFeaturesKHR {}.setPNext(
                std::bit_cast<void*>(&acceleration_feature));

        const auto next = [&]() -> void* {
            if (raytracing_available and info.enable_raytracing)
                return std::bit_cast<void*>(&rt_pipeline_feature);
            return nullptr;
        }();

        const auto create_info = vk::DeviceCreateInfo {}
                                     .setPNext(next)
                                     .setQueueCreateInfos(queue_create_infos)
                                     .setPEnabledExtensionNames(extensions)
                                     .setPEnabledFeatures(&enabled_features);

        m_physical_device->vkHandle()
            .createDevice(create_info)
            .transform(core :.monadic::set(m_vk_device))
            .transform([this, &instance] noexcept -> VulkanExpected<void> {
                VULKAN_HPP_DEFAULT_DISPATCHER.init(*vkHandle());

                m_vma_function_table
                    = vma::functionsFromDispatcher(instance.vkHandle().getDispatcher(),
                                                   vkHandle().getDispatcher());

                const auto alloc_create_info
                    = vma::AllocatorCreateInfo {}
                          .setInstance(*(instance.vkHandle()))
                          .setPhysicalDevice(*m_physical_device->vkHandle())
                          .setDevice(*vkHandle())
                          .setPVulkanFunctions(&vmaFunctionTable());

                auto&& [result, allocator] = vma::createAllocatorUnique(alloc_create_info);
                if (result != vk::Result::eSuccess) return std::unexpected { result };

                m_vma_allocator = std::move(allocator);

                return {};
            })
            .transform_error(core :.monadic::map(core :.monadic::narrow<Result>(),
                                                 core :.monadic::throw_as_exception()));

        if (raster_queue.id)
            m_raster_queue = QueueEntry { .id    = *raster_queue.id,
                                          .count = raster_queue.count,
                                          .flags = raster_queue.flags };

        setObjectName(*this,
                      std::format("StormKit:Device ({})", m_physical_device->info().device_name));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Device::waitForFences(std::span<const Ref<const Fence>> fences,
                               bool                              wait_all,
                               const std::chrono::milliseconds&  timeout) const noexcept
        -> Expected<Result> {
        const auto vk_fences = fences
                               | std::views::transform(monadic::toVkHandle())
                               | std::ranges::to<std::vector>();

        return vkCall(*m_vk_device,
                      &vk::raii::Device::waitForFences,
                      {
                          { vk::Result::eSuccess, vk::Result::eNotReady }
        },
                      vk_fences,
                      wait_all,
                      std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count())
            .transform(core :.monadic::narrow<Result>())
            .transform_error(core :.monadic::narrow<Result>());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Device::resetFences(std::span<const Ref<const Fence>> fences) const noexcept -> void {
        const auto vk_fences = fences
                               | std::views::transform(monadic::toVkHandle())
                               | std::ranges::to<std::vector>();

        m_vk_device->resetFences(vk_fences);
    }
} // namespace stormkit::gpu
