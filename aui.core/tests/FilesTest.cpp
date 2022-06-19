/*
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
