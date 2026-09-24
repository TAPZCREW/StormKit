module;

#include <stormkit/gpu/vulkan.hpp>

module stormkit.gpu.core;

namespace stormkit::gpu {
    auto initialize_backend() -> expected<void> {
        return vk::call_checked(volkInitialize);
    }
} // namespace stormkit::gpu
