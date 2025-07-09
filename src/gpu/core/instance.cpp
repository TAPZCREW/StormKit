// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#include <stormkit/log/log_macro.hpp>

#include <stormkit/core/contract_macro.hpp>

#include <volk.h>

module stormkit.gpu.core;

import std;

import stormkit.core;
import stormkit.log;

namespace stdr = std::ranges;
namespace stdv = std::views;

namespace stormkit::gpu {
    LOGGER("stormkit.gpu")

    namespace {
        constexpr auto VALIDATION_LAYERS = std::array {
            "VK_LAYER_KHRONOS_validation",
            // "VK_LAYER_LUNARG_api_dump",
            "VK_LAYER_LUNARG_monitor",
#ifdef STORMKIT_OS_LINUX
        // "VK_LAYER_MESA_overlay",
#endif
        };

        [[maybe_unused]]
        constexpr auto VALIDATION_FEATURES
          = std::array {
                VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
                VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
            };

        constexpr auto STORMKIT_VK_VERSION = vk_make_version<i32>(STORMKIT_MAJOR_VERSION,
                                                                  STORMKIT_MINOR_VERSION,
                                                                  STORMKIT_PATCH_VERSION);

        constexpr auto BASE_EXTENSIONS = std::array {
            VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
        };

        constexpr auto SURFACE_EXTENSIONS = std::array {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
            VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME,
        };

        constexpr auto WSI_SURFACE_EXTENSIONS = std::array {
#ifdef STORMKIT_OS_WINDOWS
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(STORMKIT_OS_LINUX)
            VK_KHR_XCB_SURFACE_EXTENSION_NAME,
            VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
#elif defined(STORMKIT_OS_MACOS)
            VK_MVK_MACOS_SURFACE_EXTENSION_NAME,
#elif defined(STORMKIT_OS_IOS)
            VK_MVK_IOS_SURFACE_EXTENSION_NAME,
#endif
        };

        /////////////////////////////////////
        /////////////////////////////////////
        auto debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                            VkDebugUtilsMessageTypeFlagsEXT,
                            const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                            void*) noexcept -> u32 {
            EXPECTS(callback_data);
            auto message = std::format("{}", callback_data->pMessage);

            if (check_flag_bit(severity, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT))
                ilog("{}", message);
            else if (check_flag_bit(severity, VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT))
                dlog("{}", message);
            else if (check_flag_bit(severity, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT))
                elog("{}", message);
            else if (check_flag_bit(severity, VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT))
                wlog("{}", message);

            return 0;
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto check_validation_layer_support(bool validation_layers_enabled) noexcept -> bool {
            if (!validation_layers_enabled) return false;

            const auto result = vk_enumerate<VkLayerProperties>(vkEnumerateInstanceLayerProperties);
            if (not result) return false;
            const auto layers = std::move(result).value();
            dlog("Layers found: {}", layers | stdv::transform([](auto&& layer) static noexcept {
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

            return true;
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto check_extension_support(std::span<const std::string>      supported_extensions,
                                     std::span<const std::string_view> extensions) noexcept
          -> bool {
            auto required_extensions = HashSet<std::string_view> { stdr::begin(extensions),
                                                                   stdr::end(extensions) };

            for (const auto& extension : supported_extensions) required_extensions.erase(extension);

            return required_extensions.empty();
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto check_extension_support(std::span<const std::string> supported_extensions,
                                     std::span<const CZString>    extensions) noexcept -> bool {
            const auto ext = extensions
                             | stdv::transform(core::monadic::init<std::string_view>())
                             | stdr::to<std::vector>();
            return check_extension_support(supported_extensions, ext);
        }
    } // namespace

    /////////////////////////////////////
    /////////////////////////////////////
    auto Instance::do_init() noexcept -> Expected<void> {
        return vk_enumerate<VkExtensionProperties>(vkEnumerateInstanceExtensionProperties, nullptr)
          .and_then([this](auto&& exts) noexcept {
              m_extensions = exts
                             | stdv::transform([](auto&& extension) static noexcept {
                                   return std::string { extension.extensionName };
                               })
                             | stdr::to<std::vector>();

              dlog("Instance extensions: {}", m_extensions);

              const auto validation_layers = [this]() noexcept {
                  auto output = std::vector<CZString> {};
                  m_validation_layers_enabled
                    = check_validation_layer_support(m_validation_layers_enabled);
                  if (m_validation_layers_enabled) {
                      output = VALIDATION_LAYERS | stdr::to<std::vector>();
                  }

                  return output;
              }();
              ilog("Enabled layers: {}", validation_layers);

              const auto instance_extensions = [this]() noexcept {
                  auto e = concat(BASE_EXTENSIONS, SURFACE_EXTENSIONS);

                  for (auto&& ext_ : WSI_SURFACE_EXTENSIONS) {
                      const auto ext = std::array { ext_ };
                      if (check_extension_support(m_extensions, ext)) merge(e, ext);
                  }

                  if (m_validation_layers_enabled)
                      merge(e, std::array { VK_EXT_DEBUG_UTILS_EXTENSION_NAME });

                  return e;
              }();
              ilog("Enabled instance extensions: {}", instance_extensions);

              constexpr auto ENGINE_NAME = "StormKit";

              const auto app_info = VkApplicationInfo {
                  .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                  .pNext              = nullptr,
                  .pApplicationName   = std::data(m_app_name),
                  .applicationVersion = vk_make_version<i32>(0, 0, 0),
                  .pEngineName        = ENGINE_NAME,
                  .engineVersion      = STORMKIT_VK_VERSION,
                  .apiVersion         = VK_API_VERSION_1_1,
              };

              const auto create_info = VkInstanceCreateInfo {
                  .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                  .pNext                   = nullptr,
                  .flags                   = 0,
                  .pApplicationInfo        = &app_info,
                  .enabledLayerCount       = as<u32>(stdr::size(validation_layers)),
                  .ppEnabledLayerNames     = stdr::data(validation_layers),
                  .enabledExtensionCount   = as<u32>(stdr::size(instance_extensions)),
                  .ppEnabledExtensionNames = stdr::data(instance_extensions),
              };
              return vk_call<VkInstance>(vkCreateInstance, &create_info, nullptr);
          })
          .transform(core::monadic::set(m_vk_handle))
          .and_then(bind_front(&Instance::do_load_instance, this))
          .and_then(bind_front(&Instance::do_retrieve_physical_devices, this))
          .and_then(bind_front(&Instance::do_init_debug_report_callback, this))
          .transform_error(monadic::from_vk<Result>());
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Instance::do_load_instance() noexcept -> VulkanExpected<void> {
        volkLoadInstanceOnly(m_vk_handle);
        return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Instance::do_init_debug_report_callback() noexcept -> VulkanExpected<void> {
        if (!m_validation_layers_enabled) return {};
        constexpr auto severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                  | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                  | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        constexpr auto type = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                              | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                              | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        const auto create_info = VkDebugUtilsMessengerCreateInfoEXT {
            .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext           = nullptr,
            .flags           = 0,
            .messageSeverity = severity,
            .messageType     = type,
            .pfnUserCallback = debug_callback,
            .pUserData       = nullptr,
        };

        m_vk_debug_utils_handle = { [vk_instance = m_vk_handle.value()](auto handle) noexcept {
            vkDestroyDebugUtilsMessengerEXT(vk_instance, handle, nullptr);
        } };

        return vk_call<VkDebugUtilsMessengerEXT>(vkCreateDebugUtilsMessengerEXT,
                                                 m_vk_handle,
                                                 &create_info,
                                                 nullptr)
          .transform(core::monadic::set(m_vk_debug_utils_handle))
          .transform([] static noexcept { ilog("Vulkan debug callback enabled!"); });
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Instance::do_retrieve_physical_devices() noexcept -> VulkanExpected<void> {
        return vk_enumerate<VkPhysicalDevice>(vkEnumeratePhysicalDevices, m_vk_handle)
          .transform([this](auto&& physical_devices) {
              m_physical_devices = std::forward<decltype(physical_devices)>(physical_devices)
                                   | stdv::transform([](auto&& physical_device) static noexcept {
                                         return PhysicalDevice { std::move(physical_device) };
                                     })
                                   | stdr::to<std::vector>();
          });
    }
} // namespace stormkit::gpu
