// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/api.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.parallelism.threadutils;

import std;

import stormkit.core.types;
import stormkit.core.meta.concepts;
import stormkit.core.meta.type_query;
import stormkit.core.meta.type_manipulation;

namespace stdr = std::ranges;

export namespace stormkit { inline namespace core {
    STORMKIT_CORE_API
    auto set_current_thread_name(string_view name) noexcept -> void;
    STORMKIT_CORE_API
    auto set_thread_name(std::thread& thread, string_view name) noexcept -> void;
    STORMKIT_CORE_API
    auto set_thread_name(std::jthread& thread, string_view name) noexcept -> void;
    STORMKIT_CORE_API
    auto get_current_thread_name() noexcept -> string;
    STORMKIT_CORE_API
    auto get_thread_name(const std::thread& thread) noexcept -> string;
    STORMKIT_CORE_API
    auto get_thread_name(const std::jthread& thread) noexcept -> string;

    template<meta::plain::apply_to<stdr::input_range> Range>
    auto wait_all(Range&& futures) noexcept
        requires(meta::specialization_of<meta::range_value_type<meta::to_plain_type<Range>>, std::future>);
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    ////////////////////////////////////////
    ////////////////////////////////////////
    template<meta::plain::apply_to<stdr::input_range> Range>
        STORMKIT_FORCE_INLINE
    inline auto wait_all(Range&& futures) noexcept
        requires(meta::specialization_of<meta::range_value_type<meta::to_plain_type<Range>>, std::future>)
    {
        for (const auto& future : std::forward<Range>(futures)) future.wait();
    }
}} // namespace stormkit::core
