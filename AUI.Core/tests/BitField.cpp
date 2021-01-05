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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 31.08.2020.
//

#include <boost/test/unit_test.hpp>

#include <AUI/Util/BitField.h>
#include <AUI/Util/EnumUtil.h>

using namespace boost::unit_test;


ENUM_FLAG(Flags) {
    F_FLAG1 = 0x1,
    F_FLAG2 = 0x2,
    F_FLAG3 = 0x4,
};

BOOST_AUTO_TEST_SUITE(BitField1)

    BOOST_AUTO_TEST_CASE(Put) {
        BitField<Flags> f;
        f << F_FLAG1;
        Flags x = f;
        BOOST_TEST(x & F_FLAG1);
        BOOST_TEST(!(x & ~F_FLAG1));
        f << F_FLAG2;
        x = f;
        BOOST_TEST(x & F_FLAG1);
        BOOST_TEST(x & F_FLAG2);
        BOOST_TEST((x & ~F_FLAG1));
        BOOST_TEST((x & ~F_FLAG2));
    }

    BOOST_AUTO_TEST_CASE(Take) {
        BitField<Flags> f;
        f << F_FLAG1 << F_FLAG2;
        Flags x = f;
        BOOST_TEST(x & F_FLAG1);
        BOOST_TEST(x & F_FLAG2);
        f >> F_FLAG2;
        x = f;
        BOOST_TEST(x & F_FLAG1);
        BOOST_TEST(!(x & F_FLAG2));
    }

    BOOST_AUTO_TEST_CASE(CheckTake1) {
        BitField<Flags> f;
        f << F_FLAG1 << F_FLAG2;
        Flags x = f;
        BOOST_TEST(x & F_FLAG1);
        BOOST_TEST(x & F_FLAG2);
        BOOST_TEST(f.checkAndSet(F_FLAG2));
        x = f;
        BOOST_TEST(x & F_FLAG1);
        BOOST_TEST(!(x & F_FLAG2));
    }
    BOOST_AUTO_TEST_CASE(CheckTake2) {
        BitField<Flags> f;
        f << F_FLAG1;
        Flags x = f;
        BOOST_TEST(x & F_FLAG1);
        BOOST_TEST(!(x & F_FLAG2));
        BOOST_TEST(!f.checkAndSet(F_FLAG2));
        x = f;
        BOOST_TEST(x & F_FLAG1);
        BOOST_TEST(!(x & F_FLAG2));
    }
    BOOST_AUTO_TEST_CASE(Check) {
        BitField<Flags> f;
        f << F_FLAG1;

        BOOST_TEST((f.check(F_FLAG1) && (f & F_FLAG1)));
        BOOST_TEST(!f.check(F_FLAG2));
        BOOST_TEST(!f.check(F_FLAG3));

        f << F_FLAG2;

        BOOST_TEST(f.check(F_FLAG1));
        BOOST_TEST(f.check(F_FLAG2));
        BOOST_TEST(!f.check(F_FLAG3));

        f >> F_FLAG3;
        f >> F_FLAG1;

        BOOST_TEST(!f.check(F_FLAG1));
        BOOST_TEST(f.check(F_FLAG2));
        BOOST_TEST(!f.check(F_FLAG3));
    }


BOOST_AUTO_TEST_SUITE_END()