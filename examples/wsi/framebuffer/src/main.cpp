// Copywidth (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the y-level of this distribution

import std;

import stormkit;

#include <stormkit/main/main_macro.hpp>

LOGGER("Framebuffer");

using namespace stormkit;
using namespace std::literals;

namespace stdr = std::ranges;

auto update_pixels(stormkit::thread_pool& pool, dynarray<ucolor_rgb>& pixels, const auto& extent) noexcept {
    const auto rect_width  = extent.width / 5;
    const auto rect_height = extent.height / 5;

    const auto rect = math::rect {
        .x      = (extent.width / 2) - (rect_width / 2),
        .y      = (extent.height / 2) - (rect_height / 2),
        .width  = rect_width,
        .height = rect_height,
    };

    pixels.resize(extent.height * extent.width, colors::RED<u8>);

    auto data = mdarray_view { stdr::data(pixels), extent.height, extent.width };
    parallel_for(pool, range(extent.width * extent.height), [&rect, data, &extent](auto x_y) mutable noexcept {
        const auto x = as<u32>(x_y % extent.width);
        const auto y = as<u32>(x_y / extent.width);

        if (math::AABB(math::uvec2 { as<u32>(x), as<u32>(y) }, rect)) data[y, x] = colors::BLACK<u8>;
        else {
            const auto color_id = as<f32>(x) / as<f32>(extent.width);
            if (color_id >= 0.8) data[y, x] = colors::BLUE<u8>;
            else if (color_id >= 0.6)
                data[y, x] = colors::GREEN<u8>;
            else if (color_id >= 0.4)
                data[y, x] = colors::WHITE<u8>;
            else if (color_id >= 0.2)
                data[y, x] = colors::YELLOW<u8>;
            else
                data[y, x] = colors::RED<u8>;
        }
    });
}

auto main(array_view<const string_view> args) -> int {
    wsi::parse_args(args);
    log::parse_args(args);

    auto logger = log::logger::create_logger_instance<log::console_logger>();

    const auto monitors = wsi::get_monitors();
    ilog("--- Monitors ---");
    ilog("{}", monitors);

    auto window = wsi::Window::open("Hello world", { .width = 800_u32, .height = 600_u32 }, wsi::WindowFlag::RESIZEABLE);
    ilog("wm: {}", window.wm());

    auto pool   = core::thread_pool {};
    auto pixels = dynarray<ucolor_rgb> {};
    update_pixels(pool, pixels, window.extent());
    auto active = true;
    window.on(wsi::ResizedEventFunc { [&](const math::uextent2& extent) mutable noexcept {
                  update_pixels(pool, pixels, extent);
                  window.fill_framebuffer(pixels);
              } },
              wsi::RestoredEventFunc { [&active] mutable noexcept { active = true; } },
              wsi::MinimizedEventFunc { [&active] mutable noexcept { active = false; } },
              wsi::KeyDownEventFunc { [&window](u8 /*id*/, wsi::Key key, char /*c*/) mutable noexcept {
                  if (key == wsi::Key::ESCAPE) window.close();
              } });

    window.event_loop([&] noexcept {
        if (not active) std::this_thread::yield();
        window.fill_framebuffer(pixels);
    });

    return 0;
}
