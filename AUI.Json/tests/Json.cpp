//
// Created by alex2 on 30.08.2020.
//

#include <boost/test/unit_test.hpp>
#include <AUI/Common/AString.h>
#include <AUI/Json/AJsonElement.h>
#include <AUI/Json/AJson.h>

using namespace boost::unit_test;
BOOST_AUTO_TEST_SUITE(Json)


BOOST_AUTO_TEST_CASE(ObjectAssignValue)
{
    AJsonObject o;
    o["name"] = "Alex2772";
    o["year"] = 2020;

    BOOST_CHECK_EQUAL(AJson::toString(o), "{\"name\":\"Alex2772\",\"year\":2020}");
}

BOOST_AUTO_TEST_CASE(ObjectAssignObject)
{
    AJsonObject o;
    o["name"] = "Alex2772";
    o["year"] = 2020;

    AJsonObject root;
    root["user"] = o;

    BOOST_CHECK_EQUAL(AJson::toString(root), "{\"user\":{\"name\":\"Alex2772\",\"year\":2020}}");
}

BOOST_AUTO_TEST_SUITE_END()

