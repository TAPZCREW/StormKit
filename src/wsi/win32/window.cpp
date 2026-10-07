// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

// #include <cstdlib>

#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>

module stormkit.wsi;

import stormkit.core.win32;

import :win32.window;
import :win32.keyboard;

import :common.input_base;

import :win32.log;
import :win32.keyboard;
import :win32.mouse;

namespace stdv = std::views;
namespace stdr = std::ranges;

using namespace stormkit;

template<typename CharT>
constexpr auto tag_invoke(format_as_fn<CharT>, const ::win32::POINT& point, meta::format_context auto& ctx) noexcept
  -> decltype(ctx.out()) {
    return std::format_to(ctx.out(), "[vec2 x: {}, y: {}]", point.x, point.y);
}

template<typename CharT>
constexpr auto tag_invoke(format_as_fn<CharT>, const ::win32::RECT& rect, meta::format_context auto& ctx) noexcept
  -> decltype(ctx.out()) {
    return std::format_to(ctx.out(),
                          "[rect left: {}, top: {}, right: {}, bottom: {}]",
                          rect.left,
                          rect.top,
                          rect.right,
                          rect.bottom);
}

auto adjust_extent(const math::uextent2& extent, ::win32::DWORD style, ::win32::DWORD style_ex) noexcept
  -> math::extent2<::win32::LONG> {
    auto rect = ::win32::RECT {
        .left   = 0,
        .top    = 0,
        .right  = as<::win32::LONG>(extent.width),
        .bottom = as<::win32::LONG>(extent.height)
    };

    ::win32::AdjustWindowRectEx(&rect, style, ::win32::FALSE, style_ex);

    return { rect.right - rect.left, rect.bottom - rect.top };
}

namespace stormkit::wsi::win32 {
    using HBrush = raii_capsule<::win32::HBRUSH, ::win32::CreateSolidBrush, ::win32::DeleteObject, struct HBrushTag, nullptr>;

    namespace {
        constexpr auto CLASS_NAME = "Stormkit_Window";

        auto get_client_rect(::win32::HWND window_handle) noexcept -> ::win32::RECT;

        // auto get_monitor_scale(HMONITOR monitor) -> math::fvec2;

        auto global_on_event(::win32::HWND   handle,
                             ::win32::UINT   message,
                             ::win32::WPARAM w_param,
                             ::win32::LPARAM l_param) noexcept -> ::win32::LRESULT;

        constinit auto g_window_count = std::atomic<u8> { 0 };
    } // namespace

    /////////////////////////////////////
    /////////////////////////////////////
    Window::Window(WM) noexcept {
        if (g_window_count == 0) {
            ::win32::SetProcessDpiAwarenessContext(::win32::DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
            auto h_instance   = ::win32::GetModuleHandleA(nullptr);
            auto window_class = ::win32::WNDCLASSA {};
            if (::win32::GetClassInfoA(h_instance, CLASS_NAME, &window_class) == ::win32::FALSE) {
                window_class.lpfnWndProc   = &global_on_event;
                window_class.hInstance     = ::win32::GetModuleHandleA(nullptr);
                window_class.lpszClassName = CLASS_NAME;
                ::win32::RegisterClassA(&window_class);
            }
        }

        g_window_count += 1;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    Window::~Window() noexcept {
        close();

        g_window_count -= 1;
        if (g_window_count == 0) ::win32::UnregisterClassA(CLASS_NAME, ::win32::GetModuleHandleA(nullptr));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    Window::Window(Window&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::operator=(Window&&) noexcept -> Window& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::open(string&& title, const math::uextent2& extent, WindowFlag flags) noexcept -> void {
        auto style      = ::win32::DWORD { ::win32::WS_SYSMENU | ::win32::WS_BORDER };
        auto style_ex   = ::win32::DWORD { 0 };
        auto h_instance = ::win32::GetModuleHandleA(nullptr);

        if (has_flag_bit(flags, WindowFlag::BORDERLESS)) style |= ::win32::WS_POPUP | ::win32::WS_EX_CLIENTEDGE;
        else
            style |= (::win32::WS_OVERLAPPED | ::win32::WS_CAPTION);

        if (has_flag_bit(flags, WindowFlag::RESIZEABLE)) style |= ::win32::WS_MAXIMIZEBOX | ::win32::WS_THICKFRAME;

        auto gdi = true;
        if (has_flag_bit(flags, WindowFlag::EXTERNAL_CONTEXT)) {
            m_win32_state.external_context = true;
            style_ex |= ::win32::WS_EX_NOREDIRECTIONBITMAP;
            gdi = false;
        }

        ::win32::SetThreadDpiAwarenessContext(::win32::DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);

        const auto adjusted = adjust_extent(extent, style, style_ex);
        m_window_handle     = ::win32::
          CreateWindowExA(style_ex,
                          CLASS_NAME,
                          std::data(title),
                          style,
                          ::win32::CW_USEDEFAULT,
                          ::win32::CW_USEDEFAULT,
                          adjusted.width,
                          adjusted.height,
                          nullptr,
                          nullptr,
                          h_instance,
                          this);
        m_state.open              = true;
        auto        win32_monitor = ::win32::MonitorFromWindow(m_window_handle, ::win32::MONITOR_DEFAULTTOPRIMARY);
        const auto  monitors      = get_monitors();
        const auto& monitor       = *stdr::find_if(monitors, [&win32_monitor](const auto& monitor) noexcept {
            return monitor.native_handle == win32_monitor;
        });

        set_current_monitor(monitor);

        m_win32_state.style    = as<::win32::DWORD>(::win32::GetWindowLongA(m_window_handle, ::win32::GWL_STYLE));
        m_win32_state.style_ex = as<::win32::DWORD>(::win32::GetWindowLongA(m_window_handle, ::win32::GWL_EXSTYLE));

        WindowBase::set_title(std::move(title));

        ::win32::ShowWindow(m_window_handle, ::win32::SW_SHOWNORMAL);
        m_state.active = true;

        update_geometry(extent);
        if (gdi) gdiinit();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::close() noexcept -> void {
        if (not m_win32_state.external_context) m_gdi_frame_data = GDIFrameData {};

        if (m_window_handle) ::win32::DestroyWindow(m_window_handle);

        m_keyboard_states = {};
        m_mouse_states    = {};

        m_window_handle = nullptr;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::clear(const ucolor_rgb& color) noexcept -> void {
        if (m_win32_state.external_context) return;

        auto hbrush = HBrush::create(::win32::Rgb(color.r, color.g, color.b));
        const auto
          rect = ::win32::RECT { 0, 0, as<::win32::LONG>(m_state.extent.width), as<::win32::LONG>(m_state.extent.height) };

        ::win32::FillRect(m_gdi_frame_data.context, &rect, hbrush);
        ::win32::InvalidateRect(m_window_handle, nullptr, ::win32::FALSE);
        ::win32::UpdateWindow(m_window_handle);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::fill_framebuffer(array_view<const ucolor_rgb> pixels) noexcept -> void {
        if (m_win32_state.external_context) return;

        const auto [width, height] = extent();
        const auto count           = std::min(as<u32>(stdr::size(pixels)), height * width);
        stdr::copy(pixels | stdv::take(count) | stdv::transform([](const auto& col) static noexcept {
                       return as<u32>(col.r) << 16 | as<u32>(col.g) << 8 | col.b;
                   }),
                   reinterpret_cast<u32*>(m_gdi_frame_data.pixels_ptr.load()));

        ::win32::InvalidateRect(m_window_handle, nullptr, ::win32::FALSE);
        ::win32::UpdateWindow(m_window_handle);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::handle_events() noexcept -> void {
        if (not m_window_handle) return;

        auto message = ::win32::MSG {};
        while (::win32::PeekMessageA(&message, nullptr, 0, 0, ::win32::PM_REMOVE)) {
            ::win32::TranslateMessage(&message);
            ::win32::DispatchMessageA(&message);
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_title(string&& title) noexcept -> void {
        WindowBase::set_title(std::move(title));

        ::win32::SetWindowTextA(m_window_handle, std::data(m_state.title));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_extent(const math::uextent2& extent) noexcept -> void {
        const auto adjusted = adjust_extent(extent, m_win32_state.style, m_win32_state.style_ex);
        ::win32::SetWindowPos(m_window_handle,
                              ::win32::HWND_TOP,
                              0,
                              0,
                              adjusted.width,
                              adjusted.height,
                              ::win32::SWP_NOACTIVATE | ::win32::SWP_NOZORDER | ::win32::SWP_NOMOVE | ::win32::SWP_NOOWNERZORDER);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_fullscreen(bool fullscreen) noexcept -> void {
        auto [x, y] = m_state.position.storage;

        auto style    = m_win32_state.style;
        auto style_ex = m_win32_state.style_ex;
        if (fullscreen) {
            style &= as<::win32::DWORD>(~(::win32::WS_CAPTION | ::win32::WS_THICKFRAME));
            style_ex &= as<::win32::DWORD>(~(::win32::WS_EX_DLGMODALFRAME
                                             | ::win32::WS_EX_WINDOWEDGE
                                             | ::win32::WS_EX_CLIENTEDGE
                                             | ::win32::WS_EX_STATICEDGE));

            x = 0;
            y = 0;
        }

        ::win32::SetWindowLongA(m_window_handle, ::win32::GWL_STYLE, as<::win32::LONG>(style));
        ::win32::SetWindowLongA(m_window_handle, ::win32::GWL_EXSTYLE, as<::win32::LONG>(style_ex));

        ::win32::SetWindowPos(m_window_handle,
                              nullptr,
                              x,
                              y,
                              as<i32>(m_state.extent.width),
                              as<i32>(m_state.extent.height),
                              ::win32::SWP_NOZORDER | ::win32::SWP_NOACTIVATE | ::win32::SWP_FRAMECHANGED);

        WindowBase::set_fullscreen(fullscreen);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::confine_mouse(bool confined, u8 id) noexcept -> void {
        expects(id == GLOBAL_MOUSE_ID, "stormkit::wsi win32 backend only support one mouse");
        if (is_mouse_locked(GLOBAL_MOUSE_ID)) return;

        if (confined) {
            const auto rect = get_client_rect(m_window_handle);
            ::win32::ClipCursor(&rect);
        } else
            ::win32::ClipCursor(nullptr);

        auto& state    = m_mouse_states[GLOBAL_MOUSE_ID];
        state.confined = confined;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::is_mouse_confined(u8 id) const noexcept -> bool {
        expects(id == GLOBAL_MOUSE_ID, "stormkit::wsi win32 backend only support one mouse");
        auto& state = m_mouse_states[GLOBAL_MOUSE_ID];
        return state.confined;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::lock_mouse(bool locked, u8 id) noexcept -> void {
        expects(id == GLOBAL_MOUSE_ID, "stormkit::wsi win32 backend only support one mouse");
        auto& state = m_mouse_states[GLOBAL_MOUSE_ID];

        if (locked) {
            auto mouse_position = ::win32::POINT {};
            ::win32::GetCursorPos(&mouse_position);

            auto rect   = ::win32::RECT {};
            rect.top    = mouse_position.y;
            rect.left   = mouse_position.x;
            rect.bottom = mouse_position.y;
            rect.right  = mouse_position.x;

            if (state.relative) {
                rect.top -= 5;
                rect.left -= 5;
                rect.bottom += 5;
                rect.right += 5;
            }

            ::win32::ClipCursor(&rect);

            ::win32::ScreenToClient(m_window_handle, &mouse_position);

            state.locked_at.x() = as<u32>(mouse_position.x);
            state.locked_at.y() = as<u32>(mouse_position.y);
        } else
            ::win32::ClipCursor(nullptr);

        state.locked   = locked;
        state.confined = locked;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::is_mouse_locked(u8 id) const noexcept -> bool {
        expects(id == GLOBAL_MOUSE_ID, "stormkit::wsi win32 backend only support one mouse");
        auto& state = m_mouse_states[GLOBAL_MOUSE_ID];
        return state.locked;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::hide_mouse(bool hidden, u8 id) noexcept -> void {
        expects(id == GLOBAL_MOUSE_ID, "stormkit::wsi win32 backend only support one mouse");
        if (hidden)
            while (::win32::ShowCursor(::win32::FALSE) >= 0);
        else
            ::win32::ShowCursor(::win32::TRUE);

        auto& state  = m_mouse_states[GLOBAL_MOUSE_ID];
        state.hidden = hidden;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::is_mouse_hidden(u8 id) const noexcept -> bool {
        expects(id == GLOBAL_MOUSE_ID, "stormkit::wsi win32 backend only support one mouse");
        auto& state = m_mouse_states[GLOBAL_MOUSE_ID];
        return state.hidden;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_relative_mouse(bool enabled, u8 id) noexcept -> void {
        expects(id == GLOBAL_MOUSE_ID, "stormkit::wsi win32 backend only support one mouse");
        auto& state = m_mouse_states[GLOBAL_MOUSE_ID];

        if (state.locked) {
            auto locked_at = ::win32::POINT { as<i32>(state.locked_at.x()), as<i32>(state.locked_at.y()) };

            ::win32::ClientToScreen(m_window_handle, &locked_at);

            auto rect   = ::win32::RECT {};
            rect.top    = as<::win32::LONG>(locked_at.y);
            rect.left   = as<::win32::LONG>(locked_at.x);
            rect.bottom = as<::win32::LONG>(locked_at.y);
            rect.right  = as<::win32::LONG>(locked_at.x);

            if (enabled) {
                rect.top -= 5;
                rect.left -= 5;
                rect.bottom += 5;
                rect.right += 5;
            }

            ::win32::ClipCursor(&rect);
        }

        state.relative = enabled;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::is_mouse_relative(u8 id) const noexcept -> bool {
        expects(id == GLOBAL_MOUSE_ID, "StormKit::wsi win32 backend only support one mouse");
        auto& state = m_mouse_states[GLOBAL_MOUSE_ID];
        return state.relative;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_key_repeat(bool enabled, u8 id) noexcept -> void {
        expects(id == GLOBAL_KEYBOARD_ID, "stormkit::wsi win32 backend only support one keyboard");
        auto& state      = m_keyboard_states[GLOBAL_KEYBOARD_ID];
        state.key_repeat = enabled;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::is_key_repeat_enabled(u8 id) const noexcept -> bool {
        expects(id == GLOBAL_KEYBOARD_ID, "stormkit::wsi win32 backend only support one keyboard");
        auto& state = m_keyboard_states[GLOBAL_KEYBOARD_ID];
        return state.key_repeat;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::show_virtual_keyboard(bool) noexcept -> void {
        elog("virtual keyboard support for win32 isn't yet implemented");
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::is_virtual_keyboard_visible() const noexcept -> bool {
        elog("virtual keyboard support for win32 isn't yet implemented");
        return false;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_mouse_position(const math::ivec2& position, u8 id) noexcept -> void {
        expects(id == GLOBAL_MOUSE_ID, "stormkit::wsi win32 backend only support one mouse");
        auto mouse_position = ::win32::POINT { as<long>(position.x()), as<long>(position.y()) };
        ::win32::ClientToScreen(m_window_handle, &mouse_position);
        ::win32::SetCursorPos(mouse_position.x, mouse_position.y);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::gdiinit() noexcept -> void {
        const auto hdesktop = ::win32::GetDC(nullptr);

        m_gdi_frame_data               = GDIFrameData {};
        m_gdi_frame_data.extent.width  = as<::win32::LONG>(extent().width);
        m_gdi_frame_data.extent.height = as<::win32::LONG>(extent().height);
        // as<math::extent2<::win32::LONG>>(extent());

        const auto [width, height] = m_gdi_frame_data.extent;

        const auto byte_count = as<usize>(width * height) * sizeof(u32);

        const auto frame_bitmap_info = ::win32::BITMAPINFOHEADER {
            .biSize          = sizeof(::win32::BITMAPINFOHEADER),
            .biWidth         = width,
            .biHeight        = -height,
            .biPlanes        = 1,
            .biBitCount      = 32,
            .biCompression   = ::win32::BI_RGB,
            .biSizeImage     = as<::win32::DWORD>(byte_count),
            .biXPelsPerMeter = 0,
            .biYPelsPerMeter = 0,
            .biClrUsed       = 0,
            .biClrImportant  = 0,
        };

        auto ptr                    = raw_ptr<void> { nullptr };
        m_gdi_frame_data.context    = Hdc::create(hdesktop);
        m_gdi_frame_data.bitmap     = HBitmap::create(m_gdi_frame_data.context,
                                                      reinterpret_cast<const ::win32::BITMAPINFO*>(&frame_bitmap_info),
                                                      as<::win32::UINT>(::win32::DIB_RGB_COLORS),
                                                      &ptr,
                                                      nullptr,
                                                      as<::win32::DWORD>(0));
        m_gdi_frame_data.pixels_ptr = ptr;
        m_gdi_frame_data.extent     = { width, height };
        ::win32::SelectObject(m_gdi_frame_data.context, m_gdi_frame_data.bitmap);

        ::win32::ReleaseDC(nullptr, hdesktop);
    }

    namespace {
        /////////////////////////////////////
        /////////////////////////////////////
        auto get_client_rect(::win32::HWND window_handle) noexcept -> ::win32::RECT {
            const auto client_rect = init_by<::win32::RECT>([window_handle](auto& out) noexcept {
                ::win32::GetClientRect(window_handle, &out);
            });

            const auto lefttop = init_by<::win32::POINT>([window_handle, &client_rect](::win32::POINT& out) noexcept {
                out.x = client_rect.left;
                out.y = client_rect.top;
                ::win32::ClientToScreen(window_handle, &out);
            });

            const auto rightbottom = init_by<::win32::POINT>([window_handle, &client_rect](::win32::POINT& out) noexcept {
                out.x = client_rect.right;
                out.y = client_rect.bottom;
                ::win32::ClientToScreen(window_handle, &out);
            });

            return {
                .left   = lefttop.x,
                .top    = lefttop.y,
                .right  = rightbottom.x,
                .bottom = rightbottom.y,
            };
        }

        /////////////////////////////////////
        /////////////////////////////////////
        // auto get_monitor_scale(HMONITOR monitor) -> math::fvec2 {
        //     auto scale = math::fvec2 {};

        //    auto x_dpi       = 0u;
        //    auto y_dpi       = 0u;
        //    auto default_dpi = as<f32>(USER_DEFAULT_SCREEN_DPI);

        //    GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &x_dpi, &y_dpi);

        //    scale.x = as<f32>(x_dpi) / default_dpi;
        //    scale.x = as<f32>(y_dpi) / default_dpi;

        //    return scale;
        // }

        /////////////////////////////////////
        /////////////////////////////////////
        auto handle_global_events(::win32::UINT, ::win32::WPARAM, ::win32::LPARAM) noexcept -> std::optional<::win32::LRESULT> {
            return std::nullopt;
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto handle_window_events(Window&         window,
                                  ::win32::UINT   message,
                                  ::win32::WPARAM w_param,
                                  ::win32::LPARAM l_param) noexcept -> std::optional<::win32::LRESULT> {
            const auto window_handle = reinterpret_cast<::win32::HWND>(window.native_handle());
            if (message != ::win32::WM_DESTROY and not window_handle) return 0;

            switch (message) {
                case ::win32::WM_DESTROY: ::win32::PostQuitMessage(0); return 0;
                case ::win32::WM_CLOSE:
                    if (window.closed_event()) ::win32::DestroyWindow(window_handle);
                    return 0;
                case ::win32::WM_CREATE: window.WindowBase::set_open(true); break;
                case ::win32::WM_NCDESTROY: window.WindowBase::set_open(false); break;
                case ::win32::WM_MOUSEACTIVATE: {
                    return ::win32::MA_ACTIVATEANDEAT;
                }
                case ::win32::WM_ACTIVATE: {
                    if (low_order(w_param) == ::win32::WA_INACTIVE) {
                        window.state().active = false;
                        window.deactivate_event();
                    } else {
                        window.state().active = true;
                        window.activate_event();
                    }
                } break;
                case ::win32::WM_NCLBUTTONDOWN:
                    if (::win32::SendMessageA(window_handle, ::win32::WM_NCHITTEST, w_param, l_param) == ::win32::HTCAPTION) {
                        auto pos = ::win32::POINT {};
                        ::win32::GetCursorPos(&pos);
                        ::win32::ScreenToClient(window_handle, &pos);

                        const auto y = (pos.y << 16);

                        ::win32::PostMessageA(window_handle, ::win32::WM_MOUSEMOVE, 0, pos.x | y);
                    }
                    break;
                case ::win32::WM_WINDOWPOSCHANGING: {
                    auto win32_monitor = ::win32::MonitorFromWindow(window_handle, ::win32::MONITOR_DEFAULTTONEAREST);
                    if (window.current_monitor().native_handle != win32_monitor) {
                        auto  monitors = get_monitors();
                        auto& monitor  = *stdr::find_if(monitors, [&win32_monitor](auto&& monitor) noexcept {
                            return monitor.native_handle == win32_monitor;
                        });

                        window.set_current_monitor(std::move(monitor));
                        window.monitor_changed_event(window.current_monitor());
                    }
                } break;
                case ::win32::WM_PAINT: {
                    if (window.win32_state().external_context) break;

                    const auto& gdi_frame_data = window.gdi_frame_data();

                    auto ps                               = ::win32::PAINTSTRUCT {};
                    auto hdc                              = ::win32::BeginPaint(window_handle, &ps);
                    const auto [left, top, right, bottom] = ps.rcPaint;
                    const auto cx                         = right - left;
                    const auto cy                         = bottom - top;

                    ::win32::BitBlt(hdc, left, top, cx, cy, gdi_frame_data.context, left, top, ::win32::SRCCOPY);
                    ::win32::EndPaint(window_handle, &ps);

                    return 0;
                }
                case ::win32::WM_SIZE: {
                    window.update_geometry({ as<u32>(low_order(l_param)), as<u32>(high_order(l_param)) });

                    if (not window.win32_state().external_context) {
                        auto& gdi_frame_data = window.gdi_frame_data();
                        window.gdiinit();

                        const auto width  = as<i32>(gdi_frame_data.extent.width);
                        const auto height = as<i32>(gdi_frame_data.extent.height);

                        auto ps  = ::win32::PAINTSTRUCT {};
                        auto hdc = ::win32::BeginPaint(window_handle, &ps);
                        ::win32::BitBlt(hdc, 0, 0, width, height, gdi_frame_data.context, 0, 0, ::win32::SRCCOPY);
                        ::win32::EndPaint(window_handle, &ps);
                    }

                    switch (w_param) {
                        case ::win32::SIZE_MINIMIZED: window.minimized_event(); break;
                        case ::win32::SIZE_RESTORED: window.restored_event(); break;
                        default: break;
                    }

                    window.resized_event(window.extent());

                    return 0;
                }
                default: break;
            }
            return std::nullopt;
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto handle_input_events(Window& window, ::win32::UINT message, ::win32::WPARAM w_param, ::win32::LPARAM l_param) noexcept
          -> void {
            const auto window_handle = reinterpret_cast<::win32::HWND>(window.native_handle());
            if (not window.state().active) return;

            switch (message) {
                case ::win32::WM_LBUTTONDOWN: [[fallthrough]];
                case ::win32::WM_RBUTTONDOWN: [[fallthrough]];
                case ::win32::WM_MBUTTONDOWN: [[fallthrough]];
                case ::win32::WM_XBUTTONDOWN: {
                    const auto [x, y] = extract_mouse_position(window_handle, w_param, l_param, false).storage;
                    const auto button = extract_mouse_button(message, w_param, l_param);
                    window.mouse_button_down_event(GLOBAL_MOUSE_ID, button, math::ivec2 { x, y });
                } break;
                case ::win32::WM_LBUTTONUP: [[fallthrough]];
                case ::win32::WM_RBUTTONUP: [[fallthrough]];
                case ::win32::WM_MBUTTONUP: [[fallthrough]];
                case ::win32::WM_XBUTTONUP: {
                    const auto [x, y] = extract_mouse_position(window_handle, w_param, l_param, false).storage;
                    const auto button = extract_mouse_button(message, w_param, l_param);
                    window.mouse_button_up_event(GLOBAL_MOUSE_ID, button, math::ivec2 { x, y });
                } break;
                case ::win32::WM_KEYDOWN: [[fallthrough]];
                case ::win32::WM_SYSKEYDOWN: {
                    auto& state = window.keyboard_state(GLOBAL_MOUSE_ID);

                    const auto key       = extract_key(w_param, l_param);
                    const auto character = extract_key_to_char(w_param, l_param);
                    const auto to_index  = as<underlying>(key);

                    if (state.keys[to_index] == common::KeyState::UP) {
                        window.key_down_event(GLOBAL_KEYBOARD_ID, key, character);
                        state.keys[to_index] = common::KeyState::DOWN;
                    } else if (state.key_repeat)
                        window.key_down_event(GLOBAL_KEYBOARD_ID, key, character);
                } break;
                case ::win32::WM_KEYUP: [[fallthrough]];
                case ::win32::WM_SYSKEYUP: {
                    auto& state = window.keyboard_state(GLOBAL_MOUSE_ID);

                    const auto key       = extract_key(w_param, l_param);
                    const auto character = extract_key_to_char(w_param, l_param);
                    const auto to_index  = as<underlying>(key);

                    window.key_up_event(GLOBAL_KEYBOARD_ID, key, character);
                    state.keys[to_index] = common::KeyState::UP;
                } break;
                case ::win32::WM_MOUSEMOVE: {
                    auto& state = window.mouse_state(GLOBAL_MOUSE_ID);
                    if (state.locked and not state.relative) break;

                    if (not window.win32_state().mouse_tracked) {
                        auto track_mouse_event        = ::win32::TRACKMOUSEEVENT {};
                        track_mouse_event.cbSize      = sizeof(::win32::TRACKMOUSEEVENT);
                        track_mouse_event.dwFlags     = ::win32::TME_LEAVE;
                        track_mouse_event.hwndTrack   = window_handle;
                        track_mouse_event.dwHoverTime = ::win32::HOVER_DEFAULT;
                        if (::win32::TrackMouseEvent(&track_mouse_event) != ::win32::FALSE)
                            window.win32_state().mouse_tracked = true;
                    }

                    const auto [x, y] = extract_mouse_position(window_handle, w_param, l_param, false).storage;

                    if (state.locked and state.relative) {
                        const auto relative_x = x - as<i32>(state.locked_at.x());
                        const auto relative_y = y - as<i32>(state.locked_at.y());
                        window.mouse_moved_event(GLOBAL_MOUSE_ID, math::ivec2 { relative_x, relative_y });
                    } else if (state.relative) {
                        const auto relative_x = x - as<i32>(state.last_position.x());
                        const auto relative_y = y - as<i32>(state.last_position.y());
                        window.mouse_moved_event(GLOBAL_MOUSE_ID, math::ivec2 { relative_x, relative_y });
                    } else
                        window.mouse_moved_event(GLOBAL_MOUSE_ID, math::ivec2 { x, y });
                } break;
                default: return;
            }
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto global_on_event(::win32::HWND   handle,
                             ::win32::UINT   message,
                             ::win32::WPARAM w_param,
                             ::win32::LPARAM l_param) noexcept -> ::win32::LRESULT {
            if (message == ::win32::WM_CREATE) {
                auto lp_create_params = std::bit_cast<::win32::CREATESTRUCT*>(l_param)->lpCreateParams;
                ::win32::SetWindowLongPtrA(handle, ::win32::GWLP_USERDATA, reinterpret_cast<::win32::LONG_PTR>(lp_create_params));
            }

            auto window = handle ? reinterpret_cast<Window*>(::win32::GetWindowLongPtrA(handle, ::win32::GWLP_USERDATA))
                                 : nullptr;

            if (auto result = handle_global_events(message, w_param, l_param); result != std::nullopt) return *result;

            if (window) {
                if (auto result = handle_window_events(*window, message, w_param, l_param); result != std::nullopt)
                    return *result;

                handle_input_events(*window, message, w_param, l_param);
            }

            return ::win32::DefWindowProcA(handle, message, w_param, l_param);
        }
    } // namespace
} // namespace stormkit::wsi::win32
