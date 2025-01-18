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
// Created by alex2772 on 9/10/20.
//


#include <gtest/gtest.h>
#include <AUI/IO/APath.h>
#include <AUI/IO/AFileOutputStream.h>
#include <AUI/IO/AFileInputStream.h>


TEST(Path, Unix) {
    APath p = "/home";
    ASSERT_EQ(p.parent(), "");
    ASSERT_EQ(p.parent().parent(), "");
    for (auto str: {"/home/user/work/", "/home/user/work"}) {
        p = str;
        ASSERT_EQ(p.parent(), "/home/user");
        ASSERT_EQ(p.file("one.cpp"), "/home/user/work/one.cpp");
    }

    ASSERT_EQ(APath("/home/user/file.txt").filename(), "file.txt");
}

TEST(Path, Copy) {
    _new<AFileOutputStream>("test.txt")->write("test", 4);

    APath::copy("test.txt", "test-copy.txt");

    char buf[0x100];
    auto s = _new<AFileInputStream>("test-copy.txt")->read(buf, sizeof(buf));
    ASSERT_TRUE(s == 4);
    ASSERT_TRUE(memcmp(buf, "test", 4) == 0);
}

TEST(Path, Windows) {
    APath p = "C:/home";
    ASSERT_EQ(p.parent(), "C:");
    ASSERT_EQ(p.parent().parent(), "");
    for (auto str: {"C:/home/user/work/", "C:/home/user/work"}) {
        p = str;
        ASSERT_EQ(p.parent(), "C:/home/user");
        ASSERT_EQ(p.file("one.cpp"), "C:/home/user/work/one.cpp");
    }
    ASSERT_EQ(APath("C:/home/user/file.txt").filename(), "file.txt");
}

TEST(Path, Extension) {
    EXPECT_EQ(APath("test.txt").extension(), "txt");
    EXPECT_EQ(APath("te.st.txt").extension(), "txt");
    EXPECT_EQ(APath("C:/te.st.txt").extension(), "txt");
}
