//
// Created by alex2 on 31.08.2020.
//

#define BOOST_TEST_MODULE Async

#include <boost/test/included/unit_test.hpp>
#include <AUI/Common/SharedPtr.h>
#include <AUI/Thread/AFuture.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Util/Util.h>

using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE(Async)

    template<typename T>
    std::ostream& operator<<(std::ostream& o, const std::atomic<T>& n) {
        o << *n;
        return o;
    }


    BOOST_AUTO_TEST_CASE(Repeat) {
        auto someInt = _new<std::atomic_int>(0);

        repeat(100'000) {
            (*someInt) += 1;
        };

        BOOST_CHECK_EQUAL(*someInt, 100'000);
    }

    BOOST_AUTO_TEST_CASE(RepeatAsync) {
        auto someInt = _new<std::atomic_int>(0);

        repeat_async(100'000) {
                (*someInt) += 1;
            };

        AThread::sleep(1'000);

        BOOST_CHECK_EQUAL(*someInt, 100'000);
    }

BOOST_AUTO_TEST_SUITE_END()