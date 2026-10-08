
// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
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
    class window: public ::stormkit::wsi::common::window_base {
      public:
        explicit window(window_manager) noexcept { macOS::initCocoaProcess(); };

        ~window() noexcept = default;

        window(const window&) noexcept                    = delete;
        auto operator=(const window&) noexcept -> window& = delete;

        window(window&& other) noexcept : m_window { std::move(other.m_window) } {
            m_window->updateID(std::bit_cast<u64>(std::bit_cast<uptr>(this)));
        }

        auto operator=(window&& other) noexcept -> window& {
            if (&other == this) [[unlikely]]
                return *this;

            m_window = std::move(other.m_window);
            m_window->updateID(std::bit_cast<u64>(std::bit_cast<uptr>(this)));

            return *this;
        }

        auto open(string title, const math::uextent2& size, window_flag flags) noexcept -> void {
            const auto resizeable  = has_flag_bit(flags, window_flag::resizeable);
            const auto borderless  = has_flag_bit(flags, window_flag::borderless);
            const auto metal_layer = has_flag_bit(flags, window_flag::external_context);
            m_window               = macOS::window::init(swift::String { title },
                                                         as<f64>(size.width),
                                                         as<f64>(size.height),
                                                         resizeable,
                                                         borderless,
                                                         metal_layer,
                                                         std::bit_cast<u64>(std::bit_cast<uptr>(this)));

            state_.title  = std::move(title);
            state_.active = true;
            state_.open   = true;
            state_.extent = size;
        }

        auto close() noexcept -> void {
            m_window = {};
            state_   = {};
        }

        auto handle_events() noexcept -> void { macOS::processEvents(); }

        auto clear([[maybe_unused]] const ucolor_rgb& color) noexcept -> void {
            const auto value = as<u32>(color.r) << 16 | as<u32>(color.g) << 8 | color.b;
            stdr::fill(m_pixels, value);
            m_window->drawBitmap(reinterpret_cast<uchar*>(stdr::data(m_pixels)));
        }

        auto fill_framebuffer(array_view<const ucolor_rgb> pixels) noexcept -> void {
            const auto [width, height] = extent();
            const auto count           = std::min(as<u32>(stdr::size(pixels)), height * width);
            if (stdr::size(pixels) > stdr::size(m_pixels)) m_pixels.resize(stdr::size(pixels));
            stdr::copy(pixels | stdv::reverse | stdv::take(count) | stdv::transform([](const auto& col) static noexcept {
                           return as<u32>(col.r) << 16 | as<u32>(col.g) << 8 | col.b;
                       }),
                       stdr::begin(m_pixels));
            m_window->drawBitmap(reinterpret_cast<uchar*>(stdr::data(m_pixels)));
        }

        auto set_title(string title) noexcept -> void {
            if (window_base::set_title(std::move(title))) { m_window->setTitle(swift::String { state_.title }); }
        }

        auto set_extent([[maybe_unused]] const math::uextent2& extent) noexcept -> void {}

        auto set_fullscreen(bool fullscreen) noexcept -> void {
            if (window_base::set_fullscreen(fullscreen)) {}
        }

        auto confine_mouse([[maybe_unused]] bool confined, u8) noexcept -> void {}

        [[nodiscard]]
        STORMKIT_FORCE_INLINE
        auto is_mouse_confined(u8 mouse_id) const noexcept -> bool {
            expects(mouse_id == global_mouse_id, "StormKit WSI UIKit backend only support one mouse");
            auto& state = mouse_states_[mouse_id];
            return state.confined;
        }

        auto lock_mouse([[maybe_unused]] bool locked, u8) noexcept -> void {}

        [[nodiscard]]
        STORMKIT_FORCE_INLINE
        auto is_mouse_locked(u8 mouse_id) const noexcept -> bool {
            expects(mouse_id == global_mouse_id, "StormKit WSI UIKit backend only support one mouse");
            auto& state = mouse_states_[mouse_id];
            return state.locked;
        }

        auto hide_mouse([[maybe_unused]] bool hidden, [[maybe_unused]] u8 mouse_id) noexcept -> void {}

        [[nodiscard]]
        STORMKIT_FORCE_INLINE
        auto is_mouse_hidden(u8 mouse_id) const noexcept -> bool {
            expects(mouse_id == global_mouse_id, "StormKit WSI UIKit backend only support one mouse");
            auto& state = mouse_states_[mouse_id];
            return state.hidden;
        }

        auto set_relative_mouse([[maybe_unused]] bool enabled, [[maybe_unused]] u8 mouse_id) noexcept -> void {}

        [[nodiscard]]
        STORMKIT_FORCE_INLINE
        auto is_mouse_relative(u8 mouse_id) const noexcept -> bool {
            expects(mouse_id == global_mouse_id, "StormKit WSI UIKit backend only support one mouse");
            auto& state = mouse_states_[mouse_id];
            return state.relative;
        }

        auto set_key_repeat([[maybe_unused]] bool enabled, [[maybe_unused]] u8 keyboard_id) noexcept -> void {}

        [[nodiscard]]
        STORMKIT_FORCE_INLINE
        auto is_key_repeat_enabled(u8 keyboard_id) const noexcept -> bool {
            expects(keyboard_id == global_keyboard_id, "StormKit WSI UIKit backend only support one keyboard");
            auto& state = keyboard_states_[keyboard_id];
            return state.key_repeat;
        }

        auto show_virtual_keyboard([[maybe_unused]] bool visible) noexcept -> void {}

        [[nodiscard]]
        STORMKIT_FORCE_INLINE
        auto is_virtual_keyboard_visible() const noexcept -> bool {
            return false;
        }

        auto set_mouse_position([[maybe_unused]] const math::ivec2& position, [[maybe_unused]] u8 mouse_id) noexcept -> void {}

        [[nodiscard]]
        STORMKIT_FORCE_INLINE
        inline auto is_mouse_inside() const noexcept -> bool {
            return false;
        }

        STORMKIT_FORCE_INLINE
        inline auto set_mouse_inside([[maybe_unused]] bool inside) noexcept -> void {}

        [[nodiscard]]
        STORMKIT_FORCE_INLINE
        inline auto native_handle() const noexcept -> native_handle_type {
            auto f  = m_window->nativeHandle();
            auto f2 = m_window->nativeHandle2();

            std::println("{} {}", f, f2);
            return f;
        }

      private:
        defer_init<macOS::window> m_window;

        dynarray<u32> m_pixels;
    };
} // namespace stormkit::wsi::macos
