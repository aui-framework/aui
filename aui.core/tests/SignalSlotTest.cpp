//
// Created by Alex2772 on 2/3/2022.
//


#include <gtest/gtest.h>
#include <AUI/Common/AObject.h>
#include <AUI/Common/ASignal.h>
#include <AUI/Common/AString.h>
#include <AUI/Util/kAUI.h>
#include <gmock/gmock.h>

class Master: public AObject {
public:
    void broadcastMessage(const char* msg) {
        emit message(msg);
    }

signals:
    emits<AString> message;
};


class Slave: public AObject {
public:
    ~Slave() {
        die();
    }

    MOCK_METHOD(void, acceptMessage, (const AString& msg));
    MOCK_METHOD(void, acceptMessageNoArgs, ());
    MOCK_METHOD(void, die, ());
};

/**
 * Fixture.
 */
class SignalSlot : public testing::Test {
public:
    _<Master> master;
    _<Slave> slave;

    /**
     * Run once per test case before body.
     */
    void SetUp() override {
        Test::SetUp();
        master = _new<Master>();
    }

    /**
     * Run once per test case after body.
     */
    void TearDown() override {
        Test::TearDown();
        AThread::processMessages();
        AThread::sleep(500);

        master = nullptr;
        slave = nullptr;
    }
};

TEST_F(SignalSlot, Basic) {
    slave = _new<Slave>();
    AObject::connect(master->message, slot(slave)::acceptMessage);

    EXPECT_CALL(*slave, acceptMessage(AString("hello")));
    master->broadcastMessage("hello");
}


TEST_F(SignalSlot, BasicNoArgs) {
    slave = _new<Slave>();
    AObject::connect(master->message, slot(slave)::acceptMessageNoArgs);

    EXPECT_CALL(*slave, acceptMessageNoArgs);
    master->broadcastMessage("hello");
}

TEST_F(SignalSlot, Multithread) {
    slave = _new<Slave>();

    AObject::connect(master->message, slot(slave)::acceptMessage);

    EXPECT_CALL(*slave, acceptMessage(AString("hello")));
    auto t = async {
        master->broadcastMessage("hello");
    };
    t.wait();
}

TEST_F(SignalSlot, StackAllocatedObject) {
    testing::InSequence seq;
    Slave slave;

    AObject::connect(master->message, slot(slave)::acceptMessage);

    EXPECT_CALL(slave, acceptMessage(AString("hello"))).Times(1);
    master->broadcastMessage("hello");

    EXPECT_CALL(slave, die()).Times(1);
}

TEST_F(SignalSlot, ObjectRemoval) {
    slave = _new<Slave>();
    {
        testing::InSequence s;
        AObject::connect(master->message, slot(slave)::acceptMessage); // imitate signal-slot relations
        EXPECT_CALL(*slave, die()).Times(1);
        slave = nullptr; // perform removal
    }
}


TEST_F(SignalSlot, ObjectRemovalMultithread) {

    repeat(100) {

        class Slave2 : public AObject {
        public:
            Slave2(bool& called) : mCalled(called) {}

            void acceptMessage() {
                mCalled = true;
            }

        private:
            bool& mCalled;
        };

        bool called = false;

        auto slave2 = _new<Slave2>(called);

        AObject::connect(master->message, slot(slave2)::acceptMessage);

        auto task = async {
            master->broadcastMessage("hello");
        };


        for (;;) {
            AThread::processMessages();
            task.wait();
            if (called) {
                break;
            }
        }

        task = async {
            repeat(10000) master->broadcastMessage("hello");
        };
        task.wait();
        slave2 = nullptr; // delete slave; check for crash
        AThread::processMessages();
    }
}


TEST_F(SignalSlot, ObjectRemovalMultithreadStackObject) {

    repeat(100) {

        class Slave2 : public AObject {
        public:
            Slave2(bool& called) : mCalled(called) {}

            void acceptMessage() {
                mCalled = true;
            }

        private:
            bool& mCalled;
        };

        bool called = false;

        {
            Slave2 slave2(called);

            AObject::connect(master->message, slot(slave2)::acceptMessage);

            auto task = async {
                ASSERT_DEATH({master->broadcastMessage("hello");}, "crossthread.*");
            };

            AThread::processMessages();
            task.wait();

            // slave will delete here; check for crash
        }
        AThread::processMessages();
    }
}
