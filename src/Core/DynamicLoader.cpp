// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/Core/PlatformMacro.hpp>

#ifdef STORMKIT_OS_WINDOWS
    #include <windows.h>
#else
    #include <dlfcn.h>
    #include <errno.h>
#endif

module stormkit.core;

import std;

namespace stormkit {
    /////////////////////////////////////
    /////////////////////////////////////
    DynamicLoader::~DynamicLoader() {
        if(m_library_handle != nullptr) [[likely]] {
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
    auto DynamicLoader::do_load(std::filesystem::path filepath) -> Expected<void> {
#ifdef STORMKIT_OS_WINDOWS
        const auto wfilepath = filepath.wstring();

        m_library_handle = ::LoadLibraryExW(std::data(wfilepath), nullptr, 0);

        if (not m_library_handle) [[unlikely]]
            return std::unexpected(
                std::error_code { as<Int32>(GetLastError()), std::system_category() });
#else
        m_library_handle = ::dlopen(filepath.c_str(), RTLD_LAZY | RTLD_LOCAL);

        if (not m_library_handle) [[unlikely]]
            return std::unexpected(
                std::error_code { static_cast<Int32>(errno), std::system_category() });
#endif

        m_filepath = std::move(filepath);

        return {};
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto DynamicLoader::do_get_func(std::string_view name) const -> Expected<void*> {
        expects(m_library_handle);
#ifdef STORMKIT_OS_WINDOWS
        auto func = ::GetProcAddress(std::bit_cast<HMODULE>(m_library_handle), std::data(name));

        if (not func) [[unlikely]]
            return std::unexpected(
                std::error_code { as<Int32>(::GetLastError()), std::system_category() });
#else
        auto func = ::dlsym(m_library_handle, std::data(name));

        if (not func) [[unlikely]]
            return std::unexpected(
                std::error_code { static_cast<Int32>(errno), std::system_category() });
#endif

        return { func };
    }
} // namespace stormkit
