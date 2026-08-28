// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>
#include <stormkit/core/try_expected.hpp>

#include <stormkit/gpu/api.hpp>
#define STORMKIT_DEFINE_VK_PLATFORM
#include <stormkit/gpu/vulkan.hpp>

module stormkit.gpu.core;

import std;

import stormkit.core;

namespace stdr = std::ranges;
namespace stdv = std::views;

using namespace std::literals;

namespace cmonadic = stormkit::core::monadic;

namespace stormkit::gpu {
    namespace {
        constexpr auto VALIDATION_LAYERS = into_array_of<czstring>("VK_LAYER_KHRONOS_validation",
                                                                   // "VK_LAYER_LUNARG_api_dump",
                                                                   "VK_LAYER_LUNARG_monitor",
                                                                   "VK_LAYER_MESA_overlay");

        // [[maybe_unused]]
        // constexpr auto VALIDATION_FEATURES = into_array_of<czstring>(VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
        //                                                                      VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT);

        constexpr auto
          STORMKIT_VK_VERSION = vk::make_version<i32>(STORMKIT_MAJOR_VERSION, STORMKIT_MINOR_VERSION, STORMKIT_PATCH_VERSION);

        constexpr auto BASE_EXTENSIONS = into_array_of<czstring>(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
#ifdef STORMKIT_OS_APPLE
                                                                 ,
                                                                 VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
#endif
        );

        constexpr auto SURFACE_EXTENSIONS = into_array_of<czstring>(VK_KHR_SURFACE_EXTENSION_NAME,
                                                                    VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME
                                                                    // VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME,
        );

        constexpr auto WSI_SURFACE_EXTENSIONS = into_array_of<czstring>(
#ifdef STORMKIT_OS_WINDOWS
          VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif defined(STORMKIT_OS_LINUX)
          VK_KHR_XCB_SURFACE_EXTENSION_NAME,
          VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
#elif defined(STORMKIT_OS_MACOS)
          VK_MVK_MACOS_SURFACE_EXTENSION_NAME
#elif defined(STORMKIT_OS_IOS)
          VK_MVK_IOS_SURFACE_EXTENSION_NAME
#endif
        );

        /////////////////////////////////////
        /////////////////////////////////////
        auto check_extension_support(array_view<const string>      supported_extensions,
                                     array_view<const string_view> extensions) noexcept -> std::optional<hash_set<string_view>> {
            auto required_extensions = hash_set<string_view> { stdr::begin(extensions), stdr::end(extensions) };

            for (const auto& extension : supported_extensions) required_extensions.erase(extension);

            if (not required_extensions.empty()) return required_extensions;

            return std::nullopt;
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto check_extension_support(array_view<const string>   supported_extensions,
                                     array_view<const czstring> extensions) noexcept -> std::optional<hash_set<string_view>> {
            const auto ext = transform(extensions, cmonadic::init<string_view>());
            return check_extension_support(supported_extensions, ext);
        }
    } // namespace

    template class InstanceInterface<InstanceImplementation>;
    template class InstanceInterface<view::InstanceImplementation>;

    /////////////////////////////////////
    /////////////////////////////////////
    auto InstanceImplementation::do_init(PrivateTag, const CreateInfo& create_info) noexcept -> Expected<void> {
        const auto exts = Try(vk::enumerate_checked<VkExtensionProperties>(vkEnumerateInstanceExtensionProperties, nullptr));
        m_extensions    = transform(exts, [](const auto& ext) static noexcept { return string { ext.extensionName }; });
        const auto available_layers = Try(vk::enumerate_checked<VkLayerProperties>(vkEnumerateInstanceLayerProperties));
        // std::println("{}", available_layers | stdv::transform([](const auto& layer) static noexcept {
        //                        return std::string_view { layer.layerName };
        //                    }));
        const auto validation_layers = create_info.enable_validation_layers
                                         ? transform_if(
                                             available_layers,
                                             [](const auto& layer) static noexcept {
                                                 return stdr::contains(VALIDATION_LAYERS, string_view { layer.layerName });
                                             },
                                             [](const auto& layer) static noexcept { return layer.layerName; })
                                         : dyn_array<czstring>();

        const auto instance_extensions = [enable_validation_layers = create_info.enable_validation_layers] noexcept {
            auto e = concat(BASE_EXTENSIONS, SURFACE_EXTENSIONS, WSI_SURFACE_EXTENSIONS);
            if (enable_validation_layers) e.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            return e;
        }();
        const auto result = check_extension_support(m_extensions, instance_extensions);
        if (result.has_value()) ensures(true, std::format("Missing extensions! {}", result.value()));

        constexpr auto ENGINE_NAME = "StormKit";

        const auto app_info = VkApplicationInfo {
            .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext              = nullptr,
            .pApplicationName   = std::data(create_info.application_name),
            .applicationVersion = create_info.application_version,
            .pEngineName        = ENGINE_NAME,
            .engineVersion      = STORMKIT_VK_VERSION,
            .apiVersion         = VK_API_VERSION_1_3,
        };

        const auto vk_create_info = VkInstanceCreateInfo {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
#ifdef STORMKIT_OS_APPLE
            .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
#else
            .flags = 0,
#endif
            .pApplicationInfo        = &app_info,
            .enabledLayerCount       = as<u32>(stdr::size(validation_layers)),
            .ppEnabledLayerNames     = stdr::data(validation_layers),
            .enabledExtensionCount   = as<u32>(stdr::size(instance_extensions)),
            .ppEnabledExtensionNames = stdr::data(instance_extensions),
        };

        m_vk_handle = Try(vk::call_checked<VkInstance>(vkCreateInstance, &vk_create_info, nullptr));

        Try(do_load_instance());
        Try(do_retrieve_physical_devices());

        Return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto InstanceImplementation::do_load_instance() noexcept -> Expected<void> {
        volkLoadInstanceOnly(m_vk_handle);
        Return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto InstanceImplementation::do_retrieve_physical_devices() noexcept -> Expected<void> {
        m_physical_devices = transform(Try(vk::enumerate_checked<VkPhysicalDevice>(vkEnumeratePhysicalDevices, m_vk_handle)),
                                       [this](auto physical_device) noexcept {
                                           return PhysicalDevice::create(view::Instance { *this }, std::move(physical_device));
                                       });
        Return {};
    }
} // namespace stormkit::gpu
