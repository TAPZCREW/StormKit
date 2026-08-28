// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.core;
import stormkit.test;

#include <stormkit/test/test_macro.hpp>

using namespace stormkit::core;
using namespace std::literals;

namespace {
    auto _ = test::test_suite { "core.Containers", { { "tree.node.name", [] {
                                                          static constexpr auto name = "test_node_name"sv;

                                                          auto node = tree_node {};
                                                          EXPECTS(node.name() == ""s);
                                                          node.set_name(string { name });
                                                          EXPECTS(node.name() == name);
                                                      } } } };
} // namespace
