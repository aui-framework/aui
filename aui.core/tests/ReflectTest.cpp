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
#include <AUI/Common/AMap.h>
#include <AUI/Reflect/AClass.h>
#include <AUI/Reflect/AEnumerate.h>
#include <AUI/Reflect/AReflect.h>
#include <AUI/Model/AListModel.h>

enum ATest {
    VALUE1,
    VALUE2,
    VALUE3 = 1000,
    VALUE4 = 10000,
};

namespace namespaceeee {
    enum ATest {
        V1,
        V2,
        V3,
    };
    enum class ATest2 {
        TEST2_1,
        TEST2_2,
        TEST2_3,
    };
}

struct MyStruct {};

namespace AzazaATest {
    struct ATest {};
}

enum class EnumWithoutEnumValue {
    SOME_VALUE1
};

template<typename T>
inline std::ostream& operator<<(std::ostream& o, ATest t) {
    o << int(t);
    return o;
}
template<typename T>
inline std::ostream& operator<<(std::ostream& o, namespaceeee::ATest t) {
    o << int(t);
    return o;
}
template<typename T>
inline std::ostream& operator<<(std::ostream& o, namespaceeee::ATest2 t) {
    o << int(t);
    return o;
}
template<typename T>
inline std::ostream& operator<<(std::ostream& o, EnumWithoutEnumValue t) {
    o << int(t);
    return o;
}


AUI_ENUM_VALUES(ATest, VALUE1, VALUE2, VALUE3, VALUE4)
AUI_ENUM_VALUES(namespaceeee::ATest, namespaceeee::V1, namespaceeee::V2, namespaceeee::V3)
AUI_ENUM_VALUES(namespaceeee::ATest2, namespaceeee::ATest2::TEST2_1, namespaceeee::ATest2::TEST2_2, namespaceeee::ATest2::TEST2_3)


TEST(Reflect, NameClass) {
    ASSERT_EQ(AClass<AString>::name(), "AString");
    ASSERT_EQ(AClass<AzazaATest::ATest>::nameWithoutNamespace(), "ATest");
}


TEST(Reflect, NameStruct) {
    ASSERT_EQ(AClass<MyStruct>::name(), "MyStruct");
}

TEST(Reflect, NameEnum) {

    ASSERT_EQ((AClass<ATest>::name()), "ATest");
}

TEST(Reflect, EnumerateNames) {
    AMap<AString, ATest> ref = {
        {"VALUE1", VALUE1},
        {"VALUE2", VALUE2},
        {"VALUE3", VALUE3},
    };

    ASSERT_TRUE((AEnumerate<ATest>::mapValueByName<VALUE1, VALUE2, VALUE3>() == ref));
}

TEST(Reflect, EnumerateAll) {
    AMap<AString, ATest> ref = {
            {"VALUE1", VALUE1},
            {"VALUE2", VALUE2},
            {"VALUE3", VALUE3},
            {"VALUE4", VALUE4},
    };

    auto test = AEnumerate<ATest>::all();
    ASSERT_EQ(test, ref);
}

TEST(Reflect, NamespaceEnumerateNames) {
    AMap<AString, namespaceeee::ATest> ref = {
            {"V1", namespaceeee::V1},
            {"V2", namespaceeee::V2},
            {"V3", namespaceeee::V3},
    };
    auto test = AEnumerate<namespaceeee::ATest>::all();
    ASSERT_EQ(test, ref);
}

TEST(Reflect, NamespaceEnumerateEnumClassNames) {
    AMap<AString, namespaceeee::ATest2> ref = {
            {"TEST2_1", namespaceeee::ATest2::TEST2_1},
            {"TEST2_2", namespaceeee::ATest2::TEST2_2},
            {"TEST2_3", namespaceeee::ATest2::TEST2_3},
    };
    auto test = AEnumerate<namespaceeee::ATest2>::all();
    ASSERT_TRUE((test == ref));
}

TEST(Reflect, EnumWithoutEnumValueCase) {
    ASSERT_EQ(AEnumerate<EnumWithoutEnumValue>::valueName<EnumWithoutEnumValue::SOME_VALUE1>(), "SOME_VALUE1");
}
