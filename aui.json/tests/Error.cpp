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

//
// Created by alex2 on 30.08.2020.
//

#include <gtest/gtest.h>
#include <AUI/Common/AString.h>
#include <AUI/Json/AJson.h>
#include <AUI/Json/Conversion.h>
#include <AUI/IO/AStringStream.h>



TEST(Json, Error1)
{
    EXPECT_THROW(AJson::fromString(R"({"kek": truz})"), AJsonParseException);
}
TEST(Json, Error2)
{
    EXPECT_THROW(AJson::fromString(R"({"kek": falz})"), AJsonParseException);
}
TEST(Json, Error3)
{
    EXPECT_THROW(AJson::fromString(R"({"array": [1,2,3,x]})"), AJsonParseException);
}
TEST(Json, Error4)
{
    EXPECT_THROW(AJson::fromString(R"({"array": [1,2,3})"), AJsonParseException);
}
TEST(Json, Error5)
{
    EXPECT_THROW(AJson::fromString(R"({"object": {"1"s:1,)"), AJsonParseException);
}
TEST(Json, Error6)
{
    EXPECT_THROW(AJson::fromString(R"({"object": {"1":1,})"), AJsonParseException);
}
TEST(Json, Error7)
{
    EXPECT_THROW(AJson::fromString(R"({"object": {"1":2})"), AJsonParseException);
}
TEST(Json, Error8)
{
    EXPECT_THROW(AJson::fromString(R"({"object": {"1":2 xs})"), AJsonParseException);
}
TEST(Json, Error9)
{
    EXPECT_THROW(AJson::fromString(R"(<?xml version='1.0' encoding='UTF-8'?>)"), AJsonParseException);
}
// ---- JSON path in error messages ----

struct PathTestMessage {
    AString content;
};
AJSON_FIELDS(PathTestMessage, (content, "content"))

struct PathTestRoot {
    AVector<PathTestMessage> messages;
};
AJSON_FIELDS(PathTestRoot, (messages, "messages"))

TEST(Json, ErrorPathTypeMismatch) {
    // "content" field receives a number instead of a string
    auto json = AJson::fromString(R"({"messages":[{"content":1},{"content":2},{"content":3}]})");
    try {
        aui::from_json<PathTestRoot>(json);
        FAIL() << "expected AJsonTypeMismatchException";
    } catch (const AJsonTypeMismatchException& e) {
        AString msg = e.getMessage();
        EXPECT_TRUE(msg.contains("messages")) << "path missing 'messages': " << msg;
        EXPECT_TRUE(msg.contains("content")) << "path missing 'content': " << msg;
    }
}

TEST(Json, ErrorPathMissingField) {
    // "content" field is absent
    auto json = AJson::fromString(R"({"messages":[{"other":1}]})");
    try {
        aui::from_json<PathTestRoot>(json);
        FAIL() << "expected AJsonException";
    } catch (const AJsonException& e) {
        AString msg = e.getMessage();
        EXPECT_TRUE(msg.contains("content")) << "path missing 'content': " << msg;
    }
}

TEST(Json, ErrorPathContainsIndex) {
    // index [0] should appear in the error path
    auto json = AJson::fromString(R"({"messages":[{"content":42}]})");
    try {
        aui::from_json<PathTestRoot>(json);
        FAIL() << "expected AJsonTypeMismatchException";
    } catch (const AJsonTypeMismatchException& e) {
        AString msg = e.getMessage();
        EXPECT_TRUE(msg.contains("[0]")) << "path missing '[0]': " << msg;
    }
}

TEST(Json, ErrorPathStackCleanedUp) {
    // After a failed conversion the global path stack must be empty
    auto json = AJson::fromString(R"({"messages":[{"content":1}]})");
    try { aui::from_json<PathTestRoot>(json); } catch (...) {}
    EXPECT_TRUE(aui::impl::json::gPathStack.empty()) << "path stack leaked after exception";
}

TEST(Json, Error10)
{
    EXPECT_NO_THROW(AJson::fromString(R"({
  "game": {
    "info": "http://mdownload.kara-online.ru/main/bin/info.json",
    "manifest": "http://mdownload.kara-online.ru/main/bin/manifest.json"
  }
})"));
}

