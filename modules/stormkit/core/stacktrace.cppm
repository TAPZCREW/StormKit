module;

#include <stormkit/core/api.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.stacktrace;

import std;

import stormkit.core.types;

export namespace stormkit { inline namespace core {
    STORMKIT_CORE_API
    auto print_stacktrace(u32 ignore_count = 0) noexcept -> void;
}} // namespace stormkit::core
