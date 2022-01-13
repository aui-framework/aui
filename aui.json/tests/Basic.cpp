/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 30.08.2020.
//

#include <gtest/gtest.h>
#include <AUI/Common/AString.h>
#include <AUI/Json/AJsonElement.h>
#include <AUI/Json/AJson.h>
#include <AUI/Json/JsonException.h>


// check const access
void check_girlfriend(const AJsonObject& o) {
    EXPECT_TRUE(o["girlfriend"].isNull());

    // unexisting object
    EXPECT_TRUE(o["unexisting_object"].isNull());
}

TEST(JsonBasic, ObjectAssignValue)
{
    // arrange data
    AJsonObject o;
    o["name"] = "Alex2772";
    o["year"] = 2020;
    o["girlfriend"] = AJsonValue(nullptr);

    // check for resulting json
    ASSERT_EQ(AJson::toString(o), R"({"girlfriend":null,"name":"Alex2772","year":2020})");
    check_girlfriend(o);
}

TEST(JsonBasic, ObjectAssignObject)
{
    // arrange data
    AJsonObject o;
    o["name"] = "Alex2772";
    o["year"] = 2020;

    AJsonObject root;
    // assign object to another object. this is what we want check for
    root["user"] = o;

    // check for resulting json
    ASSERT_EQ(AJson::toString(root), R"({"user":{"name":"Alex2772","year":2020}})");
}
TEST(JsonBasic, StringEscape)
{
    // arrange data
    AJsonObject root;
    root["user"] = "u\"";
    auto s = AJson::toString(root);

    // check for whole composition
    ASSERT_EQ(s, "{\"user\":\\\"u\"}");

    // check for string itself
    auto deserialized = AJson::fromString(s);
    ASSERT_EQ(deserialized["user"].asString(), "u\"");
}

TEST(JsonBasic, BraceInitialization)
{
    // arrange data
    AJsonObject root({
            {"array", AJsonArray({
                AJsonValue("value1"),
                AJsonValue("value2"),
            })},
            {"key", AJsonValue(1)},
    });

    // check for resulting json
    ASSERT_EQ(AJson::toString(root), R"({"array":["value1","value2"],"key":1})");
}

TEST(JsonBasic, Array)
{
    // arrange data
    auto root = AJsonArray::fromVariantArray({1, 2, 3});

    // check for resulting json
    EXPECT_TRUE(!root.empty());
    ASSERT_EQ(AJson::toString(root), R"([1,2,3])");

    // check push
    root << AJsonValue(10);
    root.push_back(AJsonValue(9));
    ASSERT_EQ(AJson::toString(root), R"([1,2,3,10,9])");
    ASSERT_EQ(root[1].asInt(), 2);
}

TEST(JsonBasic, SerializationDeserialization)
{
    // arrange data
    const char* str = R"({"a":null,"b":[1,2,3],"c":false,"d":true,"e":{"v":"123"}})";
    ASSERT_EQ(AJson::toString(AJson::fromString(str)), str);
}


