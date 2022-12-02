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
#include <AUI/Json/AJson.h>
#include <AUI/IO/AStringStream.h>



TEST(Json, Error1)
{
    EXPECT_THROW(AJson::fromString(R"({"kek": truz})"), AJsonParseException);
}
TEST(Json, Error2)
{
    EXPECT_THROW(AJson::fromString(R"({"kek": falz})"), AJsonParseException);
}
TEST(Json, Error3)
{
    EXPECT_THROW(AJson::fromString(R"({"array": [1,2,3,x]})"), AJsonParseException);
}
TEST(Json, Error4)
{
    EXPECT_THROW(AJson::fromString(R"({"array": [1,2,3})"), AJsonParseException);
}
TEST(Json, Error5)
{
    EXPECT_THROW(AJson::fromString(R"({"object": {"1"s:1,)"), AJsonParseException);
}
TEST(Json, Error6)
{
    EXPECT_THROW(AJson::fromString(R"({"object": {"1":1,})"), AJsonParseException);
}
TEST(Json, Error7)
{
    EXPECT_THROW(AJson::fromString(R"({"object": {"1":2})"), AJsonParseException);
}
TEST(Json, Error8)
{
    EXPECT_THROW(AJson::fromString(R"({"object": {"1":2 xs})"), AJsonParseException);
}
TEST(Json, Error9)
{
    EXPECT_THROW(AJson::fromString(R"(<?xml version='1.0' encoding='UTF-8'?>)"), AJsonParseException);
}
TEST(Json, Error10)
{
    EXPECT_NO_THROW(AJson::fromString(R"({
  "game": {
    "info": "http://mdownload.kara-online.ru/main/bin/info.json",
    "manifest": "http://mdownload.kara-online.ru/main/bin/manifest.json"
  }
})"));
}

