// Copyright (C) 2023 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>
#include <stormkit/core/try_expected.hpp>

#include <stormkit/gpu/api.hpp>
#include <stormkit/gpu/vulkan.hpp>

export module stormkit.gpu.resource:shader;

import std;

import stormkit.core;
import stormkit.gpu.core;

import :objects;

namespace stdr = std::ranges;
namespace stdv = std::views;

namespace cmeta = stormkit::core::meta;

namespace stormkit::gpu {
    export template<typename Base>
    class STORMKIT_GPU_API ShaderInterface: public DeviceObject<Base> {
      public:
        using DeviceObject<Base>::DeviceObject;
        using DeviceObject<Base>::operator=;
        using TagType = ShaderTag;

        [[nodiscard]]
        auto type() const noexcept -> ShaderStageFlag;
        [[nodiscard]]
        auto source() const noexcept -> array_view<const SpirvID>;
        [[nodiscard]]
        auto source_as_bytes() const noexcept -> byte_view<>;
    };

    class STORMKIT_GPU_API
      ShaderImplementation: public GpuObjectImplementation<ShaderTag, dyn_array<SpirvID>&&, ShaderStageFlag> {
      public:
        enum class Error {
            INVALID_SPIRV,
        };

        using LoadError = std::variant<core::SystemError, Result, Error>;
        template<typename T>
        using LoadExpected = std::expected<T, LoadError>;

        ShaderImplementation(PrivateTag, view::Device&&) noexcept;
        ~ShaderImplementation() noexcept;

        ShaderImplementation(const ShaderImplementation&) noexcept                    = delete;
        auto operator=(const ShaderImplementation&) noexcept -> ShaderImplementation& = delete;

        ShaderImplementation(ShaderImplementation&&) noexcept;
        auto operator=(ShaderImplementation&&) noexcept -> ShaderImplementation&;

        static auto load_from_file(view::Device device, const std::filesystem::path& filepath, ShaderStageFlag type) noexcept
          -> LoadExpected<Shader>;
        static auto load_from_bytes(view::Device device, byte_view<> data, ShaderStageFlag type) noexcept -> Expected<Shader>;
        static auto load_from_spirv(view::Device device, array_view<const SpirvID> data, ShaderStageFlag type) noexcept
          -> Expected<Shader>;

        static auto allocate_and_load_from_file(view::Device                 device,
                                                const std::filesystem::path& filepath,
                                                ShaderStageFlag              type) noexcept -> LoadExpected<Heap<Shader>>;
        static auto allocate_and_load_from_bytes(view::Device device, byte_view<> data, ShaderStageFlag type) noexcept
          -> Expected<Heap<Shader>>;
        static auto allocate_and_load_from_spirv(view::Device              device,
                                                 array_view<const SpirvID> data,
                                                 ShaderStageFlag           type) noexcept -> Expected<Heap<Shader>>;

        auto do_init(PrivateTag, dyn_array<SpirvID>&&, ShaderStageFlag) -> Expected<void>;

      protected:
        using NamedConstructor::allocate;
        using NamedConstructor::create;

        ShaderStageFlag    m_type   = ShaderStageFlag::NONE;
        dyn_array<SpirvID> m_source = {};
    };

    namespace view {
        class ShaderImplementation: public GpuObjectViewImplementation<ShaderTag> {
          public:
            ShaderImplementation(const gpu::Shader&) noexcept;
            template<cmeta::IsContainerOrPointerOf<gpu::Shader> TContainerOrPointer>
            ShaderImplementation(const TContainerOrPointer&) noexcept;
            ~ShaderImplementation() noexcept;

            ShaderImplementation(const ShaderImplementation&) noexcept;
            auto operator=(const ShaderImplementation&) noexcept -> ShaderImplementation&;

            ShaderImplementation(ShaderImplementation&&) noexcept;
            auto operator=(ShaderImplementation&&) noexcept -> ShaderImplementation&;

          protected:
            ShaderStageFlag           m_type;
            array_view<const SpirvID> m_source;
        };
    } // namespace view
} // namespace stormkit::gpu

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::gpu {
    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto sys_to_load_error(SystemError&& error) noexcept -> Shader::LoadError {
        return Shader::LoadError { std::move(error) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_PURE
    inline auto result_to_load_error(gpu::Result&& error) noexcept -> Shader::LoadError {
        return Shader::LoadError { std::move(error) };
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto ShaderInterface<Base>::type() const noexcept -> ShaderStageFlag {
        return Base::m_type;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto ShaderInterface<Base>::source() const noexcept -> array_view<const SpirvID> {
        return Base::m_source;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    STORMKIT_FORCE_INLINE
    inline auto ShaderInterface<Base>::source_as_bytes() const noexcept -> byte_view<> {
        return as_bytes(Base::m_source);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline ShaderImplementation::ShaderImplementation(PrivateTag, view::Device&& device) noexcept
        : GpuObjectImplementation { std::move(device), &VolkDeviceTable::vkDestroyShaderModule } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline ShaderImplementation::~ShaderImplementation() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline ShaderImplementation::ShaderImplementation(ShaderImplementation&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto ShaderImplementation::operator=(ShaderImplementation&&) noexcept -> ShaderImplementation& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto ShaderImplementation::load_from_file(view::Device                 device,
                                                     const std::filesystem::path& filepath,
                                                     ShaderStageFlag              type) noexcept -> LoadExpected<Shader> {
        expects(std::filesystem::is_regular_file(filepath), std::format("{} is not a file", filepath.string()));

        const auto data  = TryTransformError(io::read(filepath), sys_to_load_error);
        auto       spirv = dyn_array<SpirvID> { std::from_range, bytes_as_span<SpirvID>(data) };
        Return     TryTransformError(NamedConstructor::create(std::move(device), std::move(spirv), type), result_to_load_error);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto ShaderImplementation::load_from_bytes(view::Device device, byte_view<> data, ShaderStageFlag type) noexcept
      -> Expected<Shader> {
        auto spirv = dyn_array<SpirvID> { std::from_range, bytes_as_span<SpirvID>(data) };
        return NamedConstructor::create(std::move(device), std::move(spirv), type);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto ShaderImplementation::load_from_spirv(view::Device              device,
                                                      array_view<const SpirvID> data,
                                                      ShaderStageFlag           type) noexcept -> Expected<Shader> {
        auto spirv = dyn_array<SpirvID> { std::from_range, data };
        return NamedConstructor::create(std::move(device), std::move(spirv), type);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto ShaderImplementation::allocate_and_load_from_file(view::Device                 device,
                                                                  const std::filesystem::path& filepath,
                                                                  ShaderStageFlag type) noexcept -> LoadExpected<Heap<Shader>> {
        expects(std::filesystem::is_regular_file(filepath), std::format("{} is not a file", filepath.string()));

        const auto data  = TryTransformError(io::read(filepath), sys_to_load_error);
        auto       spirv = dyn_array<SpirvID> { std::from_range, bytes_as_span<SpirvID>(data) };
        Return     TryTransformError(NamedConstructor::allocate(std::move(device), std::move(spirv), type), result_to_load_error);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto ShaderImplementation::allocate_and_load_from_bytes(view::Device    device,
                                                                   byte_view<>     data,
                                                                   ShaderStageFlag type) noexcept -> Expected<Heap<Shader>> {
        auto spirv = dyn_array<SpirvID> { std::from_range, bytes_as_span<SpirvID>(data) };
        return NamedConstructor::allocate(std::move(device), std::move(spirv), type);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto ShaderImplementation::allocate_and_load_from_spirv(view::Device              device,
                                                                   array_view<const SpirvID> data,
                                                                   ShaderStageFlag type) noexcept -> Expected<Heap<Shader>> {
        auto spirv = dyn_array<SpirvID> { std::from_range, data };
        return NamedConstructor::allocate(std::move(device), std::move(spirv), type);
    }

    namespace view {
        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline ShaderImplementation::ShaderImplementation(const gpu::Shader& of) noexcept
            : GpuObjectViewImplementation { of }, m_type { of.type() }, m_source { of.source() } {
        }

        /////////////////////////////////////
        /////////////////////////////////////
        template<cmeta::IsContainerOrPointerOf<gpu::Shader> TContainerOrPointer>
        STORMKIT_FORCE_INLINE
        inline ShaderImplementation::ShaderImplementation(const TContainerOrPointer& of) noexcept
            : ShaderImplementation { *of } {
        }

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline ShaderImplementation::~ShaderImplementation() noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline ShaderImplementation::ShaderImplementation(const ShaderImplementation&) noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto ShaderImplementation::operator=(const ShaderImplementation&) noexcept -> ShaderImplementation& = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline ShaderImplementation::ShaderImplementation(ShaderImplementation&&) noexcept = default;

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE
        inline auto ShaderImplementation::operator=(ShaderImplementation&&) noexcept -> ShaderImplementation& = default;
    } // namespace view
} // namespace stormkit::gpu
