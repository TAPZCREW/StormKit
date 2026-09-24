// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#include <stormkit/core/contract_macro.hpp>

#ifdef STORMKIT_OS_WINDOWS
    #include <stormkit/core/platform/windows.hpp>
#else
    #include <dlfcn.h>
    #include <errno.h>
#endif

module stormkit.core.dynamic_loader;

import std;

import stormkit.core.typesafe.safecasts;
import stormkit.core.errors;

namespace stdfs = std::filesystem;

namespace stormkit {
    /////////////////////////////////////
    /////////////////////////////////////
    dynamic_loader::~dynamic_loader() {
        if (m_library_handle != nullptr) [[likely]] {
#ifdef STORMKIT_OS_WINDOWS
            FreeLibrary(std::bit_cast<HMODULE>(m_library_handle));
#else
            dlclose(m_library_handle);
#endif
            m_library_handle = nullptr;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto dynamic_loader::do_load(const stdfs::path& filepath) -> system_result<void> {
#ifdef STORMKIT_OS_WINDOWS
        const auto wfilepath = filepath.wstring();

        m_library_handle = ::LoadLibraryExW(std::data(wfilepath), nullptr, 0);

        if (not m_library_handle) [[unlikely]]
            return std::unexpected { error_code::from_win32() };
#else
        m_library_handle = ::dlopen(filepath.c_str(), RTLD_LAZY | RTLD_LOCAL);

        if (not m_library_handle) [[unlikely]]
            return std::unexpected { error_code::from_errno() };
#endif

        m_filepath = filepath;

        return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto dynamic_loader::do_get_func(string_view name) const -> system_result<void*> {
        EXPECTS(m_library_handle);
#ifdef STORMKIT_OS_WINDOWS
        auto func = ::GetProcAddress(std::bit_cast<HMODULE>(m_library_handle), std::data(name));

        if (not func) [[unlikely]]
            return std::unexpected { error_code::from_win32() };
#else
        auto func = ::dlsym(m_library_handle, std::data(name));

        if (not func) [[unlikely]]
            return std::unexpected { error_code::from_errno() };
#endif

        return { func };
    }
} // namespace stormkit
