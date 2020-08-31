//
// Created by alex2 on 31.08.2020.
//

#include <boost/test/unit_test.hpp>
#include <AUI/Data/ASqlDatabase.h>
#include <AUI/Data/AMeta.h>

using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE(Meta)


BOOST_AUTO_TEST_CASE(Meta) {
        Autumn::put(ASqlDatabase::connect("sqlite", ":memory:"));
        AMeta::set("kek", 4);
        AMeta::set("lol", "azaza");
        BOOST_CHECK_EQUAL(AMeta::get("kek"), 4);
        BOOST_CHECK_EQUAL(AMeta::get("lol"), "azaza");
        AMeta::set("kek", "four");
        AMeta::set("lol", 42.0);
        BOOST_CHECK_EQUAL(AMeta::get("kek"), "four");
        BOOST_CHECK_EQUAL(AMeta::get("lol"), 42.0);
}


BOOST_AUTO_TEST_SUITE_END()