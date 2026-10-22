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

#include <gtest/gtest.h>
#include <AUI/Json/AJson.h>
#include <AUI/Json/Conversion.h>

// [conv_type]
struct Point {
    int x;
    int y;
};
// [conv_type]

// [conv_impl]
template<>
struct AJsonConv<Point> {
    static AJson toJson(const Point& p) {
        return AJson::Array { p.x, p.y };   // {1, 2} -> [1,2]
    }
    static void fromJson(const AJson& json, Point& dst) {
        const auto& array = json.asArray();
        dst = { array.at(0).asInt(), array.at(1).asInt() };
    }
};
// [conv_impl]

TEST(Json, CustomConverter) {
    // [conv_use]
    EXPECT_EQ(AJson::toString(aui::to_json(Point { 1, 2 })), "[1,2]");

    auto p = aui::from_json<Point>(AJson::fromString("[3,4]"));
    EXPECT_EQ(p.x, 3);
    EXPECT_EQ(p.y, 4);
    // [conv_use]
}

// [conv_nested]
struct Shape {
    AString name;
    AVector<Point> points;
};
AJSON_FIELDS(Shape,
    (name, "name")
    (points, "points")
)
// [conv_nested]

TEST(Json, CustomConverterNested) {
    // [conv_nested_use]
    Shape shape { "triangle", { { 0, 0 }, { 1, 0 }, { 0, 1 } } };
    auto str = AJson::toString(aui::to_json(shape));
    EXPECT_EQ(str, R"({"name":"triangle","points":[[0,0],[1,0],[0,1]]})");
    // [conv_nested_use]

    auto parsed = aui::from_json<Shape>(AJson::fromString(str));
    EXPECT_EQ(parsed.name, "triangle");
    EXPECT_EQ(parsed.points.size(), 3);
}

// [enum_type]
enum class Direction {
    UP,
    DOWN,
};
AUI_ENUM_VALUES(Direction, Direction::UP, Direction::DOWN)
// [enum_type]

TEST(Json, EnumByName) {
    // [enum_use]
    EXPECT_EQ(AJson::toString(aui::to_json(Direction::DOWN)), R"("DOWN")");
    EXPECT_EQ(aui::from_json<Direction>(AJson::fromString(R"("UP")")), Direction::UP);
    // [enum_use]
}
