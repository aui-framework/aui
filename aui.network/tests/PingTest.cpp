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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "AUI/Common/AException.h"
#include "AUI/IO/AIOException.h"
#include "AUI/Network/AIcmp.h"
#include "AUI/Util/kAUI.h"

using namespace std::chrono_literals;

// cant test them on linux on github
#if !AUI_PLATFORM_LINUX
TEST(Ping, Localhost) {
    try {
        AUI_REPEAT(1000) { EXPECT_GE(AIcmp::ping(AInet4Address::fromString("127.0.0.1"))->count(), 0); }
    } catch (const AException& e) {
        FAIL() << e;
    }
}

TEST(Ping, Fail1) {
    EXPECT_ANY_THROW(AIcmp::ping(AInet4Address::fromString("192.168.10.54"), 1s)->count());
}

TEST(Ping, Fail2) {
    // Check that AIcmp implementation does not mess up with the results.

    AUI_REPEAT(10) {
        auto fail = AIcmp::ping(AInet4Address::fromString("192.168.10.54"), 1s);
        auto good = AIcmp::ping(AInet4Address::fromString("127.0.0.1"));
        EXPECT_ANY_THROW(fail->count());
        EXPECT_GE(good->count(), 0);
    }
}

/*
// we are unable to ping even to github on github actions
TEST(Ping, External) {
    EXPECT_GE(AIcmp::ping(AInet4Address::fromString("github.com"))->count(), 0);
}
*/

#endif
