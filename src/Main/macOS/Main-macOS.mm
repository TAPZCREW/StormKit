#import <Foundation/NSBundle.h>
#import <Foundation/NSFileManager.h>

#include <span>
#include <string_view>
#include <vector>

#include "stormkit-Core.hpp"

extern auto userMain(std::span<const std::string_view>) -> int;

auto main(const int argc, const char **argv) -> int {
    [[NSFileManager defaultManager]
        changeCurrentDirectoryPath:[[NSBundle mainBundle] resourcePath]];

    setup_signal_handler();
    set_current_thread_name("MainThread");

    auto args = std::vector<std::string_view> {};

    for (auto i = 0u; i < static_cast<std::size_t>(argc); ++i) args.emplace_back(argv[i]);

    return userMain(args);
}
