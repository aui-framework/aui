/*
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
    ALogger::info("Process.Self") << mSelf;
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

