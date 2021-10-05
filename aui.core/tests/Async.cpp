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
// Created by alex2 on 31.08.2020.
//

#include <boost/test/unit_test.hpp>
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

        repeat_async(1'000) {
            (*someInt) += 1;
        };

        AThread::sleep(1'000);

        BOOST_CHECK_EQUAL(*someInt, 1'000);
    }

BOOST_AUTO_TEST_SUITE_END()