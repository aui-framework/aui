//
// Created by alex2 on 31.08.2020.
//

#include <boost/test/unit_test.hpp>
#include <AUI/Crypt/ARsa.h>
#include <AUI/Util/ARandom.h>
#include <AUI/Common/AString.h>
#include <AUI/Crypt/AHash.h>
#include <AUI/IO/ByteBufferInputStream.h>

using namespace boost::unit_test;


BOOST_AUTO_TEST_SUITE(Hash)

BOOST_AUTO_TEST_CASE(Sha512) {
    BOOST_CHECK_EQUAL(AHash::sha512(*AByteBuffer::fromString("govno")).toHexString(), "b7cbd9e15895669db8806632dc00894b4551e172220bdadedbae7005291e1a1586a172012e1319fbff968760bcc13d96015acdff8c115b8f1e3e7b421126bb03");
    BOOST_CHECK_EQUAL(AHash::sha512(_new<ByteBufferInputStream>(AByteBuffer::fromString("sdfzsrsrhsrhfxbuihusebrvjmsdfbvhsrhvbhfsvbhbhlsdbhjbsdhbdfhbhlefbhlABHJ"))).toHexString(), "b45f03fb7627749aa177814526a23547436df87b3233393d86586b4ecd043327b94a67f7d1ee56ae43faf8b8290fd0cfb2b11d46134b331c4a1ff1f2da6a3ca8");
}

BOOST_AUTO_TEST_SUITE_END()