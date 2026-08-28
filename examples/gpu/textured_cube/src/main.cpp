// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#include <cstddef> // offsetof

import std;
import stormkit;

import gpu_app;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/try_expected.hpp>
#include <stormkit/log/log_macro.hpp>
#include <stormkit/main/main_macro.hpp>

#ifndef SHADER_DIR
static constexpr auto SHADER_DIR "../share/stormkit/shaders/"
#endif

#ifndef RESOURCE_DIR
  static constexpr auto RESOURCE_DIR "../share/stormkit/"
#endif

  namespace stdc = std::chrono;
namespace stdr   = std::ranges;
namespace stdfs  = std::filesystem;

using clock = stdc::high_resolution_clock;

using namespace std::literals;
using namespace stormkit;
using namespace stormkit::literals;

struct SubmissionResource {
    gpu::Fence         in_flight;
    gpu::Semaphore     image_available;
    gpu::CommandBuffer render_cmb;
    gpu::Buffer        viewer_buffer;
    gpu::DescriptorSet descriptor_set;
};

struct SwapchainImageResource {
    gpu::view::Image image;
    gpu::ImageView   view;
    gpu::Image       depth_image;
    gpu::ImageView   depth_view;
    gpu::Semaphore   render_finished;
};

struct Vertex {
    math::fvec3 position;
    math::fvec2 uv;

    static constexpr auto attribute_descriptions() noexcept -> array<gpu::VertexInputAttributeDescription, 2> {
        return to_array<gpu::VertexInputAttributeDescription>({
          { .location = 0, .binding = 0, .format = gpu::PixelFormat::RGB32F, .offset = offsetof(Vertex, position) },
          { .location = 1, .binding = 0, .format = gpu::PixelFormat::RG32F,  .offset = offsetof(Vertex, uv)       }
        });
    }

    static constexpr auto binding_description() noexcept -> gpu::VertexBindingDescription {
        return { .binding = 0, .stride = sizeof(Vertex), .input_rate = gpu::VertexInputRate::VERTEX };
    }
};

struct ViewerData {
    math::fmat4 proj;
    math::fmat4 view;
    math::fmat4 model;

    static constexpr auto layout_binding() -> gpu::DescriptorSetLayoutBinding {
        return { .binding          = 0,
                 .type             = gpu::DescriptorType::UNIFORM_BUFFER,
                 .stages           = gpu::ShaderStageFlag::VERTEX,
                 .descriptor_count = 1 };
    }
};

namespace {
    const auto     SHADER   = stdfs::path { SHADER_DIR } / "textured_cube.spv";
    const auto     TEXTURE  = stdfs::path { RESOURCE_DIR } / "textures/cube.png";
    constexpr auto VERTICES = array<Vertex, 36> {
        Vertex { { -1.f, -1.f, -1.f }, { 2.f / 3.f, 3.f / 4.f } }, // -X side
        { { -1.f, -1.f, 1.f },  { 1.f / 3.f, 3.f / 4.f } },
        { { -1.f, 1.f, 1.f },   { 1.f / 3.f, 1.f }       },
        { { -1.f, 1.f, 1.f },   { 1.f / 3.f, 1.f }       },
        { { -1.f, 1.f, -1.f },  { 2.f / 3.f, 1.f }       },
        { { -1.f, -1.f, -1.f }, { 2.f / 3.f, 3.f / 4.f } },

        { { -1.f, -1.f, -1.f }, { 1.f / 3.f, 2.f / 4.f } }, // -Z side
        { { 1.f, 1.f, -1.f },   { 0.f, 1.f / 4.f }       },
        { { 1.f, -1.f, -1.f },  { 0.f, 2.f / 4.f }       },
        { { -1.f, -1.f, -1.f }, { 1.f / 3.f, 2.f / 4.f } },
        { { -1.f, 1.f, -1.f },  { 1.f / 3.f, 1.f / 4.f } },
        { { 1.f, 1.f, -1.f },   { 0.f, 1.f / 4.f }       },

        { { -1.f, -1.f, -1.f }, { 2.f / 3.f, 2.f / 4.f } }, // -Y side
        { { 1.f, -1.f, -1.f },  { 2.f / 3.f, 3.f / 4.f } },
        { { 1.f, -1.f, 1.f },   { 1.f / 3.f, 3.f / 4.f } },
        { { -1.f, -1.f, -1.f }, { 2.f / 3.f, 2.f / 4.f } },
        { { 1.f, -1.f, 1.f },   { 1.f / 3.f, 3.f / 4.f } },
        { { -1.f, -1.f, 1.f },  { 1.f / 3.f, 2.f / 4.f } },

        { { -1.f, 1.f, -1.f },  { 2.f / 3.f, 0.f }       }, // +Y side
        { { -1.f, 1.f, 1.f },   { 1.f / 3.f, 0.f }       },
        { { 1.f, 1.f, 1.f },    { 1.f / 3.f, 1.f / 4.f } },
        { { -1.f, 1.f, -1.f },  { 2.f / 3.f, 0.f }       },
        { { 1.f, 1.f, 1.f },    { 1.f / 3.f, 1.f / 4.f } },
        { { 1.f, 1.f, -1.f },   { 2.f / 3.f, 1.f / 4.f } },

        { { 1.f, 1.f, -1.f },   { 2.f / 3.f, 1.f / 4.f } }, // +X side
        { { 1.f, 1.f, 1.f },    { 1.f / 3.f, 1.f / 4.f } },
        { { 1.f, -1.f, 1.f },   { 1.f / 3.f, 2.f / 4.f } },
        { { 1.f, -1.f, 1.f },   { 1.f / 3.f, 2.f / 4.f } },
        { { 1.f, -1.f, -1.f },  { 2.f / 3.f, 2.f / 4.f } },
        { { 1.f, 1.f, -1.f },   { 2.f / 3.f, 1.f / 4.f } },

        { { -1.f, 1.f, 1.f },   { 2.f / 3.f, 1.f / 4.f } }, // +Z side
        { { -1.f, -1.f, 1.f },  { 2.f / 3.f, 2.f / 4.f } },
        { { 1.f, 1.f, 1.f },    { 1.f, 1.f / 4.f }       },
        { { -1.f, -1.f, 1.f },  { 2.f / 3.f, 2.f / 4.f } },
        { { 1.f, -1.f, 1.f },   { 1.f, 2.f / 4.f }       },
        { { 1.f, 1.f, 1.f },    { 1.f, 1.f / 4.f }       },
    };

    constexpr auto VERTICES_SIZE   = sizeof(Vertex) * stdr::size(VERTICES);
    constexpr auto BUFFERING_COUNT = 2_u32;
    constexpr auto POOL_SIZES      = to_array<gpu::DescriptorPool::Size>({
      {
       .type             = gpu::DescriptorType::UNIFORM_BUFFER,
       .descriptor_count = BUFFERING_COUNT,
       },
      {
       .type             = gpu::DescriptorType::COMBINED_IMAGE_SAMPLER,
       .descriptor_count = BUFFERING_COUNT,
       }
    });

    constexpr auto OFFSETS        = array { 0_u64 };
    constexpr auto PIPELINE_FLAGS = array { gpu::PipelineStageFlag::COLOR_ATTACHMENT_OUTPUT };
} // namespace

class Application: public base::Application {
  public:
    auto init_example() {
        m_descriptor_pool = TryAssert(gpu::DescriptorPool::create(m_device, POOL_SIZES, BUFFERING_COUNT * 2),
                                      "Failed to create descriptor pool!");

        // load shaders
        m_vertex_shader = TryAssert(gpu::Shader::load_from_file(m_device, SHADER, gpu::ShaderStageFlag::VERTEX),
                                    std::format("Failed to load vertex shader {}!", SHADER.string()));

        m_fragment_shader = TryAssert(gpu::Shader::load_from_file(m_device, SHADER, gpu::ShaderStageFlag::FRAGMENT),
                                      std::format("Failed to load fragment shader {}!", SHADER.string()));

        m_descriptor_set_layout = TryAssert(gpu::DescriptorSetLayout::
                                              create(m_device,
                                                     into_dynarray(ViewerData::layout_binding(),
                                                                    gpu::DescriptorSetLayoutBinding {
                                                                      1,
                                                                      gpu::DescriptorType::COMBINED_IMAGE_SAMPLER,
                                                                      gpu::ShaderStageFlag::FRAGMENT,
                                                                      1 })),
                                            "Failed to create descriptor set layout!");

        m_pipeline_layout = TryAssert(gpu::PipelineLayout::
                                        create(m_device,
                                               gpu::RasterPipelineLayout {
                                                 .descriptor_set_layouts = gpu::to_views(m_descriptor_set_layout) }),
                                      "Failed to create pipeline layout!");
        // initialize render pass
        const auto depth_format = [this] {
            const auto formats_properties = m_physical_device->formats_properties();
            const auto candidates         = array { gpu::PixelFormat::DEPTH32F,
                                                    gpu::PixelFormat::DEPTH32F_STENCIL8U,
                                                    gpu::PixelFormat::DEPTH24_UNORM_STENCIL8U };

            for (const auto format : candidates) {
                const auto properties = stdr::find_if(formats_properties, [format](const auto& pair) {
                    return pair.first == format;
                });
                ENSURES(properties != stdr::cend(formats_properties));
                if (check_flag_bit(properties->second.optimal_tiling_features,
                                   gpu::FormatFeatureFlag::DEPTH_STENCIL_ATTACHMENT)) {
                    return format;
                }
            }

            ensures(false, "No supported depth format found!");
            std::unreachable();
        }();

        const auto depth_aspect_flag = [depth_format] {
            auto flag = gpu::ImageAspectFlag::DEPTH;
            if (gpu::is_depth_stencil_format(depth_format)) flag |= gpu::ImageAspectFlag::STENCIL;
            return flag;
        }();

        // initialize render pipeline
        const auto window_extent   = m_window->extent();
        const auto window_viewport = gpu::Viewport {
            .position = { 0.f, 0.f },
            .extent   = window_extent.to<f32>(),
            .depth    = { 0.f, 1.f },
        };
        const auto scissor = gpu::Scissor {
            .offset = { 0, 0 },
            .extent = window_extent,
        };

        const auto state = gpu::RasterPipelineState {
            .input_assembly_state = { .topology = gpu::PrimitiveTopology::TRIANGLE_LIST, },
            .viewport_state       = { .viewports = { window_viewport },
                                     .scissors  = { scissor }, },
            .rasterization_state = {
                .cull_mode = gpu::CullModeFlag::BACK,
                .front_face = gpu::FrontFace::CLOCKWISE,
            },
            .color_blend_state
            = { .attachments = { { .blend_enable           = true,
                                   .src_color_blend_factor = gpu::BlendFactor::SRC_ALPHA,
                                   .dst_color_blend_factor = gpu::BlendFactor::ONE_MINUS_SRC_ALPHA,
                                   .src_alpha_blend_factor = gpu::BlendFactor::SRC_ALPHA,
                                   .dst_alpha_blend_factor = gpu::BlendFactor::ONE_MINUS_SRC_ALPHA,
                                   .alpha_blend_operation  = gpu::BlendOperation::ADD, }, }, },
            .shader_state  = to_views(m_vertex_shader, m_fragment_shader),
            .vertex_input_state = {
                .binding_descriptions = into_dynarray(Vertex::binding_description()),
                .input_attribute_descriptions = to_dynarray(Vertex::attribute_descriptions()),
            },
                .depth_stencil_state = {
                .depth_test_enable =  true,
                .depth_write_enable = true
            },
        };

        const auto rendering_info = gpu::RasterPipelineRenderingInfo {
            .color_attachment_formats = { m_swapchain->pixel_format() },
            .depth_attachment_format  = depth_format,
        };

        m_pipeline = TryAssert(gpu::Pipeline::create(m_device,
                                                     gpu::Pipeline::RasterizationCreateInfo { .state          = as_ref(state),
                                                                                              .layout         = m_pipeline_layout,
                                                                                              .rendering_info = rendering_info }),
                               "Failed to create raster pipeline!");

        // load texture
        auto image = image::Image {};
        TryAssert(image.load_from_file(TEXTURE), std::format("Failed to load texture file {}!", TEXTURE.string()));

        m_texture = TryAssert(gpu::Image::create(m_device,
                                                 { .extent     = image.extent(),
                                                   .format     = gpu::PixelFormat::RGBA8_UNORM,
                                                   .usages     = gpu::ImageUsageFlag::SAMPLED | gpu::ImageUsageFlag::TRANSFER_DST,
                                                   .properties = gpu::MemoryPropertyFlag::DEVICE_LOCAL }),
                              "Failed to allocate texture!");

        {
            auto cpy_fence      = TryAssert(gpu::Fence::create(m_device), "Failed to create copy texture buffer fence!");
            auto staging_buffer = TryAssert(gpu::Buffer::create(m_device,
                                                                { .usages = gpu::BufferUsageFlag::TRANSFER_SRC,
                                                                  .size   = image.size() }),
                                            "Failed to allocate gpu texture staging buffer!");
            TryAssert(staging_buffer.upload(image.data()), "Failed to upload texture data to staging buffer!");

            auto copy_cmb = TryAssert(m_command_pool->create_command_buffer(), "Failed to allocate copy texture buffer");
            DiscardTryAssert((copy_cmb.record([&](auto cmb) noexcept {
                                 const auto copy = array {
                                     gpu::BufferImageCopy {
                                                           .buffer_offset       = 0,
                                                           .buffer_row_length   = 0,
                                                           .buffer_image_height = 0,
                                                           .subresource_layers  = {},
                                                           .offset              = {},
                                                           .extent              = image.extent() }
                                 };

                                 cmb.begin_debug_region("Upload texture data")
                                   .transition_image_layout(m_texture,
                                                            gpu::ImageLayout::UNDEFINED,
                                                            gpu::ImageLayout::TRANSFER_DST_OPTIMAL)
                                   .copy_buffer_to_image(staging_buffer, m_texture, copy)
                                   .end_debug_region()
                                   .begin_debug_region("Transition texture data")
                                   .transition_image_layout(m_texture,
                                                            gpu::ImageLayout::TRANSFER_DST_OPTIMAL,
                                                            gpu::ImageLayout::SHADER_READ_ONLY_OPTIMAL)
                                   .end_debug_region();
                             })),
                             "Failed to record texture upload and transition cmb!");

            DiscardTryAssert(copy_cmb.submit(m_raster_queue, {}, {}, {}, cpy_fence),
                             "Failed to submit texture upload command buffer!");

            DiscardTryAssert(cpy_fence.wait(), "Failed to create texture view!");
        }

        m_texture_view         = TryAssert(gpu::ImageView::create(m_device, { m_texture }), "Failed to create texture view!");
        m_sampler              = TryAssert(gpu::Sampler::create(m_device, {}), "Failed to create sampler!");
        m_submission_resources = dynarray<SubmissionResource> {};
        m_submission_resources.reserve(BUFFERING_COUNT);

        for (auto _ : range(BUFFERING_COUNT)) {
            m_submission_resources.push_back(
              { .in_flight       = TryAssert(gpu::Fence::create_signaled(m_device), "Failed to create swapchain image!"),
                .image_available = TryAssert(gpu::Semaphore::create(m_device), "Failed to create present image!"),
                .render_cmb      = TryAssert(m_command_pool->create_command_buffer(), "Failed to create buffers!"),
                .viewer_buffer   = TryAssert(gpu::Buffer::create(m_device,
                                                                 {
                                                                   .usages              = gpu::BufferUsageFlag::UNIFORM,
                                                                   .size                = sizeof(ViewerData),
                                                                   .persistently_mapped = true,
                                                                 }),
                                             "Failed to allocate gpu viewer buffer!"),
                .descriptor_set  = TryAssert(m_descriptor_pool->create_descriptor_set(m_descriptor_set_layout),
                                             "Failed to create descriptor set!") });
            auto&      res  = m_submission_resources.back();
            const auto sets = array<gpu::Descriptor, 2> {
                gpu::BufferDescriptor {
                                       .binding = 0,
                                       .buffer  = res.viewer_buffer,
                                       .range   = sizeof(ViewerData),
                                       .offset  = 0,
                                       },
                gpu::ImageDescriptor {
                                       .binding    = 1,
                                       .layout     = gpu::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
                                       .image_view = m_texture_view,
                                       .sampler    = m_sampler,
                                       }
            };
            res.descriptor_set.update(sets);
        }

        const auto& images = m_swapchain->images();

        const auto image_count     = stdr::size(images);
        auto       transition_cmbs = TryAssert(m_command_pool->create_command_buffers(image_count),
                                               "Failed to create transition command buffers!");

        m_image_resources = dynarray<SwapchainImageResource> {};
        m_image_resources.reserve(stdr::size(images));

        auto image_index = 0u;
        for (const auto& swap_image : images) {
            auto view = TryAssert(gpu::ImageView::create(m_device, { swap_image }), "Failed to create swapchain image view!");
            auto depth_image = TryAssert(gpu::Image::create(m_device,
                                                            { .extent     = swap_image.extent(),
                                                              .format     = depth_format,
                                                              .usages     = gpu::ImageUsageFlag::DEPTH_STENCIL_ATTACHMENT,
                                                              .properties = gpu::MemoryPropertyFlag::DEVICE_LOCAL }),
                                         "Failed to create depth image!");

            auto depth_view = TryAssert(gpu::ImageView::create(m_device,
                                                               { .image             = depth_image,
                                                                 .subresource_range = gpu::
                                                                   ImageSubresourceRange { .aspect_mask = depth_aspect_flag } }),
                                        "Failed to create depth image view!");

            m_image_resources.push_back({ .image           = swap_image,
                                          .view            = std::move(view),
                                          .depth_image     = std::move(depth_image),
                                          .depth_view      = std::move(depth_view),
                                          .render_finished = TryAssert(gpu::Semaphore::create(m_device),
                                                                       "Failed to create render!") });

            const auto& resources = m_image_resources.back();

            auto& transition_cmb = transition_cmbs[image_index];
            DiscardTryAssert((transition_cmb.record([&](auto cmb) noexcept {
                                 cmb.begin_debug_region(std::format("Transition image {}", image_index))
                                   .transition_image_layout(swap_image,
                                                            gpu::ImageLayout::UNDEFINED,
                                                            gpu::ImageLayout::PRESENT_SRC)
                                   .end_debug_region()
                                   .begin_debug_region(std::format("Transition depth image {}", image_index))
                                   .transition_image_layout(resources.depth_image,
                                                            gpu::ImageLayout::UNDEFINED,
                                                            gpu::ImageLayout::ATTACHMENT_OPTIMAL,
                                                            { .aspect_mask = depth_aspect_flag })
                                   .end_debug_region();
                             })),
                             std::format("Failed to record transition cmb {}!", image_index));

            ++image_index;
        }

        const auto fence = TryAssert(gpu::Fence::create(m_device), "Failed to create transition fence!");

        const auto cmbs = to_views(transition_cmbs);
        DiscardTryAssert(m_raster_queue->submit({ .command_buffers = cmbs }, fence),
                         "Failed to submit texture transition command buffers!");

        // setup vertex buffer
        m_vertex_buffer = TryAssert(gpu::Buffer::create(m_device,
                                                        { .usages     = gpu::BufferUsageFlag::VERTEX
                                                                        | gpu::BufferUsageFlag::TRANSFER_DST,
                                                          .size       = VERTICES_SIZE,
                                                          .properties = gpu::MemoryPropertyFlag::DEVICE_LOCAL }),
                                    "Failed to allocate gpu vertex buffer!");

        {
            auto staging_buffer = TryAssert(gpu::Buffer::create(m_device,
                                                                { .usages = gpu::BufferUsageFlag::TRANSFER_SRC,
                                                                  .size   = VERTICES_SIZE }),
                                            "Failed to allocate gpu vertex staging buffer!");

            TryAssert(staging_buffer.upload(VERTICES), "Failed to upload vertex data to staging buffer!");

            auto cpy_fence = TryAssert(gpu::Fence::create(m_device), "Failed to create copy vertex buffer fence!");

            auto copy_cmb = TryAssert(m_command_pool->create_command_buffer(), "Failed to allocate copy vertex buffer");
            TryAssert(copy_cmb.begin(), "Failed to begin vertices upload command buffer");

            copy_cmb.begin_debug_region("Upload vertex data to vertex buffer")
              .copy_buffer(staging_buffer, m_vertex_buffer, VERTICES_SIZE)
              .end_debug_region();

            DiscardTryAssert(copy_cmb.end(), "Failed to begin vertices upload command buffer");
            DiscardTryAssert(copy_cmb.submit(m_raster_queue, {}, {}, {}, cpy_fence),
                             "Failed to submit vertices upload command buffer!");
            TryAssert(cpy_fence.wait(), "Failed to acquire next swapchain image!");
        }

        TryAssert(fence.wait(), "");
    }

    auto run_example() {
        LOG_MODULE.flush();

        const auto current_time = clock::now();

        const auto window_extent     = m_window->extent();
        const auto window_extent_f32 = window_extent.to<f32>();
        auto       viewer_data       = ViewerData {
            .proj  = math::perspective(math::angle::radians(45.f),
                                       window_extent_f32.width / window_extent_f32.height,
                                       0.1f,
                                       100.f),
            .view  = math::look_at(math::fvec3 { 0.f, 3.f, 5.f }, { 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }),
            .model = math::fmat4::identity(),
        };

        // get next swapchain image
        auto& submission_resource = m_submission_resources[m_current_frame];

        const auto& wait      = submission_resource.image_available;
        auto&       in_flight = submission_resource.in_flight;

        TryAssert(in_flight.wait(), "Failed to wait in_flight fence!");
        TryAssert(in_flight.reset(), "Failed to reset in_flight fence!");

        const auto&& [_, image_index] = TryAssert(m_swapchain->acquire_next_image(100ms, wait),
                                                  "Failed to acquire next swapchain image!");

        const auto& swapchain_image_resource = m_image_resources[image_index];
        const auto& signal                   = swapchain_image_resource.render_finished;

        // update viewer data and upload
        const auto time   = stdc::duration_cast<fsecond>(current_time - m_start_time).count();
        viewer_data.model = math::transpose(math::rotate(math::fmat4::identity(),
                                                         time * math::angle::radians(90.f),
                                                         math::fvec3 { 0.f, 1.f, 0.f }));

        auto& viewer_buffer = submission_resource.viewer_buffer;
        TryAssert(viewer_buffer.upload(viewer_data), "Failed to upload texture to gpu!");

        const auto rendering_info = gpu::RenderingInfo {
            .render_area = { .x = 0, .y = 0, .width = window_extent.to<i32>().width, .height = window_extent.to<i32>().height },
            .color_attachments = { { .image_view  = swapchain_image_resource.view,
                                     .layout      = gpu::ImageLayout::ATTACHMENT_OPTIMAL,
                                     .clear_value = gpu::ClearColor { .color = colors::SILVER<f32> } } },
            .depth_attachment  = { { .image_view  = swapchain_image_resource.depth_view,
                                     .layout      = gpu::ImageLayout::ATTACHMENT_OPTIMAL,
                                     .clear_value = gpu::ClearDepthStencil {} } }
        };

        // render in it
        auto&       render_cmb     = submission_resource.render_cmb;
        const auto& descriptor_set = submission_resource.descriptor_set;

        TryAssert(render_cmb.reset(), std::format("Failed to reset render cmb {}!", image_index));
        DiscardTryAssert((render_cmb.record([&](auto cmb) noexcept {
                             cmb
                               .transition_image_layout(swapchain_image_resource.image,
                                                        gpu::ImageLayout::PRESENT_SRC,
                                                        gpu::ImageLayout::ATTACHMENT_OPTIMAL)
                               .begin_debug_region("Render cube")
                               .begin_rendering(rendering_info)
                               .bind_pipeline(m_pipeline)
                               .bind_vertex_buffers(gpu::as_views(m_vertex_buffer), OFFSETS)
                               .bind_descriptor_sets(m_pipeline, m_pipeline_layout, gpu::as_views(descriptor_set), {})
                               .draw(stdr::size(VERTICES))
                               .end_rendering()
                               .end_debug_region()
                               .transition_image_layout(swapchain_image_resource.image,
                                                        gpu::ImageLayout::ATTACHMENT_OPTIMAL,
                                                        gpu::ImageLayout::PRESENT_SRC);
                         })),
                         std::format("Failed to record render cmb {}!", image_index));
        DiscardTryAssert(render_cmb.submit(m_raster_queue, gpu::as_views(wait), PIPELINE_FLAGS, gpu::as_views(signal), in_flight),
                         "Failed to submit render command buffer!");

        // present it
        TryAssert(m_raster_queue->present(gpu::as_views(m_swapchain), gpu::as_views(signal), as_view(image_index)),
                  "Failed to present swapchain image!");

        if (++m_current_frame >= BUFFERING_COUNT) m_current_frame = 0;
    }

    constexpr auto example_name() const noexcept -> string_view { return "Textured Cube"; }

  private:
    DeferInit<gpu::DescriptorPool> m_descriptor_pool;

    DeferInit<gpu::Shader>              m_vertex_shader;
    DeferInit<gpu::Shader>              m_fragment_shader;
    DeferInit<gpu::DescriptorSetLayout> m_descriptor_set_layout;
    DeferInit<gpu::PipelineLayout>      m_pipeline_layout;
    DeferInit<gpu::Pipeline>            m_pipeline;
    DeferInit<gpu::Image>               m_texture;
    DeferInit<gpu::ImageView>           m_texture_view;
    DeferInit<gpu::Sampler>             m_sampler;
    dynarray<SubmissionResource>       m_submission_resources;
    dynarray<SwapchainImageResource>   m_image_resources;
    DeferInit<gpu::Buffer>              m_vertex_buffer;
    usize                               m_current_frame = 0_usize;
    decltype(clock::now())              m_start_time    = clock::now();
};

auto main(array_view<const string_view> args) -> int {
    auto app = Application {};
    app.run(args);
    return 0;
}
