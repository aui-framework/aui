//
// Created by alex2 on 30.08.2020.
//

#define BOOST_TEST_MODULE String

#include <boost/test/included/unit_test.hpp>
#include <AUI/Common/AString.h>

using namespace boost::unit_test;
BOOST_AUTO_TEST_SUITE(String)


BOOST_AUTO_TEST_CASE(UTF8)
{
    const char* someUtfString = u8"Ютф строка! \u0228";

    AString s = someUtfString;

    BOOST_TEST(s == someUtfString);
    BOOST_TEST(memcmp(s.toUtf8()->data(), someUtfString, strlen(someUtfString)) == 0);

}

BOOST_AUTO_TEST_SUITE_END()

