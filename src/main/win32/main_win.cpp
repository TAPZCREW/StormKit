// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#include <windows.h>
#undef __nullnullterminated

#include <cstdio>
#include <cstdlib>

#include <fcntl.h>
#include <io.h>

import std;

import stormkit.core;

#include <version>

using namespace std::string_view_literals;
using namespace stormkit;

namespace {
    // constexpr auto BUF_SIZE          = 1024;
    constexpr auto MAX_CONSOLE_LINES = WORD { 500 };

    // https://stackoverflow.com/questions/191842/how-do-i-get-console-output-in-c-with-a-windows-program
    auto redirect_io_to_console(bool alloc_console) -> bool {
        auto has_console = ::AttachConsole(ATTACH_PARENT_PROCESS) == TRUE;
        auto allocated   = false;
        if (!has_console and alloc_console) {
            // We weren't launched from a console, so make one.
            has_console = ::AllocConsole() == TRUE;
            allocated   = has_console;
        }

        if (has_console) {
            // redirect unbuffered STDOUT / STDERR /STDIN handles to the console
            auto std_handle = ::GetStdHandle(STD_INPUT_HANDLE);
            auto fd         = _open_osfhandle(std::bit_cast<std::intptr_t>(std_handle), _O_TEXT);
            auto fp         = _fdopen(fd, "r");
            _dup2(_fileno(fp), _fileno(stdin));
            // ::setvbuf(stdin, nullptr, _IONBF, 0);

            std_handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
            fd         = _open_osfhandle(std::bit_cast<std::intptr_t>(std_handle), _O_TEXT);
            fp         = _fdopen(fd, "w");
            _dup2(_fileno(fp), _fileno(stdout));
            // ::setvbuf(stdout, nullptr, _IONBF, 0);

            std_handle = ::GetStdHandle(STD_ERROR_HANDLE);
            fd         = _open_osfhandle(std::bit_cast<std::intptr_t>(std_handle), _O_TEXT);
            fp         = _fdopen(fd, "w");
            _dup2(_fileno(fp), _fileno(stderr));
            // ::setvbuf(stderr, nullptr, _IONBF, 0);

            if (alloc_console) {
                // set the screen buffer to be big enough to let us scroll text
                auto coninfo = zeroed<CONSOLE_SCREEN_BUFFER_INFO>();

                ::GetConsoleScreenBufferInfo(std_handle, &coninfo);
                coninfo.dwSize.Y = MAX_CONSOLE_LINES;
                ::SetConsoleScreenBufferSize(std_handle, coninfo.dwSize);
            }

            std::locale::global(std::locale { "" });
            ::SetConsoleOutputCP(CP_UTF8);
            ::SetConsoleCP(CP_UTF8);

            // make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
            // point to console as well
            std::ios::sync_with_stdio(true);
        }

        return allocated;
    }
} // namespace

extern auto user_main(std::span<const std::string_view>) -> int;

auto __stdcall main(int argc, char** argv) -> int {
    auto args = std::vector<std::string_view> {};
    args.reserve(as<usize>(argc));

    for (auto&& i : stormkit::range(argc)) { args.emplace_back(argv[i]); }

    redirect_io_to_console(false);

    setup_signal_handler();
    set_current_thread_name("MainThread");

    return user_main(args);
}

auto __stdcall WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) -> int {
    const auto argc = __argc;
    const auto argv = __argv;

    auto args = std::vector<std::string_view> {};
    args.reserve(as<usize>(argc));

    auto alloc_console = false;
    for (auto&& i : stormkit::range(argc)) {
        if (argv[i] == "--console"sv) alloc_console = true;
        args.emplace_back(argv[i]);
    }

    const auto has_allocated = redirect_io_to_console(alloc_console);

    setup_signal_handler();
    set_current_thread_name("MainThread");

    std::println("AAAAAAAAAAAAAAAAAAAAAA");
    std::cout << "BBBBBBBBBBBBBBBBBBBBBB" << std::endl;
    printf("CCCCCCCCCCCCCCCCCC");

    const auto ret_value = user_main(args);
    if (has_allocated) ::FreeConsole();

    return ret_value;
}
