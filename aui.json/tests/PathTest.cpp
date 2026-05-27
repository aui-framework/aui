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
#include <AUI/Json/AJson.h>
#include <AUI/Json/Conversion.h>
#include <AUI/Json/Exception.h>

// ---- model ----
struct PathTestMessage {
    AString content;
};
AJSON_FIELDS(PathTestMessage,
    (content, "content")
)

struct PathTestRoot {
    AVector<PathTestMessage> messages;
};
AJSON_FIELDS(PathTestRoot,
    (messages, "messages")
)

// ---- helpers ----
static AString exceptionPath(auto fn) {
    try {
        fn();
    } catch (const AException& e) {
        return AString(e.what());
    }
    return {};
}

// ---- tests ----

TEST(JsonPath, FieldTypeMismatch) {
    // "content" is a number instead of a string
    auto json = AJson::fromString(R"({"messages":[{"content":42}]})");
    auto what = exceptionPath([&]{ aui::from_json<PathTestRoot>(json); });
    EXPECT_TRUE(what.contains("messages")) << "path missing 'messages': " << what;
    EXPECT_TRUE(what.contains("[0]"))       << "path missing '[0]': "      << what;
    EXPECT_TRUE(what.contains("content"))   << "path missing 'content': "  << what;
}

TEST(JsonPath, NestedIndex) {
    // element at index 2 is wrong type
    auto json = AJson::fromString(R"({"messages":[{"content":"ok"},{"content":"ok"},{"content":99}]})");
    auto what = exceptionPath([&]{ aui::from_json<PathTestRoot>(json); });
    EXPECT_TRUE(what.contains("[2]")) << "path missing '[2]': " << what;
    EXPECT_TRUE(what.contains("content")) << "path missing 'content': " << what;
}

TEST(JsonPath, MissingField) {
    // "content" key is absent
    auto json = AJson::fromString(R"({"messages":[{}]})");
    auto what = exceptionPath([&]{ aui::from_json<PathTestRoot>(json); });
    EXPECT_TRUE(what.contains("content")) << "error should mention field name: " << what;
}

TEST(JsonPath, StackClearedAfterSuccess) {
    // After a successful parse the path stack must be empty (no leftover segments)
    auto json = AJson::fromString(R"({"messages":[{"content":"hello"}]})");
    EXPECT_NO_THROW(aui::from_json<PathTestRoot>(json));
    EXPECT_TRUE(aui::impl::json::gPathStack.empty()) << "path stack leaked segments";
}

TEST(JsonPath, ManualIndexing) {
    // Manual chained access via at(): json.at("messages").at(0).at("content")
    auto json = AJson::fromString(R"({"messages":[{"content":"hello"}]})");
    AString content = json.at("messages").at(0).at("content").asString();
    EXPECT_EQ(content, "hello");

    // Wrong type — error message must contain the full path: messages[0].content
    auto json2 = AJson::fromString(R"({"messages":[{"content":42}]})");
    auto what = exceptionPath([&]{ json2.at("messages").at(0).at("content").asString(); });
    EXPECT_FALSE(what.empty())              << "expected an exception";
    EXPECT_TRUE(what.contains("messages"))  << "path missing 'messages': "  << what;
    EXPECT_TRUE(what.contains("[0]"))       << "path missing '[0]': "       << what;
    EXPECT_TRUE(what.contains("content"))   << "path missing 'content': "   << what;
}

TEST(JsonPath, IndependentLookups) {
    // Two sequential lookups must not bleed into each other.
    // First parse: messages[0]["content"] succeeds — stack must be clean afterwards.
    // Second parse: messages[1]["content"] is wrong type — reported path must contain
    // exactly "[1]" and "content" but must NOT contain "[0]" as a leftover from the
    // first lookup.
    auto json1 = AJson::fromString(R"({"messages":[{"content":"ok"}]})");
    EXPECT_NO_THROW(aui::from_json<PathTestRoot>(json1));
    EXPECT_TRUE(aui::impl::json::gPathStack.empty()) << "path stack leaked after first (successful) lookup";

    auto json2 = AJson::fromString(R"({"messages":[{"content":"ok"},{"content":99}]})");
    auto what = exceptionPath([&]{ aui::from_json<PathTestRoot>(json2); });
    EXPECT_TRUE(what.contains("[1]"))     << "path missing '[1]': "   << what;
    EXPECT_TRUE(what.contains("content")) << "path missing 'content': " << what;
    // "[0]" must not leak from the first successful lookup
    EXPECT_FALSE(what.startsWith("[0]"))  << "path contains stale '[0]' from previous lookup: " << what;
    EXPECT_TRUE(aui::impl::json::gPathStack.empty()) << "path stack leaked after second (failed) lookup";
}
