//
// Created by alex2 on 31.08.2020.
//

#include <boost/test/unit_test.hpp>
#include <AUI/Util/kAUI.h>
#include <AUI/Common/AString.h>
#include <AUI/Common/AVector.h>
#include <AUI/Common/ASignal.h>

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
        auto builder = _new<SomeBuilderClass>() by ({
            setUsername("John");
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

BOOST_AUTO_TEST_SUITE_END()