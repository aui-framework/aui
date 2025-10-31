/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//
// Created by Alex2772 on 2/3/2022.
//


#include <gtest/gtest.h>
#include <AUI/Common/AObject.h>
#include <AUI/Common/ASignal.h>
#include <AUI/Common/AString.h>
#include <AUI/Util/kAUI.h>
#include <gmock/gmock.h>
#include <random>

using namespace std::chrono_literals;

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
    MOCK_METHOD(void, acceptMessageInt, (int));
    MOCK_METHOD(void, acceptMessageNoArgs, ());
    MOCK_METHOD(void, die, ());
};

/**
 * Fixture.
 */
class SignalSlotTest : public testing::Test {
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
        AThread::sleep(500ms);

        master = nullptr;
        slave = nullptr;
    }

    template<typename... Args>
    static auto& connections(ASignal<Args...>& signal) {
        return signal.mOutgoingConnections;
    }

    static auto& connections(AObject& object) {
        return object.mIngoingConnections;
    }
};

TEST_F(SignalSlotTest, Basic) {
    slave = _new<Slave>();
    AObject::connect(master->message, AUI_SLOT(slave)::acceptMessage);

    EXPECT_CALL(*slave, acceptMessage(AString("hello")));
    EXPECT_CALL(*slave, die());
    master->broadcastMessage("hello");
}

TEST_F(SignalSlotTest, GenericObserver) {
    /// [GENERIC_OBSERVER]
    struct State {
        bool called = false;
    };
    auto state = _new<State>();
    AObject::connect(master->message, AObject::GENERIC_OBSERVER, [state] {
        state->called = true;
    });
    master->broadcastMessage("hello");
    EXPECT_TRUE(state->called);
    /// [GENERIC_OBSERVER]
}

TEST_F(SignalSlotTest, BasicNoArgs) {
    slave = _new<Slave>();
    AObject::connect(master->message, AUI_SLOT(slave)::acceptMessageNoArgs);

    EXPECT_CALL(*slave, acceptMessageNoArgs);
    EXPECT_CALL(*slave, die());
    master->broadcastMessage("hello");
}

TEST_F(SignalSlotTest, Multithread) {
    slave = _new<Slave>();

    AObject::connect(master->message, AUI_SLOT(slave)::acceptMessage);

    EXPECT_CALL(*slave, acceptMessage(AString("hello")));
    EXPECT_CALL(*slave, die());
    auto t = AUI_THREADPOOL {
        master->broadcastMessage("hello");
    };
    t.wait();
}

TEST_F(SignalSlotTest, StackAllocatedObject) {
    testing::InSequence seq;
    Slave slave;

    AObject::connect(master->message, AUI_SLOT(slave)::acceptMessage);

    EXPECT_CALL(slave, acceptMessage(AString("hello"))).Times(1);
    master->broadcastMessage("hello");

    EXPECT_CALL(slave, die()).Times(1);
}

/**
 * Checks that the program is not crashed when one of the object is destroyed.
 * slave is destroyed first.
 */
TEST_F(SignalSlotTest, ObjectRemoval1) {
    slave = _new<Slave>();

    AObject::connect(master->message, AUI_SLOT(slave)::acceptMessage); // imitate signal-slot relations
    EXPECT_EQ(connections(master->message).size(), 1);

    testing::InSequence s;
    EXPECT_CALL(*slave, acceptMessage(AString("test"))).Times(1);
    master->broadcastMessage("test");

    EXPECT_CALL(*slave, die()).Times(1);
    slave = nullptr;

    EXPECT_EQ(connections(master->message).size(), 0);
    master->broadcastMessage("test");
}

/**
 * Checks that the program is not crashed when one of the object is destroyed.
 * master is destroyed first.
 */
TEST_F(SignalSlotTest, ObjectRemoval2) {
    slave = _new<Slave>();

    AObject::connect(master->message, AUI_SLOT(slave)::acceptMessage); // imitate signal-slot relations
    EXPECT_EQ(connections(master->message).size(), 1);

    testing::InSequence s;
    EXPECT_CALL(*slave, acceptMessage(AString("test"))).Times(1);
    master->broadcastMessage("test");

    master = nullptr;
    EXPECT_EQ(connections(*slave).size(), 0);

    EXPECT_CALL(*slave, die()).Times(1);
}


/**
 * Checks for nested connection.
 */
TEST_F(SignalSlotTest, NestedConnection) {
    slave = _new<Slave>();
    AObject::connect(master->message, slave, [this, slave = slave.get()] (const AString& msg) {
        slave->acceptMessage(msg);
        AObject::connect(master->message, slave, [slave] (const AString& msg) {
            slave->acceptMessage(msg);
        });
    });

    EXPECT_CALL(*slave, acceptMessage(AString("hello"))).Times(3);
    EXPECT_CALL(*slave, die());
    master->broadcastMessage("hello");
    master->broadcastMessage("hello");
}

/**
 * Checks for disconnect functionality.
 */
TEST_F(SignalSlotTest, ObjectDisconnect1) {
    slave = _new<Slave>();
    AObject::connect(master->message, slave, [slave = slave.get()] (const AString& msg) {
        slave->acceptMessage(msg);
        AObject::disconnect();
    });

    EXPECT_CALL(*slave, acceptMessage(AString("hello"))).Times(1);
    EXPECT_CALL(*slave, die());
    master->broadcastMessage("hello");
    master->broadcastMessage("hello");
}

/**
 * Checks for disconnect functionality when one of the signals disconnected.
 */
TEST_F(SignalSlotTest, ObjectDisconnect2) {
    slave = _new<Slave>();

    bool called = false;
    AObject::connect(master->message, slave, [&, slave = slave.get()] (const AString& msg) {
        slave->acceptMessage(msg);
        called = true;
    });

    AObject::connect(master->message, slave, [slave = slave.get()] (const AString& msg) {
        slave->acceptMessage(msg);
        AObject::disconnect();
    });

    EXPECT_CALL(*slave, acceptMessage(AString("hello"))).Times(3);
    EXPECT_CALL(*slave, die());
    master->broadcastMessage("hello");
    called = false;
    master->broadcastMessage("hello");
    EXPECT_TRUE(called);
}


/**
 * Checks for both disconnect and nested connect.
 */
TEST_F(SignalSlotTest, ObjectNestedConnectWithDisconnect) {
    slave = _new<Slave>();

    bool called1 = false;
    bool called2 = false;

    AObject::connect(master->message, slave, [&] (const AString& msg) {
        AObject::disconnect();
        EXPECT_FALSE(called1);
        called1 = true;

        AObject::connect(master->message, slave, [&] (const AString& msg) {
            EXPECT_TRUE(called1);
            EXPECT_FALSE(called2);
            called2 = true;
        });
    });

    master->broadcastMessage("hello");
    master->broadcastMessage("hello");
    EXPECT_CALL(*slave, die());
    EXPECT_TRUE(called1);
    EXPECT_TRUE(called2);
}

/**
 * Destroys master in a signal handler
 */
TEST_F(SignalSlotTest, ObjectDestroyMasterInSignalHandler) {
    slave = _new<Slave>();
    EXPECT_CALL(*slave, die());
    {
        testing::InSequence s;
        AObject::connect(master->message, slave, [&] {
            master = nullptr;
        });
        master->broadcastMessage("hello");
        EXPECT_TRUE(master == nullptr);
    }
}

/**
 * Destroys slave in it's signal handler
 */
TEST_F(SignalSlotTest, ObjectDestroySlaveInSignalHandler) {
    slave = _new<Slave>();
    EXPECT_CALL(*slave, die());
    {
        testing::InSequence s;
        AObject::connect(master->message, slave, [&] {
            slave = nullptr;
        });
        master->broadcastMessage("hello");
        EXPECT_TRUE(slave == nullptr);
    }
}


/*
TEST_F(SignalSlotTest, ObjectRemovalMultithread) {
    static constexpr auto SEND_COUNT = 10000;
    std::mt19937 re;
    re.seed(0);

    AUI_REPEAT(100) {

        class Slave2 : public AObject {
        public:
            Slave2(uint32_t& called) : mCalled(called) {}

            void acceptMessage() {
                mCalled += 1;
            }

        private:
            uint32_t& mCalled;
        };

        uint32_t called = 0;

        auto slave2 = _new<Slave2>(called);

        AObject::connect(master->message, AUI_SLOT(slave2)::acceptMessage);

        auto task = AUI_THREADPOOL {
            master->broadcastMessage("hello");
        };


        for (;;) {
            AThread::processMessages();
            task.wait();
            if (called) {
                break;
            }
        }

        task = AUI_THREADPOOL {
            AUI_REPEAT(SEND_COUNT) master->broadcastMessage("hello");
        };

        auto waitUntil = std::uniform_int_distribution<uint32_t>(0, SEND_COUNT)(re);
        while (called < waitUntil) {}
        slave2 = nullptr; // delete slave; check for crash
        task.wait();
        AThread::processMessages();
    }
}*/

/**
 * Check exception does not break the workflow.
 */
TEST_F(SignalSlotTest, Exception1) {
    slave = _new<Slave>();
    AObject::connect(master->message, slave, [slave = slave.get()] (const AString& msg) {
      throw AException("bruh");
    });
    AObject::connect(master->message, slave, [slave = slave.get()] (const AString& msg) {
        slave->acceptMessage(msg);
    });

    EXPECT_CALL(*slave, acceptMessage(AString("hello"))).Times(1);
    EXPECT_CALL(*slave, die());
    master->broadcastMessage("hello");
}

/**
 * Check exception does not break the workflow.
 */
TEST_F(SignalSlotTest, Exception2) {
    slave = _new<Slave>();
    AObject::connect(master->message, slave, [slave = slave.get()] (const AString& msg) {
        throw AException("bruh");
    });
    AObject::connect(master->message, slave, [slave = slave.get()] (const AString& msg) {
        slave->acceptMessage(msg);
    });

    EXPECT_CALL(*slave, acceptMessage(AString("hello"))).Times(1);
    EXPECT_CALL(*slave, die());

    auto task = AUI_THREADPOOL {
        master->broadcastMessage("hello");
    };
    *task;
}

TEST_F(SignalSlotTest, CopyTest) {
    struct CopyTrap {
    public:
        CopyTrap() {

        }
        CopyTrap(const CopyTrap& c) {
            throw AException("CopyTrap triggered!");
        }
        CopyTrap& operator=(const CopyTrap& c) {
            throw AException("CopyTrap triggered!");
        }
        mutable int value = 0;
    };

    class Sender: public AObject {
    public:
        emits<CopyTrap> copyTrapSignal;
    };

    class Receiver: public AObject {
    public:
        void receive(const CopyTrap& c) {
            c.value = 1;
        }
    };
    Sender s;
    Receiver r;

    AObject::connect(s.copyTrapSignal, AUI_SLOT(r)::receive);

    CopyTrap copyTrap;
    EXPECT_EQ(copyTrap.value, 0);
    s ^ s.copyTrapSignal(copyTrap);
    EXPECT_EQ(copyTrap.value, 1);
}
