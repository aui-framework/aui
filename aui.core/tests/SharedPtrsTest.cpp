// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
    AUI_PERFORM_AS_MEMBER(builder, {
       setUsername("John");
       setAge(23);
    });

    ASSERT_EQ(builder->getUsername(), "John");
    ASSERT_EQ(builder->getAge(), 23);
}

TEST(SharedPtrs, Builder2) {
    auto builder = _new<SomeBuilderClass>() let {
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


    AUI_PERFORM_AS_MEMBER(sender, {
        invokeSignal();
    });

    ASSERT_TRUE(receiver->mSignalInvoked);
}

TEST(SharedPtrs, Performance1) {
    repeat (1'000) {
        AByteBuffer buffer;
        repeat (10'000) {
            buffer << uint8_t(228);
        }
        _<AByteBuffer> shared_ptr = _new<AByteBuffer>(buffer);
    }
}

