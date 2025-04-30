// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.core;

import Test;

using namespace stormkit::core;

#define expects(x) test::expects(x, #x)

namespace {
    auto _ = test::TestSuite {
        "Core.typesafe",
        {
          { "Ref.to_refs.std_vector.all_ref",
              [] static noexcept {
                  auto a = 0;
                  auto b = 1;
                  auto c = 2;
                  auto d = 3;
                  auto e = 4;
                  auto f = 5;

                  auto refs = to_refs<std::vector>(a, b, c, d, e, f);

                  auto i = 0;
                  for (auto&& ref : refs) expects(*ref == i++);
              } },
          { "Ref.as_refs.std_array.all_ref",
              [] static noexcept {
                  auto a = 0;
                  auto b = 1;
                  auto c = 2;
                  auto d = 3;
                  auto e = 4;
                  auto f = 5;

                  auto refs = as_refs<std::array>(a, b, c, d, e, f);

                  auto i = 0;
                  for (auto&& ref : refs) expects(*ref == i++);
              } },
          { "Ref.as_refs.default.all_ref",
              [] static noexcept {
                  auto a = 0;
                  auto b = 1;
                  auto c = 2;
                  auto d = 3;
                  auto e = 4;
                  auto f = 5;

                  auto refs = as_refs(a, b, c, d, e, f);

                  auto i = 0;
                  for (auto&& ref : refs) expects(*ref == i++);
              } },
          { "Ref.to_refs.std_vector.all_ptr",
              [] static noexcept {
                  auto a = std::make_unique<int>(0);
                  auto b = std::make_unique<int>(1);
                  auto c = std::make_unique<int>(2);
                  auto d = new int { 3 };
                  auto e = new int { 4 };
                  auto f = 5;

                  auto refs = to_refs<std::vector>(a, b, c, d, e, &f);

                  auto i = 0;
                  for (auto&& ref : refs) expects(*ref == i++);

                  delete d;
                  delete e;
              } },
          { "Ref.as_refs.std_array.all_ptr",
              [] static noexcept {
                  auto a = std::make_unique<int>(0);
                  auto b = std::make_unique<int>(1);
                  auto c = std::make_unique<int>(2);
                  auto d = new int { 3 };
                  auto e = new int { 4 };
                  auto f = 5;

                  auto refs = as_refs<std::array>(a, b, c, d, e, &f);

                  auto i = 0;
                  for (auto&& ref : refs) expects(*ref == i++);
              } },
          { "Ref.as_refs.default.all_ptr",
              [] static noexcept {
                  auto a = std::make_unique<int>(0);
                  auto b = std::make_unique<int>(1);
                  auto c = std::make_unique<int>(2);
                  auto d = new int { 3 };
                  auto e = new int { 4 };
                  auto f = 5;

                  auto refs = as_refs(a, b, c, d, e, &f);

                  auto i = 0;
                  for (auto&& ref : refs) expects(*ref == i++);
              } },
          { "Ref.to_refs.std_set",
              [] static noexcept {
                  auto vec  = std::vector { 1, 3, 5, 6, 9 };
                  auto refs = to_refs<std::set>(vec);

                  auto i = 0;
                  for (auto&& ref : refs) expects(*ref == vec[i++]);
              } },
          { "Ref.to_refs.default",
              [] static noexcept {
                  constexpr auto vec  = std::array { 1, 3, 5, 6, 9 };
                  auto           refs = to_refs(vec);

                  auto i = 0;
                  for (auto&& ref : refs) expects(ref == vec[i++]);
              } },
          }
    };
} // namespace
