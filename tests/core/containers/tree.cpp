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
    auto _ = test::TestSuite { "Core.Containers",
                               { { "Tree.Node.name", [] {
                                      static constexpr auto name = "TestNodeName"sv;

                                      auto node = TreeNode {};
                                      EXPECTS(node.name() == ""s);
                                      node.set_name(std::string { name });
                                      EXPECTS(node.name() == name);
                                  } } } };
} // namespace
