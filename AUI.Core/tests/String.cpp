//
// Created by alex2 on 30.08.2020.
//

#include <boost/test/unit_test.hpp>
#include <AUI/Common/AString.h>

using namespace boost::unit_test;
BOOST_AUTO_TEST_SUITE(String)


BOOST_AUTO_TEST_CASE(UTF8)
{
    const char* someUtfString = "Ютф строка! \u0228";

    AString s = someUtfString;

    BOOST_TEST(s == someUtfString);
    BOOST_TEST(memcmp(s.toUtf8()->data(), someUtfString, strlen(someUtfString)) == 0);

}

BOOST_AUTO_TEST_SUITE_END()

