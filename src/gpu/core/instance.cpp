// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <vulkan/vulkan_hpp_macros.hpp>

#include <stormkit/core/platform_macro.hpp>
#include <stormkit/log/log_macro.hpp>

module stormkit.gpu;

import std;

import stormkit.core;
import stormkit.log;

import stormkit.gpu.vulkan;

namespace stormkit::gpu {
    LOGGER("stormkit.gpu")

    namespace {
        constexpr auto VALIDATION_LAYERS = std::array {
            "VK_LAYER_KHRONOS_validation",
#ifdef STORMKIT_OS_LINUX
            "VK_LAYER_MESA_overlay",
#endif
        };

        [[maybe_unused]]
        constexpr auto VALIDATION_FEATURES
            = std::array { VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
                           VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT };

        constexpr auto STORMKIT_VK_VERSION = vkMakeVersion<Int32>(STORMKIT_MAJOR_VERSION,
                                                                  STORMKIT_MINOR_VERSION,
                                                                  STORMKIT_PATCH_VERSION);

        constexpr auto BASE_EXTENSIONS = std::array { "VK_KHR_get_physical_device_properties2" };

        constexpr auto SURFACE_EXTENSIONS = std::array {
            "VK_KHR_surface",
        };

        constexpr auto WSI_SURFACE_EXTENSIONS = std::array {
#ifdef STORMKIT_OS_WINDOWS
            "VK_KHR_win32_surface"
#elif defined(STORMKIT_OS_LINUX)
            "VK_KHR_xcb_surface",
            "VK_KHR_wayland_surface"
#elif defined(STORMKIT_OS_MACOS)
            "VK_MVK_MACOS_surface"
#elif defined(STORMKIT_OS_IOS)
            "VK_MVK_IOS_surface"
#endif
        };

        /////////////////////////////////////
        /////////////////////////////////////
        auto debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                           VkDebugUtilsMessageTypeFlagsEXT,
                           const VkDebugUtilsMessengerCallbackDataEXT& callback_data,
                           void*) -> bool {
            auto message = std::format("{}", callback_data.pMessage);

            if (checkFlag(severity, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT))
                ilog("{}", message);
            else if (checkFlag(severity, VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
                dlog("{}", message);
            else if (checkFlag(severity, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
                elog("{}", message);
            else if (checkFlag(severity, VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
                wlog("{}", message);

            return false;
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto checkValidationLayerSupport(bool validation_layers_enabled) noexcept -> bool {
            if (!validation_layers_enabled) return false;

            const auto layers = vkEnumerate<VkLayerProperties>(vkEnumerateInstanceLayerProperties);
            dlog("Layers found: {}",
                 layers | std::views::transform([](auto&& layer) static noexcept {
                     return std::string_view { layer.layerName };
                 }));
            for (const auto& layer_name : std::as_const(VALIDATION_LAYERS)) {
                auto layer_found = false;

                for (const auto& layer_properties : layers) {
                    if (std::strcmp(layer_name, layer_properties.layerName) == 0) {
                        layer_found = true;
                        break;
                    }
                }

                if (!layer_found) {
                    dlog("Failed to find validation layers, disabling...");
                    return false;
                }
            }

            return validation_layers_enabled;
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto checkExtensionSupport(std::span<const std::string>      supported_extensions,
                                   std::span<const std::string_view> extensions) noexcept -> bool {
            auto required_extensions = HashSet<std::string_view> { std::ranges::begin(extensions),
                                                                   std::ranges::end(extensions) };

            for (const auto& extension : supported_extensions) required_extensions.erase(extension);

            return required_extensions.empty();
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto checkExtensionSupport(std::span<const std::string> supported_extensions,
                                   std::span<const CZString>    extensions) noexcept -> bool {
            // clang-format off
            const auto ext = extensions 
                             | std::views::transform(core::monadic::init<std::string_view>())
                             | std::ranges::to<std::vector>();
            // clang-format on
            return checkExtensionSupport(supported_extensions, ext);
        }
    } // namespace

    /////////////////////////////////////
    /////////////////////////////////////
    auto Instance::doInitInstance() noexcept -> VulkanExpected<void> {
        const auto exts
            = vkEnumerate<VkExtensionProperties>(vkEnumerateInstanceExtensionProperties);

        m_extensions = exts
                       | std::views::transform([](auto&& extension) noexcept {
                             return std::string { extension.extensionName };
                         })
                       | std::ranges::to<std::vector>();

        dlog("Instance extensions: {}", m_extensions);

        const auto validation_layers = [this]() noexcept {
            auto output                 = std::vector<CZString> {};
            m_validation_layers_enabled = checkValidationLayerSupport(m_validation_layers_enabled);
            if (m_validation_layers_enabled) {
                ilog("Enabling layers: {}", VALIDATION_LAYERS);

                output = VALIDATION_LAYERS | std::ranges::to<std::vector>();
            }

            return output;
        }();

        const auto instance_extensions = [this]() noexcept {
            auto e = concat(BASE_EXTENSIONS, SURFACE_EXTENSIONS);

            for (auto&& ext_ : WSI_SURFACE_EXTENSIONS) {
                const auto ext = std::array { ext_ };
                if (checkExtensionSupport(m_extensions, ext)) merge(e, ext);
            }

            if (m_validation_layers_enabled) merge(e, std::array { "VK_EXT_debug_utils" });

            return e;
        }();

        constexpr auto ENGINE_NAME = "StormKit";

        const auto app_info = VkApplicationInfo { .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                                  .pApplicationName = std::data(m_app_name),
                                                  .pEngineName      = ENGINE_NAME,
                                                  .engineVersion    = STORMKIT_VK_VERSION,
                                                  .apiVersion = vkMakeVersion<Int32>(1, 0, 0) };

        const auto create_info = VkInstanceCreateInfo {
            .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo        = &app_info,
            .enabledExtensionCount   = std::ranges::size(instance_extensions),
            .ppEnabledExtensionNames = std::ranges::data(instance_extensions),
            .enabledLayerCount       = std::ranges::size(validation_layers),
            .ppEnabledLayerNames     = std::ranges::data(validation_layers)
        };

        return vkCall < (create_info)
                            .transform(core ::monadic::set(m_vk_instance))
                            .and_then(bindFront(&Instance::doInitDebugReportCallback, this))
                            .transform([this]() noexcept {
                                VULKAN_HPP_DEFAULT_DISPATCHER.init(*m_vk_instance.get());
                            });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Instance::doInitDebugReportCallback() noexcept -> VulkanExpected<void> {
        if (!m_validation_layers_enabled) return {};
        constexpr auto severity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
                                  | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                                  | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

        constexpr auto type = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                              | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
                              | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

        const auto create_info = vk::DebugUtilsMessengerCreateInfoEXT {
            .messageSeverity = severity,
            .messageType     = type,
            .pfnUserCallback
            = std::bit_cast<decltype(vk::DebugUtilsMessengerCreateInfoEXT::pfnUserCallback)>(
                &debugCallback)
        };

        return m_vk_instance->createDebugUtilsMessengerEXT(create_info)
            .transform(core :.monadic::set(m_vk_messenger))
            .transform([] noexcept { ilog("Validation layers successfully enabled !"); });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Instance::doRetrievePhysicalDevices() noexcept -> VulkanExpected<void> {
        return m_vk_instance->enumeratePhysicalDevices().transform([this](auto&& physical_devices) {
            // clang-format off
            m_physical_devices = std::forward<decltype(physical_devices)>(physical_devices) 
              | std::views::transform([](auto&& physical_device) static noexcept {
                       return PhysicalDevice { std::move(physical_device) };
              })
              | std::ranges::to<std::vector>();
            // clang-format on
        });
    }
} // namespace stormkit::gpu
