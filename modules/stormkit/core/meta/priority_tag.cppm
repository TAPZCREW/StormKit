// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

export module stormkit.core.meta.priority_tag;

import std;

import stormkit.core.types;

export namespace stormkit { inline namespace core { namespace meta {
    template<usize I>
    struct priority_tag: priority_tag<I - 1> {};

    template<>
    struct priority_tag<0> {};
}}} // namespace stormkit::core::meta
