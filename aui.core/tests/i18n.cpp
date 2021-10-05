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
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

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