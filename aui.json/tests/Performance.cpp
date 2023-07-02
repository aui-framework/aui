// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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
#include <AUI/Json/AJson.h>
#include <AUI/Curl/ACurl.h>
#include <AUI/Util/kAUI.h>
#include <chrono>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/IO/AFileOutputStream.h>
#include "AUI/IO/APath.h"

using namespace std::chrono;
using namespace std::chrono_literals;

/*
TEST(Json, Performance) {
    AByteBuffer buffer;
    if (APath("tmp.json").isRegularFileExists()) {
        buffer = AByteBuffer::fromStream(AFileInputStream("tmp.json"));
    } else {
        buffer = ACurl::Builder("https://raw.githubusercontent.com/json-iterator/test-data/master/large-file.json").toByteBuffer();
        AFileOutputStream("tmp.json") << buffer;
    }

    auto start = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
    auto j = AJson::fromBuffer(buffer);
    auto end = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
    auto delta = end - start;
    ASSERT_TRUE((delta < 3s)) << "too slow (" << delta.count() << "ms)";
    ASSERT_TRUE((delta > 5ms)) << "too fast (probably something went wrong)";
}
*/