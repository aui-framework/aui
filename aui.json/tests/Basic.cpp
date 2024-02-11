// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by alex2 on 30.08.2020.
//

#include <gtest/gtest.h>
#include <AUI/Common/AString.h>
#include <AUI/Json/AJson.h>
#include <AUI/Json/AJson.h>


// check const access
void check_girlfriend(const AJson& o) {
    EXPECT_TRUE(o["girlfriend"].isNull());

    // unexisting object
    EXPECT_TRUE(o["unexisting_object"].isEmpty());
}

TEST(Json, ObjectAssignValue)
{
    // arrange data
    AJson o;
    o["name"] = "Alex2772";
    o["year"] = 2020;
    o["girlfriend"] = nullptr;

    // check for resulting json
    ASSERT_EQ(AJson::toString(o), R"({"girlfriend":null,"name":"Alex2772","year":2020})");
    check_girlfriend(o);
}

TEST(Json, ObjectAssignObject)
{
    // arrange data
    AJson o;
    o["name"] = "Alex2772";
    o["year"] = 2020;

    AJson root;
    // assign object to another object. this is what we want check for
    root["user"] = o;

    // check for resulting json
    ASSERT_EQ(AJson::toString(root), R"({"user":{"name":"Alex2772","year":2020}})");
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
    // arrange data
    AJson root = {
        {"array", AJson::Array{
            "value1",
            "value2",
        }},
        {"key", 1},
    };

    // check for resulting json
    auto str = AJson::toString(root);
    ASSERT_EQ(str, R"({"array":["value1","value2"],"key":1})");
}

TEST(Json, Array)
{
    // arrange data
    AJson root = AJson::Array{1, 2, 3};

    // check for resulting json
    ASSERT_EQ(AJson::toString(root), R"([1,2,3])");

    // check push
    root.push_back(10);
    root.push_back(9);
    ASSERT_EQ(AJson::toString(root), R"([1,2,3,10,9])");
    ASSERT_EQ(root[1].asInt(), 2);
}

TEST(Json, SerializationDeserialization)
{
    // arrange data
    const char* str = R"({"a":null,"b":[1,2,3],"c":false,"d":true,"e":{"v":"123"}})";
    ASSERT_EQ(AJson::toString(AJson::fromString(str)), str);
}


TEST(Json, NegativeNumber)
{
    auto v = AJson::fromString(R"({"code":-2010,"msg":"error"})");
    EXPECT_EQ(v["code"].asNumber(), -2010);
    EXPECT_EQ(v["msg"].asString(), "error");
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
    auto v = AJson::fromString(R"([" \u043c\u0438\u440e, "])");
    EXPECT_EQ(v.asArray()[0].asString(), " мире, ");
}
