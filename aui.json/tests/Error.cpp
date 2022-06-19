/*
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

