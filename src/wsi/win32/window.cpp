// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform/windows.hpp>

#include <cstdlib>

#include <shellscalingapi.h>

#include <stormkit/core/platform_macro.hpp>

#include <stormkit/core/contract_macro.hpp>

module stormkit.wsi;

import :win32.window;
import :win32.keyboard;

import :common.input_base;

import :win32.log;
import :win32.keyboard;
import :win32.mouse;

namespace stdv = std::views;
namespace stdr = std::ranges;

using namespace stormkit;

template<typename FormatContext>
constexpr auto format_as(const POINT& point, FormatContext& ctx) -> decltype(ctx.out()) {
    return std::format_to(ctx.out(), "[vec2 x: {}, y: {}]", point.x, point.y);
}

template<typename FormatContext>
constexpr auto format_as(const RECT& rect, FormatContext& ctx) -> decltype(ctx.out()) {
    return std::format_to(ctx.out(),
                          "[rect left: {}, top: {}, right: {}, bottom: {}]",
                          rect.left,
                          rect.top,
                          rect.right,
                          rect.bottom);
}

auto adjust_extent(const math::uextent2& extent, DWORD style, DWORD style_ex) noexcept -> math::extent2<LONG> {
    auto rect = RECT { .left = 0, .top = 0, .right = as<LONG>(extent.width), .bottom = as<LONG>(extent.height) };

    AdjustWindowRectEx(&rect, style, FALSE, style_ex);

    return { rect.right - rect.left, rect.bottom - rect.top };
}

namespace stormkit::wsi::win32 {
    using HBrush = RAIICapsule<HBRUSH, CreateSolidBrush, DeleteObject, struct HBrushTag, nullptr>;

    namespace {
        constexpr auto CLASS_NAME = "Stormkit_Window";

        auto get_client_rect(HWND window_handle) noexcept -> RECT;

        // auto get_monitor_scale(HMONITOR monitor) -> math::fvec2;

        auto CALLBACK global_on_event(HWND handle, UINT message, WPARAM w_param, LPARAM l_param) noexcept -> LRESULT;

        constinit auto g_window_count = std::atomic<u8> { 0 };
    } // namespace

    /////////////////////////////////////
    /////////////////////////////////////
    Window::Window(WM) noexcept {
        if (g_window_count == 0) {
            SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
            auto h_instance   = GetModuleHandleA(nullptr);
            auto window_class = WNDCLASSA {};
            if (GetClassInfoA(h_instance, CLASS_NAME, &window_class) == FALSE) {
                window_class.lpfnWndProc   = &global_on_event;
                window_class.hInstance     = GetModuleHandleA(nullptr);
                window_class.lpszClassName = CLASS_NAME;
                RegisterClassA(&window_class);
            }
        }

        g_window_count += 1;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    Window::~Window() noexcept {
        close();

        g_window_count -= 1;
        if (g_window_count == 0) UnregisterClassA(CLASS_NAME, GetModuleHandleA(nullptr));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    Window::Window(Window&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::operator=(Window&&) noexcept -> Window& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::open(string title, const math::uextent2& extent, WindowFlag flags) noexcept -> void {
        auto style      = DWORD { WS_SYSMENU | WS_BORDER };
        auto style_ex   = DWORD { 0 };
        auto h_instance = GetModuleHandleA(nullptr);

        if (has_flag_bit(flags, WindowFlag::BORDERLESS)) style |= WS_POPUP | WS_EX_CLIENTEDGE;
        else
            style |= (WS_OVERLAPPED | WS_CAPTION);

        if (has_flag_bit(flags, WindowFlag::RESIZEABLE)) style |= WS_MAXIMIZEBOX | WS_THICKFRAME;

        auto gdi = true;
        if (has_flag_bit(flags, WindowFlag::EXTERNAL_CONTEXT)) {
            m_win32_state.external_context = true;
            style_ex |= WS_EX_NOREDIRECTIONBITMAP;
            gdi = false;
        }

        SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);

        const auto adjusted       = adjust_extent(extent, style, style_ex);
        m_window_handle           = CreateWindowExA(style_ex,
                                                    CLASS_NAME,
                                                    std::data(title),
                                                    style,
                                                    CW_USEDEFAULT,
                                                    CW_USEDEFAULT,
                                                    adjusted.width,
                                                    adjusted.height,
                                                    nullptr,
                                                    nullptr,
                                                    h_instance,
                                                    this);
        m_state.open              = true;
        auto        win32_monitor = MonitorFromWindow(m_window_handle, MONITOR_DEFAULTTONEAREST);
        const auto  monitors      = get_monitors();
        const auto& monitor       = *stdr::find_if(monitors, [&win32_monitor](auto&& monitor) noexcept {
            return monitor.native_handle == win32_monitor;
        });

        set_current_monitor(monitor);

        m_win32_state.style    = as<DWORD>(GetWindowLongA(m_window_handle, GWL_STYLE));
        m_win32_state.style_ex = as<DWORD>(GetWindowLongA(m_window_handle, GWL_EXSTYLE));

        m_state.title = std::move(title);

        ShowWindow(m_window_handle, SW_SHOWNORMAL);
        m_state.active = true;

        update_geometry(extent);
        if (gdi) gdiinit();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::close() noexcept -> void {
        if (not m_win32_state.external_context) m_gdi_frame_data = GDIFrameData {};

        if (m_window_handle) DestroyWindow(m_window_handle);

        m_keyboard_states = {};
        m_mouse_states    = {};

        m_window_handle = nullptr;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::clear(const ucolor_rgb& color) noexcept -> void {
        if (m_win32_state.external_context) return;

        auto       hbrush = HBrush::create(RGB(color.r, color.g, color.b));
        const auto rect   = RECT { 0, 0, as<LONG>(m_state.extent.width), as<LONG>(m_state.extent.height) };

        FillRect(m_gdi_frame_data.context, &rect, hbrush);
        InvalidateRect(m_window_handle, nullptr, FALSE);
        UpdateWindow(m_window_handle);
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
                   std::bit_cast<u32*>(m_gdi_frame_data.pixels_ptr.load()));

        InvalidateRect(m_window_handle, nullptr, FALSE);
        UpdateWindow(m_window_handle);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::handle_events() noexcept -> void {
        if (not m_window_handle) return;

        auto message = MSG {};
        while (PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_title(string title) noexcept -> void {
        SetWindowTextA(m_window_handle, std::data(title));

        WindowBase::set_title(std::move(title));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_extent(const math::uextent2& extent) noexcept -> void {
        const auto adjusted = adjust_extent(extent, m_win32_state.style, m_win32_state.style_ex);
        SetWindowPos(m_window_handle,
                     HWND_TOP,
                     0,
                     0,
                     adjusted.width,
                     adjusted.height,
                     SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOOWNERZORDER);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_fullscreen(bool fullscreen) noexcept -> void {
        auto [x, y] = m_state.position;

        auto style    = m_win32_state.style;
        auto style_ex = m_win32_state.style_ex;
        if (fullscreen) {
            style &= as<DWORD>(~(WS_CAPTION | WS_THICKFRAME));
            style_ex &= as<DWORD>(~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));

            x = 0;
            y = 0;
        }

        SetWindowLongA(m_window_handle, GWL_STYLE, as<LONG>(style));
        SetWindowLongA(m_window_handle, GWL_EXSTYLE, as<LONG>(style_ex));

        SetWindowPos(m_window_handle,
                     nullptr,
                     x,
                     y,
                     as<i32>(m_state.extent.width),
                     as<i32>(m_state.extent.height),
                     SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

        WindowBase::set_fullscreen(fullscreen);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::confine_mouse(bool confined, u8 id) noexcept -> void {
        expects(id == GLOBAL_MOUSE_ID, "StormKit WSI win32 backend only support one mouse");
        if (is_mouse_locked(GLOBAL_MOUSE_ID)) return;

        if (confined) {
            const auto rect = get_client_rect(m_window_handle);
            ClipCursor(&rect);
        } else
            ClipCursor(nullptr);

        auto& state    = m_mouse_states[GLOBAL_MOUSE_ID];
        state.confined = confined;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::is_mouse_confined(u8 id) const noexcept -> bool {
        expects(id == GLOBAL_MOUSE_ID, "StormKit WSI win32 backend only support one mouse");
        auto& state = m_mouse_states[GLOBAL_MOUSE_ID];
        return state.confined;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::lock_mouse(bool locked, u8 id) noexcept -> void {
        expects(id == GLOBAL_MOUSE_ID, "StormKit WSI win32 backend only support one mouse");
        auto& state = m_mouse_states[GLOBAL_MOUSE_ID];

        if (locked) {
            auto mouse_position = POINT {};
            GetCursorPos(&mouse_position);

            auto rect   = RECT {};
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

            ClipCursor(&rect);

            ScreenToClient(m_window_handle, &mouse_position);

            state.locked_at.x = as<u32>(mouse_position.x);
            state.locked_at.y = as<u32>(mouse_position.y);
        } else
            ClipCursor(nullptr);

        state.locked   = locked;
        state.confined = locked;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::is_mouse_locked(u8 id) const noexcept -> bool {
        expects(id == GLOBAL_MOUSE_ID, "StormKit WSI win32 backend only support one mouse");
        auto& state = m_mouse_states[GLOBAL_MOUSE_ID];
        return state.locked;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::hide_mouse(bool hidden, u8 id) noexcept -> void {
        expects(id == GLOBAL_MOUSE_ID, "StormKit WSI win32 backend only support one mouse");
        if (hidden)
            while (ShowCursor(FALSE) >= 0);
        else
            ShowCursor(TRUE);

        auto& state  = m_mouse_states[GLOBAL_MOUSE_ID];
        state.hidden = hidden;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::is_mouse_hidden(u8 id) const noexcept -> bool {
        expects(id == GLOBAL_MOUSE_ID, "StormKit WSI win32 backend only support one mouse");
        auto& state = m_mouse_states[GLOBAL_MOUSE_ID];
        return state.hidden;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_relative_mouse(bool enabled, u8 id) noexcept -> void {
        expects(id == GLOBAL_MOUSE_ID, "StormKit WSI win32 backend only support one mouse");
        auto& state = m_mouse_states[GLOBAL_MOUSE_ID];

        if (state.locked) {
            auto locked_at = POINT { as<i32>(state.locked_at.x), as<i32>(state.locked_at.y) };

            ClientToScreen(m_window_handle, &locked_at);

            auto rect   = RECT {};
            rect.top    = as<LONG>(locked_at.y);
            rect.left   = as<LONG>(locked_at.x);
            rect.bottom = as<LONG>(locked_at.y);
            rect.right  = as<LONG>(locked_at.x);

            if (enabled) {
                rect.top -= 5;
                rect.left -= 5;
                rect.bottom += 5;
                rect.right += 5;
            }

            ClipCursor(&rect);
        }

        state.relative = enabled;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::is_mouse_relative(u8 id) const noexcept -> bool {
        expects(id == GLOBAL_MOUSE_ID, "StormKit WSI win32 backend only support one mouse");
        auto& state = m_mouse_states[GLOBAL_MOUSE_ID];
        return state.relative;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::set_key_repeat(bool enabled, u8 id) noexcept -> void {
        expects(id == GLOBAL_KEYBOARD_ID, "StormKit WSI win32 backend only support one keyboard");
        auto& state      = m_keyboard_states[GLOBAL_KEYBOARD_ID];
        state.key_repeat = enabled;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::is_key_repeat_enabled(u8 id) const noexcept -> bool {
        expects(id == GLOBAL_KEYBOARD_ID, "StormKit WSI win32 backend only support one keyboard");
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
        expects(id == GLOBAL_MOUSE_ID, "StormKit WSI win32 backend only support one mouse");
        auto mouse_position = POINT { as<long>(position.x), as<long>(position.y) };
        ClientToScreen(m_window_handle, &mouse_position);
        SetCursorPos(mouse_position.x, mouse_position.y);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto Window::gdiinit() noexcept -> void {
        const auto hdesktop = GetDC(nullptr);

        m_gdi_frame_data        = GDIFrameData {};
        m_gdi_frame_data.extent = extent().to<LONG>();

        const auto [width, height] = m_gdi_frame_data.extent;

        const auto byte_count = as<usize>(width * height) * sizeof(u32);

        const auto frame_bitmap_info = BITMAPINFOHEADER {
            .biSize          = sizeof(BITMAPINFOHEADER),
            .biWidth         = width,
            .biHeight        = -height,
            .biPlanes        = 1,
            .biBitCount      = 32,
            .biCompression   = BI_RGB,
            .biSizeImage     = as<DWORD>(byte_count),
            .biXPelsPerMeter = 0,
            .biYPelsPerMeter = 0,
            .biClrUsed       = 0,
            .biClrImportant  = 0,
        };

        auto ptr                    = core::ptr<void> { nullptr };
        m_gdi_frame_data.context    = Hdc::create(hdesktop);
        m_gdi_frame_data.bitmap     = HBitmap::create(m_gdi_frame_data.context,
                                                      std::bit_cast<const BITMAPINFO*>(&frame_bitmap_info),
                                                      as<UINT>(DIB_RGB_COLORS),
                                                      &ptr,
                                                      nullptr,
                                                      as<DWORD>(0));
        m_gdi_frame_data.pixels_ptr = ptr;
        m_gdi_frame_data.extent     = { width, height };
        SelectObject(m_gdi_frame_data.context, m_gdi_frame_data.bitmap);

        ReleaseDC(nullptr, hdesktop);
    }

    namespace {
        /////////////////////////////////////
        /////////////////////////////////////
        auto get_client_rect(HWND window_handle) noexcept -> RECT {
            const auto client_rect = init_by<RECT>([window_handle](auto& out) noexcept { GetClientRect(window_handle, &out); });

            const auto lefttop = init_by<POINT>([window_handle, &client_rect](POINT& out) noexcept {
                out.x = client_rect.left;
                out.y = client_rect.top;
                ClientToScreen(window_handle, &out);
            });

            const auto rightbottom = init_by<POINT>([window_handle, &client_rect](POINT& out) noexcept {
                out.x = client_rect.right;
                out.y = client_rect.bottom;
                ClientToScreen(window_handle, &out);
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
        auto handle_global_events(UINT, WPARAM, LPARAM) noexcept -> std::optional<LRESULT> {
            return std::nullopt;
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto handle_window_events(Window& window, UINT message, WPARAM w_param, LPARAM l_param) noexcept
          -> std::optional<LRESULT> {
            const auto window_handle = std::bit_cast<HWND>(window.native_handle());
            if (message != WM_DESTROY and not window_handle) return 0;

            switch (message) {
                case WM_DESTROY: PostQuitMessage(0); return 0;
                case WM_CLOSE:
                    if (window.closed_event()) DestroyWindow(window_handle);
                    return 0;
                case WM_CREATE: window.WindowBase::set_open(true); break;
                case WM_NCDESTROY: window.WindowBase::set_open(false); break;
                case WM_MOUSEACTIVATE: {
                    return MA_ACTIVATEANDEAT;
                }
                case WM_ACTIVATE: {
                    if (LOWORD(w_param) == WA_INACTIVE) {
                        window.state().active = false;
                        window.deactivate_event();
                    } else {
                        window.state().active = true;
                        window.activate_event();
                    }
                } break;
                case WM_NCLBUTTONDOWN:
                    if (SendMessageA(window_handle, WM_NCHITTEST, w_param, l_param) == HTCAPTION) {
                        auto pos = POINT {};
                        GetCursorPos(&pos);
                        ScreenToClient(window_handle, &pos);

                        const auto y = (pos.y << 16);

                        PostMessage(window_handle, WM_MOUSEMOVE, 0, pos.x | y);
                    }
                    break;
                case WM_WINDOWPOSCHANGING: {
                    auto win32_monitor = MonitorFromWindow(window_handle, MONITOR_DEFAULTTONEAREST);
                    if (window.current_monitor().native_handle != win32_monitor) {
                        const auto  monitors = get_monitors();
                        const auto& _monitor = *stdr::find_if(monitors, [&win32_monitor](auto&& monitor) noexcept {
                            return monitor.native_handle == win32_monitor;
                        });

                        window.set_current_monitor(_monitor);
                        window.monitor_changed_event(std::move(_monitor));
                    }
                } break;
                case WM_PAINT: {
                    if (window.win32_state().external_context) break;

                    const auto& gdi_frame_data = window.gdi_frame_data();

                    auto ps                               = PAINTSTRUCT {};
                    auto hdc                              = BeginPaint(window_handle, &ps);
                    const auto [left, top, right, bottom] = ps.rcPaint;
                    const auto cx                         = right - left;
                    const auto cy                         = bottom - top;

                    BitBlt(hdc, left, top, cx, cy, gdi_frame_data.context, left, top, SRCCOPY);
                    EndPaint(window_handle, &ps);

                    return 0;
                }
                case WM_SIZE: {
                    window.update_geometry({ as<u32>(LOWORD(l_param)), as<u32>(HIWORD(l_param)) });

                    if (not window.win32_state().external_context) {
                        auto& gdi_frame_data = window.gdi_frame_data();
                        window.gdiinit();

                        const auto width  = as<i32>(gdi_frame_data.extent.width);
                        const auto height = as<i32>(gdi_frame_data.extent.height);

                        auto ps  = PAINTSTRUCT {};
                        auto hdc = BeginPaint(window_handle, &ps);
                        BitBlt(hdc, 0, 0, width, height, gdi_frame_data.context, 0, 0, SRCCOPY);
                        EndPaint(window_handle, &ps);
                    }

                    switch (w_param) {
                        case SIZE_MINIMIZED: window.minimized_event(); break;
                        case SIZE_RESTORED: window.restored_event(); break;
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
        auto handle_input_events(Window& window, UINT message, WPARAM w_param, LPARAM l_param) noexcept -> void {
            const auto window_handle = std::bit_cast<HWND>(window.native_handle());
            if (not window.state().active) return;

            switch (message) {
                case WM_LBUTTONDOWN: [[fallthrough]];
                case WM_RBUTTONDOWN: [[fallthrough]];
                case WM_MBUTTONDOWN: [[fallthrough]];
                case WM_XBUTTONDOWN: {
                    const auto [x, y] = extract_mouse_position(window_handle, w_param, l_param, false);
                    const auto button = extract_mouse_button(message, w_param, l_param);
                    window.mouse_button_down_event(GLOBAL_MOUSE_ID, button, math::ivec2 { x, y });
                } break;
                case WM_LBUTTONUP: [[fallthrough]];
                case WM_RBUTTONUP: [[fallthrough]];
                case WM_MBUTTONUP: [[fallthrough]];
                case WM_XBUTTONUP: {
                    const auto [x, y] = extract_mouse_position(window_handle, w_param, l_param, false);
                    const auto button = extract_mouse_button(message, w_param, l_param);
                    window.mouse_button_up_event(GLOBAL_MOUSE_ID, button, math::ivec2 { x, y });
                } break;
                case WM_KEYDOWN: [[fallthrough]];
                case WM_SYSKEYDOWN: {
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
                case WM_KEYUP: [[fallthrough]];
                case WM_SYSKEYUP: {
                    auto& state = window.keyboard_state(GLOBAL_MOUSE_ID);

                    const auto key       = extract_key(w_param, l_param);
                    const auto character = extract_key_to_char(w_param, l_param);
                    const auto to_index  = as<underlying>(key);

                    window.key_up_event(GLOBAL_KEYBOARD_ID, key, character);
                    state.keys[to_index] = common::KeyState::UP;
                } break;
                case WM_MOUSEMOVE: {
                    auto& state = window.mouse_state(GLOBAL_MOUSE_ID);
                    if (state.locked and not state.relative) break;

                    if (not window.win32_state().mouse_tracked) {
                        auto track_mouse_event        = TRACKMOUSEEVENT {};
                        track_mouse_event.cbSize      = sizeof(TRACKMOUSEEVENT);
                        track_mouse_event.dwFlags     = TME_LEAVE;
                        track_mouse_event.hwndTrack   = window_handle;
                        track_mouse_event.dwHoverTime = HOVER_DEFAULT;
                        if (TrackMouseEvent(&track_mouse_event) != FALSE) window.win32_state().mouse_tracked = true;
                    }

                    const auto [x, y] = extract_mouse_position(window_handle, w_param, l_param, false);

                    if (state.locked and state.relative) {
                        const auto relative_x = x - as<i32>(state.locked_at.x);
                        const auto relative_y = y - as<i32>(state.locked_at.y);
                        window.mouse_moved_event(GLOBAL_MOUSE_ID, math::ivec2 { relative_x, relative_y });
                    } else if (state.relative) {
                        const auto relative_x = x - as<i32>(state.last_position.x);
                        const auto relative_y = y - as<i32>(state.last_position.y);
                        window.mouse_moved_event(GLOBAL_MOUSE_ID, math::ivec2 { relative_x, relative_y });
                    } else
                        window.mouse_moved_event(GLOBAL_MOUSE_ID, math::ivec2 { x, y });
                } break;
                default: return;
            }
        }

        /////////////////////////////////////
        /////////////////////////////////////
        auto global_on_event(HWND handle, UINT message, WPARAM w_param, LPARAM l_param) noexcept -> LRESULT {
            if (message == WM_CREATE) {
                auto lp_create_params = std::bit_cast<CREATESTRUCT*>(l_param)->lpCreateParams;
                SetWindowLongPtrA(handle, GWLP_USERDATA, std::bit_cast<LONG_PTR>(lp_create_params));
            }

            auto window = handle ? std::bit_cast<Window*>(GetWindowLongPtrA(handle, GWLP_USERDATA)) : nullptr;

            if (auto result = handle_global_events(message, w_param, l_param); result != std::nullopt) return *result;

            if (window) {
                if (auto result = handle_window_events(*window, message, w_param, l_param); result != std::nullopt)
                    return *result;

                handle_input_events(*window, message, w_param, l_param);
            }

            return DefWindowProcA(handle, message, w_param, l_param);
        }
    } // namespace
} // namespace stormkit::wsi::win32
