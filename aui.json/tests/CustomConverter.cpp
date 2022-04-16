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
#include <AUI/Json/AJson.h>
#include <AUI/Json/AJson.h>
#include <AUI/Traits/parameter_pack.h>
#include <AUI/Traits/members.h>

// ORM data class
struct Data2 {
    AVector<int> values;
    int i;

    bool operator==(const Data2& o) const {
        return std::tie(values, i) == std::tie(o.values, o.i);
    }
};

AJSON_FIELDS(Data2,
        (values, "values")
        (i, "i")
)

std::ostream& operator<<(std::ostream& o, const Data2& d) {
    o << '[';
    for (auto& v : d.values) {
        o << v << ", ";
    }
    o << "]," << d.i;
    return o;
}

TEST(JsonCustomConverter, CustomConverter)
{
    // arrange
    Data2 d = { {1, 2, 3}, 228 };

    // check for serialization
    auto jsonObject = aui::to_json(d);
    ASSERT_EQ(AJson::toString(jsonObject), R"({"i":228,"values":[1,2,3]})");

    auto d2 = aui::from_json<Data2>(jsonObject);
    ASSERT_EQ(d, d2);
}
