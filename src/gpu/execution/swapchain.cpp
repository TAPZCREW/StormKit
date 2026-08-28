module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/try_expected.hpp>

#include <volk.h>

module stormkit.gpu.execution;

import std;

import stormkit.core;

import stormkit.gpu.core;
import stormkit.gpu.resource;

namespace stormkit::gpu {
    namespace {
        /////////////////////////////////////
        /////////////////////////////////////
        auto choose_swap_surface_format(array_view<const VkSurfaceFormatKHR> formats) noexcept -> VkSurfaceFormatKHR {
            for (const auto& format : formats) {
                if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                    return format;
            }

            return formats[0];
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto choose_swap_present_mode(array_view<const VkPresentModeKHR> present_modes) noexcept -> VkPresentModeKHR {
            auto present_mode_ = VK_PRESENT_MODE_FIFO_KHR;

            for (const auto& present_mode : present_modes) {
                if (present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR) return present_mode;
                else if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
                    return present_mode;
            }

            return present_mode_;
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities, const math::uextent2& extent) noexcept
          -> VkExtent2D {
            static constexpr auto int_max = std::numeric_limits<u32>::max();

            if (capabilities.currentExtent.width != int_max && capabilities.currentExtent.height != int_max)
                return capabilities.currentExtent;

            auto actual_extent   = vk::to_vk(extent);
            actual_extent.width  = std::max(capabilities.minImageExtent.width,
                                            std::min(capabilities.maxImageExtent.width, actual_extent.width));
            actual_extent.height = std::max(capabilities.minImageExtent.height,
                                            std::min(capabilities.maxImageExtent.height, actual_extent.height));

            return actual_extent;
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto choose_image_count(const VkSurfaceCapabilitiesKHR& capabilities) noexcept -> u32 {
            auto image_count = capabilities.minImageCount + 1;

            if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount)
                image_count = capabilities.maxImageCount;

            return image_count;
        }
    } // namespace

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Base>
    auto SwapChainInterface<Base>::acquire_next_image(std::chrono::nanoseconds wait,
                                                      view::Semaphore image_available) const noexcept -> Expected<NextImage> {
        const auto& device       = Base::owner();
        const auto& device_table = device.device_table();

        auto       id     = u32 { 0 };
        const auto result = Try((vk::call_checked<VkResult, VK_ERROR_OUT_OF_DATE_KHR, VK_SUBOPTIMAL_KHR>(
          device_table.vkAcquireNextImageKHR,
          device,
          *this,
          wait.count(),
          image_available,
          nullptr,
          &id)));
        Return     SwapChain::NextImage { .result = vk::from_vk<Result>(result), .id = id };
    }

    template class SwapChainInterface<SwapChainImplementation>;
    template class SwapChainInterface<view::SwapChainImplementation>;

    /////////////////////////////////////
    /////////////////////////////////////
    auto SwapChainImplementation::do_init(PrivateTag, const CreateInfo& create_info) noexcept -> Expected<void> {
        const auto& device          = owner();
        const auto& device_table    = device.device_table();
        const auto& physical_device = device.physical_device();

        const auto capabilities  = Try(vk::call_checked<VkSurfaceCapabilitiesKHR>(vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
                                                                                  physical_device,
                                                                                  create_info.surface));
        const auto formats       = Try(vk::enumerate_checked<VkSurfaceFormatKHR>(vkGetPhysicalDeviceSurfaceFormatsKHR,
                                                                                 physical_device,
                                                                                 create_info.surface));
        const auto present_modes = Try(vk::enumerate_checked<VkPresentModeKHR>(vkGetPhysicalDeviceSurfacePresentModesKHR,
                                                                               physical_device,
                                                                               create_info.surface));

        const auto format             = choose_swap_surface_format(formats);
        const auto present_mode       = choose_swap_present_mode(present_modes);
        const auto swapchain_extent   = choose_swap_extent(capabilities, create_info.extent.to<2>());
        const auto image_count        = choose_image_count(capabilities);
        const auto image_sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
        const auto image_usage        = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        m_extent       = create_info.extent;
        m_pixel_format = vk::from_vk<PixelFormat>(format.format);

        const auto vk_create_info = VkSwapchainCreateInfoKHR {
            .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext                 = VK_NULL_HANDLE,
            .flags                 = 0,
            .surface               = create_info.surface,
            .minImageCount         = image_count,
            .imageFormat           = format.format,
            .imageColorSpace       = format.colorSpace,
            .imageExtent           = swapchain_extent,
            .imageArrayLayers      = 1,
            .imageUsage            = image_usage,
            .imageSharingMode      = image_sharing_mode,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices   = nullptr,
            .preTransform          = capabilities.currentTransform,
            .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode           = present_mode,
            .clipped               = true,
            .oldSwapchain          = create_info.old,
        };

        ENSURES(device_table.vkCreateSwapchainKHR != nullptr);
        ENSURES(device_table.vkGetSwapchainImagesKHR != nullptr);

        m_vk_handle = Try(vk::call_checked<VkSwapchainKHR>(device_table.vkCreateSwapchainKHR, device, &vk_create_info, nullptr));
        const auto vk_images = Try(vk::enumerate_checked<VkImage>(device_table.vkGetSwapchainImagesKHR, device, m_vk_handle));

        m_image_count = as<u32>(stdr::size(vk_images));
        m_images      = transform(vk_images, [this, &device](auto image) noexcept {
            const auto create_info = Image::CreateInfo {
                .extent = { m_extent.width, m_extent.height, 1_u32 },
                .format = m_pixel_format
            };
            return Image::from_existing(device, create_info, image);
        });

        Return {};
    }
} // namespace stormkit::gpu
