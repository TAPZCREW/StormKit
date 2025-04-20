#ifndef STORMKIT_CORE_MAIN_HPP
#define STORMKIT_CORE_MAIN_HPP

#include <string_view>

auto setupSignalHandler() -> void;
auto setCurrentThreadName(std::string_view name) -> void;

#endif
