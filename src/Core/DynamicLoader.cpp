// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module stormkit.Core;

import std;

import :Utils.DynamicLoaderImpl;

namespace stormkit {
    /////////////////////////////////////
    /////////////////////////////////////
    DynamicLoader::DynamicLoader() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    DynamicLoader::DynamicLoader(DynamicLoader&& other) noexcept
        : m_filepath { std::move(other.m_filepath) }, m_impl { std::move(other.m_impl) } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    DynamicLoader::~DynamicLoader() = default;

    /////////////////////////////////////
    /////////////////////////////////////
    auto DynamicLoader::operator=(DynamicLoader&& other) noexcept -> DynamicLoader& {
        if (&other == this) [[unlikely]]
            return *this;

        m_impl     = std::move(other.m_impl);
        m_filepath = std::move(other.m_filepath);

        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto DynamicLoader::doLoad(std::filesystem::path filepath) -> Expected<void> {
        m_filepath = std::move(filepath);

        return m_impl->doLoad(m_filepath);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    auto DynamicLoader::doGetFunc(std::string_view name) const -> Expected<void*> {
        return m_impl->doGetFunc(name);
    }
}// namespace stormkit::core
