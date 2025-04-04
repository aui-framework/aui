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

#include <gtest/gtest.h>
#include <AUI/Common/AString.h>
#include <AUI/Common/AMap.h>
#include <AUI/Reflect/AClass.h>
#include <AUI/Reflect/AEnumerate.h>
#include <AUI/Reflect/AReflect.h>
#include <AUI/Model/AListModel.h>
#include "AUI/Reflect/for_each_field.h"

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
}   // namespace namespaceeee

struct MyStruct {};

namespace AzazaATest {
struct ATest {};
}   // namespace AzazaATest

enum class EnumWithoutEnumValue { SOME_VALUE1 };

template <typename T>
inline std::ostream &operator<<(std::ostream &o, ATest t) {
    o << int(t);
    return o;
}

template <typename T>
inline std::ostream &operator<<(std::ostream &o, namespaceeee::ATest t) {
    o << int(t);
    return o;
}

template <typename T>
inline std::ostream &operator<<(std::ostream &o, namespaceeee::ATest2 t) {
    o << int(t);
    return o;
}

template <typename T>
inline std::ostream &operator<<(std::ostream &o, EnumWithoutEnumValue t) {
    o << int(t);
    return o;
}

AUI_ENUM_VALUES(ATest, VALUE1, VALUE2, VALUE3, VALUE4)

AUI_ENUM_VALUES(namespaceeee::ATest, namespaceeee::V1, namespaceeee::V2, namespaceeee::V3)

AUI_ENUM_VALUES(
    namespaceeee::ATest2, namespaceeee::ATest2::TEST2_1, namespaceeee::ATest2::TEST2_2, namespaceeee::ATest2::TEST2_3)

TEST(Reflect, NameClass) {
    ASSERT_EQ(AClass<AString>::name(), "AString");
    ASSERT_EQ(AClass<AzazaATest::ATest>::nameWithoutNamespace(), "ATest");
}

TEST(Reflect, NameStruct) { ASSERT_EQ(AClass<MyStruct>::name(), "MyStruct"); }

TEST(Reflect, NameEnum) { ASSERT_EQ((AClass<ATest>::name()), "ATest"); }

TEST(Reflect, EnumerateAll) {
    AMap<AString, ATest> ref = {
        { "VALUE1", VALUE1 },
        { "VALUE2", VALUE2 },
        { "VALUE3", VALUE3 },
        { "VALUE4", VALUE4 },
    };

    auto test = AEnumerate<ATest>::nameToValueMap();
    ASSERT_EQ(test, ref);
}

TEST(Reflect, NamespaceEnumerateNames) {
    AMap<AString, namespaceeee::ATest> ref = {
        { "V1", namespaceeee::V1 },
        { "V2", namespaceeee::V2 },
        { "V3", namespaceeee::V3 },
    };
    auto test = AEnumerate<namespaceeee::ATest>::nameToValueMap();
    ASSERT_EQ(test, ref);
}

TEST(Reflect, NamespaceEnumerateEnumClassNames) {
    AMap<AString, namespaceeee::ATest2> ref = {
        { "TEST2_1", namespaceeee::ATest2::TEST2_1 },
        { "TEST2_2", namespaceeee::ATest2::TEST2_2 },
        { "TEST2_3", namespaceeee::ATest2::TEST2_3 },
    };
    auto test = AEnumerate<namespaceeee::ATest2>::nameToValueMap();
    ASSERT_TRUE((test == ref));
}

TEST(Reflect, EnumWithoutEnumValueCase) {
    ASSERT_EQ(AEnumerate<EnumWithoutEnumValue>::valueName<EnumWithoutEnumValue::SOME_VALUE1>(), "SOME_VALUE1");
}

/// [member_v]
struct SomeStruct {
    int someInt;
    std::string someString;

    long someFunc(float arg) { return 0; }
};
/// [member_v]

TEST(Reflect, FieldCount) {
    struct Data {
        int a;
        std::string b;
    };
    EXPECT_EQ(aui::reflect::detail::fields_count<Data>(), 2);
}

TEST(Reflect, ForEachField) {
    /// [for_each_field_value]
    struct Data {
        int a;
        std::string b;
    };
    AString result;
    aui::reflect::for_each_field_value(Data{ .a = 123, .b = "abc" }, [&](const auto& v) {
        result += "{};"_format(v);
    });
    EXPECT_EQ(result, "123;abc;");
    /// [for_each_field_value]
}

TEST(Reflect, ForEachFieldRefs) {
    struct Data {
        int a;
        std::string b;
    };
    Data data { .a = 123, .b = "abc" };
    aui::reflect::for_each_field_value(data, [&]<typename T>(T& v) {
        v = T{};
    });
    EXPECT_EQ(data.a, 0);
    EXPECT_EQ(data.b, "");
}
