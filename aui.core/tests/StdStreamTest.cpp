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
#include "AUI/IO/AStdIStream.h"
#include "AUI/IO/AStringStream.h"


TEST(StdStream, Input) {
    AStdIStream is(_new<AStringStream>("hello world"_as));

    std::string v;
    is >> v;
    EXPECT_EQ(v, "hello");
    is >> v;
    EXPECT_EQ(v, "world");
}
