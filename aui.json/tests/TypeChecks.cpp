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
/*

#include <gtest/gtest.h>
#include <AUI/Common/AString.h>
#include <AUI/Json/AJson.h>
#include <AUI/Json/AJson.h>
#include <AUI/Json/JsonException.h>


TEST(JsonTypeChecks, TypeObject)
{
    // arrange data
    AJsonObject root;

    // type checks
    EXPECT_TRUE(root.isObject());
    EXPECT_TRUE(!root.isInt());
    EXPECT_TRUE(!root.isString());
    EXPECT_TRUE(!root.isBool());
    EXPECT_TRUE(!root.isVariant());
    EXPECT_TRUE(!root.isNull());
    EXPECT_TRUE(!root.isArray());

    ASSERT_NO_THROW(root.asObject());
    ASSERT_THROW(root.asArray(), JsonException);
    ASSERT_THROW(root.asVariant(), JsonException);
    ASSERT_THROW(root.asInt(), JsonException);
    ASSERT_THROW(root.asBool(), JsonException);
}
TEST(JsonTypeChecks, TypeVariant)
{
    // arrange data
    AJsonValue root("string");


    // isType checks
    EXPECT_TRUE(!root.isInt());
    EXPECT_TRUE(root.isString());
    EXPECT_TRUE(!root.isBool());
    EXPECT_TRUE(root.isVariant());
    EXPECT_TRUE(!root.isNull());
    EXPECT_TRUE(!root.isArray());
    EXPECT_TRUE(!root.isObject());

    ASSERT_THROW(root.asObject(), JsonException);
    ASSERT_THROW(root.asArray(), JsonException);
}

TEST(JsonTypeChecks, TypeArray)
{
    // arrange data
    AJsonArray root;


    // isType checks
    EXPECT_TRUE(!root.isInt());
    EXPECT_TRUE(!root.isString());
    EXPECT_TRUE(!root.isBool());
    EXPECT_TRUE(!root.isVariant());
    EXPECT_TRUE(!root.isNull());
    EXPECT_TRUE(root.isArray());
    EXPECT_TRUE(!root.isObject());

    ASSERT_THROW(root.asObject(), JsonException);
    ASSERT_NO_THROW(root.asArray());
    ASSERT_THROW(root.asVariant(), JsonException);
}

TEST(JsonTypeChecks, TypeNull)
{
    // arrange data
    AJson root(nullptr);


    // isType checks
    EXPECT_TRUE(!root.isInt());
    EXPECT_TRUE(!root.isString());
    EXPECT_TRUE(!root.isBool());
    EXPECT_TRUE(!root.isVariant());
    EXPECT_TRUE(root.isNull());
    EXPECT_TRUE(!root.isArray());
    EXPECT_TRUE(!root.isObject());

    ASSERT_THROW(root.asObject(), JsonException);
    ASSERT_THROW(root.asArray(), JsonException);
    ASSERT_THROW(root.asVariant(), JsonException);
}
*/