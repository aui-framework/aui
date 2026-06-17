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

#include "AUI/Common/AByteBuffer.h"
#include "AUI/IO/AFileInputStream.h"
#include "AUI/IO/AFileOutputStream.h"

#include <gtest/gtest.h>
#include <AUI/Common/AString.h>
#include <AUI/Json/AJson.h>
#include <AUI/Json/AJson.h>

// this suite of cases is designed to test AJson's ability to reproduce exactly the same JSON.

// uncomment this to eat a large json file for further analysis with diff tool.
// TEST(Json, ReproduceFile)
// {
//     AString original = AString::fromUtf8(AByteBuffer::fromStream(AFileInputStream("original.json")));
//     auto processed = AJson::toString(AJson::fromString(original));
//     AFileOutputStream("processed.json") << processed; // for debugging
//     EXPECT_EQ(original, processed);
// }


static void reproduceCase(const AString& string) {
    auto asJson = AJson::fromString(string);
    EXPECT_EQ(AJson::toString(asJson), string);
}

TEST(Json, ReproduceEscaping1) {
    reproduceCase(R"("{\"query\":\"chat\\\\(|chatStreaming\\\\(|ALOG_DEBUG.*Response\",\"isRegexp\":true,\"includePattern\":\"src/**\"}")");
}

TEST(Json, ReproduceEscaping2) {
    reproduceCase(R"("\\\\")");
}

TEST(Json, ReproduceEscaping3) {
    reproduceCase(R"("\"")");
}
