// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#include <cstdio>
#include <cstdlib>

#include <fcntl.h>
#include <io.h>

import std;

import stormkit.core;
import stormkit.core.win32;

#include <version>

using namespace std::string_view_literals;
using namespace stormkit;

namespace {
    // constexpr auto BUF_SIZE          = 1024;
    constexpr auto max_console_lines = ::win32::WORD { 500 };

    // https://stackoverflow.com/questions/191842/how-do-i-get-console-output-in-c-with-a-windows-program
    auto redirect_io_to_console(bool alloc_console) -> bool {
        auto has_console = ::win32::AttachConsole(::win32::ATTACH_PARENT_PROCESS) == ::win32::TRUE;
        auto allocated   = false;
        if (!has_console and alloc_console) {
            // We weren't launched from a console, so make one.
            has_console = ::win32::AllocConsole() == ::win32::TRUE;
            allocated   = has_console;
        }

        if (has_console) {
            // redirect unbuffered STDOUT / STDERR /STDIN handles to the console
            auto std_handle = ::win32::GetStdHandle(::win32::STD_INPUT_HANDLE);
            auto fd         = _open_osfhandle(std::bit_cast<iptr>(std_handle), _O_TEXT);
            auto fp         = _fdopen(fd, "r");
            _dup2(_fileno(fp), _fileno(stdin));
            // ::setvbuf(stdin, nullptr, _IONBF, 0);

            std_handle = ::win32::GetStdHandle(::win32::STD_OUTPUT_HANDLE);
            fd         = _open_osfhandle(std::bit_cast<iptr>(std_handle), _O_TEXT);
            fp         = _fdopen(fd, "w");
            _dup2(_fileno(fp), _fileno(stdout));
            // ::setvbuf(stdout, nullptr, _IONBF, 0);

            std_handle = ::win32::GetStdHandle(::win32::STD_ERROR_HANDLE);
            fd         = _open_osfhandle(std::bit_cast<iptr>(std_handle), _O_TEXT);
            fp         = _fdopen(fd, "w");
            _dup2(_fileno(fp), _fileno(stderr));
            // ::setvbuf(stderr, nullptr, _IONBF, 0);

            if (alloc_console) {
                // set the screen buffer to be big enough to let us scroll text
                auto coninfo = ::win32::CONSOLE_SCREEN_BUFFER_INFO {};

                ::win32::GetConsoleScreenBufferInfo(std_handle, &coninfo);
                coninfo.dwSize.Y = max_console_lines;
                ::win32::SetConsoleScreenBufferSize(std_handle, coninfo.dwSize);
            }

            std::locale::global(std::locale { "" });
            ::win32::SetConsoleOutputCP(::win32::CP_UTF8);
            ::win32::SetConsoleCP(::win32::CP_UTF8);

            // make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
            // point to console as well
            std::ios::sync_with_stdio(true);
        }

        return allocated;
    }
} // namespace

extern auto user_main(array_view<const string_view>) -> int;

auto main(int argc, char** argv) -> int {
    auto args = dynarray<string_view> {};
    args.reserve(as<usize>(argc));

    for (const auto& i : stormkit::range(argc)) args.emplace_back(argv[i]);

    redirect_io_to_console(false);

    setup_signal_handler();
    set_current_thread_name("stormkit:main_thread");

    return user_main(args);
}

auto WinMain(::win32::HINSTANCE, ::win32::HINSTANCE, ::win32::LPSTR, int) -> int {
    const auto argc = __argc;
    const auto argv = __argv;

    auto args = dynarray<string_view> {};
    args.reserve(as<usize>(argc));

    for (auto i : stormkit::range(argc)) args.emplace_back(argv[i]);

    const auto has_allocated = redirect_io_to_console(false);

    setup_signal_handler();
    set_current_thread_name("stormkit:main_thread");

    const auto ret_value = user_main(args);
    if (has_allocated) ::win32::FreeConsole();

    return ret_value;
}
