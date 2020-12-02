//
// Created by alex2772 on 9/10/20.
//


#include <boost/test/unit_test.hpp>
#include <AUI/IO/APath.h>

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

        l = APath(".").listDir(LF_DEFAULT_FLAGS | LF_DONT_IGNORE_DOTS);
#ifdef WIN32
        BOOST_TEST(l.contains("./Tests.exe"));
#else
        BOOST_TEST(l.contains("./Tests"));
#endif
        BOOST_TEST(l.contains("./."));
        BOOST_TEST(l.contains("./.."));

        l = APath(".").listDir(LF_DIRS);
        BOOST_TEST(l.empty());
    }

BOOST_AUTO_TEST_SUITE_END()