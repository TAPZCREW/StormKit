module stormkit.Gpu;

import std;

import stormkit.Core;

import stormkit.Gpu.Vulkan;

import :Core;
import :Resource.Swapchain;

namespace stormkit::gpu {
    namespace {
        /////////////////////////////////////
        /////////////////////////////////////
        auto chooseSwapSurfaceFormat(std::span<const vk::SurfaceFormatKHR> formats) noexcept
            -> vk::SurfaceFormatKHR {
            for (const auto& format : formats) {
                if (format.format == vk::Format::eB8G8R8A8Unorm &&
                    format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
                    return format;
            }

            return formats[0];
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto chooseSwapPresentMode(std::span<const vk::PresentModeKHR> present_modes) noexcept
            -> vk::PresentModeKHR {
            auto present_mode_ = vk::PresentModeKHR::eFifo;

            for (const auto& present_mode : present_modes) {
                if (present_mode == vk::PresentModeKHR::eImmediate) return present_mode;
                else if (present_mode == vk::PresentModeKHR::eMailbox)
                    return present_mode;
            }

            return present_mode_;
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities,
                              const math::ExtentU&              extent) noexcept -> vk::Extent2D {
            constexpr static auto int_max = std::numeric_limits<UInt32>::max();

            if (capabilities.currentExtent.width != int_max &&
                capabilities.currentExtent.height != int_max)
                return capabilities.currentExtent;

            auto actual_extent = as<vk::Extent2D>(extent);
            actual_extent.width =
                std::max(capabilities.minImageExtent.width,
                         std::min(capabilities.maxImageExtent.width, actual_extent.width));
            actual_extent.height =
                std::max(capabilities.minImageExtent.height,
                         std::min(capabilities.maxImageExtent.height, actual_extent.height));

            return actual_extent;
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto chooseImageCount(const vk::SurfaceCapabilitiesKHR& capabilities) noexcept -> UInt32 {
            auto image_count = capabilities.minImageCount + 1;

            if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount)
                image_count = capabilities.maxImageCount;

            return image_count;
        }
    } // namespace

    /////////////////////////////////////
    /////////////////////////////////////
    Swapchain::Swapchain(Tag,
                         const Device&                         device,
                         const Surface&                        surface,
                         const math::ExtentU&                  extent,
                         std::optional<vk::raii::SwapchainKHR> old_swapchain) {
        const auto& physical_device = device.physicalDevice();
        const auto  capabilities =
            physical_device.vkHandle().getSurfaceCapabilitiesKHR(*surface.vkHandle());
        const auto formats = physical_device.vkHandle().getSurfaceFormatsKHR(*(surface.vkHandle()));
        const auto present_modes =
            physical_device.vkHandle().getSurfacePresentModesKHR(*(surface.vkHandle()));

        const auto format             = chooseSwapSurfaceFormat(formats);
        const auto present_mode       = chooseSwapPresentMode(present_modes);
        const auto swapchain_extent   = chooseSwapExtent(capabilities, extent);
        const auto image_count        = chooseImageCount(capabilities);
        const auto image_sharing_mode = vk::SharingMode::eExclusive;

        const auto create_info = [&] noexcept -> decltype(auto) {
            auto info =
                vk::SwapchainCreateInfoKHR { .surface          = *surface.vkHandle(),
                                             .minImageCount    = image_count,
                                             .imageFormat      = format.format,
                                             .imageColorSpace  = format.colorSpace,
                                             .imageExtent      = swapchain_extent,
                                             .imageArrayLayers = 1,
                                             .imageUsage = vk::ImageUsageFlagBits::eTransferDst,
                                             .imageSharingMode = image_sharing_mode,
                                             .preTransform     = capabilities.currentTransform,
                                             .compositeAlpha =
                                                 vk::CompositeAlphaFlagBitsKHR::eOpaque,
                                             .presentMode = present_mode,
                                             .clipped     = true };
            if (old_swapchain) info.oldSwapchain = **old_swapchain;
            return info;
        }();

        device.vkHandle()
            .createSwapchainKHR(create_info)
            .transform(core::monadic::set(m_vk_swapchain))
            .transform_error(core::monadic::map(core::monadic::narrow<Result>(), core::monadic::throwAsException()));

        m_extent       = as<math::ExtentU>(swapchain_extent);
        m_image_count  = as<UInt32>(std::size(m_images));
        m_pixel_format = narrow<PixelFormat>(format.format);
        // clang-format off
        m_images       = m_vk_swapchain->getImages() 
        | std::views::transform([&, this](auto&& image) noexcept {
             return SwapchainImage::create(m_extent, m_pixel_format, image);
        }) 
        | std::ranges::to<std::vector>();
        // clang-format on
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Swapchain::acquireNextImage(std::chrono::nanoseconds wait,
                                     const Semaphore&         image_available) const noexcept
        -> Expected<std::pair<gpu::Result, UInt32>> {
        auto&& [result, index] =
            m_vk_swapchain->acquireNextImage(wait.count(), toVkHandle(image_available));
        const auto possible_results = std::array { vk::Result::eSuccess,
                                                   vk::Result::eErrorOutOfDateKHR,
                                                   vk::Result::eSuboptimalKHR };

        if (not std::ranges::any_of(possible_results, core::monadic::is(result))) [[likely]]
            return std::unexpected { narrow<gpu::Result>(result) };

        return std::make_pair(narrow<gpu::Result>(result), index);
    }

} // namespace stormkit::gpu
