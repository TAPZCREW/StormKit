// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.Core;

import Test;

using namespace stormkit::core;
using namespace std::literals;

#define expects(x) test::expects(x, #x)

namespace {
    auto _ = test::TestSuite { "Core.Containers", { { "Tree.Node.name", [] {
                                                         static constexpr auto name = "TestNodeName"s;

                                                         auto node = TreeNode {};
                                                         expects(node.name() == ""s);
                                                         node.setName(name);
                                                         expects(node.name() == name);
                                                     } } } };
} // namespace
