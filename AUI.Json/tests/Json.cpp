/**
 * =====================================================================================================================
 * Copyright (c) 2020 Alex2772
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


BOOST_AUTO_TEST_CASE(ObjectAssignValue)
{
    AJsonObject o;
    o["name"] = "Alex2772";
    o["year"] = 2020;

    BOOST_CHECK_EQUAL(AJson::toString(o), "{\"name\":\"Alex2772\",\"year\":2020}");
}

BOOST_AUTO_TEST_CASE(ObjectAssignObject)
{
    AJsonObject o;
    o["name"] = "Alex2772";
    o["year"] = 2020;

    AJsonObject root;
    root["user"] = o;

    BOOST_CHECK_EQUAL(AJson::toString(root), "{\"user\":{\"name\":\"Alex2772\",\"year\":2020}}");
}
BOOST_AUTO_TEST_CASE(StringEscape)
{
    AJsonObject root;
    root["user"] = "u\"";
    auto s = AJson::toString(root);
    BOOST_CHECK_EQUAL(s, "{\"user\":\"u\\\"\"}");
    auto deserialized = AJson::fromString(s);
    BOOST_CHECK_EQUAL(deserialized["user"].asString(), "u\"");
}

BOOST_AUTO_TEST_SUITE_END()

