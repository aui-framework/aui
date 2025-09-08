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
#include <AUI/Json/AJson.h>
#include <AUI/Json/AJson.h>
#include <AUI/Traits/parameter_pack.h>
#include <AUI/Traits/members.h>
#include "AUI/Common/AProperty.h"

#include <AUI/Model/AListModel.h>

// ORM data class
struct Data2 {
    AVector<int> values;
    int i;

    bool operator==(const Data2& o) const {
        return values == o.values && i == o.i;
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

TEST(Json, FieldsTestBasic)
{
    // arrange
    Data2 d = { {1, 2, 3}, 228 };

    // check for serialization
    auto jsonObject = aui::to_json(d);
    ASSERT_EQ(AJson::toString(jsonObject), R"({"values":[1,2,3],"i":228})");

    auto d2 = aui::from_json<Data2>(jsonObject);
    ASSERT_EQ(d, d2);

    EXPECT_THROW(aui::from_json<Data2>(AJson::fromString(R"({"i":228})") /* no "values" field present */), AJsonException);
}

// [clang_format_toggle]
struct DataOptional {
    int v1;
    int v2;
};

// clang-format off
AJSON_FIELDS(DataOptional,
             (v1, "v1")
             (v2, "v2", AJsonFieldFlags::OPTIONAL))
// clang-format on
// [clang_format_toggle]

TEST(Json, FieldsTestOptional)
    {
    // should not throw an exception since v2 is optional
    DataOptional dst { 1, 2 };
    EXPECT_NO_THROW(aui::from_json<DataOptional>(AJson::fromString(R"({"v1":228})"), dst));
    EXPECT_EQ(dst.v1, 228); // should have modified presented value
    EXPECT_EQ(dst.v2, 2); // but should have kept previous value

    // it should not ignore the value when it presents
    auto d2 = aui::from_json<DataOptional>(AJson::fromString(R"({"v1":228, "v2":229})"));
    EXPECT_EQ(d2.v1, 228);
    EXPECT_EQ(d2.v2, 229);

    // here it should throw an exception since v1 is not optional
    EXPECT_THROW(aui::from_json<DataOptional>(AJson::fromString(R"({"v2":228})")), AJsonException);
}

struct DataProperty {
    AProperty<int> v1;
    AProperty<AString> v2;
};

AJSON_FIELDS(DataProperty,
             (v1, "v1")
             (v2, "v2"))

TEST(Json, DataProperty)
{
    DataProperty property {
        .v1 = 111,
        .v2 = "string",
    };

    auto str = AJson::toString(aui::to_json(property));
    EXPECT_EQ(str, R"({"v1":111,"v2":"string"})");

    // and back
    auto parsed = aui::from_json<DataProperty>(AJson::fromString(str));
    EXPECT_EQ(parsed.v1, property.v1);
    EXPECT_EQ(parsed.v2, property.v2);
}

TEST(Json, ListModel) {
    auto list = AListModel<AString>::fromVector(AVector<AString>{"1", "2", "3"});
    auto str = AJson::toString(aui::to_json(list));
    EXPECT_EQ(str, R"(["1","2","3"])");

    // and back
    auto parsed = aui::from_json<_<AListModel<AString>>>(AJson::fromString(str));
    EXPECT_EQ(parsed->size(), list->size());
    EXPECT_EQ(*parsed, *list);
}
