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
#include <AUI/Common/SharedPtr.h>
#include <AUI/Thread/AFuture.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Util/Util.h>
#include <random>
#include <ctime>
#include <AUI/Common/AUuid.h>
#include <AUI/Util/ARandom.h>

TEST(Uuid, Canonical) {
    AString s = "123e4567-e89b-12d3-a456-426655440000";
    AUuid uuid = s;
    ASSERT_EQ(uuid.toString(), s);
}
TEST(Uuid, Raw) {
    AString s = "123e4567e89b12d3a456426655440000";
    AUuid uuid = s;
    ASSERT_EQ(uuid.toRawString(), s);
}
TEST(Uuid, Random) {
    ARandom r;
    AUI_REPEAT(100) {
        auto u = r.nextUuid();
        AUuid(u.toString());
        AUuid(u.toRawString());
    }
}

TEST(Uuid, ExceptionOverflow) {
    ASSERT_THROW(AUuid("000000000000000000000000000000000"), AUuidException);
}
TEST(Uuid, ExceptionUnderflow) {
    ASSERT_THROW(AUuid("0000000000000000000000000000000"), AUuidException);
}
TEST(Uuid, ExceptionInvalidSymbol) {
    ASSERT_THROW(AUuid("000000000000z0000000000000000000"), AUuidException);
}
TEST(Uuid, ExceptionInvalidDash) {
    ASSERT_THROW(AUuid("0-0000000000000000000000000000000"), AUuidException);
    ASSERT_THROW(AUuid("-0-0000000000000000000000000000000"), AUuidException);
    ASSERT_THROW(AUuid("-0-0000000000000000000000000000000-"), AUuidException);
    ASSERT_THROW(AUuid("-0-00000000000000-000000-000000-00000-"), AUuidException);
}
