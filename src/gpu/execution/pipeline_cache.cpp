// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/try_expected.hpp>

#include <stormkit/gpu/vulkan.hpp>

module stormkit.gpu.execution;

import std;

import stormkit.core;
import stormkit.log;

import stormkit.gpu.core;

namespace stdr  = std::ranges;
namespace stdfs = std::filesystem;

using namespace stormkit::literals;

namespace stormkit::gpu {
    namespace {
        /////////////////////////////////////
        /////////////////////////////////////
        auto sys_to_load_error(SystemError error) noexcept -> LoadSaveError {
            return LoadSaveError { { error } };
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto result_to_load_error(Result error) noexcept -> LoadSaveError {
            return LoadSaveError { { error } };
        }
    } // namespace

    template class PipelineCacheInterface<PipelineCacheImplementation>;
    template class PipelineCacheInterface<view::PipelineCacheImplementation>;

    /////////////////////////////////////
    /////////////////////////////////////
    auto PipelineCacheImplementation::do_init(PrivateTag, stdfs::path&& path) noexcept -> LoadSaveExpected<void> {
        m_path = std::move(path);
        Return read_pipeline_cache();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto PipelineCacheImplementation::create_new_pipeline_cache() noexcept -> LoadSaveExpected<void> {
        const auto& device                = owner();
        const auto& device_table          = device.device_table();
        const auto& physical_device_infos = device.physical_device().info();

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

        m_vk_handle = TryTransformError(vk::call_checked<
                                          VkPipelineCache>(device_table.vkCreatePipelineCache, device, &create_info, nullptr),
                                        result_to_load_error);

        Return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto PipelineCacheImplementation::read_pipeline_cache() noexcept -> LoadSaveExpected<void> {
        if (not stdfs::exists(m_path)) Return create_new_pipeline_cache();

        const auto& device                = owner();
        const auto& device_table          = device.device_table();
        const auto& physical_device_infos = device.physical_device().info();

        auto file = TryTransform(io::File::open(m_path, io::Access::READ), sys_to_load_error);
        TryTransform(file.read_to(as_mutable_bytes(m_serialized.guard)), sys_to_load_error);
        TryTransform(file.read_to(as_mutable_bytes(m_serialized.infos)), sys_to_load_error);
        TryTransform(file.read_to(as_mutable_bytes(m_serialized.uuid.value)), sys_to_load_error);

        if (m_serialized.guard.magic != MAGIC) Return create_new_pipeline_cache();
        if (m_serialized.infos.version != VERSION) Return create_new_pipeline_cache();
        if (m_serialized.infos.vendor_id != physical_device_infos.vendor_id) Return create_new_pipeline_cache();
        if (m_serialized.infos.device_id != physical_device_infos.device_id) Return create_new_pipeline_cache();
        if (not stdr::equal(m_serialized.uuid.value, physical_device_infos.pipeline_cache_uuid))
            Return create_new_pipeline_cache();

        auto data = byte_dyn_array {};
        data.resize(m_serialized.guard.data_size);

        TryTransform(io::read_to(m_path, data), sys_to_load_error);

        const auto create_info = VkPipelineCacheCreateInfo {
            .sType           = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
            .pNext           = nullptr,
            .flags           = 0,
            .initialDataSize = as<u32>(stdr::size(data)),
            .pInitialData    = stdr::data(data),
        };

        m_vk_handle = TryTransform(vk::call_checked<
                                          VkPipelineCache>(device_table.vkCreatePipelineCache, device, &create_info, nullptr),
                                        result_to_load_error);

        Return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto PipelineCacheImplementation::save_cache() noexcept -> LoadSaveExpected<void> {
        const auto& device       = owner();
        const auto& device_table = device.device_table();

        auto size = 0_usize;
        TryTransform(vk::call_checked(device_table.vkGetPipelineCacheData, device, m_vk_handle, &size, nullptr),
                          result_to_load_error);
        auto data = byte_dyn_array {};
        data.resize(size, 0_b);
        TryTransform(vk::call_checked(device_table.vkGetPipelineCacheData, device, m_vk_handle, &size, stdr::data(data)),
                          result_to_load_error);

        m_serialized.guard.data_size = stdr::size(data);
        m_serialized.guard.data_hash = 0u;

        hash_combine(m_serialized.guard.data_hash, data);

        auto file = TryTransform(io::File::open(m_path, io::Access::WRITE), sys_to_load_error);
        TryTransform(file.write(as_bytes(m_serialized.infos)), sys_to_load_error);
        TryTransform(file.write(as_bytes(m_serialized.uuid.value)), sys_to_load_error);
        TryTransform(file.write(as_bytes(data)), sys_to_load_error);

        Return {};
    }
} // namespace stormkit::gpu
