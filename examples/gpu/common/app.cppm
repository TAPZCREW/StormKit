// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/try_expected.hpp>

#include <stormkit/log/log_macro.hpp>

#include <stormkit/gpu/vulkan.hpp>

export module gpu_app;

import std;
import stormkit;

export import :logger;

using namespace stormkit;

namespace stdr = std::ranges;
namespace stdv = std::views;

NAMED_LOGGER(vulkan_logger, "vulkan")

extern "C" auto debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                               VkDebugUtilsMessageTypeFlagsEXT,
                               const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                               void*) noexcept -> u32 {
    EXPECTS(callback_data);
    auto message = std::format("{}", callback_data->pMessage);

    if (check_flag_bit(severity, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)) vulkan_logger.ilog("{}", message);
    else if (check_flag_bit(severity, VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT))
        vulkan_logger.dlog("{}", message);
    else if (check_flag_bit(severity, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT))
        vulkan_logger.elog("{}", message);
    else if (check_flag_bit(severity, VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT))
        vulkan_logger.wlog("{}", message);

    return 0;
}

static constexpr auto ENABLE_VALIDATION_LAYERS = false;

export namespace base {
    class Application {
      public:
        auto run(this auto& self, array_view<const string_view> args) {
            wsi::parse_args(args);
            log::parse_args(args);

            const auto example_name = self.example_name();

            auto logger_singleton = log::Logger::create_logger_instance<log::ConsoleLogger>();

            self.init_window(example_name);
            self.init_gpu(example_name);

            self.init_example();

            self.m_window->event_loop([&self] noexcept { self.run_example(); });

            DiscardTryAssert(self.m_raster_queue->wait_idle(), "Failed to wait for raster queue");
            self.m_device->wait_idle();

            if constexpr (requires { self.deinit(); }) self.deinit();
        }

      protected:
        DeferInit<wsi::Window>               m_window;
        DeferInit<gpu::Instance>             m_instance;
        DeferInit<gpu::DebugCallback>        m_debug_callback;
        DeferInit<gpu::Surface>              m_surface;
        DeferInit<gpu::view::PhysicalDevice> m_physical_device;
        DeferInit<gpu::Device>               m_device;
        DeferInit<gpu::SwapChain>            m_swapchain;
        DeferInit<gpu::Queue>                m_raster_queue;
        DeferInit<gpu::CommandPool>          m_command_pool;

      private:
        auto init_window(string_view example_name) noexcept -> void {
            m_window = wsi::Window::open(std::format("Stormkit GPU {} example", example_name),
                                         { 800_u32, 600_u32 },
                                         wsi::WindowFlag::DEFAULT | wsi::WindowFlag::EXTERNAL_CONTEXT);
            m_window->on<wsi::EventType::KEY_DOWN>([this](u8 /*id*/, wsi::Key key, char /*c*/) mutable noexcept {
                if (key == wsi::Key::ESCAPE) m_window->close();
            });
        }

        auto init_gpu(string_view example_name) noexcept -> void {
            // initialize gpu backend (vulkan or webgpu depending the platform)
            DiscardTryAssert(gpu::initialize_backend(), "Failed to initialize gpu backend");

            // create gpu instance and attach surface to window
            m_instance = TryAssert(gpu::Instance::create({ .application_name         = string { example_name },
                                                           .enable_validation_layers = ENABLE_VALIDATION_LAYERS }),
                                   "Failed to initialize gpu instance");

            if (ENABLE_VALIDATION_LAYERS) {
                m_debug_callback = TryAssert(gpu::DebugCallback::create(m_instance, { .messenger_closure = debug_callback }),
                                             "Failed to initialize gpu instance");
            }

            m_surface = TryAssert(gpu::Surface::create_from_window(m_instance, m_window),
                                  "Failed to initialize window gpu surface");

            // pick the best physical device
            const auto& physical_devices = m_instance->physical_devices();
            if (stdr::empty(physical_devices)) {
                elog("No render physical device found!");
                return;
            }
            ilog("Physical devices: {}", physical_devices);

            m_physical_device = physical_devices.front();
            auto score        = gpu::score_physical_device(*m_physical_device);
            for (auto i : range(1_u32, stdr::size(physical_devices))) {
                const auto& d       = physical_devices[i];
                const auto  d_score = gpu::score_physical_device(d);
                if (d_score > score) {
                    m_physical_device = d;
                    score             = d_score;
                }
            }

            ilog("Picked gpu: {}", *m_physical_device);

            // create gpu device
            m_device = TryAssert(gpu::Device::create(m_physical_device, {}), "Failed to initialize gpu device");

            // create swapchain
            const auto window_extent = m_window->extent();
            m_swapchain              = TryAssert(gpu::SwapChain::create(m_device, { gpu::as_view(m_surface), window_extent }),
                                                 "Failed to create swapchain");

            const auto queue_entries = m_device->queue_entries();
            const auto it            = stdr::find_if(queue_entries, gpu::monadic::find_queue<gpu::QueueFlag::GRAPHICS>());
            ensures(it != stdr::cend(queue_entries), "No raster queue found!");

            m_raster_queue = gpu::Queue::create(m_device, *it);

            m_command_pool = TryAssert(gpu::CommandPool::create(m_device, { .queue = m_raster_queue }),
                                       "Failed to create command pool "
                                       "command pool");
        }
    };
} // namespace base
