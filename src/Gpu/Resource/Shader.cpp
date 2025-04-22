// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module stormkit.Gpu;

import std;

import stormkit.core;

import :Core;
import stormkit.Gpu.Vulkan;

import :Resource.Shader;

namespace stormkit::gpu {
    /////////////////////////////////////
    /////////////////////////////////////
    auto Shader::reflect() noexcept -> void {
#ifdef STORMKIT_ENABLE_SPIRV_INTROSPECT
        auto ir = std::vector<SpirvID> {};
        ir.resize(std::size(m_source) / sizeof(SpirvID));
        std::memcpy(std::data(ir), std::data(m_source), std::size(m_source));

        auto       compiler     = spirv_cross::CompilerGLSL { std::move(ir) };
        const auto add_bindings = [this, &compiler](span<const spirv_cross::Resource> resources,
                                                    gpu::DescriptorType               type) {
            for (const auto& resource : resources) {
                /*const auto set =
                    spvc_compiler_get_decoration(compiler, resources[i].id,
                   SpvDecorationDescriptorSet);*/
                const auto binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
                // const auto name = spvc_compiler_get_name(compiler, resources[i].id);

                m_descriptor_set_layout.addBinding({ binding,
                                                     type,
                                                     gpu::ShaderStageFlag::Vertex
                                                         | gpu::ShaderStageFlag::Fragment
                                                         | gpu::ShaderStageFlag::Compute,
                                                     1 });
            }
        };

        auto resources = compiler.get_shader_resources();
        add_bindings(resources.uniform_buffers, DescriptorType::Uniform_Buffer);
        add_bindings(resources.storage_buffers, DescriptorType::Storage_Buffer);
        add_bindings(resources.sampled_images, DescriptorType::Sampled_Image);
        add_bindings(resources.storage_images, DescriptorType::Storage_Image);

#endif
        // m_descriptor_set_layout.bake();
    }
} // namespace stormkit::gpu
