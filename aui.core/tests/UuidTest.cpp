/**
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
    repeat(100) {
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
