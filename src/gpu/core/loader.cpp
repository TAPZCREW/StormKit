module stormkit.gpu;

import stormkit.gpu.vulkan;

namespace stormkit::gpu {
    auto initialize_backend() -> Expected<void> {
        return vk_call(volkInitialize).transform_error(monadic::from_vkflags<Result>());
    }
} // namespace stormkit::gpu
