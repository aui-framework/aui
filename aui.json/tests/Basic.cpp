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
#include <AUI/IO/AStringStream.h>


// check const access
void check_girlfriend(const AJson& o) {
    EXPECT_TRUE(o["girlfriend"].isNull());

    // unexisting object
    EXPECT_TRUE(o["unexisting_object"].isEmpty());
}

TEST(Json, ObjectAssignValue)
{
    // [write_object]
    AJson o;
    o["name"] = "Alex2772";
    o["year"] = 2020;
    o["girlfriend"] = nullptr;

    ASSERT_EQ(AJson::toString(o), R"({"name":"Alex2772","year":2020,"girlfriend":null})");
    // [write_object]
    check_girlfriend(o);
}

TEST(Json, ObjectAssignObject)
{
    // [write_nested]
    AJson o;
    o["name"] = "Alex2772";
    o["year"] = 2020;

    AJson root;
    root["user"] = o;

    ASSERT_EQ(AJson::toString(root), R"({"user":{"name":"Alex2772","year":2020}})");
    // [write_nested]
}
TEST(Json, StringEscape)
{
    // arrange data
    AJson root;
    root["user"] = "u\"";
    auto s = AJson::toString(root);

    // check for whole composition
    EXPECT_EQ(s, "{\"user\":\"u\\\"\"}");

    // check for string itself
    auto deserialized = AJson::fromString(s);
    EXPECT_EQ(deserialized["user"].asString(), "u\"");
}

TEST(Json, BraceInitialization)
{
    // [write_brace_init]
    AJson root = {
        {"array", AJson::Array{
            "value1",
            "value2",
        }},
        {"key", 1},
    };

    auto str = AJson::toString(root);
    ASSERT_EQ(str, R"({"array":["value1","value2"],"key":1})");
    // [write_brace_init]
}

TEST(Json, Array)
{
    // [write_array]
    AJson root = AJson::Array{1, 2, 3};
    ASSERT_EQ(AJson::toString(root), R"([1,2,3])");

    root.push_back(10);
    root.push_back(9);
    ASSERT_EQ(AJson::toString(root), R"([1,2,3,10,9])");
    ASSERT_EQ(root[1].asInt(), 2);
    // [write_array]
}

TEST(Json, ParseAccess)
{
    // [parse_access]
    auto json = AJson::fromString(R"({"name":"Alex2772","year":2020,"tags":["a","b"],"girlfriend":null})");

    EXPECT_EQ(json["name"].asString(), "Alex2772");
    EXPECT_EQ(json["year"].asInt(), 2020);
    EXPECT_EQ(json["tags"].asArray().size(), 2);
    EXPECT_EQ(json["tags"][1].asString(), "b");
    EXPECT_TRUE(json["girlfriend"].isNull());
    // [parse_access]
}

TEST(Json, ParseTypeChecks)
{
    // [parse_type_checks]
    auto json = AJson::fromString(R"({"year":2020})");

    // optional accessors never throw
    EXPECT_EQ(json["year"].asIntOpt(), 2020);
    EXPECT_FALSE(json["year"].asStringOpt());

    // regular accessors throw on type mismatch
    EXPECT_THROW(json["year"].asString(), AJsonTypeMismatchException);
    // [parse_type_checks]
}

TEST(Json, ParseContains)
{
    // [parse_contains]
    const auto json = AJson::fromString(R"({"year":2020})");

    EXPECT_TRUE(json.contains("year"));
    EXPECT_FALSE(json.contains("name"));
    if (auto year = json.containsOpt("year")) {
        EXPECT_EQ(year->asInt(), 2020);
    }
    // [parse_contains]
}

TEST(Json, ParseStream)
{
    // [parse_stream]
    auto json = AJson::fromStream(AStringStream(AString(R"({"hello":"world"})")));
    EXPECT_EQ(json["hello"].asString(), "world");
    // [parse_stream]
}

TEST(Json, SerializationDeserialization)
{
    // [round_trip]
    const char* str = R"({"a":null,"b":[1,2,3],"c":false,"d":true,"e":{"v":"123"}})";
    ASSERT_EQ(AJson::toString(AJson::fromString(str)), str);
    // [round_trip]
}


TEST(Json, NegativeNumber)
{
    auto v = AJson::fromString(R"({"code":-2010,"msg":"error"})");
    EXPECT_EQ(v["code"].asNumber(), -2010);
    EXPECT_EQ(v["msg"].asString(), "error");
}

TEST(Json, ExponentNumber1)
{
    auto v = AJson::fromString(R"({"code":-6.94e-7,"msg":"error"})");
    EXPECT_DOUBLE_EQ(v["code"].asNumber(), -6.94e-7);
    EXPECT_EQ(v["msg"].asString(), "error");
}

TEST(Json, ExponentNumber2)
{
    auto v = AJson::fromString(R"({"code":-6.94e7,"msg":"error"})");
    EXPECT_DOUBLE_EQ(v["code"].asNumber(), -6.94e7);
    EXPECT_EQ(v["msg"].asString(), "error");
}

TEST(Json, ExponentNumber3)
{
    auto v = AJson::fromString(R"({"code":6.94e7,"msg":"error"})");
    EXPECT_DOUBLE_EQ(v["code"].asNumber(), 6.94e7);
    EXPECT_EQ(v["msg"].asString(), "error");
}

TEST(Json, ExponentNumber4)
{
    auto v = AJson::fromString(R"({"code":6.94e11,"msg":"error"})");
    EXPECT_DOUBLE_EQ(v["code"].asNumber(), 6.94e11);
    EXPECT_EQ(v["msg"].asString(), "error");
}

TEST(Json, ExponentNumber5)
{
    auto v = AJson::fromString(R"({"code":6.94e-11,"msg":"error"})");
    EXPECT_DOUBLE_EQ(v["code"].asNumber(), 6.94e-11);
    EXPECT_EQ(v["msg"].asString(), "error");
}

TEST(Json, ExponentNumber6)
{
    auto v = AJson::fromString(R"({"code":6.94e-11})");
    EXPECT_DOUBLE_EQ(v["code"].asNumber(), 6.94e-11);
}

TEST(Json, ExponentNumber7)
{
    auto v = AJson::fromString(R"({"code":6e10})");
    EXPECT_DOUBLE_EQ(v["code"].asNumber(), 6e10);
}

TEST(Json, ExponentNumber8)
{
    auto v = AJson::fromString(R"({"code":6E10})");
    EXPECT_DOUBLE_EQ(v["code"].asNumber(), 6e10);
}

TEST(Json, Long)
{
    auto v = AJson::fromString(R"({"value":-1499040000000})");
    EXPECT_EQ(v["value"].asLongInt(), -1499040000000);
}

TEST(Json, Double)
{
    auto v = AJson::fromString(R"({"value":-3.102})");
    EXPECT_NEAR(v["value"].asNumber(), -3.102, 0.0001);
}



TEST(Json, Double2)
{
    auto v = AJson::fromString(R"([0.0645069876039,-0.13864406])");
    EXPECT_NEAR(v.asArray()[0].asNumber(), 0.0645069876039, 0.0001);
    EXPECT_NEAR(v.asArray()[1].asNumber(), -0.13864406, 0.0001);
}

TEST(Json, Utf8Sequence1)
{
    auto v = AJson::fromString(R"(["\u0425\u0443\u0439"])");
    EXPECT_EQ(v.asArray()[0].asString(), "Хуй");
}

TEST(Json, Utf8Sequence2)
{
    auto v = AJson::fromString(R"(["\ud83e\udd21"])");
    EXPECT_EQ(v.asArray()[0].asString(), "🤡");
}

