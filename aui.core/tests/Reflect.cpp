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
// Created by alex2 on 30.08.2020.
//

#include <boost/test/unit_test.hpp>
#include <AUI/Common/AString.h>
#include <AUI/Common/AMap.h>
#include <AUI/Reflect/AClass.h>
#include <AUI/Reflect/AEnumerate.h>
#include <AUI/Reflect/AReflect.h>
#include <AUI/Model/AListModel.h>

using namespace boost::unit_test;
enum Test {
    VALUE1,
    VALUE2,
    VALUE3 = 1000,
    VALUE4 = 10000,
};

namespace namespaceeee {
    enum Test {
        V1,
        V2,
        V3,
    };
}

struct MyStruct {};

namespace AzazaTest {
    struct Test {};
}

ENUM_VALUES(Test, VALUE1, VALUE2, VALUE3, VALUE4)
ENUM_VALUES(namespaceeee::Test, namespaceeee::V1, namespaceeee::V2, namespaceeee::V3)

BOOST_AUTO_TEST_SUITE(Reflect)

    BOOST_AUTO_TEST_CASE(NameClass) {
        BOOST_CHECK_EQUAL(AClass<AString>::name(), "AString");
        BOOST_CHECK_EQUAL(AClass<AzazaTest::Test>::nameWithoutNamespace(), "Test");
    }

    BOOST_AUTO_TEST_CASE(NameClassPtr) {
        AObject* ptr = new AListModel<AString>();
        BOOST_CHECK_EQUAL(AReflect::name(ptr), "AListModel<AString>");
    }

    BOOST_AUTO_TEST_CASE(NameStruct) {
        BOOST_CHECK_EQUAL(AClass<MyStruct>::name(), "MyStruct");
    }

    BOOST_AUTO_TEST_CASE(NameEnum) {

        BOOST_CHECK_EQUAL((AClass<Test>::name()), "Test");
    }

    BOOST_AUTO_TEST_CASE(EnumerateNames) {
        AMap<AString, Test> ref = {
            {"VALUE1", VALUE1},
            {"VALUE2", VALUE2},
            {"VALUE3", VALUE3},
        };

        BOOST_TEST((AEnumerate<Test>::names<VALUE1, VALUE2, VALUE3>() == ref));
    }

    BOOST_AUTO_TEST_CASE(EnumerateAll) {
        AMap<AString, Test> ref = {
                {"VALUE1", VALUE1},
                {"VALUE2", VALUE2},
                {"VALUE3", VALUE3},
        };

        BOOST_TEST((AEnumerate<Test>::names<VALUE1, VALUE2, VALUE3>() == ref));
    }

    BOOST_AUTO_TEST_CASE(NamespaceEnumerateNames) {
        AMap<AString, namespaceeee::Test> ref = {
                {"V1", namespaceeee::V1},
                {"V2", namespaceeee::V2},
                {"V3", namespaceeee::V3},
        };

        BOOST_TEST((AEnumerate<namespaceeee::Test>::all() == ref));
    }

BOOST_AUTO_TEST_SUITE_END()