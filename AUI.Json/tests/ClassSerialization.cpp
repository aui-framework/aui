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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 30.08.2020.
//

#include <boost/test/unit_test.hpp>
#include <AUI/Common/AString.h>
#include <AUI/Json/AJsonElement.h>
#include <AUI/Json/AJson.h>

using namespace boost::unit_test;
BOOST_AUTO_TEST_SUITE(Json)

// ORM data class
struct Data {
    AString name;
    int year;

    AJSON_FIELDS(name, year)
};

BOOST_AUTO_TEST_CASE(ClassSerialization)
{
    // arrange
    Data d = {"Alex2772", 2020};

    // check for serialization
    BOOST_CHECK_EQUAL(AJson::toString(d.toJson()), R"({"name":"Alex2772","year":2020})");
}

BOOST_AUTO_TEST_CASE(ClassDeserialization)
{
    // arrange
    auto jsonString = R"({"name":"Azaza","year":2021})";

    // deserialize
    auto jsonParsed = AJson::fromString(jsonString);

    // when serialized back, the json should not change
    BOOST_CHECK_EQUAL(AJson::toString(jsonParsed), jsonString);

    // read json
    Data d;
    d.readJson(jsonParsed);

    // assert
    BOOST_CHECK_EQUAL(d.name, "Azaza");
    BOOST_CHECK_EQUAL(d.year, 2021);
}


BOOST_AUTO_TEST_SUITE_END()

