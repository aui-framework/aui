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
