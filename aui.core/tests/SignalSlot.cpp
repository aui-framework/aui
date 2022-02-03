//
// Created by Alex2772 on 2/3/2022.
//


#include <gtest/gtest.h>
#include <AUI/Common/AObject.h>
#include <AUI/Common/ASignal.h>
#include <AUI/Common/AString.h>
#include <AUI/Util/kAUI.h>

class Master: public AObject {
public:
    void broadcastMessage(const char* msg) {
        emit message(msg);
    }

signals:
    emits<const char*> message;
};


class Slave: public AObject {
private:
    AString mExpectedMessage;

public:
    bool mMessageReceived = false;
    Slave(const AString& expectedMessage) : mExpectedMessage(expectedMessage) {
    }
    ~Slave() {
        EXPECT_TRUE(getThread().get() == AThread::current().get());
        EXPECT_TRUE(mMessageReceived);
    }

    void acceptMessage(const char* msg) {
        EXPECT_EQ(mExpectedMessage, AString(msg));
        mMessageReceived = true;

        EXPECT_TRUE(getThread().get() == AThread::current().get());
    }
    void acceptMessageNoArgs() {
        mMessageReceived = true;
    }
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
        AThread::current()->processMessages();
        AThread::sleep(500);
        ASSERT_TRUE(slave->mMessageReceived);
        master = nullptr;
        slave = nullptr;
    }
};

TEST_F(SignalSlot, Basic) {
    slave = _new<Slave>("hello");
    AObject::connect(master->message, slot(slave)::acceptMessage);
    master->broadcastMessage("hello");
}


TEST_F(SignalSlot, BasicNoArgs) {
    slave = _new<Slave>("hello");
    AObject::connect(master->message, slot(slave)::acceptMessageNoArgs);
    master->broadcastMessage("hello");
}

TEST_F(SignalSlot, Multithread) {
    auto t = async {
        slave = _new<Slave>("hello");
    };
    t.wait();

    ASSERT_FALSE(slave->getThread() == master->getThread());

    AObject::connect(master->message, slot(slave)::acceptMessage);
    master->broadcastMessage("hello");
}
