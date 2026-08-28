// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/try_expected.hpp>

#include <stormkit/gpu/vulkan.hpp>

module stormkit.gpu.execution;

import std;

import stormkit.core;

import stormkit.gpu.core;

namespace stdr = std::ranges;
namespace stdv = std::views;

using namespace std::literals;

namespace stormkit::gpu {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto DescriptorSetInterface<Base>::update(array_view<const Descriptor> descriptors) const noexcept -> void {
        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        auto&& [_, _, _writes] = [this, descriptors = std::move(descriptors)] noexcept -> decltype(auto) {
            auto buffers = dyn_array<VkDescriptorBufferInfo> {};
            auto images  = dyn_array<VkDescriptorImageInfo> {};
            auto writes  = dyn_array<VkWriteDescriptorSet> {};
            buffers.reserve(std::size(descriptors));
            images.reserve(std::size(descriptors));
            writes.reserve(std::size(descriptors));

            stdr::for_each(std::move(descriptors),
                           core::monadic::either(
                             [this, &buffers, &writes](const BufferDescriptor& descriptor) noexcept -> decltype(auto) {
                                 buffers.push_back(VkDescriptorBufferInfo {
                                   .buffer = descriptor.buffer,
                                   .offset = descriptor.offset,
                                   .range  = descriptor.range.value_or(VK_WHOLE_SIZE),
                                 });
                                 const auto& buffer_descriptor = buffers.back();

                                 writes.push_back(VkWriteDescriptorSet {
                                   .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                   .pNext            = nullptr,
                                   .dstSet           = *this,
                                   .dstBinding       = descriptor.binding,
                                   .dstArrayElement  = 0,
                                   .descriptorCount  = 1,
                                   .descriptorType   = vk::to_vk<VkDescriptorType>(descriptor.type),
                                   .pImageInfo       = nullptr,
                                   .pBufferInfo      = &buffer_descriptor,
                                   .pTexelBufferView = nullptr,
                                 });
                             },
                             [this, &images, &writes](const ImageDescriptor& descriptor) noexcept -> decltype(auto) {
                                 images.push_back(VkDescriptorImageInfo {
                                   .sampler     = descriptor.sampler,
                                   .imageView   = descriptor.image_view,
                                   .imageLayout = vk::to_vk<VkImageLayout>(descriptor.layout),
                                 });
                                 const auto& image_descriptor = images.back();

                                 writes.push_back(VkWriteDescriptorSet {
                                   .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                   .pNext            = nullptr,
                                   .dstSet           = *this,
                                   .dstBinding       = descriptor.binding,
                                   .dstArrayElement  = 0,
                                   .descriptorCount  = 1,
                                   .descriptorType   = vk::to_vk<VkDescriptorType>(descriptor.type),
                                   .pImageInfo       = &image_descriptor,
                                   .pBufferInfo      = nullptr,
                                   .pTexelBufferView = nullptr,
                                 });
                             }));

            return std::tuple { std::move(buffers), std::move(images), std::move(writes) };
        }();

        vk::call(device_table.vkUpdateDescriptorSets, device, stdr::size(_writes), stdr::data(_writes), 0, nullptr);
    }

    template class DescriptorSetInterface<DescriptorSetImplementation>;
    template class DescriptorSetInterface<view::DescriptorSetImplementation>;

    /////////////////////////////////////
    /////////////////////////////////////
    auto DescriptorSetImplementation::do_init(PrivateTag,
                                              VkDescriptorSet&&      descriptor_set,
                                              DescriptorSetDeleter&& deleter) noexcept -> Expected<void> {
        m_vk_handle = std::move(descriptor_set);
        m_deleter   = std::move(deleter);

        return {};
    }
} // namespace stormkit::gpu
