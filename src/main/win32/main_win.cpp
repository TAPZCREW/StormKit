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

namespace {
    constexpr auto BUF_SIZE          = 1024;
    constexpr auto MAX_CONSOLE_LINES = WORD { 500 };

    // https://stackoverflow.com/questions/191842/how-do-i-get-console-output-in-c-with-a-windows-program
    auto redirect_io_to_console(bool alloc_console) -> void {
        const auto has_console = ::AttachConsole(ATTACH_PARENT_PROCESS) == TRUE and alloc_console;
        if (!has_console) {
            // We weren't launched from a console, so make one.
            AllocConsole();

            // set the screen buffer to be big enough to let us scroll text
            auto coninfo = stormkit::zeroed<CONSOLE_SCREEN_BUFFER_INFO>();

            ::GetConsoleScreenBufferInfo(stormkit::get_stdout(), &coninfo);
            coninfo.dwSize.Y = MAX_CONSOLE_LINES;
            ::SetConsoleScreenBufferSize(stormkit::get_stdout(), coninfo.dwSize);
        }

        const auto handles = std::array<std::tuple<DWORD, std::FILE*, stormkit::CZString>, 3> {
            std::tuple { STD_OUTPUT_HANDLE, stdout, "w" },
            { STD_ERROR_HANDLE,  stderr, "w" },
            { STD_INPUT_HANDLE,  stdin,  "r" },
        };

        // redirect unbuffered STDOUT / STDERR /STDIN handles to the console
        std::FILE* fp = nullptr;
        if (alloc_console) {
            for (const auto& [handle, std_file_handle, access] : handles) {
                const auto std_handle         = ::GetStdHandle(handle);
                const auto std_handle_address = std::bit_cast<std::intptr_t>(std_handle);
                const auto con_handle         = ::_open_osfhandle(std_handle_address, _O_TEXT);
                fp                            = ::_fdopen(con_handle, access);
                *std_file_handle              = *fp;
            }
        } else {
            // ::freopen_s(&fp, "CONIN$", "r", stdin);
            // ::freopen_s(&fp, "CONOUT$", "w", stdout);
            // ::freopen_s(&fp, "CONOUT$", "w", stderr);
        }

        // make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
        // point to console as well
        std::ios::sync_with_stdio(true);

        // Clear the error state for each of the C++ standard streams.
        // std::wcout.clear();
        // std::cout.clear();
        // std::wcerr.clear();
        // std::cerr.clear();
        // std::wcin.clear();
        // std::cin.clear();
    }

} // namespace

extern auto user_main(std::span<const std::string_view>) -> int;

using namespace stormkit;

auto __stdcall main(int argc, char** argv) -> int {
    auto args = std::vector<std::string_view> {};
    args.reserve(as<usize>(argc));

    for (auto&& i : stormkit::range(argc)) { args.emplace_back(argv[i]); }

    redirect_io_to_console(false);

    std::locale::global(std::locale { "" });
    ::SetConsoleOutputCP(CP_UTF8);
    ::SetConsoleCP(CP_UTF8);

    // on Windows 10+ we need buffering or console will get 1 byte at a time (screwing up utf-8
    // encoding)
    ::setvbuf(stderr, nullptr, _IOFBF, BUF_SIZE);
    ::setvbuf(stdout, nullptr, _IOFBF, BUF_SIZE);
    ::setvbuf(stdin, nullptr, _IOFBF, BUF_SIZE);

    stormkit::setup_signal_handler();
    stormkit::set_current_thread_name("MainThread");

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

    redirect_io_to_console(alloc_console);

    std::locale::global(std::locale { "" });
    ::SetConsoleOutputCP(CP_UTF8);
    ::SetConsoleCP(CP_UTF8);

    // on Windows 10+ we need buffering or console will get 1 byte at a time (screwing up utf-8
    // encoding)
    ::setvbuf(stderr, nullptr, _IOFBF, BUF_SIZE);
    ::setvbuf(stdout, nullptr, _IOFBF, BUF_SIZE);
    ::setvbuf(stdin, nullptr, _IOFBF, BUF_SIZE);

    stormkit::setup_signal_handler();
    stormkit::set_current_thread_name("MainThread");

    std::println(stdout, "ZEAAZOIEUAUZEOIUOIAUZEIOUIAZ");
    std::cout << "DJLKQJLKDKQLJDLJQSKJDJ" << std::endl;
    printf("DJLQSDLJCWXJCOQOIDUQOFQHFLJQDPQO");

    return user_main(args);
}
