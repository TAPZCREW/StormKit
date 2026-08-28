// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

export module stormkit.core.overload_set;

import std;

import stormkit.core.types;

export namespace stormkit { inline namespace core {
    template<class... Ts>
    struct overload_set: Ts... {
        using Ts::operator()...;
    };

    template<class... Ts>
    overload_set(Ts...) -> overload_set<Ts...>;
}} // namespace stormkit::core
