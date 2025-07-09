#include <stormkit/core/platform_macro.hpp>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#define VOLK_IMPLEMENTATION
#include <volk.h>

#include "assert.hpp"

#define VMA_CALL_PRE STORMKIT_API
#define VMA_IMPLEMENTATION
#define VMA_ASSERT(expr) vma_assert(expr, #expr)
#include <vk_mem_alloc.h>
