#include <gtest/gtest.h>

#include "cli/TreeText.h"
#include "network/ViewTree.h"

using namespace aui_remote;

namespace {

AJson node(const char* type, std::initializer_list<const char*> children = {}, int x = 0, const char* text = nullptr) {
    AJson::Array c;
    for (auto ref : children) {
        c.push_back(AJson(AString(ref)));
    }
    AJson result {
        { "type", type }, { "x", x }, { "y", 0 }, { "w", 10 }, { "h", 10 }, { "absX", x }, { "absY", 0 },
        { "visible", true }, { "children", AJson(std::move(c)) },
    };
    if (text) {
        result["text"] = AString(text);
    }
    return result;
}

AJson diff(AJson base, const char* root, AJson upsert, AJson remove = AJson(AJson::Array {})) {
    return AJson { { "base", std::move(base) }, { "root", root }, { "upsert", std::move(upsert) },
                   { "remove", std::move(remove) } };
}

AJson full() {
    return diff(
        nullptr, "w",
        AJson {
            { "w", node("AWindow", { "a", "b" }) },
            { "a", node("ALabel", {}, 0, "hello") },
            { "b", node("AContainer", { "c" }, 20) },
            { "c", node("AButton", {}, 20, "ok") },
        });
}

ViewTree make(const AJson& d, int seq) {
    ViewTree t;
    t.apply(d, seq);
    return t;
}

}   // namespace

TEST(ViewTreeTest, EmptyByDefault) {
    ViewTree t;
    EXPECT_TRUE(t.empty());
    EXPECT_FALSE(t.baseSeq());
    EXPECT_TRUE(t.toJson().isNull());
}

TEST(ViewTreeTest, FullTree) {
    auto t = make(full(), 3);
    EXPECT_FALSE(t.empty());
    EXPECT_EQ(t.baseSeq(), 3);
    EXPECT_EQ(t.size(), 4u);
    auto json = t.toJson();
    EXPECT_EQ(json["ref"].asString(), "w");
    EXPECT_EQ(json["type"].asString(), "AWindow");
    ASSERT_EQ(json["children"].asArray().size(), 2u);
    EXPECT_EQ(json["children"][1]["children"][0]["ref"].asString(), "c");
    EXPECT_EQ(json["children"][1]["children"][0]["text"].asString(), "ok");
}

TEST(ViewTreeTest, RendersAsText) {
    auto t = make(full(), 1);
    EXPECT_EQ(renderTreeText(t.toJson()),
              "AWindow [w] 10x10 @0,0\n"
              "  ALabel \"hello\" [a] 10x10 @0,0\n"
              "  AContainer [b] 10x10 @20,0\n"
              "    AButton \"ok\" [c] 10x10 @20,0\n");
}

TEST(ViewTreeTest, ChangedAddedRemoved) {
    auto t = make(full(), 1);
    // a: text changed; d: added under b; c: removed
    t.apply(diff(1, "w",
                 AJson { { "a", node("ALabel", {}, 0, "bye") }, { "b", node("AContainer", { "d" }, 20) },
                         { "d", node("ACheckBox", {}, 25) } },
                 AJson::Array { AJson("c") }),
            2);
    EXPECT_EQ(t.baseSeq(), 2);
    auto json = t.toJson();
    EXPECT_EQ(json["children"][0]["text"].asString(), "bye");
    ASSERT_EQ(json["children"][1]["children"].asArray().size(), 1u);
    EXPECT_EQ(json["children"][1]["children"][0]["type"].asString(), "ACheckBox");
    EXPECT_EQ(t.size(), 4u);
}

TEST(ViewTreeTest, MovedView) {
    auto t = make(full(), 1);
    // c moves from b to w
    t.apply(diff(1, "w", AJson { { "w", node("AWindow", { "a", "b", "c" }) }, { "b", node("AContainer", {}, 20) } }), 2);
    auto json = t.toJson();
    EXPECT_EQ(json["children"].asArray().size(), 3u);
    EXPECT_EQ(json["children"][2]["ref"].asString(), "c");
    EXPECT_TRUE(json["children"][1]["children"].asArray().empty());
}

TEST(ViewTreeTest, ReorderedChildren) {
    auto t = make(full(), 1);
    t.apply(diff(1, "w", AJson { { "w", node("AWindow", { "b", "a" }) } }), 2);
    EXPECT_EQ(t.toJson()["children"][0]["ref"].asString(), "b");
}

TEST(ViewTreeTest, RootReplaced) {
    auto t = make(full(), 1);
    t.apply(diff(1, "n", AJson { { "n", node("ANew", { "a" }) } }), 2);
    EXPECT_EQ(t.toJson()["ref"].asString(), "n");
    EXPECT_EQ(t.size(), 2u);   // b and c left the tree
}

TEST(ViewTreeTest, DroppedFramesLeaveGapsInSeq) {
    // the client acked 2; the agent dropped 3..6 and diffs frame 7 against 2
    auto t = make(full(), 2);
    t.apply(diff(2, "w", AJson { { "a", node("ALabel", {}, 0, "seven") } }), 7);
    EXPECT_EQ(t.baseSeq(), 7);
    EXPECT_EQ(t.toJson()["children"][0]["text"].asString(), "seven");
    t.apply(diff(7, "w", AJson { { "a", node("ALabel", {}, 0, "nine") } }), 9);
    EXPECT_EQ(t.baseSeq(), 9);
}

TEST(ViewTreeTest, DesyncIsDetectedAndLeavesTheTreeUntouched) {
    auto t = make(full(), 2);
    EXPECT_THROW(t.apply(diff(5, "w", AJson { { "a", node("ALabel", {}, 0, "x") } }), 6), AException);
    EXPECT_EQ(t.baseSeq(), 2);
    EXPECT_EQ(t.toJson()["children"][0]["text"].asString(), "hello");

    ViewTree empty;
    EXPECT_THROW(empty.apply(diff(1, "w", AJson::Object {}), 2), AException);   // diff against nothing
    EXPECT_TRUE(empty.empty());
}

TEST(ViewTreeTest, FullFrameResyncs) {
    auto t = make(full(), 2);
    t.apply(diff(nullptr, "x", AJson { { "x", node("AOther") } }), 8);
    EXPECT_EQ(t.size(), 1u);
    EXPECT_EQ(t.baseSeq(), 8);
}

TEST(ViewTreeTest, InvalidDiffsThrow) {
    auto t = make(full(), 1);
    EXPECT_THROW(t.apply(AJson { { "base", 1 } }, 2), AException);                          // no root
    EXPECT_THROW(t.apply(diff(1, "zzz", AJson::Object {}), 2), AException);                 // unknown root
    EXPECT_THROW(t.apply(diff(1, "w", AJson::Object {}, AJson::Array { AJson("a") }), 2),
                 AException);                                                                // w refers to removed a
    EXPECT_THROW(t.apply(diff(1, "w", AJson { { "w", node("AWindow", { "w" }) } }), 2), AException);   // cycle
    EXPECT_THROW(t.apply(diff(1, "w", AJson { { "a", 5 } }), 2), AException);               // node is not an object
    EXPECT_THROW(t.apply(AJson(5), 2), AException);
    EXPECT_EQ(t.baseSeq(), 1);
    EXPECT_EQ(t.size(), 4u);
}

TEST(ViewTreeTest, UnknownFieldsAreKept) {
    auto d = full();
    d["upsert"]["a"]["role"] = "button";
    auto t = make(d, 1);
    EXPECT_EQ(t.toJson()["children"][0]["role"].asString(), "button");
}

TEST(ViewTreeTest, DiffRoundTrip) {
    auto a = make(full(), 1);
    auto bJson = diff(1, "w",
                      AJson { { "w", node("AWindow", { "a", "b", "e" }) }, { "a", node("ALabel", {}, 0, "changed") },
                              { "b", node("AContainer", {}, 20) }, { "e", node("AEdit", {}, 3) } },
                      AJson::Array { AJson("c") });
    ViewTree b = a;
    b.apply(bJson, 5);
    ASSERT_FALSE(a.sameContent(b));

    auto d = ViewTree::diff(a, b);
    EXPECT_EQ(d["base"].asInt(), 1);
    EXPECT_EQ(d["remove"].asArray().size(), 1u);   // c
    EXPECT_EQ(d["upsert"].asObject().size(), 4u);  // w, a, b, e; but not unchanged views
    ViewTree restored = a;
    restored.apply(d, 5);
    EXPECT_TRUE(restored.sameContent(b));
    EXPECT_EQ(AJson::toString(restored.toJson()), AJson::toString(b.toJson()));

    // nothing changed: nothing to send
    auto noop = ViewTree::diff(b, b);
    EXPECT_TRUE(noop["upsert"].asObject().empty());
    EXPECT_TRUE(noop["remove"].asArray().empty());

    // against an empty tree: the full tree
    auto fromScratch = ViewTree::diff(ViewTree(), b);
    EXPECT_TRUE(fromScratch["base"].isNull());
    ViewTree c;
    c.apply(fromScratch, 5);
    EXPECT_TRUE(c.sameContent(b));
}

namespace {

AJson box(const char* type, int x, int y, int w, int h, std::initializer_list<const char*> children = {}) {
    auto result = node(type, children, x);
    result["y"] = y;
    result["absY"] = y;
    result["w"] = w;
    result["h"] = h;
    return result;
}

// window 100x100: `left` covers the left half, `top` overlaps it, `hidden` is invisible
ViewTree hitTree() {
    auto hidden = box("AView", 0, 0, 100, 100);
    hidden["visible"] = false;
    return make(
        diff(
            nullptr, "w",
            AJson {
              { "w", box("AWindow", 0, 0, 100, 100, { "left", "top", "hidden" }) },
              { "left", box("AViewContainer", 0, 0, 50, 100, { "button" }) },
              { "button", box("AButton", 10, 10, 20, 20) },
              { "top", box("ALabel", 40, 0, 20, 20) },
              { "hidden", hidden },
            }),
        1);
}

}   // namespace

TEST(ViewTreeTest, Accessors) {
    auto t = make(full(), 1);
    ASSERT_TRUE(t.fields("a"));
    EXPECT_EQ((*t.fields("a"))["text"].asString(), "hello");
    EXPECT_FALSE(t.fields("nope"));
    EXPECT_EQ(t.children("w"), (AVector<AString> { "a", "b" }));
    EXPECT_TRUE(t.children("nope").empty());
}

TEST(ViewTreeTest, PathTo) {
    auto t = make(full(), 1);
    EXPECT_EQ(t.pathTo("c"), (AVector<AString> { "w", "b", "c" }));
    EXPECT_EQ(t.pathTo("w"), (AVector<AString> { "w" }));
    EXPECT_TRUE(t.pathTo("nope").empty());
}

TEST(ViewTreeTest, HitTestPicksDeepestTopmostVisible) {
    auto t = hitTree();
    EXPECT_EQ(t.hitTest({ 15, 15 }), (AVector<AString> { "w", "left", "button" }));
    EXPECT_EQ(t.hitTest({ 5, 50 }), (AVector<AString> { "w", "left" }));
    // `top` is after `left`, so it's above; `hidden` is above both but invisible
    EXPECT_EQ(t.hitTest({ 45, 5 }), (AVector<AString> { "w", "top" }));
    EXPECT_EQ(t.hitTest({ 80, 80 }), (AVector<AString> { "w" }));
    EXPECT_TRUE(t.hitTest({ 150, 10 }).empty());
    EXPECT_TRUE(ViewTree().hitTest({ 0, 0 }).empty());
}

TEST(ViewTreeTest, BoxModel) {
    auto withBox = box("AViewContainer", 10, 20, 30, 40);
    withBox["padding"] = AJson(AJson::Array { AJson(1), AJson(2), AJson(3), AJson(4) });
    withBox["margin"] = AJson(AJson::Array { AJson(5), AJson(6), AJson(7), AJson(8) });
    auto t = make(diff(nullptr, "w", AJson { { "w", box("AWindow", 0, 0, 100, 100, { "v" }) }, { "v", withBox } }), 1);
    auto b = t.box("v");
    ASSERT_TRUE(b);
    EXPECT_EQ(b->position, glm::ivec2(10, 20));
    EXPECT_EQ(b->size, glm::ivec2(30, 40));
    EXPECT_EQ(b->padding, (std::array { 1, 2, 3, 4 }));
    EXPECT_EQ(b->margin, (std::array { 5, 6, 7, 8 }));
    // an agent that doesn't send the box model
    EXPECT_EQ(t.box("w")->padding, (std::array { 0, 0, 0, 0 }));
    EXPECT_FALSE(t.box("nope"));
}

TEST(ViewTreeTest, PrettyName) {
    EXPECT_EQ(ViewTree::prettyName(AJson { { "type", "AButton" } }), "AButton");
    EXPECT_EQ(ViewTree::prettyName(AJson { { "type", "AViewContainer" }, { "layout", "AHorizontalLayout" } }),
              "AHorizontalLayout (AViewContainer)");
    EXPECT_EQ(ViewTree::prettyName(AJson { { "type", "class AButton" }, { "name", "first .default" } }),
              "AButton .default");
    EXPECT_EQ(ViewTree::prettyName(AJson { { "type", "AView" }, { "name", ".a declarative::detail::x" } }),
              "AView .a");
    EXPECT_EQ(ViewTree::prettyName(AJson { { "type", "LayoutVisitor::Stacked" } }), "AStacked");
}
