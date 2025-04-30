// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module stormkit.Engine;

import std;

import stormkit.core;
import stormkit.Gpu;

import :Renderer.FrameGraph;

namespace stormkit::engine {
    //////////////////////////////////////
    //////////////////////////////////////
    auto FrameGraphBuilder::bake() -> void {
        expects(not m_baked);

        for (auto& task : m_tasks)
            task.m_ref_count = std::size(task.creates()) + std::size(task.writes());

        for (auto& resource : m_resources)
            std::visit([](auto& resource) { resource.m_ref_count = std::size(resource.readers()); },
                       resource);

        cullUnreferencedResources();
        buildPhysicalDescriptions();

        m_baked = true;
    }

    //////////////////////////////////////
    //////////////////////////////////////
    auto FrameGraphBuilder::createFrameGraph(const gpu::Device&      device,
                                             const gpu::CommandPool& command_pool,
                                             BakedFrameGraph*        old) -> BakedFrameGraph {
        auto&& [backbuffer, data] = allocatePhysicalResources(command_pool, device);

        return BakedFrameGraph { backbuffer, std::move(data), old };
    }

    //////////////////////////////////////
    //////////////////////////////////////
    auto FrameGraphBuilder::allocateFrameGraph(const gpu::Device&      device,
                                               const gpu::CommandPool& command_pool,
                                               BakedFrameGraph*        old)
        -> std::unique_ptr<BakedFrameGraph> {
        auto&& [backbuffer, data] = allocatePhysicalResources(command_pool, device);

        return allocate<BakedFrameGraph>(backbuffer, std::move(data), old);
    }

    //////////////////////////////////////
    //////////////////////////////////////
    auto FrameGraphBuilder::prepareTask(GraphTask& task) noexcept -> void {
        auto task_builder = GraphTaskBuilder { task, *this };
        auto data         = m_datas[task.dataID()];
        task.onSetup(*std::bit_cast<Byte*>(&data), task_builder);
    }

    //////////////////////////////////////
    //////////////////////////////////////
    auto FrameGraphBuilder::cullUnreferencedResources() noexcept -> void {
        auto unreferenced_resources = std::stack<Ref<GraphResourceVariant>> {};

        constexpr auto decrementRefcount = [](auto& value) noexcept {
            if (value.m_ref_count > 0) --value.m_ref_count;
        };

        constexpr auto isUnreferenced = [](const auto& resource) noexcept {
            return resource.refCount() == 0 and resource.transient();
        };

        constexpr auto shouldCull = [](const auto& task) noexcept {
            return task.refCount() == 0 and not task.cullImune();
        };

        const auto cull = [&decrementRefcount, &isUnreferenced, &unreferenced_resources, this](
                              auto& task) noexcept {
            for (const auto id : task.reads()) {
                auto& resource = getResource(id);

                if (std::visit(
                        [&decrementRefcount, &isUnreferenced](auto& value) {
                            decrementRefcount(value);
                            return isUnreferenced(value);
                        },
                        resource))
                    unreferenced_resources.push(resource);
            }
        };

        for (auto& resource : m_resources)
            if (std::visit(isUnreferenced, resource)) unreferenced_resources.push(resource);

        while (!std::empty(unreferenced_resources)) {
            auto resource = unreferenced_resources.top();
            unreferenced_resources.pop();

            auto& creator
                = getTask(std::visit([](auto& resource) noexcept { return resource.creator(); },
                                     resource.get()));
            decrementRefcount(creator);

            if (shouldCull(creator)) cull(creator);

            for (const auto id : creator.writes()) {
                auto& writer = getTask(id);

                decrementRefcount(writer);
                if (shouldCull(writer)) cull(writer);
            }
        }
    }

    //////////////////////////////////////
    //////////////////////////////////////
    auto FrameGraphBuilder::buildPhysicalDescriptions() noexcept -> void {
        auto layouts = HashMap<GraphID, gpu::ImageLayout> {};
        m_preprocessed_framegraph
            = m_tasks
              | std::views::filter([](const auto& task) noexcept {
                    return not(task.refCount() == 0 and not task.cullImune());
                })
              | std::views::transform(
                  [&layouts, this](const auto& task) noexcept -> decltype(auto) {
                      return Pass { .id         = task.id(),
                                    .type       = task.type(),
                                    .renderpass = buildRenderPassPhysicalDescription(task, layouts),
                                    .name       = task.name(),
                                    .buffers    = buildBufferPhysicalDescriptions(task),
                                    .images     = buildImagePhysicalDescriptions(task) };
                  })
              | std::ranges::to<std::vector>();
    }

    //////////////////////////////////////
    //////////////////////////////////////
    auto FrameGraphBuilder::buildImagePhysicalDescriptions(const GraphTask& task) noexcept
        -> std::vector<ImageInfo> {
        return task.creates()
               | std::views::filter([this](const auto& id) noexcept {
                     const auto& resource = getResource(id);

                     return is<GraphImage>(resource)
                            and getResource<ImageDescription>(id).transient();
                 })
               | std::views::transform([this](const auto& id) noexcept -> decltype(auto) {
                     const auto& resource    = getResource<ImageDescription>(id);
                     const auto& description = resource.description();

                     const auto usages = [&description] noexcept {
                         if (gpu::isDepthStencilFormat(description.format))
                             return gpu::ImageUsageFlag::Depth_Stencil_Attachment
                                    | gpu::ImageUsageFlag::Transfert_Src;

                         return gpu::ImageUsageFlag::Color_Attachment
                                | gpu::ImageUsageFlag::Transfert_Src;
                     }();

                     const auto clear_value = [&description] noexcept -> gpu::ClearValue {
                         if (gpu::isDepthStencilFormat(description.format))
                             return gpu::ClearDepthStencil {};

                         return gpu::ClearColor {};
                     }();

                     const auto& name = resource.name();

                     return ImageInfo {
                         .id = id,
                         .create_info =
                             gpu::Image::CreateInfo {
                                                     .extent = description.extent,
                                                     .format = description.format,
                                                     .layers = description.layers,
                                                     .type   = description.type,
                                                     .usages = usages,
                                                     },
                         .clear_value = clear_value,
                         .name        = name
                     };
                 })
               | std::ranges::to<std::vector>();
    }

    //////////////////////////////////////
    //////////////////////////////////////
    auto FrameGraphBuilder::buildBufferPhysicalDescriptions(const GraphTask& task) noexcept
        -> std::vector<BufferInfo> {
        return task.creates()
               | std::views::filter([this](const auto& id) noexcept {
                     const auto& resource = getResource(id);

                     return is<GraphBuffer>(resource)
                            and getResource<BufferDescription>(id).transient();
                 })
               | std::views::transform([this](const auto& id) noexcept -> decltype(auto) {
                     const auto& resource    = getResource<BufferDescription>(id);
                     const auto& description = resource.description();

                     const auto usages
                         = gpu::BufferUsageFlag::Transfert_Src | gpu::BufferUsageFlag::Storage;

                     const auto& name = resource.name();

                     return BufferInfo {
                         .id = id,
                         .create_info
                         = gpu::Buffer::CreateInfo { .usages = usages, .size = description.size },
                         //.setMemoryProperty(gpu::MemoryPropertyFlag::eDeviceLocal),
                         .name = name
                     };
                 })
               | std::ranges::to<std::vector>();
    }

    //////////////////////////////////////
    //////////////////////////////////////
    auto FrameGraphBuilder::buildRenderPassPhysicalDescription(
        const GraphTask&                    task,
        HashMap<GraphID, gpu::ImageLayout>& layouts) noexcept -> RenderPassData {
        auto to_remove = std::vector<GraphID> {};

        const auto creates
            = task.creates()
              | std::views::filter([this](const auto resource_id) noexcept {
                    const auto& resource = getResource(resource_id);
                    return is<GraphImage>(resource);
                })
              | std::views::transform([&, this](const auto id) noexcept {
                    const auto& resource    = getResource<ImageDescription>(id);
                    const auto& description = resource.description();

                    auto attachment_description = gpu::AttachmentDescription {
                        .format             = description.format,
                        .load_op            = gpu::AttachmentLoadOperation::Clear,
                        .store_op           = gpu::AttachmentStoreOperation::Store,
                        .stencil_load_op    = gpu::AttachmentLoadOperation::Dont_Care,
                        .stencil_store_op   = gpu::AttachmentStoreOperation::Dont_Care,
                        .source_layout      = gpu::ImageLayout::UNDEFINED,
                        .destination_layout = gpu::ImageLayout::Color_Attachment_Optimal
                    };

                    if (isDepthStencilFormat(description.format)) [[unlikely]] {
                        std::swap(attachment_description.load_op,
                                  attachment_description.stencil_load_op);
                        std::swap(attachment_description.store_op,
                                  attachment_description.stencil_store_op);
                        attachment_description.destination_layout
                            = gpu::ImageLayout::Depth_Stencil_Attachment_Optimal;
                    }

                    layouts[id] = attachment_description.destination_layout;

                    return attachment_description;
                })
              | std::ranges::to<std::vector>();

        const auto writes = task.writes()
                            | std::views::filter([this](const auto id) noexcept {
                                  const auto& resource = getResource(id);
                                  return is<GraphImage>(resource);
                              })
                            | std::views::transform([&, this](const auto id) {
                                  const auto& resource    = getResource<ImageDescription>(id);
                                  const auto& description = resource.description();

                                  auto attachment_description = gpu::AttachmentDescription {
                                      .format           = description.format,
                                      .load_op          = gpu::AttachmentLoadOperation::Clear,
                                      .store_op         = gpu::AttachmentStoreOperation::Store,
                                      .stencil_load_op  = gpu::AttachmentLoadOperation::Dont_Care,
                                      .stencil_store_op = gpu::AttachmentStoreOperation::Dont_Care,
                                      .source_layout    = layouts.at(id),
                                      .destination_layout = layouts.at(id)
                                  };

                                  if (isDepthStencilFormat(description.format)) [[unlikely]] {
                                      std::swap(attachment_description.load_op,
                                                attachment_description.stencil_load_op);
                                      std::swap(attachment_description.store_op,
                                                attachment_description.stencil_store_op);
                                      attachment_description.destination_layout
                                          = gpu::ImageLayout::Depth_Stencil_Attachment_Optimal;
                                  }

                                  layouts[id] = attachment_description.destination_layout;

                                  return attachment_description;
                              })
                            | std::ranges::to<std::vector>();

        const auto reads = task.reads()
                           | std::views::filter([this](const auto id) noexcept {
                                 const auto& resource = getResource(id);
                                 return is<GraphImage>(resource);
                             })
                           | std::views::transform([&, this](const auto id) {
                                 const auto& resource    = getResource<ImageDescription>(id);
                                 const auto& description = resource.description();

                                 auto attachment_description = gpu::AttachmentDescription {
                                     .format             = description.format,
                                     .load_op            = gpu::AttachmentLoadOperation::Load,
                                     .store_op           = gpu::AttachmentStoreOperation::Dont_Care,
                                     .stencil_load_op    = gpu::AttachmentLoadOperation::Dont_Care,
                                     .stencil_store_op   = gpu::AttachmentStoreOperation::Dont_Care,
                                     .source_layout      = layouts.at(id),
                                     .destination_layout = layouts.at(id)
                                 };

                                 if (std::ranges::any_of(task.writes(), monadic::is(id))) {
                                     to_remove.emplace_back(id);
                                     attachment_description.store_op
                                         = gpu::AttachmentStoreOperation::Store;
                                 }

                                 if (isDepthStencilFormat(description.format)) [[unlikely]] {
                                     std::swap(attachment_description.load_op,
                                               attachment_description.stencil_load_op);
                                     std::swap(attachment_description.store_op,
                                               attachment_description.stencil_store_op);
                                     attachment_description.destination_layout
                                         = gpu::ImageLayout::Depth_Stencil_Attachment_Optimal;
                                 }

                                 layouts[id] = attachment_description.destination_layout;

                                 return attachment_description;
                             })
                           | std::ranges::to<std::vector>();

        auto output = RenderPassData {};
        output.description.attachments
            = move_and_concat(std::move(creates), std::move(reads), std::move(writes));

        auto color_refs = std::vector<gpu::Subpass::Ref> {};
        color_refs.reserve(std::size(output.description.attachments));

        auto depth_attachment_ref = std::optional<gpu::Subpass::Ref> {};
        for (auto&& [i, attachment] : output.description.attachments | std::views::enumerate) {
            if (isDepthFormat(attachment.format))
                depth_attachment_ref
                    = gpu::Subpass::Ref { .attachment_id = as<UInt32>(i),
                                          .layout        = attachment.destination_layout };
            else
                color_refs.emplace_back(
                    gpu::Subpass::Ref { .attachment_id = as<UInt32>(i),
                                        .layout        = attachment.destination_layout });
        }

        // TODO support multiple subpasses
        output.description.subpasses.emplace_back(gpu::Subpass {
            .bind_point = task.type() == GraphTask::Type::Raster ? gpu::PipelineBindPoint::Graphics
                                                                 : gpu::PipelineBindPoint::Compute,
            .color_attachment_refs = std::move(color_refs),
            .depth_attachment_ref  = std::move(depth_attachment_ref) });

        return output;
    }

    auto FmameGraph::allocatePhysicalResources(const gpu::CommandPool& command_pool,
                                               const gpu::Device&      device) -> void {
        auto output = BakedFrameGraph {};
        output.cmb  = command_pool.createCommandBuffer(device);
        device.setObjectName(*output.cmb, "FrameGraph:CommandBuffer:Main");

        output.semaphore = *gpu::Semaphore::create(device);
        device.setObjectName(*output.semaphore, "FrameGraph:Semaphore:Main");

        output.fence = *gpu::Fence::createSignaled(device);
        device.setObjectName(*output.fence, "FrameGraph:Fence:Main");

        output.tasks.reserve(std::size(m_preprocessed_framegraph));

        // TODO support of async Compute and Transfert queue
        for (auto&& pass : m_preprocessed_framegraph) {
            output.buffers.reserve(std::size(output.buffers) + std::size(pass.buffers));
            output.images.reserve(std::size(output.images) + std::size(pass.images));
            output.image_views.reserve(std::size(output.image_views) + std::size(pass.images));

            for (auto&& buffer : pass.buffers) {
                auto& gpu_buffer
                    = output.buffers.emplace_back(gpu::Buffer::create(device, buffer.create_info)
                                                      .transform_error(expects())
                                                      .value());
                device.setObjectName(gpu_buffer, std::format("FrameGraph:Buffer:{}", buffer.name));
            }

            auto extent       = math::ExtentU {};
            auto clear_values = std::vector<gpu::ClearValue> {};
            auto attachments  = std::vector<Ref<const gpu::ImageView>> {};
            for (const auto& image : pass.images) {
                extent.width  = std::max(image.create_info.extent.width, extent.width);
                extent.height = std::max(image.create_info.extent.height, extent.height);

                clear_values.emplace_back(image.clear_value);
                auto& gpu_image
                    = output.images.emplace_back(gpu::Image::create(device, image.create_info)
                                                     .transform_error(expects())
                                                     .value());
                device.setObjectName(gpu_image, std::format("FrameGraph:Image:{}", image.name));

                if (image.id == m_final_resource) output.backbuffer = as_ref(gpu_image);

                auto& gpu_image_view = output.image_views.emplace_back(
                    gpu::ImageView::create(device, gpu_image).transform_error(expects()).value());
                device.setObjectName(gpu_image_view,
                                     std::format("FrameGraph:ImageView:{}", image.name));

                attachments.emplace_back(gpu_image_view);
            }

            expects(backbuffer != std::nullopt, "No final resource set !");

            auto renderpass = *gpu::RenderPass::create(device, pass.renderpass.description);
            device.setObjectName(renderpass, std::format("FrameGraph:RenderPass:{}", pass.name));

            auto framebuffer = *gpu::FrameBuffer::create(device, renderpass, extent, attachments);
            device.setObjectName(framebuffer, std::format("FrameGraph:FrameBuffer:{}", pass.name));

            auto cmb = command_pool.createCommandBuffer(device, gpu::CommandBufferLevel::Secondary);
            device.setObjectName(cmb, std::format("FrameGraph:CommandBuffer:{}", pass.name));

            cmb.begin(false, gpu::InheritanceInfo { &renderpass, 0, &framebuffer });
            auto&& graph_task = getTask(pass.id);
            graph_task.onExecute(m_datas[pass.id].front(), &renderpass, cmb);
            cmb.end();

            output.tasks.emplace_back(
                BakedFrameGraphBuilder::RasterTask { .id           = pass.id,
                                                     .cmb          = std::move(cmb),
                                                     .clear_values = std::move(clear_values),
                                                     .renderpass   = std::move(renderpass),
                                                     .framebuffer  = std::move(framebuffer) });
        }

        output.cmb->begin();
        const auto visitors
            = Overloaded { [&output](const BakedFrameGraphBuilder::Data::RasterTask& task) {
                              output.cmb->beginRenderPass(task.renderpass,
                                                          task.framebuffer,
                                                          task.clear_values,
                                                          true);

                              const auto command_buffers = as_refs<std::array>(task.cmb);
                              output.cmb->executeSubCommandBuffers(command_buffers);
                              output.cmb->endRenderPass();
                          },
                           [&output](const BakedFrameGraphBuilder::Data::ComputeTask& task) {
                               const auto command_buffers = as_refs<std::array>(task.cmb);
                               output.cmb->executeSubCommandBuffers(command_buffers);
                           } };
        for (auto&& task : output.tasks) std::visit(visitors, task);
        output.cmb->end();

        return { std::in_place_t, std::move(output) };
    } // namespace stormkit::engine
} // namespace stormkit::engine
