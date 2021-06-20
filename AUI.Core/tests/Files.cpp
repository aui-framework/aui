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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

//
// Created by alex2772 on 9/10/20.
//


#include <boost/test/unit_test.hpp>
#include <AUI/IO/APath.h>
#include <AUI/IO/FileOutputStream.h>
#include <AUI/IO/FileInputStream.h>

using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE(Path)

    BOOST_AUTO_TEST_CASE(Unix) {
        APath p = "/home";
        BOOST_CHECK_EQUAL(p.parent(), "");
        BOOST_CHECK_EQUAL(p.parent().parent(), "");
        for (auto str: {"/home/user/work/", "/home/user/work"}) {
            p = str;
            BOOST_CHECK_EQUAL(p.parent(), "/home/user");
            BOOST_CHECK_EQUAL(p.file("one.cpp"), "/home/user/work/one.cpp");
        }

        BOOST_CHECK_EQUAL(APath("/home/user/file.txt").filename(), "file.txt");
    }

    BOOST_AUTO_TEST_CASE(Copy) {
        _new<FileOutputStream>("test.txt")->write("test", 4);

        APath::copy("test.txt", "test-copy.txt");

        char buf[0x100];
        auto s = _new<FileInputStream>("test-copy.txt")->read(buf, sizeof(buf));
        BOOST_TEST(s == 4);
        BOOST_TEST(memcmp(buf, "test", 4) == 0);
    }
    BOOST_AUTO_TEST_CASE(Windows) {
        APath p = "C:/home";
        BOOST_CHECK_EQUAL(p.parent(), "C:");
        BOOST_CHECK_EQUAL(p.parent().parent(), "");
        for (auto str: {"C:/home/user/work/", "C:/home/user/work"}) {
            p = str;
            BOOST_CHECK_EQUAL(p.parent(), "C:/home/user");
            BOOST_CHECK_EQUAL(p.file("one.cpp"), "C:/home/user/work/one.cpp");
        }
        BOOST_CHECK_EQUAL(APath("C:/home/user/file.txt").filename(), "file.txt");
    }

    BOOST_AUTO_TEST_CASE(List) {
        auto l = APath(".").listDir();
#ifdef WIN32
        BOOST_TEST(l.contains("./Tests.exe"));
#else
        BOOST_TEST(l.contains("./Tests"));
#endif
        BOOST_TEST(!l.contains("./."));
        BOOST_TEST(!l.contains("./.."));

        l = APath(".").listDir(ListFlags::LF_DEFAULT_FLAGS | LF_DONT_IGNORE_DOTS);
#ifdef WIN32
        BOOST_TEST(l.contains("./Tests.exe"));
#else
        BOOST_TEST(l.contains("./Tests"));
#endif
        BOOST_TEST(l.contains("./."));
        BOOST_TEST(l.contains("./.."));

        l = APath(".").listDir(ListFlags::LF_DIRS);
        BOOST_TEST(l.empty());
    }

BOOST_AUTO_TEST_SUITE_END()