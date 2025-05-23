#include <stormkit/core/platform_macro.hpp>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#define VOLK_IMPLEMENTATION
#include <volk.h>

#define VMA_CALL_PRE STORMKIT_API
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
