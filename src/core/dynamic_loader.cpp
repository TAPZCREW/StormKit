// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#include <stormkit/core/contract_macro.hpp>

#ifndef STORMKIT_OS_WINDOWS
    #include <dlfcn.h>
    #include <errno.h>
#endif

module stormkit.core.dynamic_loader;

import std;

#ifdef STORMKIT_OS_WINDOWS
import stormkit.core.win32;
#endif
import stormkit.core.typesafe.safecasts;
import stormkit.core.errors;

namespace stdfs = std::filesystem;

namespace stormkit {
    /////////////////////////////////////
    /////////////////////////////////////
    dynamic_loader::~dynamic_loader() {
        if (library_handle_ != nullptr) [[likely]] {
#ifdef STORMKIT_OS_WINDOWS
            ::win32::FreeLibrary(reinterpret_cast<::win32::HMODULE>(library_handle_));
#else
            dlclose(library_handle_);
#endif
            library_handle_ = nullptr;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto dynamic_loader::do_load(const stdfs::path& filepath) -> system_result<void> {
#ifdef STORMKIT_OS_WINDOWS
        const auto wfilepath = filepath.wstring();

        library_handle_ = ::win32::LoadLibraryExW(std::data(wfilepath), nullptr, 0);

        if (not library_handle_) [[unlikely]]
            return std::unexpected { error_code::from_win32() };
#else
        library_handle_ = ::dlopen(filepath.c_str(), RTLD_LAZY | RTLD_LOCAL);

        if (not library_handle_) [[unlikely]]
            return std::unexpected { error_code::from_errno() };
#endif

        filepath_ = filepath;

        return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto dynamic_loader::do_get_func(string_view name) const -> system_result<void*> {
        EXPECTS(library_handle_);
#ifdef STORMKIT_OS_WINDOWS
        auto func = ::win32::GetProcAddress(std::bit_cast<::win32::HMODULE>(library_handle_), std::data(name));

        if (not func) [[unlikely]]
            return std::unexpected { error_code::from_win32() };
#else
        auto func = ::dlsym(library_handle_, std::data(name));

        if (not func) [[unlikely]]
            return std::unexpected { error_code::from_errno() };
#endif

        return { func };
    }
} // namespace stormkit
