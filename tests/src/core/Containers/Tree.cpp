// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.Core;

import Test;

using namespace stormkit::core;

#define expects(x) test::expects(x, #x)

namespace {
    auto _ = test::TestSuite { "Core.Containers", { { "Tree.Node.name", [] {
                                                         const auto name = "TestNodeName";

                                                         auto node = TreeNode {};
                                                         expects(node.name() == "");
                                                         node.setName(name);
                                                         expects(node.name() == name);
                                                     } } } };
} // namespace
