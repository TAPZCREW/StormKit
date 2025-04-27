#ifndef STORMKIT_CORE_MAIN_HPP
#define STORMKIT_CORE_MAIN_HPP

#include <string_view>

auto setup_signal_handler() -> void;
auto set_current_thread_name(std::string_view name) -> void;

#endif
