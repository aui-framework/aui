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
// Created by alex2 on 31.08.2020.
//

#include <gtest/gtest.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Common/AString.h>
#include <AUI/Common/AVector.h>
#include <AUI/Common/ASignal.h>
#include <AUI/Reflect/AClass.h>
#include <AUI/Common/AByteBuffer.h>


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


TEST(SharedPtrs, Builder1) {
    auto builder = _new<SomeBuilderClass>();
    AUI_PERFORM_AS_MEMBER(*builder, {
       setUsername("John");
       setAge(23);
    });

    ASSERT_EQ(builder->getUsername(), "John");
    ASSERT_EQ(builder->getAge(), 23);
}

TEST(SharedPtrs, Builder2) {
    auto builder = _new<SomeBuilderClass>() AUI_LET {
        it->setUsername("John");
        it->setAge(23);
    };


    ASSERT_EQ(builder->getUsername(), "John");
    ASSERT_EQ(builder->getAge(), 23);
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

TEST(SharedPtrs, ConnectBuilder) {
    auto receiver = _new<ReceiverObject>();

    auto sender = _new<SendObject>()
            .connect(&SendObject::someSignal, receiver, &ReceiverObject::receiveSignal);


    AUI_PERFORM_AS_MEMBER(*sender, {
        invokeSignal();
    });

    ASSERT_TRUE(receiver->mSignalInvoked);
}

TEST(SharedPtrs, Performance1) {
    AUI_REPEAT (1'000) {
        AByteBuffer buffer;
        AUI_REPEAT (10'000) {
            buffer << uint8_t(228);
        }
        _<AByteBuffer> shared_ptr = _new<AByteBuffer>(buffer);
    }
}

