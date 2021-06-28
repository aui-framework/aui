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

#include <AUI/Common/AString.h>
#include <AUI/Json/AJson.h>
#include <AUI/Json/AJsonElement.h>


#include <boost/test/unit_test.hpp>
using namespace boost::unit_test;
BOOST_AUTO_TEST_SUITE(Json)



// ORM data class
struct Data2 {
    AVector<int> values;

    bool operator==(const Data2& o) const {
        return values == o.values;
    }

    AJSON_FIELDS(values)
};

std::ostream& operator<<(std::ostream& o, const Data2& d) {
    o << '[';
    for (auto& v : d.values) {
        o << v << ", ";
    }
    o << ']';
    return o;
}

BOOST_AUTO_TEST_CASE(CustomConverter)
{
    // arrange
    Data2 d = { {1, 2, 3} };

    // check for serialization
    auto jsonObject = d.toJson();
    BOOST_CHECK_EQUAL(AJson::toString(jsonObject), R"({"values":[1,2,3]})");

    Data2 d2;
    d2.readJson(jsonObject);
    BOOST_CHECK_EQUAL(d, d2);
}



BOOST_AUTO_TEST_SUITE_END()

