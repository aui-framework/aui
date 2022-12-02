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
