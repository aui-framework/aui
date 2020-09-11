//
// Created by alex2772 on 9/10/20.
//


#include <boost/test/unit_test.hpp>
#include <AUI/IO/APath.h>

using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE(Path)

    BOOST_AUTO_TEST_CASE(Unix) {
        APath p = "/home";
        BOOST_CHECK_EQUAL(p.up(), "");
        BOOST_CHECK_EQUAL(p.up().up(), "");
        for (auto str: {"/home/user/work/", "/home/user/work"}) {
            p = str;
            BOOST_CHECK_EQUAL(p.up(), "/home/user");
            BOOST_CHECK_EQUAL(p.file("one.cpp"), "/home/user/work/one.cpp");
        }

        BOOST_CHECK_EQUAL(APath("/home/user/file.txt").filename(), "file.txt");
    }
    BOOST_AUTO_TEST_CASE(Windows) {
        APath p = "C:/home";
        BOOST_CHECK_EQUAL(p.up(), "C:");
        BOOST_CHECK_EQUAL(p.up().up(), "");
        for (auto str: {"C:/home/user/work/", "C:/home/user/work"}) {
            p = str;
            BOOST_CHECK_EQUAL(p.up(), "C:/home/user");
            BOOST_CHECK_EQUAL(p.file("one.cpp"), "C:/home/user/work/one.cpp");
        }
        BOOST_CHECK_EQUAL(APath("C:/home/user/file.txt").filename(), "file.txt");
    }

    BOOST_AUTO_TEST_CASE(List) {
        auto l = APath(".").listDir();
        BOOST_TEST(l.contains("tests"));
        BOOST_TEST(!l.contains("."));
        BOOST_TEST(!l.contains(".."));

        l = APath(".").listDir(LF_DEFAULT_FLAGS | LF_DONT_IGNORE_DOTS);
        BOOST_TEST(l.contains("tests"));
        BOOST_TEST(l.contains("."));
        BOOST_TEST(l.contains(".."));

        l = APath(".").listDir(LF_DIRS);
        BOOST_TEST(l.empty());
    }

BOOST_AUTO_TEST_SUITE_END()