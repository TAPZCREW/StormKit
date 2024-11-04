#ifdef STORMKIT_BUILD_MODULES
module Renderer;

import Constants;
#else
    #include <stormkit/std.hpp>

    #include <stormkit/Core.hpp>

    #include "Constants.mpp"
    #include "Renderer.mpp"
#endif

using namespace stormkit;

Renderer::Renderer(const wsi::Window& window) : m_window { &window } {
    doInitBaseRenderObjects();
    doInitMeshRenderObjects();
}

Renderer::~Renderer() {
    m_device->waitIdle();
}

Renderer::Renderer(Renderer&&) noexcept = default;

auto Renderer::operator=(Renderer&&) noexcept -> Renderer& = default;

auto Renderer::renderFrame() -> void {
    const auto& surface_extent  = m_surface->extent();
    const auto  surface_extentf = math::ExtentF { surface_extent };

    if (m_surface->needRecreate()) {
        m_surface->recreate();
        doInitPerFrameObjects();
    }

    const auto viewports = [&] {
        auto v = std::vector<gpu::Viewport> {};
        v.emplace_back(gpu::Viewport {
            .extent = surface_extentf,
            .depth  = { 0, 1 }
        });

        return v;
    }();

    const auto scissors = [&] {
        auto s = std::vector<gpu::Scissor> {};
        s.emplace_back(gpu::Scissor { .extent = surface_extent });

        return s;
    }();

    auto sets = makeConstObserverStaticArray(m_board.descriptor_set);

    auto  frame         = std::move(m_surface->acquireNextFrame().value());
    auto& framebuffer   = m_framebuffers[frame.image_index];
    auto& commandbuffer = m_command_buffers[frame.image_index];

    auto wait   = makeConstObserverStaticArray(frame.image_available);
    auto signal = makeConstObserverStaticArray(frame.render_finished);

    m_current_fence = frame.in_flight;

    commandbuffer.reset();
    commandbuffer.begin();
    commandbuffer.beginRenderPass(*m_render_pass, framebuffer);
    commandbuffer.bindRasterPipeline(*m_board.pipeline);
    commandbuffer.setViewport(0, viewports);
    commandbuffer.setScissor(0, scissors);
    commandbuffer.bindDescriptorSets(*m_board.pipeline, sets, {});

    commandbuffer.draw(4);

    commandbuffer.endRenderPass();
    commandbuffer.end();
    commandbuffer.submit(wait, signal, frame.in_flight);

    m_surface->present(frame);
}

auto Renderer::updateBoard(const stormkit::image::Image& board) -> void {
    m_board.current_image += 1;
    m_board.current_image %= std::size(m_board.images);

    m_board.images[m_board.current_image].loadFromImage(board);

    if (m_current_fence) m_current_fence->wait();

    const auto descriptors = makeStaticArray(gpu::Descriptor {
        gpu::ImageDescriptor { .type    = gpu::DescriptorType::Combined_Image_Sampler,
                              .binding = 0,
                              .layout  = gpu::ImageLayout::Shader_Read_Only_Optimal,
                              .image_view
                               = makeConstObserver(m_board.image_views[m_board.current_image]),
                              .sampler = makeConstObserver(m_board.sampler) }
    });
    m_board.descriptor_set->update(descriptors);
}

auto Renderer::doInitBaseRenderObjects() -> void {
    // We create an instance and initialize device on best available GPU
    m_instance = makeUnique<gpu::Instance>();
    ilog("Render backend successfully initialized");
    ilog("Using StormKit {}.{}.{} {} {}, built with {}",
         STORMKIT_MAJOR_VERSION,
         STORMKIT_MINOR_VERSION,
         STORMKIT_PATCH_VERSION,
         STORMKIT_GIT_BRANCH,
         STORMKIT_GIT_COMMIT_HASH,
         STORMKIT_COMPILER);

    ilog("--------- Physical Devices ----------");
    for (const auto& device : m_instance->physicalDevices()) ilog("{}", device.info());

    auto surface = m_instance->allocateWindowSurface(*m_window);

    const auto& physical_device = m_instance->pickPhysicalDevice(*surface);

    m_surface = std::move(surface);

    const auto& physical_device_info = physical_device.info();

    ilog("Using physical device {} ({:#06x})",
         physical_device_info.device_name,
         physical_device_info.device_id);

    m_device = physical_device.allocateLogicalDevice();

    m_surface->initialize(*m_device);

    m_queue           = makeConstObserver(m_device->graphicsQueue());
    m_command_buffers = m_queue->createCommandBuffers(m_surface->bufferingCount());
}

auto Renderer::doInitMeshRenderObjects() -> void {
    const auto& surface_extent  = m_surface->extent();
    const auto  surface_extentf = math::ExtentF { surface_extent };

    m_board.vertex_shader   = m_device->allocateShader(SHADER_DATA, gpu::ShaderStageFlag::Vertex);
    m_board.fragment_shader = m_device->allocateShader(SHADER_DATA, gpu::ShaderStageFlag::Fragment);

    const auto description
        = gpu::RenderPassDescription { .attachments = { { .format = m_surface->pixelFormat() } },
                                       .subpasses
                                       = { { .bind_point      = gpu::PipelineBindPoint::Graphics,
                                             .attachment_refs = { { .attachment_id = 0u } } } } };

    m_descriptor_set_layout = m_device->allocateDescriptorSetLayout();
    m_descriptor_set_layout->addBinding({ .binding = 0,
                                          .type    = gpu::DescriptorType::Combined_Image_Sampler,
                                          .stages  = gpu::ShaderStageFlag::Fragment,
                                          .descriptor_count = 1 });
    m_descriptor_set_layout->bake();
    m_descriptor_pool = m_device->allocateDescriptorPool(
        std::array {
            gpu::DescriptorPool::Size { gpu::DescriptorType::Combined_Image_Sampler, 1 }
    },
        1);

    m_render_pass = m_device->allocateRenderPass(description);

    m_board.pipeline = m_device->allocateRasterPipeline();
    const auto state = gpu::RasterPipelineState {
        .input_assembly_state = { .topology = gpu::PrimitiveTopology::Triangle_Strip },
        .viewport_state
        = { .viewports = { gpu::Viewport { .position = { 0.f, 0.f },
                                           .extent   = surface_extentf,
                                           .depth    = { 0.f, 1.f } } },
                                 .scissors  = { gpu::Scissor { .offset = { 0, 0 }, .extent = surface_extent } } },
        .color_blend_state
        = { .attachments = { { .blend_enable           = true,
                               .src_color_blend_factor = gpu::BlendFactor::Src_Alpha,
                               .dst_color_blend_factor = gpu::BlendFactor::One_Minus_Src_Alpha,
                               .src_alpha_blend_factor = gpu::BlendFactor::Src_Alpha,
                               .dst_alpha_blend_factor = gpu::BlendFactor::One_Minus_Src_Alpha,
                               .alpha_blend_operation  = gpu::BlendOperation::Add } } },
        .dynamic_state = { { gpu::DynamicState::Viewport, gpu::DynamicState::Scissor } },
        .shader_state
        = { .shaders = makeConstObserverArray(m_board.vertex_shader, m_board.fragment_shader) },
        /*.vertex_input_state   = { .binding_descriptions =
                                    toArray(MESH_VERTEX_BINDING_DESCRIPTIONS),
                                  .input_attribute_descriptions =
                                    toArray(MESH_VERTEX_ATTRIBUTE_DESCRIPTIONS) },*/
        .layout = { .descriptor_set_layouts = makeConstObserverArray(m_descriptor_set_layout) }
    };

    m_board.pipeline->setState(state);
    m_board.pipeline->setRenderPass(*m_render_pass);
    m_board.pipeline->bake();

    for (auto i : range(BOARD_BUFFERING_COUNT)) {
        auto& img = m_board.images.emplace_back(*m_device,
                                                gpu::Image::CreateInfo {
                                                    .extent = { BOARD_SIZE, BOARD_SIZE }
        });

        m_board.image_views.emplace_back(img.createView());
    }

    m_board.sampler = m_device->allocateSampler(
        gpu::Sampler::Settings { .mag_filter     = gpu::Filter::Nearest,
                                 .min_filter     = gpu::Filter::Nearest,
                                 .address_mode_u = gpu::SamplerAddressMode::Clamp_To_Edge,
                                 .address_mode_v = gpu::SamplerAddressMode::Clamp_To_Edge });

    m_board.descriptor_set = m_descriptor_pool->allocateDescriptorSet(*m_descriptor_set_layout);

    doInitPerFrameObjects();
}

auto Renderer::doInitPerFrameObjects() -> void {
    const auto& surface_extent  = m_surface->extent();
    const auto  surface_extentf = math::ExtentF { surface_extent };
    const auto  buffering_count = m_surface->bufferingCount();

    m_surface_views.clear();
    m_surface_views.reserve(std::size(m_surface->images()));
    for (const auto& image : m_surface->images()) m_surface_views.emplace_back(image.createView());

    m_framebuffers.clear();
    m_framebuffers.reserve(buffering_count);

    for (auto i : range(buffering_count)) {
        const auto& image_view  = m_surface_views[i];
        auto        attachments = makeConstRefArray(image_view);

        m_framebuffers.emplace_back(*m_render_pass, surface_extent, std::move(attachments));
    }
}
