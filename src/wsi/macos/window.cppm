
// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#include <stormkit/core/contract_macro.hpp>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnullability-extension"
#pragma clang diagnostic ignored "-Wc11-extensions"
#pragma clang diagnostic ignored "-Wsign-conversion"
#if __has_include(<wsi-Swift.h>)
    #include <wsi-Swift.h>
#endif
#pragma clang diagnostic pop

#include "swift/CppBridge.hpp"

export module stormkit.wsi:macos.window;

import std;

import stormkit.core;
import stormkit.wsi;

import :common.window_base;

namespace stdr = std::ranges;
namespace stdv = std::views;

export namespace stormkit::wsi::macos {
    class Window: public ::stormkit::wsi::common::WindowBase {
      public:
        explicit Window(WM) noexcept { macOS::initCocoaProcess(); };

        ~Window() noexcept = default;

        Window(const Window&) noexcept                    = delete;
        auto operator=(const Window&) noexcept -> Window& = delete;

        Window(Window&& other) noexcept : m_window { std::move(other.m_window) } {
            m_window->updateID(std::bit_cast<u64>(std::bit_cast<uptr>(this)));
        }

        auto operator=(Window&& other) noexcept -> Window& {
            if (&other == this) [[unlikely]]
                return *this;

            m_window = std::move(other.m_window);
            m_window->updateID(std::bit_cast<u64>(std::bit_cast<uptr>(this)));

            return *this;
        }

        auto open(string title, const math::uextent2& size, WindowFlag flags) noexcept
          -> void {
            const auto resizeable  = has_flag_bit(flags, WindowFlag::RESIZEABLE);
            const auto borderless  = has_flag_bit(flags, WindowFlag::BORDERLESS);
            const auto metal_layer = has_flag_bit(flags, WindowFlag::EXTERNAL_CONTEXT);
            m_window               = macOS::Window::init(swift::String { title },
                                           as<f64>(size.width),
                                           as<f64>(size.height),
                                           resizeable,
                                           borderless,
                                           metal_layer,
                                           std::bit_cast<u64>(std::bit_cast<uptr>(this)));

            m_state.title  = std::move(title);
            m_state.active = true;
            m_state.open   = true;
            m_state.extent = size;
        }

        auto close() noexcept -> void {
            m_window = {};
            m_state  = {};
        }

        auto handle_events() noexcept -> void { macOS::processEvents(); }

        auto clear([[maybe_unused]] const ucolor_rgb& color) noexcept -> void {
            const auto value = as<u32>(color.r) << 16 | as<u32>(color.g) << 8 | color.b;
            stdr::fill(m_pixels, value);
            m_window->drawBitmap(std::bit_cast<unsigned char*>(stdr::data(m_pixels)));
        }

        auto fill_framebuffer(array_view<const ucolor_rgb> pixels) noexcept -> void {
            const auto [width, height] = extent();
            const auto count           = std::min(as<u32>(stdr::size(pixels)), height * width);
            if (stdr::size(pixels) > stdr::size(m_pixels)) m_pixels.resize(stdr::size(pixels));
            stdr::copy(pixels
                         | stdv::reverse
                         | stdv::take(count)
                         | stdv::transform([](const auto& col) static noexcept {
                               return as<u32>(col.r) << 16 | as<u32>(col.g) << 8 | col.b;
                           }),
                       stdr::begin(m_pixels));
            m_window->drawBitmap(std::bit_cast<unsigned char*>(stdr::data(m_pixels)));
        }

        auto set_title(string title) noexcept -> void {
            if (WindowBase::set_title(std::move(title))) {
                m_window->setTitle(swift::String { m_state.title });
            }
        }

        auto set_extent([[maybe_unused]] const math::uextent2& extent) noexcept -> void {}

        auto set_fullscreen(bool fullscreen) noexcept -> void {
            if (WindowBase::set_fullscreen(fullscreen)) {}
        }

        auto confine_mouse([[maybe_unused]] bool confined, u8) noexcept -> void {}

        [[nodiscard]]
        STORMKIT_FORCE_INLINE
        auto is_mouse_confined(u8 mouse_id) const noexcept -> bool {
            expects(mouse_id == GLOBAL_MOUSE_ID,
                    "StormKit WSI UIKit backend only support one mouse");
            auto& state = m_mouse_states[mouse_id];
            return state.confined;
        }

        auto lock_mouse([[maybe_unused]] bool locked, u8) noexcept -> void {}

        [[nodiscard]]
        STORMKIT_FORCE_INLINE
        auto is_mouse_locked(u8 mouse_id) const noexcept -> bool {
            expects(mouse_id == GLOBAL_MOUSE_ID,
                    "StormKit WSI UIKit backend only support one mouse");
            auto& state = m_mouse_states[mouse_id];
            return state.locked;
        }

        auto hide_mouse([[maybe_unused]] bool hidden, [[maybe_unused]] u8 mouse_id) noexcept
          -> void {}

        [[nodiscard]]
        STORMKIT_FORCE_INLINE
        auto is_mouse_hidden(u8 mouse_id) const noexcept -> bool {
            expects(mouse_id == GLOBAL_MOUSE_ID,
                    "StormKit WSI UIKit backend only support one mouse");
            auto& state = m_mouse_states[mouse_id];
            return state.hidden;
        }

        auto set_relative_mouse([[maybe_unused]] bool enabled,
                                [[maybe_unused]] u8   mouse_id) noexcept -> void {}

        [[nodiscard]]
        STORMKIT_FORCE_INLINE
        auto is_mouse_relative(u8 mouse_id) const noexcept -> bool {
            expects(mouse_id == GLOBAL_MOUSE_ID,
                    "StormKit WSI UIKit backend only support one mouse");
            auto& state = m_mouse_states[mouse_id];
            return state.relative;
        }

        auto set_key_repeat([[maybe_unused]] bool enabled, [[maybe_unused]] u8 keyboard_id) noexcept
          -> void {}

        [[nodiscard]]
        STORMKIT_FORCE_INLINE
        auto is_key_repeat_enabled(u8 keyboard_id) const noexcept -> bool {
            expects(keyboard_id == GLOBAL_KEYBOARD_ID,
                    "StormKit WSI UIKit backend only support one keyboard");
            auto& state = m_keyboard_states[keyboard_id];
            return state.key_repeat;
        }

        auto show_virtual_keyboard([[maybe_unused]] bool visible) noexcept -> void {}

        [[nodiscard]]
        STORMKIT_FORCE_INLINE
        auto is_virtual_keyboard_visible() const noexcept -> bool {
            return false;
        }

        auto set_mouse_position([[maybe_unused]] const math::ivec2& position,
                                [[maybe_unused]] u8                 mouse_id) noexcept -> void {}

        [[nodiscard]]
        STORMKIT_FORCE_INLINE
        inline auto is_mouse_inside() const noexcept -> bool {
            return false;
        }

        STORMKIT_FORCE_INLINE
        inline auto set_mouse_inside([[maybe_unused]] bool inside) noexcept -> void {}

        [[nodiscard]]
        STORMKIT_FORCE_INLINE
        inline auto native_handle() const noexcept -> NativeHandle {
            auto f  = m_window->nativeHandle();
            auto f2 = m_window->nativeHandle2();

            std::println("{} {}", f, f2);
            return f;
        }

      private:
        defer_init<macOS::Window> m_window;

        dynarray<u32> m_pixels;
    };
} // namespace stormkit::wsi::macos
