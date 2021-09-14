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

#include <boost/test/unit_test.hpp>
#include <AUI/Common/AString.h>
#include <AUI/Json/AJsonElement.h>
#include <AUI/Json/AJson.h>
#include <AUI/Json/JsonException.h>

using namespace boost::unit_test;
BOOST_AUTO_TEST_SUITE(Json)


BOOST_AUTO_TEST_CASE(TypeObject)
{
    // arrange data
    AJsonObject root;

    // type checks
    BOOST_CHECK(root.isObject());
    BOOST_CHECK(!root.isInt());
    BOOST_CHECK(!root.isString());
    BOOST_CHECK(!root.isBool());
    BOOST_CHECK(!root.isVariant());
    BOOST_CHECK(!root.isNull());
    BOOST_CHECK(!root.isArray());

    BOOST_CHECK_NO_THROW(root.asObject());
    BOOST_CHECK_THROW(root.asArray(), JsonException);
    BOOST_CHECK_THROW(root.asVariant(), JsonException);
    BOOST_CHECK_THROW(root.asInt(), JsonException);
    BOOST_CHECK_THROW(root.asBool(), JsonException);
}
BOOST_AUTO_TEST_CASE(TypeVariant)
{
    // arrange data
    AJsonValue root("string");


    // isType checks
    BOOST_CHECK(!root.isInt());
    BOOST_CHECK(root.isString());
    BOOST_CHECK(!root.isBool());
    BOOST_CHECK(root.isVariant());
    BOOST_CHECK(!root.isNull());
    BOOST_CHECK(!root.isArray());
    BOOST_CHECK(!root.isObject());

    BOOST_CHECK_THROW(root.asObject(), JsonException);
    BOOST_CHECK_THROW(root.asArray(), JsonException);
}

BOOST_AUTO_TEST_CASE(TypeArray)
{
    // arrange data
    AJsonArray root;


    // isType checks
    BOOST_CHECK(!root.isInt());
    BOOST_CHECK(!root.isString());
    BOOST_CHECK(!root.isBool());
    BOOST_CHECK(!root.isVariant());
    BOOST_CHECK(!root.isNull());
    BOOST_CHECK(root.isArray());
    BOOST_CHECK(!root.isObject());

    BOOST_CHECK_THROW(root.asObject(), JsonException);
    BOOST_CHECK_NO_THROW(root.asArray());
    BOOST_CHECK_THROW(root.asVariant(), JsonException);
}

BOOST_AUTO_TEST_CASE(TypeNull)
{
    // arrange data
    AJsonElement root(nullptr);


    // isType checks
    BOOST_CHECK(!root.isInt());
    BOOST_CHECK(!root.isString());
    BOOST_CHECK(!root.isBool());
    BOOST_CHECK(!root.isVariant());
    BOOST_CHECK(root.isNull());
    BOOST_CHECK(!root.isArray());
    BOOST_CHECK(!root.isObject());

    BOOST_CHECK_THROW(root.asObject(), JsonException);
    BOOST_CHECK_THROW(root.asArray(), JsonException);
    BOOST_CHECK_THROW(root.asVariant(), JsonException);
}

BOOST_AUTO_TEST_SUITE_END()

