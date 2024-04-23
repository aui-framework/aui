// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "AUI/Common/AException.h"
#include "AUI/IO/AIOException.h"
#include "AUI/Network/AIcmp.h"
#include "AUI/Util/kAUI.h"

using namespace std::chrono_literals;

TEST(Ping, Localhost) {
    AUI_REPEAT(1000) {
        EXPECT_GE(AIcmp::ping(AInet4Address("127.0.0.1"))->count(), 0);
    }
}

TEST(Ping, Fail1) {
    EXPECT_ANY_THROW(AIcmp::ping(AInet4Address("192.168.10.54"), 1s)->count());
}

TEST(Ping, Fail2) {
    // Check that AIcmp implementation does not mess up with the results.

    AUI_REPEAT(10) {
        auto fail = AIcmp::ping(AInet4Address("192.168.10.54"), 1s);
        auto good = AIcmp::ping(AInet4Address("127.0.0.1"));
        EXPECT_ANY_THROW(fail->count());
        EXPECT_GE(good->count(), 0);
    }
}

/*
// we are unable to ping even to github on github actions
TEST(Ping, External) {
    EXPECT_GE(AIcmp::ping(AInet4Address("github.com"))->count(), 0);
}
*/
