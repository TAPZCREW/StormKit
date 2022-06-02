// Copyright (C) 2022 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#pragma once

#include <array>
#include <memory>
#include <optional>
#include <span>
#include <vector>

#include <stormkit/core/MemoryMacro.hpp>

namespace stormkit::gpu {
    struct PhysicalDeviceInfo;
    struct RenderCapabilities;

    class Instance;
    DECLARE_PTR_AND_REF(Instance);

    class PhysicalDevice;
    DECLARE_PTR_AND_REF(PhysicalDevice);

    class Device;
    DECLARE_PTR_AND_REF(Device);

    class Surface;
    DECLARE_PTR_AND_REF(Surface);

    class WindowSurface;
    DECLARE_PTR_AND_REF(WindowSurface);

    class OffscreenSurface;
    DECLARE_PTR_AND_REF(OffscreenSurface);

    class Queue;
    DECLARE_PTR_AND_REF(Queue);

    class CommandBuffer;
    DECLARE_PTR_AND_REF(CommandBuffer);
} // namespace stormkit::gpu
