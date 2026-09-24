#include <stormkit/core/platform_macro.hpp>

#include <stormkit/gpu/api.hpp>

#define STORMKIT_DEFINE_VK_PLATFORM
#define VOLK_IMPLEMENTATION
#include <stormkit/gpu/vulkan.hpp>

#include "assert.hpp"

#define VMA_CALL_PRE STORMKIT_GPU_API
#define VMA_IMPLEMENTATION
#define VMA_ASSERT(expr) vma_assert(expr, #expr)
#include <vk_mem_alloc.h>
