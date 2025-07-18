module;

#include <stormkit/core/contract_macro.hpp>

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
        auto choose_swap_surface_format(std::span<const VkSurfaceFormatKHR> formats) noexcept
          -> VkSurfaceFormatKHR {
            for (const auto& format : formats) {
                if (format.format == VK_FORMAT_B8G8R8A8_UNORM
                    && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                    return format;
            }

            return formats[0];
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto choose_swap_present_mode(std::span<const VkPresentModeKHR> present_modes) noexcept
          -> VkPresentModeKHR {
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
        auto choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities,
                                const math::Extent2<u32>&       extent) noexcept -> VkExtent2D {
            static constexpr auto int_max = std::numeric_limits<u32>::max();

            if (capabilities.currentExtent.width != int_max
                && capabilities.currentExtent.height != int_max)
                return capabilities.currentExtent;

            auto actual_extent   = to_vk(extent);
            actual_extent.width  = std::max(capabilities.minImageExtent.width,
                                           std::min(capabilities.maxImageExtent.width,
                                                    actual_extent.width));
            actual_extent.height = std::max(capabilities.minImageExtent.height,
                                            std::min(capabilities.maxImageExtent.height,
                                                     actual_extent.height));

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
    auto SwapChain::do_init(const Device&             device,
                            const Surface&            surface,
                            const math::Extent3<u32>& extent,
                            VkSwapchainKHR            old_swapchain) noexcept -> Expected<void> {
        const auto& physical_device = device.physical_device();

        return vk_call<VkSurfaceCapabilitiesKHR>(vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
                                                 physical_device.native_handle(),
                                                 surface.native_handle())
          .and_then([&physical_device, &surface](auto&& capabilities) noexcept {
              return vk_enumerate<VkSurfaceFormatKHR>(vkGetPhysicalDeviceSurfaceFormatsKHR,
                                                      physical_device.native_handle(),
                                                      surface.native_handle())
                .transform(core::monadic::as_tuple(std::move(capabilities)));
          })
          .and_then(core::monadic::unpack_tuple_to([&physical_device,
                                                    &surface](auto&& capabilities,
                                                              auto&& formats) noexcept {
              return vk_enumerate<VkPresentModeKHR>(vkGetPhysicalDeviceSurfacePresentModesKHR,
                                                    physical_device.native_handle(),
                                                    surface.native_handle())
                .transform(core::monadic::as_tuple(std::move(capabilities), std::move(formats)));
          }))
          .and_then(core::monadic::unpack_tuple_to([this,
                                                    &surface,
                                                    &extent,
                                                    &old_swapchain](auto&& capabilities,
                                                                    auto&& formats,
                                                                    auto&& present_modes) noexcept {
              const auto format             = choose_swap_surface_format(formats);
              const auto present_mode       = choose_swap_present_mode(present_modes);
              const auto swapchain_extent   = choose_swap_extent(capabilities, extent.to<2uz>());
              const auto image_count        = choose_image_count(capabilities);
              const auto image_sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
              const auto image_usage        = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                                       | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

              m_extent       = extent.to<u32>();
              m_pixel_format = from_vk<PixelFormat>(format.format);

              const auto create_info = VkSwapchainCreateInfoKHR {
                  .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                  .pNext                 = nullptr,
                  .flags                 = 0,
                  .surface               = surface.native_handle(),
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
                  .oldSwapchain          = old_swapchain
              };

              ENSURES(m_vk_device_table->vkCreateSwapchainKHR != nullptr);
              ENSURES(m_vk_device != nullptr);

              return vk_call<VkSwapchainKHR>(m_vk_device_table->vkCreateSwapchainKHR,
                                             m_vk_device,
                                             &create_info,
                                             nullptr);
          }))
          .transform(core::monadic::set(m_vk_handle))
          .and_then([this] noexcept {
              return vk_enumerate<VkImage>(m_vk_device_table->vkGetSwapchainImagesKHR,
                                           m_vk_device,
                                           m_vk_handle);
          })
          .transform([this, &device](auto&& vk_images) noexcept {
              m_image_count = as<u32>(std::ranges::size(vk_images));
              m_images      = vk_images
                         | std::views::transform([this, &device](auto&& image) noexcept {
                               const auto create_info = Image::CreateInfo {
                                   .extent = m_extent,
                                   .format = m_pixel_format
                               };
                               return Image::create(device, create_info, std::move(image));
                           })
                         | std::ranges::to<std::vector>();
          })
          .transform_error(monadic::from_vk<Result>());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto SwapChain::acquire_next_image(std::chrono::nanoseconds wait,
                                       const Semaphore&         image_available) const noexcept
      -> Expected<NextImage> {
        auto id = u32 { 0 };
        return vk_call<VkResult>(m_vk_device_table->vkAcquireNextImageKHR,
                                 { VK_SUCCESS, VK_ERROR_OUT_OF_DATE_KHR, VK_SUBOPTIMAL_KHR },
                                 m_vk_device,
                                 m_vk_handle,
                                 wait.count(),
                                 image_available.native_handle(),
                                 nullptr,
                                 &id)
          .transform([&id](auto&& result) {
              return NextImage { .result = from_vk<Result>(result), .id = id };
          })
          .transform_error(monadic::from_vk<Result>());
    }
} // namespace stormkit::gpu
