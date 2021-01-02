//
// Created by alex2 on 30.08.2020.
//

#include <boost/test/unit_test.hpp>
#include <AUI/Common/SharedPtr.h>
#include <AUI/Thread/AFuture.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Util/Util.h>
#include <random>
#include <ctime>

using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE(Threading)

    BOOST_AUTO_TEST_CASE(Async) {
        auto someInt = _new<std::atomic_int>(0);
        for (int i = 0; i < 100; ++i)
            async{
                    (*someInt) += 1;
            };

        AThread::sleep(1000);

        BOOST_CHECK_EQUAL(*someInt, 100);
    }

    BOOST_AUTO_TEST_CASE(Future1, *boost::unit_test::tolerance(10)) {
        ADeque < _ < AFuture < double>>> taskList;
        auto time = Util::measureTimeInMillis([&]() {
            repeat(1000)
            {
                taskList << async{
                        double i = 2.0;
                        repeat(1000)
                        {
                            i *= 2;
                        }
                        return i;
                };
            }

            taskList.forEach(&AFuture<double>::get);

            printf("Ok, result: %f\n", **taskList.first());
        });

        printf("Finished in %llu ms\n", time);
        BOOST_CHECK_EQUAL(**taskList.first(),
                          21430172143725346418968500981200036211228096234110672148875007767407021022498722449863967576313917162551893458351062936503742905713846280871969155149397149607869135549648461970842149210124742283755908364306092949967163882534797535118331087892154125829142392955373084335320859663305248773674411336138752.000000);
        BOOST_TEST(time < 1000);
    }

    BOOST_AUTO_TEST_CASE(Future2) {
        auto b = _new<bool>(false);
        async{
                AThread::sleep(1000);
                *b = true;
        };
        BOOST_TEST(!*b);
        AThread::sleep(2000);
        BOOST_TEST(*b);
    }

    BOOST_AUTO_TEST_CASE(Future3) {
        auto b = _new<bool>(false);
        auto v8 = async{
                AThread::sleep(1000);
                *b = true;
                return 8;
        };
        auto v1231 = async{
                AThread::sleep(1000);
                *b = true;
                return 1231.f;
        };
        BOOST_TEST(!*b);
        AThread::sleep(2000);
        BOOST_TEST(*b);

        BOOST_CHECK_EQUAL(**v8, 8);
        BOOST_CHECK_EQUAL(**v1231, 1231.f);
    }

    BOOST_AUTO_TEST_CASE(Fence) {
        std::default_random_engine e(std::time(nullptr));
        repeat(10) {
            std::atomic_int test = 0;
            std::mutex m;

            AThreadPool::global().fence([&]() {
                repeat(1000) {
                    asyncX [&]() {
                        int s;
                        {
                            std::unique_lock lock(m);
                            s = std::uniform_int_distribution(5, 100)(e);
                        }
                        AThread::sleep(s);
                        ++test;
                    };
                }

                BOOST_CHECK_LE(test, 1000);
            });
            BOOST_CHECK_EQUAL(test, 1000);
        }
    }

BOOST_AUTO_TEST_SUITE_END()