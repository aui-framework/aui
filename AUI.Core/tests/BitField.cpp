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