/**
 * =====================================================================================================================
 * Copyright (c) 2020 Alex2772
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
#include <AUI/Autumn/Autumn.h>

using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE(DependencyInjection)

BOOST_AUTO_TEST_CASE(Anonymous) {
        class MyData {
            private:
            AString mString;

            public:
            MyData(const AString& string) : mString(string) {}

            const AString& getString() const {
                return mString;
            }
        };
        Autumn::put(_new<MyData>("hello"));

        BOOST_CHECK_EQUAL(Autumn::get<MyData>()->getString(), "hello");
}

BOOST_AUTO_TEST_CASE(Named) {
        class MyData {
            private:
            AString mString;

            public:
            MyData(const AString& string) : mString(string) {}

            const AString& getString() const {
                return mString;
            }
        };
        Autumn::put("obj_foo", _new<MyData>("foo"));
        Autumn::put("obj_bar", _new<MyData>("bar"));

        BOOST_CHECK_EQUAL(Autumn::get<MyData>("obj_foo")->getString(), "foo");
        BOOST_CHECK_EQUAL(Autumn::get<MyData>("obj_bar")->getString(), "bar");
        BOOST_CHECK_THROW(Autumn::get<MyData>("obj_nonexistent"), AException);
}

BOOST_AUTO_TEST_CASE(Constructor) {
        class BasicComponent {
            private:
            AString mName;

            public:
            BasicComponent(const AString& name) : mName(name) {}

            virtual ~BasicComponent() = default;

            const AString& getName() const {
                return mName;
            }

        };

        class Motherboard : public BasicComponent {
            public:
            Motherboard(const AString& name) : BasicComponent(name) {}

            virtual ~Motherboard() = default;

        };
        class CPU : public BasicComponent {
            public:
            CPU(const AString& name) : BasicComponent(name) {}

            virtual ~CPU() = default;
        };
        class Videocard : public BasicComponent {
            public:
            Videocard(const AString& name) : BasicComponent(name) {}

            virtual ~Videocard() = default;

        };

        class Computer {
            private:
            _<Motherboard> mMotherboard;
            _<CPU> mCPU;
            _<Videocard> mVideocard;

            public:
            Computer(const _<Motherboard>& motherboard, const _<CPU>& cpu, const _<Videocard>& videocard)
            : mMotherboard(
                    motherboard), mCPU(cpu), mVideocard(videocard) {}

            const _<Motherboard>& getMotherboard() const {
                return mMotherboard;
            }

            const _<CPU>& getCpu() const {
                return mCPU;
            }

            const _<Videocard>& getVideocard() const {
                return mVideocard;
            }
        };

        Autumn::put(_new<Motherboard>("B450"));
        Autumn::put(_new<CPU>("3600U"));
        Autumn::put(_new<Videocard>("750Ti"));

        auto computer = Autumn::construct<Computer>::with<Motherboard, CPU, Videocard>();

        BOOST_CHECK_EQUAL(computer->getMotherboard()->getName(), "B450");
        BOOST_CHECK_EQUAL(computer->getCpu()->getName(), "3600U");
        BOOST_CHECK_EQUAL(computer->getVideocard()->getName(), "750Ti");
}

BOOST_AUTO_TEST_SUITE_END()