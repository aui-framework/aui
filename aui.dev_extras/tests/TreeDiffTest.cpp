/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include "AUI/Remote/Agent/TreeDiff.h"

using namespace aui::remote::tree_diff;

namespace {

Node node(AString type, AVector<AString> children = {}, int x = 0) {
    Node n;
    n.type = std::move(type);
    n.children = std::move(children);
    n.x = x;
    n.w = 10;
    n.h = 10;
    return n;
}

/// root -> a, b; a -> a1
Tree sample() {
    Tree tree;
    tree.root = "r";
    tree.nodes["r"] = node("AWindow", { "a", "b" });
    tree.nodes["a"] = node("AViewContainer", { "a1" });
    tree.nodes["a1"] = node("ALabel");
    tree.nodes["a1"].text = "hello";
    tree.nodes["b"] = node("AButton");
    return tree;
}

void expectRoundTrip(const Tree& a, const Tree& b) {
    const auto d = diff(a, 1, b);
    EXPECT_EQ(apply(d, a), b);
    // over the wire
    const auto wire = diffFromJson(AJson::fromString(AJson::toString(toJson(d))));
    EXPECT_EQ(apply(wire, a), b);
}
}   // namespace

TEST(TreeDiff, FullDiffCarriesEverything) {
    const auto tree = sample();
    const auto d = diff(tree);
    EXPECT_FALSE(d.base);
    EXPECT_EQ(d.upsert.size(), tree.nodes.size());
    EXPECT_TRUE(d.remove.empty());
    EXPECT_EQ(apply(d, Tree {}), tree);
    // a stale base must not leak into the result
    Tree other;
    other.nodes["zzz"] = node("AView");
    EXPECT_EQ(apply(d, other), tree);
}

TEST(TreeDiff, NothingChanged) {
    const auto tree = sample();
    const auto d = diff(tree, 5, tree);
    ASSERT_TRUE(d.base);
    EXPECT_EQ(*d.base, 5);
    EXPECT_TRUE(d.upsert.empty());
    EXPECT_TRUE(d.remove.empty());
    expectRoundTrip(tree, tree);
}

TEST(TreeDiff, ChangedNodeOnly) {
    auto a = sample();
    auto b = a;
    b.nodes["a1"].text = "bye";
    const auto d = diff(a, 1, b);
    ASSERT_EQ(d.upsert.size(), 1u);
    EXPECT_TRUE(d.upsert.contains("a1"));
    expectRoundTrip(a, b);
}

TEST(TreeDiff, GeometryAndVisibilityAreProperties) {
    auto a = sample();
    auto b = a;
    b.nodes["b"].absX = 7;
    b.nodes["a"].visible = false;
    b.nodes["a"].name = "container";
    EXPECT_EQ(diff(a, 1, b).upsert.size(), 2u);
    expectRoundTrip(a, b);
}

TEST(TreeDiff, AddedView) {
    auto a = sample();
    auto b = a;
    b.nodes["c"] = node("ALabel");
    b.nodes["b"].children = { "c" };
    const auto d = diff(a, 1, b);
    EXPECT_TRUE(d.upsert.contains("c"));
    EXPECT_TRUE(d.upsert.contains("b"));   // its children list differs
    EXPECT_FALSE(d.upsert.contains("a"));
    expectRoundTrip(a, b);
}

TEST(TreeDiff, RemovedViewsWithSubtree) {
    auto a = sample();
    auto b = a;
    b.nodes.erase("a");
    b.nodes.erase("a1");
    b.nodes["r"].children = { "b" };
    const auto d = diff(a, 1, b);
    EXPECT_EQ(d.remove.size(), 2u);
    EXPECT_TRUE(d.remove.contains("a"));
    EXPECT_TRUE(d.remove.contains("a1"));
    expectRoundTrip(a, b);
}

TEST(TreeDiff, MovedView) {
    auto a = sample();
    auto b = a;
    // a1 moves from a to b
    b.nodes["a"].children = {};
    b.nodes["b"].children = { "a1" };
    const auto d = diff(a, 1, b);
    EXPECT_EQ(d.upsert.size(), 2u);   // only the parents: a1 itself is the same
    EXPECT_FALSE(d.upsert.contains("a1"));
    expectRoundTrip(a, b);
}

TEST(TreeDiff, ReorderedChildren) {
    auto a = sample();
    auto b = a;
    b.nodes["r"].children = { "b", "a" };
    EXPECT_EQ(diff(a, 1, b).upsert.size(), 1u);
    expectRoundTrip(a, b);
}

TEST(TreeDiff, RootChanged) {
    auto a = sample();
    Tree b;
    b.root = "r2";
    b.nodes["r2"] = node("AWindow");
    expectRoundTrip(a, b);
}

TEST(TreeDiff, DroppedIntermediateFrames) {
    // the client acknowledged `acked` only; frames 2 and 3 were dropped. Frame 4 is diffed against `acked`.
    const auto acked = sample();

    auto frame2 = acked;
    frame2.nodes["c"] = node("ALabel");
    frame2.nodes["b"].children = { "c" };

    auto frame3 = frame2;
    frame3.nodes.erase("a1");
    frame3.nodes["a"].children = {};
    frame3.nodes["c"].text = "three";

    auto frame4 = frame3;
    frame4.nodes["b"].children = {};
    frame4.nodes.erase("c");   // added in 2, gone in 4: the client never knew it
    frame4.nodes["a"].x = 42;

    const auto d = diff(acked, 1, frame4);
    ASSERT_TRUE(d.base);
    EXPECT_EQ(*d.base, 1);
    EXPECT_FALSE(d.upsert.contains("c"));
    EXPECT_TRUE(d.remove.contains("a1"));
    EXPECT_EQ(apply(d, acked), frame4);

    // the dropped frames, had they arrived, give the same result
    EXPECT_EQ(apply(diff(frame3, 3, frame4), apply(diff(acked, 1, frame3), acked)), frame4);
}

TEST(TreeDiff, JsonShape) {
    auto a = sample();
    auto b = a;
    b.nodes["a1"].text = "x";
    b.nodes.erase("b");
    b.nodes["r"].children = { "a" };
    const auto json = toJson(diff(a, 3, b));
    EXPECT_EQ(json["base"].asLongInt(), 3);
    EXPECT_EQ(json["root"].asString(), "r");
    EXPECT_EQ(json["remove"].asArray().size(), 1u);
    const auto& a1 = json["upsert"]["a1"];
    EXPECT_EQ(a1["type"].asString(), "ALabel");
    EXPECT_EQ(a1["text"].asString(), "x");
    EXPECT_FALSE(a1.contains("name"));
    EXPECT_TRUE(a1["visible"].asBool());
    EXPECT_TRUE(a1["children"].asArray().empty());
    EXPECT_TRUE(toJson(diff(a))["base"].isNull());
    EXPECT_FALSE(a1.contains("layout"));
    EXPECT_FALSE(a1.contains("padding"));
    EXPECT_FALSE(a1.contains("margin"));
}

TEST(TreeDiff, BoxModelRoundTrip) {
    auto a = sample();
    a.nodes["a"].layout = "AHorizontalLayout";
    a.nodes["a"].padding = { 1, 2, 3, 4 };
    a.nodes["a"].margin = { 5, 6, 7, 8 };
    const auto json = toJson(diff(a));
    const auto& node = json["upsert"]["a"];
    EXPECT_EQ(node["layout"].asString(), "AHorizontalLayout");
    EXPECT_EQ(node["padding"].asArray().size(), 4u);
    EXPECT_EQ(node["margin"][3].asInt(), 8);
    EXPECT_EQ(apply(diffFromJson(json), {}), a);
}
