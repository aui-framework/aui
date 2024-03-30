// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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
#include <AUI/Common/AVector.h>
#include <AUI/Common/AByteBuffer.h>
#include <AUI/Traits/iterators.h>
#include <array>
#include "AUI/Platform/AProcess.h"
#include "AUI/Util/kAUI.h"
#include "AUI/Logging/ALogger.h"
#include <gmock/gmock.h>

using namespace std::chrono_literals;

class Process : public ::testing::Test {
protected:
    APath mSelf;

    void SetUp() override {
        Test::SetUp();
        mSelf = AProcess::self()->getPathToExecutable();
    }
};

TEST_F(Process, Self) {
#if AUI_PLATFORM_WIN
    EXPECT_EQ(mSelf.filename(), "Tests.exe");
#else
    EXPECT_EQ(mSelf.filename(), "Tests");
#endif
    EXPECT_TRUE(mSelf.isAbsolute());
    EXPECT_TRUE(mSelf.isRegularFileExists());
}

TEST_F(Process, ExitCode) {
    EXPECT_EQ(AProcess::executeWaitForExit(mSelf, "--help"), 0);
}

TEST_F(Process, Stdout) {
    auto p = AProcess::make(mSelf, "--help");

    AString accumulator;
    AObject::connect(p->stdOut, p, [&](const AByteBuffer& buffer) {
        accumulator += AString::fromUtf8(buffer);
    });
    p->run();
    p->waitForExitCode();
    AThread::processMessages();
    EXPECT_TRUE(accumulator.contains("This program contains tests written using Google Test."));
}

class ProcessSignalReceiver: public AObject {
public:
    MOCK_METHOD(void, slotMock, ());
};

TEST_F(Process, FinishedSignal) {
    auto receiver = _new<ProcessSignalReceiver>();
    EXPECT_CALL(*receiver, slotMock()).Times(1);
    auto p = AProcess::make(mSelf, "--help");
    AObject::connect(p->finished, slot(receiver)::slotMock);
    p->run();
    p->waitForExitCode();

    AThread::sleep(500ms);

    AThread::processMessages();

    receiver = nullptr; // gmock wants object to be removed
}


TEST_F(Process, StdoutSignal) {
    auto receiver = _new<ProcessSignalReceiver>();
    EXPECT_CALL(*receiver, slotMock()).Times(testing::AtLeast(1));
    auto p = AProcess::make(mSelf, "--help");
    AObject::connect(p->stdOut, slot(receiver)::slotMock);
    p->run();
    p->waitForExitCode();

    AThread::sleep(500ms);

    AThread::processMessages();

    receiver = nullptr; // gmock wants object to be removed
}

