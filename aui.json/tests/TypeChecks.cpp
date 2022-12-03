// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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