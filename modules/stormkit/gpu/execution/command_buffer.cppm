// Copyright (C) 2023 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>
#include <stormkit/core/try_expected.hpp>

#include <stormkit/gpu/api.hpp>
#include <stormkit/gpu/vulkan.hpp>

export module stormkit.gpu.execution:command_buffer;

import std;

import stormkit.core;
import stormkit.gpu.core;
import stormkit.gpu.resource;

import :descriptors;
import :render_pass;
import :pipeline;
import :swapchain;
import :objects;

namespace stdr = std::ranges;

namespace cmonadic = stormkit::core::monadic;

namespace stormkit::gpu {
    export {
        struct RenderPassInheritanceInfo {
            std::optional<view::RenderPass>  render_pass = std::nullopt;
            u32                              subpass     = 0;
            std::optional<view::FrameBuffer> framebuffer = std::nullopt;
        };

        struct RenderingInheritanceInfo {
            u32                        view_mask             = 0;
            dyn_array<PixelFormat>     color_attachments     = {};
            std::optional<PixelFormat> depth_attachment      = std::nullopt;
            std::optional<PixelFormat> stencil_attachment    = std::nullopt;
            SampleCountFlag            rasterization_samples = SampleCountFlag::C1;
        };

        using InheritanceInfo = std::variant<std::monostate, RenderPassInheritanceInfo, RenderingInheritanceInfo>;

        struct RenderingInfo {
            struct Attachment {
                struct Resolve {
                    view::ImageView image_view;
                    ResolveModeFlag mode;
                    ImageLayout     layout = ImageLayout::ATTACHMENT_OPTIMAL;
                };

                view::ImageView image_view;
                ImageLayout     layout = ImageLayout::ATTACHMENT_OPTIMAL;

                std::optional<Resolve> resolve = std::nullopt;

                AttachmentLoadOperation  load_op  = AttachmentLoadOperation::CLEAR;
                AttachmentStoreOperation store_op = AttachmentStoreOperation::STORE;

                std::optional<ClearValue> clear_value = std::nullopt;
            };

            math::irect render_area;
            u32         layer_count = 1u;
            u32         view_mask   = 0u;

            dyn_array<Attachment>     color_attachments  = {};
            std::optional<Attachment> depth_attachment   = std::nullopt;
            std::optional<Attachment> stencil_attachment = std::nullopt;
        };
    }

    struct QueueInterfaceBase {
        struct SubmitInfo {
            array_view<const view::Semaphore>     wait_semaphores   = {};
            array_view<const PipelineStageFlag>   wait_dst_stages   = {};
            array_view<const view::CommandBuffer> command_buffers   = {};
            array_view<const view::Semaphore>     signal_semaphores = {};
        };
    };

    export template<typename Base>
    class QueueInterface final: public DeviceObject<Base>, public QueueInterfaceBase {
      public:
        using DeviceObject<Base>::DeviceObject;
        using DeviceObject<Base>::operator=;
        using TagType = QueueTag;

        using QueueInterfaceBase::SubmitInfo;

        auto wait_idle() const noexcept -> Expected<void>;

        auto submit(array_view<const SubmitInfo> submit_infos, std::optional<view::Fence> fence = std::nullopt) const noexcept
          -> Expected<void>;

        auto submit(const SubmitInfo& submit_info, std::optional<view::Fence> fence = std::nullopt) const noexcept
          -> Expected<void>;

        [[nodiscard]]
        auto present(array_view<const view::SwapChain> swapchains,
                     array_view<const view::Semaphore> wait_semaphores,
                     array_view<const u32>             image_indices) const noexcept -> Expected<Result>;

        [[nodiscard]]
        auto entry() const noexcept -> const QueueEntry&;
    };

    struct CommandBufferInterfaceBase {
        enum class State {
            INITIAL,
            RECORDING,
            EXECUTABLE,
        };
        using RecordClosure = std23::function_ref<void(view::CommandBuffer)>;
    };

    export {
        template<typename Base>
        class CommandBufferInterface final: public DeviceObject<Base>, public CommandBufferInterfaceBase {
          public:
            using DeviceObject<Base>::DeviceObject;
            using DeviceObject<Base>::operator=;
            using TagType = CommandBufferTag;

            using CommandBufferInterfaceBase::RecordClosure;
            using CommandBufferInterfaceBase::State;

            [[nodiscard]]
            auto state() const noexcept -> State;
            [[nodiscard]]
            auto level() const noexcept -> CommandBufferLevel;

            auto record(this auto&,
                        RecordClosure   record_closure,
                        bool            one_time_submit  = false,
                        InheritanceInfo inheritance_info = std::monostate {}) noexcept -> Expected<void>;

            auto reset() noexcept -> Expected<void>;
            auto begin(bool one_time_submit = false, InheritanceInfo inheritance_info = std::monostate {}) noexcept
              -> Expected<void>;
            auto end() noexcept -> Expected<void>;

            auto begin_debug_region(string_view name, const fcolor_rgb& color = colors::WHITE<f32>) const noexcept
              -> const CommandBufferInterface&;
            auto insert_debug_label(string_view name, const fcolor_rgb& color = colors::WHITE<f32>) const noexcept
              -> const CommandBufferInterface&;
            auto end_debug_region() const noexcept -> const CommandBufferInterface&;

            auto begin_rendering(const RenderingInfo& info, bool secondary_commandbuffers = false) const noexcept
              -> const CommandBufferInterface&;
            auto begin_render_pass(view::RenderPass             render_pass,
                                   view::FrameBuffer            framebuffer,
                                   array_view<const ClearValue> clear_values = array { ClearValue {
                                     ClearColor { .color = colors::SILVER<f32> } } },
                                   bool secondary_commandbuffers = false) const noexcept -> const CommandBufferInterface&;
            auto next_subpass() const noexcept -> const CommandBufferInterface&;
            auto end_render_pass() const noexcept -> const CommandBufferInterface&;
            auto end_rendering() const noexcept -> const CommandBufferInterface&;

            auto bind_pipeline(view::Pipeline pipeline) const noexcept -> const CommandBufferInterface&;
            auto set_viewport(u32 first_viewport, array_view<const Viewport> viewports) const noexcept
              -> const CommandBufferInterface&;
            auto set_scissor(u32 first_scissor, array_view<const Scissor> scissors) const noexcept
              -> const CommandBufferInterface&;
            auto set_line_width(f32 width) const noexcept -> const CommandBufferInterface&;
            auto set_depth_bias(f32 constant_factor, f32 clamp, f32 slope_factor) const noexcept -> const CommandBufferInterface&;
            auto set_blend_constants(array_view<const f32> constants) const noexcept -> const CommandBufferInterface&;
            auto set_depth_bounds(f32 min, f32 max) const noexcept -> const CommandBufferInterface&;
            auto set_stencil_compare_mask(StencilFaceFlag face, u32 mask) const noexcept -> const CommandBufferInterface&;
            auto set_stencil_write_mask(StencilFaceFlag face, u32 mask) const noexcept -> const CommandBufferInterface&;
            auto set_stencil_reference(StencilFaceFlag face, u32 reference) const noexcept -> const CommandBufferInterface&;

            auto dispatch(u32 group_count_x, u32 group_count_y, u32 group_count_z) const noexcept
              -> const CommandBufferInterface&;

            auto draw(u32 vertex_count, u32 instance_count = 1u, u32 first_vertex = 0, u32 first_instance = 0) const noexcept
              -> const CommandBufferInterface&;
            auto draw_indexed(u32 index_count,
                              u32 instance_count = 1u,
                              u32 first_index    = 0u,
                              i32 vertex_offset  = 0,
                              u32 first_instance = 0u) const noexcept -> const CommandBufferInterface&;
            auto draw_indirect(view::Buffer buffer, usize offset, u32 draw_count, u32 stride) const noexcept
              -> const CommandBufferInterface&;
            auto draw_indexed_indirect(view::Buffer buffer, usize offset, u32 draw_count, u32 stride) const noexcept
              -> const CommandBufferInterface&;

            auto bind_vertex_buffers(array_view<const view::Buffer> buffers, array_view<const u64> offsets) const noexcept
              -> const CommandBufferInterface&;
            auto bind_index_buffer(view::Buffer buffer, u64 offset = 0, bool large_indices = false) const noexcept
              -> const CommandBufferInterface&;
            auto bind_descriptor_sets(view::Pipeline                        pipeline,
                                      view::PipelineLayout                  layout,
                                      array_view<const view::DescriptorSet> descriptor_sets,
                                      array_view<const u32> dynamic_offsets = {}) const noexcept -> const CommandBufferInterface&;

            auto copy_buffer(view::Buffer src, view::Buffer dst, usize size, u64 src_offset = 0u, u64 dst_offset = 0u)
              const noexcept -> const CommandBufferInterface&;
            auto copy_buffer_to_image(view::Buffer                      src,
                                      view::Image                       dst,
                                      array_view<const BufferImageCopy> buffer_image_copies = {}) const noexcept
              -> const CommandBufferInterface&;
            auto copy_image_to_buffer(view::Image                       src,
                                      view::Buffer                      dst,
                                      array_view<const BufferImageCopy> buffer_image_copies = {}) const noexcept
              -> const CommandBufferInterface&;
            auto copy_image(view::Image                   src,
                            view::Image                   dst,
                            ImageLayout                   src_layout,
                            ImageLayout                   dst_layout,
                            const ImageSubresourceLayers& src_subresource_layers,
                            const ImageSubresourceLayers& dst_subresource_layers,
                            const math::uextent3&         extent) const noexcept -> const CommandBufferInterface&;

            auto resolve_image(view::Image                   src,
                               view::Image                   dst,
                               ImageLayout                   src_layout,
                               ImageLayout                   dst_layout,
                               const ImageSubresourceLayers& src_subresource_layers = {},
                               const ImageSubresourceLayers& dst_subresource_layers = {}) const noexcept
              -> const CommandBufferInterface&;

            auto blit_image(view::Image                  src,
                            view::Image                  dst,
                            ImageLayout                  src_layout,
                            ImageLayout                  dst_layout,
                            array_view<const BlitRegion> regions,
                            Filter                       filter) const noexcept -> const CommandBufferInterface&;

            auto transition_image_layout(view::Image                  image,
                                         ImageLayout                  src_layout,
                                         ImageLayout                  dst_layout,
                                         const ImageSubresourceRange& subresource_range = {}) const noexcept
              -> const CommandBufferInterface&;

            auto execute_sub_command_buffers(array_view<const view::CommandBuffer> commandbuffers) const noexcept
              -> const CommandBufferInterface&;

            auto pipeline_barrier(PipelineStageFlag                     src_mask,
                                  PipelineStageFlag                     dst_mask,
                                  DependencyFlag                        dependency,
                                  array_view<const MemoryBarrier>       memory_barriers,
                                  array_view<const BufferMemoryBarrier> buffer_memory_barriers,
                                  array_view<const ImageMemoryBarrier>  image_memory_barriers) const noexcept
              -> const CommandBufferInterface&;

            auto push_constants(view::PipelineLayout pipeline_layout, ShaderStageFlag stage, byte_view<> data, u32 offset = 0u)
              const noexcept -> const CommandBufferInterface&;

            auto submit(this const auto&,
                        view::Queue                         queue,
                        array_view<const view::Semaphore>   wait_semaphores   = {},
                        array_view<const PipelineStageFlag> wait_dst_stages   = {},
                        array_view<const view::Semaphore>   signal_semaphores = {},
                        std::optional<view::Fence>          fence             = std::nullopt) noexcept -> Expected<void>;
        };
    }

    class STORMKIT_GPU_API QueueImplementation: public GpuObjectImplementation<QueueTag, const QueueEntry&> {
      public:
        using SubmitInfo = QueueInterfaceBase::SubmitInfo;

        QueueImplementation(PrivateTag, view::Device&& device) noexcept;
        ~QueueImplementation() noexcept;

        QueueImplementation(const QueueImplementation&) = delete;
        auto operator=(const QueueImplementation&)      = delete;

        QueueImplementation(QueueImplementation&&) noexcept;
        auto operator=(QueueImplementation&&) noexcept -> QueueImplementation&;

        auto do_init(PrivateTag, const QueueEntry&) -> void;

      protected:
        QueueEntry m_entry;

        friend class QueueInterface<QueueImplementation>;
    };

    namespace view {
        class QueueImplementation: public GpuObjectViewImplementation<QueueTag> {
          public:
            using SubmitInfo = QueueInterfaceBase::SubmitInfo;

            QueueImplementation(const gpu::Queue& of) noexcept;
            template<cmeta::IsContainerOrPointerOf<gpu::Queue> TContainerOrPointer>
            QueueImplementation(const TContainerOrPointer&) noexcept;
            ~QueueImplementation() noexcept;

            QueueImplementation(const QueueImplementation&) noexcept;
            auto operator=(const QueueImplementation&) noexcept -> QueueImplementation&;

            QueueImplementation(QueueImplementation&&) noexcept;
            auto operator=(QueueImplementation&&) noexcept -> QueueImplementation&;

          protected:
            QueueEntry m_entry;
        };
    } // namespace view

    using CommandBufferDeleter = std::function<void(view::Device, view::CommandPool, VkCommandBuffer)>;

    class STORMKIT_GPU_API CommandBufferImplementation
        : public GpuObjectImplementation<CommandBufferTag, CommandBufferLevel, VkCommandBuffer&&, CommandBufferDeleter&&> {
      public:
        using RecordClosure = CommandBufferInterfaceBase::RecordClosure;
        using State         = CommandBufferInterfaceBase::State;

        CommandBufferImplementation(PrivateTag, view::Device&&) noexcept;
        ~CommandBufferImplementation() noexcept;

        CommandBufferImplementation(const CommandBufferImplementation&)                    = delete;
        auto operator=(const CommandBufferImplementation&) -> CommandBufferImplementation& = delete;

        CommandBufferImplementation(CommandBufferImplementation&&) noexcept;
        auto operator=(CommandBufferImplementation&&) noexcept -> CommandBufferImplementation&;

        auto do_init(PrivateTag, CommandBufferLevel, VkCommandBuffer&&, CommandBufferDeleter&&) noexcept -> void;

      protected:
        using NamedConstructor::allocate;
        using NamedConstructor::create;

        Heap<State>        m_state;
        CommandBufferLevel m_level = CommandBufferLevel::PRIMARY;

        CommandBufferDeleter m_deleter;

        friend class CommandPoolInterface<CommandPoolImplementation>;
        friend class CommandPoolInterface<view::CommandPoolImplementation>;
        friend class view::CommandBufferImplementation;
    };

    namespace view {
        class CommandBufferImplementation: public GpuObjectViewImplementation<CommandBufferTag> {
          public:
            using RecordClosure = CommandBufferInterfaceBase::RecordClosure;
            using State         = CommandBufferInterfaceBase::State;

            CommandBufferImplementation(const gpu::CommandBuffer& of) noexcept;
            template<cmeta::IsContainerOrPointerOf<gpu::CommandBuffer> TContainerOrPointer>
            CommandBufferImplementation(const TContainerOrPointer&) noexcept;
            ~CommandBufferImplementation() noexcept;

            CommandBufferImplementation(const CommandBufferImplementation&) noexcept;
            auto operator=(const CommandBufferImplementation&) noexcept -> CommandBufferImplementation&;

            CommandBufferImplementation(CommandBufferImplementation&&) noexcept;
            auto operator=(CommandBufferImplementation&&) noexcept -> CommandBufferImplementation&;

          protected:
            ref<State>         m_state;
            CommandBufferLevel m_level;
        };
    } // namespace view

    export {
        struct CommandPoolInterfaceBase {
            struct CreateInfo {
                view::Queue queue;
                bool        reset     = true;
                bool        transient = false;
            };
        };

        template<typename Base>
        class CommandPoolInterface final: public DeviceObject<Base>, public CommandPoolInterfaceBase {
          public:
            using DeviceObject<Base>::DeviceObject;
            using DeviceObject<Base>::operator=;
            using TagType = CommandPoolTag;

            auto create_command_buffer(CommandBufferLevel level = CommandBufferLevel::PRIMARY) const noexcept
              -> Expected<CommandBuffer>;
            auto create_command_buffers(usize count, CommandBufferLevel level = CommandBufferLevel::PRIMARY) const noexcept
              -> Expected<dyn_array<CommandBuffer>>;

            auto allocate_command_buffer(CommandBufferLevel level = CommandBufferLevel::PRIMARY) const noexcept
              -> Expected<Heap<CommandBuffer>>;
            auto allocate_command_buffers(usize count, CommandBufferLevel level = CommandBufferLevel::PRIMARY) const noexcept
              -> Expected<dyn_array<Heap<CommandBuffer>>>;

          protected:
            auto create_vk_command_buffers(usize, CommandBufferLevel) const noexcept -> Expected<dyn_array<VkCommandBuffer>>;

            static auto delete_vk_command_buffers(view::Device, view::CommandPool, VkCommandBuffer) noexcept -> void;
        };
    }

    class STORMKIT_GPU_API
      CommandPoolImplementation: public GpuObjectImplementation<CommandPoolTag, const CommandPoolInterfaceBase::CreateInfo&> {
      public:
        using CreateInfo = CommandPoolInterfaceBase::CreateInfo;

        CommandPoolImplementation(PrivateTag, view::Device&& device) noexcept;
        ~CommandPoolImplementation() noexcept;

        CommandPoolImplementation(const CommandPoolImplementation&) = delete;
        auto operator=(const CommandPoolImplementation&)            = delete;

        CommandPoolImplementation(CommandPoolImplementation&&) noexcept;
        auto operator=(CommandPoolImplementation&&) noexcept -> CommandPoolImplementation&;

        auto do_init(PrivateTag, const CreateInfo&) noexcept -> Expected<void>;
    };

    namespace view {
        class CommandPoolImplementation: public GpuObjectViewImplementation<CommandPoolTag> {
          public:
            using GpuObjectViewImplementation<CommandPoolTag>::GpuObjectViewImplementation;
            using GpuObjectViewImplementation<CommandPoolTag>::operator=;
        };
    } // namespace view
} // namespace stormkit::gpu

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::gpu {
    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto QueueInterface<Base>::submit(const SubmitInfo& submit_info, std::optional<view::Fence> fence) const noexcept
      -> Expected<void> {
        return QueueInterface<Base>::submit(array_view<const SubmitInfo> { &submit_info, 1 }, std::move(fence));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto QueueInterface<Base>::entry() const noexcept -> const QueueEntry& {
        return Base::m_entry;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto CommandBufferInterface<Base>::submit(this const auto&                    self,
                                                     view::Queue                         queue,
                                                     array_view<const view::Semaphore>   wait_semaphores,
                                                     array_view<const PipelineStageFlag> wait_dst_stages,
                                                     array_view<const view::Semaphore>   signal_semaphores,
                                                     std::optional<view::Fence>          fence) noexcept -> Expected<void> {
        auto cmbs         = as_views(self);
        auto submit_infos = array {
            Queue::SubmitInfo {
                               .wait_semaphores   = wait_semaphores,
                               .wait_dst_stages   = wait_dst_stages,
                               .command_buffers   = cmbs,
                               .signal_semaphores = signal_semaphores }
        };

        return queue.submit(submit_infos, std::move(fence));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto CommandBufferInterface<Base>::state() const noexcept -> State {
        EXPECTS(Base::m_state != nullptr);
        return *Base::m_state;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto CommandBufferInterface<Base>::level() const noexcept -> CommandBufferLevel {
        return Base::m_level;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto CommandBufferInterface<Base>::record(this auto&      self,
                                                     RecordClosure   record_closure,
                                                     bool            one_time_submit,
                                                     InheritanceInfo inheritance_info) noexcept -> Expected<void> {
        Try(self.begin(one_time_submit, std::move(inheritance_info)));
        record_closure(gpu::as_view(self));
        Try(self.end());
        Return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto CommandPoolInterface<Base>::create_command_buffer(CommandBufferLevel level) const noexcept
      -> Expected<CommandBuffer> {
        auto   device    = Base::owner();
        auto   vk_handle = Try(create_vk_command_buffers(1, level)).front();
        Return CommandBuffer::create(std::move(device), level, std::move(vk_handle), delete_vk_command_buffers);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto CommandPoolInterface<Base>::create_command_buffers(usize count, CommandBufferLevel level) const noexcept
      -> Expected<dyn_array<CommandBuffer>> {
        Return transform(Try(create_vk_command_buffers(count, level)), [this, &level](auto vk_handle) noexcept {
            auto device = Base::owner();
            return CommandBuffer::create(std::move(device), level, std::move(vk_handle), delete_vk_command_buffers);
        });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto CommandPoolInterface<Base>::allocate_command_buffer(CommandBufferLevel level) const noexcept
      -> Expected<Heap<CommandBuffer>> {
        auto   device    = Base::owner();
        auto   vk_handle = Try(create_vk_command_buffers(1, level)).front();
        Return CommandBuffer::allocate(std::move(device), level, std::move(vk_handle), delete_vk_command_buffers);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto CommandPoolInterface<Base>::allocate_command_buffers(usize count, CommandBufferLevel level) const noexcept
      -> Expected<dyn_array<Heap<CommandBuffer>>> {
        Return transform(Try(create_vk_command_buffers(count, level)), [this, &level](auto vk_handle) noexcept {
            auto device = Base::owner();
            return CommandBuffer::allocate(std::move(device), level, std::move(vk_handle), delete_vk_command_buffers);
        });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline QueueImplementation::QueueImplementation(PrivateTag, view::Device&& device) noexcept
        : GpuObjectImplementation { std::move(device), cmonadic::noop() } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline QueueImplementation::~QueueImplementation() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline QueueImplementation::QueueImplementation(QueueImplementation&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto QueueImplementation::operator=(QueueImplementation&&) noexcept -> QueueImplementation& = default;

    namespace view {
        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline QueueImplementation::QueueImplementation(const gpu::Queue& of) noexcept
            : GpuObjectViewImplementation { of }, m_entry { of.entry() } {
        }

        /////////////////////////////////////
        /////////////////////////////////////
        template<cmeta::IsContainerOrPointerOf<gpu::Queue> TContainerOrPointer>
        STORMKIT_FORCE_INLINE
        inline QueueImplementation::QueueImplementation(const TContainerOrPointer& of) noexcept
            : QueueImplementation { *of } {
        }

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline QueueImplementation::~QueueImplementation() noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline QueueImplementation::QueueImplementation(const QueueImplementation&) noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto QueueImplementation::operator=(const QueueImplementation&) noexcept -> QueueImplementation& = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline QueueImplementation::QueueImplementation(QueueImplementation&&) noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto QueueImplementation::operator=(QueueImplementation&&) noexcept -> QueueImplementation& = default;
    } // namespace view

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline CommandBufferImplementation::CommandBufferImplementation(PrivateTag, view::Device&& device) noexcept
        : GpuObjectImplementation { std::move(device), cmonadic::noop() } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline CommandBufferImplementation::~CommandBufferImplementation() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline CommandBufferImplementation::CommandBufferImplementation(CommandBufferImplementation&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto CommandBufferImplementation::operator=(CommandBufferImplementation&&) noexcept
      -> CommandBufferImplementation& = default;

    namespace view {
        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline CommandBufferImplementation::CommandBufferImplementation(const gpu::CommandBuffer& of) noexcept
            : GpuObjectViewImplementation { of }, m_state { as_ref_mut(of.m_state) }, m_level { of.level() } {
        }

        /////////////////////////////////////
        /////////////////////////////////////
        template<cmeta::IsContainerOrPointerOf<gpu::CommandBuffer> TContainerOrPointer>
        STORMKIT_FORCE_INLINE
        inline CommandBufferImplementation::CommandBufferImplementation(const TContainerOrPointer& of) noexcept
            : CommandBufferImplementation { *of } {
        }

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline CommandBufferImplementation::~CommandBufferImplementation() noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline CommandBufferImplementation::CommandBufferImplementation(const CommandBufferImplementation&) noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto CommandBufferImplementation::operator=(const CommandBufferImplementation& other) noexcept
          -> CommandBufferImplementation& {
            if (&other == this) [[unlikely]]
                return *this;

            GpuObjectViewImplementation::operator=(other);

            m_level = other.m_level;
            m_state = as_ref_mut(other.m_state);

            return *this;
        }

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline CommandBufferImplementation::CommandBufferImplementation(CommandBufferImplementation&&) noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto CommandBufferImplementation::operator=(CommandBufferImplementation&&) noexcept
          -> CommandBufferImplementation& = default;
    } // namespace view

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline CommandPoolImplementation::CommandPoolImplementation(PrivateTag, view::Device&& device) noexcept
        : GpuObjectImplementation { std::move(device), &VolkDeviceTable::vkDestroyCommandPool } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline CommandPoolImplementation::~CommandPoolImplementation() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline CommandPoolImplementation::CommandPoolImplementation(CommandPoolImplementation&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto CommandPoolImplementation::operator=(CommandPoolImplementation&&) noexcept
      -> CommandPoolImplementation& = default;
} // namespace stormkit::gpu
