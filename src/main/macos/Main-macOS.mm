#include <span>
#include <string_view>
#include <vector>

#include "stormkit_core.hpp"

extern auto user_main(std::span<const std::string_view>) -> int;

auto main(const int argc, const char** argv) -> int {
    setup_signal_handler();
    set_current_thread_name("stormkit:main_thread");

    auto args = std::vector<std::string_view> {};

    for (auto i = 0u; i < static_cast<std::size_t>(argc); ++i) args.emplace_back(argv[i]);

    return user_main(args);
}
