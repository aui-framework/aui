//
// Created by alex2 on 31.08.2020.
//

#include <boost/test/unit_test.hpp>
#include <AUI/Crypt/ARsa.h>
#include <AUI/Util/ARandom.h>
#include <AUI/Common/AString.h>
#include <AUI/Crypt/AHash.h>

using namespace boost::unit_test;


BOOST_AUTO_TEST_SUITE(Hash)

BOOST_AUTO_TEST_CASE(Sha512) {
    BOOST_CHECK_EQUAL(AHash::sha512(*AByteBuffer::fromString("govno")).toHexString(), "b7cbd9e15895669db8806632dc00894b4551e172220bdadedbae7005291e1a1586a172012e1319fbff968760bcc13d96015acdff8c115b8f1e3e7b421126bb03");
}

BOOST_AUTO_TEST_SUITE_END()