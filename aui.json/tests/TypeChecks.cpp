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