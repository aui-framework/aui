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
