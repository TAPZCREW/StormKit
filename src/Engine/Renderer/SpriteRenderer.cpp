module;

#include <cstddef>

module stormkit.Engine;

import std;

import stormkit.core;
import stormkit.Gpu;

import :SpriteRenderer;

namespace stormkit::engine {
    namespace {
        constexpr auto Quad_Sprite_Shader = as_bytes(
#include <QuadSprite.spv.h>
        );

        constexpr auto Sprite_Vertex_Size                 = sizeof(SpriteVertex);
        constexpr auto Sprite_Vertex_Binding_Descriptions = std::array {
            gpu::VertexBindingDescription { .binding = 0, .stride = Sprite_Vertex_Size }
        };
        constexpr auto Sprite_Vertex_Attribute_Descriptions = std::array {
            gpu::VertexInputAttributeDescription { .location = 0,
                                                  .binding  = 0,
                                                  .format   = gpu::format::Float2,
                                                  .offset   = offsetof(SpriteVertex, position) },
            gpu::VertexInputAttributeDescription { .location = 1,
                                                  .binding  = 0,
                                                  .format   = gpu::format::Float2,
                                                  .offset   = offsetof(SpriteVertex, uv)       }
        };

        constexpr auto Sprite_Vertex_Buffer_Size = Sprite_Vertex_Size * 4;
    } // namespace

    //////////////////////////////////////
    //////////////////////////////////////
    SpriteRenderer::SpriteRenderer(const Renderer& renderer, const math::ExtentF& viewport, Tag)
        : m_renderer { as_ref(renderer) }, m_viewport { viewport } {
        m_render_data = allocate<RenderData>();
        gpu::Shader::fromBytes(renderer.device(), Quad_Sprite_Shader, gpu::ShaderStageFlag::Vertex)
            .transform(monadic::set(m_render_data->vertex_shader))
            .transform_error(monadic::throw_as_exception());

        gpu::Shader::fromBytes(renderer.device(),
                               Quad_Sprite_Shader,
                               gpu::ShaderStageFlag::Fragment)
            .transform(monadic::set(m_render_data->fragment_shader))
            .transform_error(monadic::throw_as_exception());

        gpu::PipelineLayout::create(std::cref(renderer.device()), gpu::RasterPipelineLayout {})
            .transform(monadic::set(m_render_data->pipeline_layout))
            .transform_error(monadic::throw_as_exception());

        m_render_data->pipeline_state = gpu::RasterPipelineState {
            .input_assembly_state = { .topology = gpu::PrimitiveTopology::Triangle_Strip },
            .viewport_state       = { .viewports = { { .extent = m_viewport, .depth = { 0, 1 } } },
                                     .scissors  = { { .extent = m_viewport } } },
            .color_blend_state    = { .attachments = { {} } },
            .shader_state         = { .shaders = as_refs<std::vector>(m_render_data->vertex_shader,
                                     m_render_data->fragment_shader) },
            .vertex_input_state
            = { .binding_descriptions = to_dyn_array(Sprite_Vertex_Binding_Descriptions),
                                     .input_attribute_descriptions
                = to_dyn_array(Sprite_Vertex_Attribute_Descriptions) },
        };
    }

    //////////////////////////////////////
    //////////////////////////////////////
    auto SpriteRenderer::updateFrameGraph(FrameGraphBuilder& graph) noexcept -> void {
        namespace views  = std::views;
        namespace ranges = std::ranges;

        struct GeometryTransferTask {
            Ref<GraphBuffer> staging_buffer;
            Ref<GraphBuffer> vertex_buffers;
        };

        if (not m_vertex_buffer) {
            m_vertex_buffer
                = gpu::Buffer::create(m_renderer->device(),
                                      { .usages = gpu::BufferUsageFlag::Vertex
                                                  | gpu::BufferUsageFlag::Transfer_Dst,
                                        .size     = Sprite_Vertex_Buffer_Size,
                                        .property = gpu::MemoryPropertyFlag::Device_Local })
                      .transform_error(monadic::assert())
                      .value();
        }

        auto& vertex_buffer
            = graph.setRetainedResource("StormKit:VertexBuffer",
                                        BufferDescription { .size = Sprite_Vertex_Buffer_Size },
                                        *m_vertex_buffer);

        auto transfer_task_data = OptionalRef<const GeometryTransferTask> {};
        if (not m_vertex_buffer) {
            m_vertex_buffer
                = gpu::Buffer::create(m_renderer->device(),
                                      { .usages = gpu::BufferUsageFlag::Vertex
                                                  | gpu::BufferUsageFlag::Transfer_Dst,
                                        .size     = Sprite_Vertex_Buffer_Size,
                                        .property = gpu::MemoryPropertyFlag::Device_Local })
                      .transform_error(monadic::assert())
                      .value();

            const auto& task = graph.addTransferTask<GeometryTransferTask>(
                "StormKit:SpriteGeometryTranferTask",
                [this](GeometryTransferTask& task_data, GraphTaskBuilder& builder) noexcept {
                    task_data.staging_buffer = as_ref_mut(
                        builder.create("StagingBuffer",
                                       BufferDescription { .size = Sprite_Vertex_Buffer_Size }));
                    task_data.vertex_buffer = builder.write(*m_vertex_buffer);
                },
                [](const GeometryTransferTask&  task_data,
                   OptionalRef<gpu::RenderPass> _,
                   gpu::CommandBuffer&          cmb,
                   const BakedFrameGraph::Data& graph_data) static noexcept {
                    auto staging_buffer = graph_data.getActualResource(*task_data.staging_buffer);
                    auto vertex_buffer  = graph_data.getActualResource(*task_data.vertex_buffer);

                    cmb.copyBuffer(staging_buffer, vertex_buffer, Sprite_Vertex_Buffer_Size);
                });
            transfer_task_data = as_ref(graph.getTaskData<GeometryTransferTask>(task.dataID()));
            m_dirty            = false;
        }

        struct DrawTask {
            Ref<GraphBuffer> vertex_buffers;
            Ref<GraphImage>  backbuffer;
        };

        graph.addRasterTask<DrawTask>(
            "StormKit:SpriteRenderTask",
            [&](DrawTask& task_data, GraphTaskBuilder& builder) noexcept {
                task_data.backbuffer
                    = as_ref(builder.create("color",
                                            engine::ImageDescription {
                                                .extent = m_viewport,
                                                .type   = gpu::ImageType::T2D,
                                                .format = gpu::PixelFormat::BGRA8_UNORM }));

                if (transfer_task_data)
                    task_data.vertex_buffers = builder.read((*transfer_task_data)->vertex_buffers);
                else
                    task_data.vertex_buffer = builder.read(vertex_buffer);

                graph.setFinalResource(task_data.backbuffer->id());
            },
            [this](const DrawTask&              task_data,
                   OptionalRef<gpu::RenderPass> render_pass,
                   gpu::CommandBuffer&          cmb,
                   const BakedFrameGraph::Data&) noexcept {
                if (not m_render_data->pipeline) {
                    m_render_data->pipeline
                        = gpu::Pipeline::create(m_renderer->device(),
                                                m_render_data->pipeline_state,
                                                m_render_data->pipeline_layout,
                                                *render_pass)
                              .transform_error(assert("Failed to create pipeline"))
                              .value();
                }
                auto buffers = as_refs<std::array>(task_data.vertex_buffer);

                cmb.bindPipeline(m_render_data->pipeline);
                cmb.bindVertexBuffers(buffers);
                for (auto&& [_, sprite_data] : m_sprites)
                    cmb.draw(std::size(sprite_data.sprite.vertices));
            },
            true);
    }
} // namespace stormkit::engine
