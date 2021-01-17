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
#include <AUI/Util/kAUI.h>
#include <AUI/Common/AString.h>
#include <AUI/Common/AVector.h>
#include <AUI/Common/ASignal.h>
#include <AUI/Reflect/AClass.h>

using namespace boost::unit_test;

class SomeBuilderClass {
private:
    AString mUsername;
    int mAge = -1;
public:
    SomeBuilderClass() {}

    void setUsername(const AString& username) {
        mUsername = username;
    }

    void setAge(int age) {
        mAge = age;
    }


    const AString& getUsername() const {
        return mUsername;
    }

    int getAge() const {
        return mAge;
    }
};


BOOST_AUTO_TEST_SUITE(SharedPtrs)

    BOOST_AUTO_TEST_CASE(Builder1) {
        auto builder = _new<SomeBuilderClass>();
        apply(builder, {
           setUsername("John");
           setAge(23);
        });

        BOOST_CHECK_EQUAL(builder->getUsername(), "John");
        BOOST_CHECK_EQUAL(builder->getAge(), 23);
    }

    BOOST_AUTO_TEST_CASE(Builder2) {
        auto builder = _new<SomeBuilderClass>() let (SomeBuilderClass, {
            setUsername("John");
            setAge(23);
        });


        BOOST_CHECK_EQUAL(builder->getUsername(), "John");
        BOOST_CHECK_EQUAL(builder->getAge(), 23);
    }


    BOOST_AUTO_TEST_CASE(NullSafety) {
        class Person {
        private:
            AString mName;
            int mAge;
        public:

            Person(const AString& name, int age)
                    : mName(name),
                      mAge(age) {
            }


            [[nodiscard]] const AString& getName() const {
                return mName;
            }

            void setName(const AString& name) {
                mName = name;
            }

            [[nodiscard]] int getAge() const {
                return mAge;
            }

            void setAge(const int age) {
                mAge = age;
            }
        };

        AVector<_<Person>> persons = {
                _new<Person>("John", 23),
                _new<Person>("Jenny", 21),
                nullptr
        };

        for (auto& person : persons) {
            person.safe()
                    (&Person::setAge, 80)
                    (&Person::setName, "Loh");
        }
        BOOST_CHECK_EQUAL(persons[0]->getAge(), 80);
        BOOST_CHECK_EQUAL(persons[1]->getAge(), 80);
        BOOST_CHECK_EQUAL(persons[0]->getName(), "Loh");
        BOOST_CHECK_EQUAL(persons[1]->getName(), "Loh");
        BOOST_CHECK_EQUAL(persons[2], nullptr);
    }

    class SendObject : public AObject {
    public:
        void invokeSignal() {
            emit someSignal();
        }

    signals:
        emits<> someSignal;
    };

    class ReceiverObject : public AObject {
    public:
        void receiveSignal() {
            mSignalInvoked = true;
        }

        bool mSignalInvoked = false;
    };

    BOOST_AUTO_TEST_CASE(ConnectBuilder) {
        auto receiver = _new<ReceiverObject>();

        auto sender = _new<SendObject>()
                .connect(&SendObject::someSignal, receiver, &ReceiverObject::receiveSignal);


        apply(sender, {
            invokeSignal();
        });

        BOOST_TEST(receiver->mSignalInvoked);
    }
    BOOST_AUTO_TEST_CASE(MoveToShared) {
        AByteBuffer buffer = AByteBuffer::fromString(
                "sdfzsrsrhsrhfxbuihusebrvjmsdfbvhsrhvbhfsvbhbhlsdbhjbsdhbdfhbhlefbhlABHJ");
        _<AByteBuffer> buffer_shared = buffer >> shared;

        BOOST_TEST(buffer.data() == nullptr);
        BOOST_TEST(buffer_shared->data() != nullptr);
    }

    BOOST_AUTO_TEST_CASE(Performance1) {
        repeat (1'000) {
            AByteBuffer buffer;
            repeat (10'000) {
                buffer << uint8_t(228);
            }
            _<AByteBuffer> shared_ptr = _new<AByteBuffer>(buffer);
        }
    }
    BOOST_AUTO_TEST_CASE(Performance2) {
        repeat (1'000) {
            AByteBuffer buffer;
            repeat (10'000) {
                buffer << uint8_t(228);
            }
            _<AByteBuffer> shared_ptr = buffer >> shared;
        }
    }

BOOST_AUTO_TEST_SUITE_END()
