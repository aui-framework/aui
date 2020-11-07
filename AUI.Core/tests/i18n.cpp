//
// Created by alex2 on 07.11.2020.
//


#include <boost/test/unit_test.hpp>
#include <AUI/IO/APath.h>
#include <AUI/Common/AException.h>
#include <AUI/i18n/ALanguageCode.h>

using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE(i18n)


BOOST_AUTO_TEST_CASE(LanguageCode) {
        BOOST_CHECK_THROW(ALanguageCode("en_US"), AException);
        BOOST_CHECK_THROW(ALanguageCode("en-us"), AException);
        BOOST_CHECK_THROW(ALanguageCode("en-UdS"), AException);
        BOOST_CHECK_THROW(ALanguageCode("end-US"), AException);
        BOOST_CHECK_THROW(ALanguageCode(""), AException);
        BOOST_CHECK_THROW(ALanguageCode("fweo,fwoepmo"), AException);
        BOOST_CHECK_THROW(ALanguageCode("fweo,-"), AException);
        BOOST_CHECK_THROW(ALanguageCode("-afs"), AException);
        BOOST_CHECK_NO_THROW(ALanguageCode("en-US"));

        ALanguageCode c("ru-UA");

        BOOST_CHECK_EQUAL(c.toString(), "ru-UA");
        BOOST_CHECK_EQUAL(c.getGroup(), "ru");
        BOOST_CHECK_EQUAL(c.getSubGroup(), "UA");
}

BOOST_AUTO_TEST_SUITE_END()