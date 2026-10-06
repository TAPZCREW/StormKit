// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
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
    struct foo {
        int a;

        auto bar() -> int { return a; }
    };

    constexpr auto VAL = 2;
    auto           _   = test::test_suite {
        "core.typesafe",
        {
          { "ref.from_reference",
            [] static noexcept {
                auto a = 0;

                auto ref  = ref_ptr { a };
                auto ref2 = view_of(a);
                auto ref3 = ref_of(a);

                EXPECTS(*ref == *ref2);
                EXPECTS(*ref2 == *ref3);
            } },
          { "ref.view_pointers",
            [] static noexcept {
                auto a = 0;

                auto ref  = ref_ptr { &a };
                auto ref2 = ref_ptr { ref };
                EXPECTS(*ref == *ref2);
            } },
          { "ref.owning_pointer",
            [] static noexcept {
                auto a = allocate_unsafe<int>(2);

                auto ref = ref_ptr { a };
                EXPECTS(*ref == *a);
            } },
          { "ref.operator->",
            [] static noexcept {
                auto a = allocate_unsafe<foo>(2);

                auto ref = ref_ptr { a };
                EXPECTS(a->bar() == 2);
            } },
          { "ref.refs_of_dynarray.all_ref",
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
          { "ref.refs_of_hash_set.all_ref",
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
          { "ref.refs_of_dynarray.all_ptr",
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
          { "ref.refs_of_hash_set.all_ptr",
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
          }
    };
} // namespace
