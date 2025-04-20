// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module stormkit.Gpu;

import std;

import stormkit.Core;
import stormkit.Log;

#include <stormkit/Log/LogMacro.hpp>

import stormkit.Gpu.Vulkan;
import :Core;
import :Execution.Pipeline;

namespace stormkit::gpu {
    LOGGER("stormkit.Gpu")

    /////////////////////////////////////
    /////////////////////////////////////
    auto PipelineCache::createNewPipelineCache(const Device& device) -> VulkanExpected<void> {
        const auto physical_device_infos = device.physicalDevice().info();

        m_serialized.guard.magic     = MAGIC;
        m_serialized.guard.data_size = 0u;
        m_serialized.guard.data_hash = 0u;

        m_serialized.infos.version   = VERSION;
        m_serialized.infos.vendor_id = physical_device_infos.vendor_id;
        m_serialized.infos.device_id = physical_device_infos.device_id;

        std::ranges::copy(physical_device_infos.pipeline_cache_uuid,
                          std::ranges::begin(m_serialized.uuid.value));

        return device.vkHandle()
            .createPipelineCache({})
            .transform(core::monadic::set(m_vk_pipeline_cache))
            .transform([this] noexcept -> void {
                ilog("Created new pipeline cache at {}", m_path.string());
            });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto PipelineCache::readPipelineCache(const Device& device) -> VulkanExpected<void> {
        const auto physical_device_infos = device.physicalDevice().info();

        auto stream = std::ifstream { m_path.string(), std::ios::binary };
        read(stream, asByteView(m_serialized.guard));
        read(stream, asByteView(m_serialized.infos));
        read(stream, asByteView(m_serialized.uuid.value));

        if (m_serialized.guard.magic != MAGIC) {
            elog("Invalid pipeline cache magic number, have {}, expected: {}",
                 m_serialized.guard.magic,
                 MAGIC);

            return createNewPipelineCache(device);
        }

        if (m_serialized.infos.version != VERSION) {
            elog("Mismatch pipeline cache version, have {}, expected: {}",
                 m_serialized.infos.version,
                 VERSION);

            return createNewPipelineCache(device);
        }

        if (m_serialized.infos.vendor_id != physical_device_infos.vendor_id) {
            elog("Mismatch pipeline cache vendor id, have {:#06x}, expected: {:#06x}",
                 m_serialized.infos.vendor_id,
                 physical_device_infos.vendor_id);

            return createNewPipelineCache(device);
        }

        if (m_serialized.infos.device_id != physical_device_infos.device_id) {
            elog("Mismatch pipeline cache device id, have {:#06x}, expected: {:#06x}",
                 m_serialized.infos.device_id,
                 physical_device_infos.device_id);

            return createNewPipelineCache(device);
        }

        if (!std::equal(std::cbegin(m_serialized.uuid.value),
                        std::cend(m_serialized.uuid.value),
                        std::cbegin(physical_device_infos.pipeline_cache_uuid))) {
            elog("Mismatch pipeline cache device UUID");

            return createNewPipelineCache(device);
        }

        const auto data = read(stream, m_serialized.guard.data_size);

        const auto create_info = vk::PipelineCacheCreateInfo {}.setInitialData<Byte>(data);

        return device.vkHandle()
            .createPipelineCache(create_info)
            .transform(core::monadic::set(m_vk_pipeline_cache))
            .transform([this]() noexcept -> void {
                ilog("Loading pipeline cache {}", m_path.string());
                return;
            });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto PipelineCache::saveCache() -> void {
        auto data                    = m_vk_pipeline_cache->getData();
        m_serialized.guard.data_size = std::size(data);
        m_serialized.guard.data_hash = 0u;

        for (auto v : data) hashCombine(m_serialized.guard.data_hash, v);

        auto stream = std::ofstream { m_path.string(), std::ios::binary | std::ios::trunc };

        write(stream, asByteView(m_serialized.guard));

        write(stream, asByteView(m_serialized.infos));
        write(stream, asByteView(m_serialized.uuid.value));
        write(stream, asByteView(data));
        ilog("Saving pipeline cache at {}", m_path.string());
        // elog("Failed to save pipeline cache at {}, reason: {}", m_path.string(), error);
        // return error;
    }
} // namespace stormkit::gpu
