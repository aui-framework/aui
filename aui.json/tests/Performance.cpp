/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
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