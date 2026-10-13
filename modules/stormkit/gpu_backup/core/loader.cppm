module;

#include <stormkit/gpu/api.hpp>

export module stormkit.gpu.core:loader;

import :structs;

export namespace stormkit::gpu {
    STORMKIT_GPU_API
    auto initialize_backend() -> expected<void>;
} // namespace stormkit::gpu
