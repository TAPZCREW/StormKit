// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.core;
import stormkit.test;

#include <stormkit/test/test_macro.hpp>

using namespace stormkit::core;

static_assert(meta::has_hasher<ref_ptr<int>>);
static_assert(meta::trivially_relocatable<ref_ptr<int>>);

namespace {
    constexpr auto VAL = 2;
    auto           _   = test::test_suite {
        "core.typesafe",
        {
          { "ref.refs_of<dynarray>.all_ref",
            [] static noexcept {
                auto a = 0;
                auto b = 1;
                auto c = 2;
                auto d = 3;
                auto e = 4;
                auto f = 5;

                auto refs = refs_of<dynarray>(a, b, c, d, e, f);

                auto i = 0;
                for (const auto& ref : refs) EXPECTS(*ref == i++);
            } },
          { "ref.refs_of.all_ref",
            [] static noexcept {
                auto a = 0;
                auto b = 1;
                auto c = 2;
                auto d = 3;
                auto e = 4;
                auto f = 5;

                auto refs = refs_of(a, b, c, d, e, f);

                auto i = 0;
                for (const auto& ref : refs) EXPECTS(*ref == i++);
            } },
          { "ref.refs_of<hash_set>.all_ref",
            [] static noexcept {
                auto a = 0;
                auto b = 1;
                auto c = 2;
                auto d = 3;
                auto e = 4;
                auto f = 5;

                auto refs = refs_of<hash_set>(a, b, c, d, e, f);

                auto i = 0;
                for (const auto& ref : refs) EXPECTS(*ref == i++);
            } },
          { "ref.refs_of<dynarray>.all_ptr",
            [] static noexcept {
                auto a = allocate_unsafe<int>(0);
                auto b = allocate_unsafe<int>(1);
                auto c = ref_ptr<const int> { VAL };
                auto d = new int { 3 };
                auto e = new int { 4 };
                auto f = 5;

                auto refs = refs_of<dynarray>(a, b, c, d, e, &f);

                auto i = 0;
                for (const auto& ref : refs) EXPECTS(*ref == i++);

                delete d;
                delete e;
            } },
          { "ref.refs_of.all_ptr",
            [] static noexcept {
                auto a = allocate_unsafe<int>(0);
                auto b = allocate_unsafe<int>(1);
                auto c = ref_ptr<const int> { VAL };
                auto d = new int { 3 };
                auto e = new int { 4 };
                auto f = 5;

                auto refs = refs_of(a, b, c, d, e, &f);

                auto i = 0;
                for (const auto& ref : refs) EXPECTS(*ref == i++);

                delete d;
                delete e;
            } },
          { "ref.refs_of<hash_set>.all_ptr",
            [] static noexcept {
                auto a = allocate_unsafe<int>(0);
                auto b = allocate_unsafe<int>(1);
                auto c = ref_ptr<const int> { VAL };
                auto d = new int { 3 };
                auto e = new int { 4 };
                auto f = 5;

                auto refs = refs_of<hash_set>(a, b, c, d, e, &f);

                auto i = 0;
                for (const auto& ref : refs) EXPECTS(*ref == i++);

                delete d;
                delete e;
            } },
          { "ref.as<dynarray>.from_range",
            [] static noexcept {
                // auto vec  = array { 1, 3, 5, 6, 9 };
                // auto refs = as<dynarray>(vec);

                // auto i = 0u;
                // for (const auto& ref : refs) EXPECTS(*ref == vec[i++]);
            } },
          { "ref.as<hash_set>.from_range",
            [] static noexcept {
                // auto vec  = array { 1, 3, 5, 6, 9 };
                // auto refs = as<hash_set>(vec);

                // auto i = 0u;
                // for (const auto& ref : refs) EXPECTS(*ref == vec[i++]);
            } },
          }
    };
} // namespace
