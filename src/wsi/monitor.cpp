// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#include <stormkit/core/contract_macro.hpp>

module stormkit.wsi;

import std;

import stormkit.core;

import :monitor;
import :core;

#if defined(STORMKIT_OS_WINDOWS)
import :win32.monitor;
namespace impl = stormkit::wsi::win32;
#elif defined(STORMKIT_OS_LINUX)
import :linux.monitor;
namespace impl = stormkit::wsi::linux;
#elif defined(STORMKIT_OS_MACOS)
import :macos.monitor;
namespace impl = stormkit::wsi::macos;
#elif defined(STORMKIT_OS_IOS)
import :ios.monitor;
namespace impl = stormkit::wsi::ios;
#else
    #error "OS not supported !"
#endif

using namespace std::literals;

namespace stormkit::wsi {
    /////////////////////////////////////
    /////////////////////////////////////
    auto get_monitors(bool update) noexcept -> array_view<const Monitor> {
        return impl::get_monitors(wm(), update);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto get_primary_monitor() noexcept -> const Monitor& {
        return impl::get_primary_monitor(wm());
    }
} // namespace stormkit::wsi
