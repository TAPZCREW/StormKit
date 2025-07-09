// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/log/log_macro.hpp>

#include <volk.h>

module stormkit.gpu.execution;

import std;

import stormkit.core;
import stormkit.log;

import stormkit.gpu.core;

namespace stdr = stdr;

LOGGER("stormkit.gpu")

namespace stormkit::gpu {

    /////////////////////////////////////
    /////////////////////////////////////
    auto PipelineCache::create_new_pipeline_cache(const Device& device) noexcept
      -> VulkanExpected<void> {
        const auto physical_device_infos = device.physical_device().info();

        m_serialized.guard.magic     = MAGIC;
        m_serialized.guard.data_size = 0u;
        m_serialized.guard.data_hash = 0u;

        m_serialized.infos.version   = VERSION;
        m_serialized.infos.vendor_id = physical_device_infos.vendor_id;
        m_serialized.infos.device_id = physical_device_infos.device_id;

        stdr::copy(physical_device_infos.pipeline_cache_uuid, stdr::begin(m_serialized.uuid.value));

        const auto create_info = VkPipelineCacheCreateInfo {
            .sType           = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
            .pNext           = nullptr,
            .flags           = 0,
            .initialDataSize = 0,
            .pInitialData    = nullptr,
        };

        return vk_call<VkPipelineCache>(m_vk_device_table->vkCreatePipelineCache,
                                        m_vk_device,
                                        &create_info,
                                        nullptr)
          .transform(core::monadic::set(m_vk_handle))
          .transform([this] noexcept -> void {
              ilog("Created new pipeline cache at {}", m_path.string());
          });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto PipelineCache::read_pipeline_cache(const Device& device) noexcept -> VulkanExpected<void> {
        const auto physical_device_infos = device.physical_device().info();

        auto stream = std::ifstream { m_path.string(), std::ios::binary };
        io::read(stream, as_bytes(m_serialized.guard));
        io::read(stream, as_bytes(m_serialized.infos));
        io::read(stream, as_bytes(m_serialized.uuid.value));

        if (m_serialized.guard.magic != MAGIC) {
            elog("Invalid pipeline cache magic number, have {}, expected: {}",
                 m_serialized.guard.magic,
                 MAGIC);

            return create_new_pipeline_cache(device);
        }

        if (m_serialized.infos.version != VERSION) {
            elog("Mismatch pipeline cache version, have {}, expected: {}",
                 m_serialized.infos.version,
                 VERSION);

            return create_new_pipeline_cache(device);
        }

        if (m_serialized.infos.vendor_id != physical_device_infos.vendor_id) {
            elog("Mismatch pipeline cache vendor id, have {:#06x}, expected: {:#06x}",
                 m_serialized.infos.vendor_id,
                 physical_device_infos.vendor_id);

            return create_new_pipeline_cache(device);
        }

        if (m_serialized.infos.device_id != physical_device_infos.device_id) {
            elog("Mismatch pipeline cache device id, have {:#06x}, expected: {:#06x}",
                 m_serialized.infos.device_id,
                 physical_device_infos.device_id);

            return create_new_pipeline_cache(device);
        }

        if (!std::equal(std::cbegin(m_serialized.uuid.value),
                        std::cend(m_serialized.uuid.value),
                        std::cbegin(physical_device_infos.pipeline_cache_uuid))) {
            elog("Mismatch pipeline cache device UUID");

            return create_new_pipeline_cache(device);
        }

        const auto data = io::read(stream, m_serialized.guard.data_size);

        const auto create_info = VkPipelineCacheCreateInfo {
            .sType           = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
            .pNext           = nullptr,
            .flags           = 0,
            .initialDataSize = as<u32>(stdr::size(data)),
            .pInitialData    = stdr::data(data),
        };

        return vk_call<VkPipelineCache>(m_vk_device_table->vkCreatePipelineCache,
                                        m_vk_device,
                                        &create_info,
                                        nullptr)
          .transform(core::monadic::set(m_vk_handle))
          .transform([this]() noexcept -> void {
              ilog("Pipeline cache loaded from {}", m_path.string());
              return;
          });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto PipelineCache::save_cache() noexcept -> void {
        vk_enumerate<Byte, usize>(m_vk_device_table->vkGetPipelineCacheData,
                                  m_vk_device,
                                  m_vk_handle)
          .transform([this](auto&& data) noexcept {
              m_serialized.guard.data_size = stdr::size(data);
              m_serialized.guard.data_hash = 0u;

              for (auto v : data) hash_combine(m_serialized.guard.data_hash, v);

              auto stream = std::ofstream { m_path.string(), std::ios::binary | std::ios::trunc };

              io::write(stream, as_bytes(m_serialized.guard));

              io::write(stream, as_bytes(m_serialized.infos));
              io::write(stream, as_bytes(m_serialized.uuid.value));
              io::write(stream, as_bytes(data));
              ilog("Pipeline cache successfully saved at {}", m_path.string());
          })
          .transform_error([this](auto&& error) noexcept {
              elog("Failed to save pipeline cache at {}, reason: {}",
                   m_path.string(),
                   from_vk<Result>(error));
              return error;
          });
    }
} // namespace stormkit::gpu
