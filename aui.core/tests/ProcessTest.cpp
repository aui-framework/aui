/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
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
#include <AUI/Common/AVector.h>
#include <AUI/Common/AByteBuffer.h>
#include <AUI/Traits/iterators.h>
#include <array>
#include "AUI/Platform/AProcess.h"
#include "AUI/Util/kAUI.h"
#include "AUI/Logging/ALogger.h"
#include <gmock/gmock.h>

using namespace std::chrono_literals;

class ProcessTest : public ::testing::Test {
protected:
    APath mSelf;

    void SetUp() override {
        Test::SetUp();
        mSelf = AProcess::self()->getPathToExecutable();
    }

    auto info() {
        AProcess::ProcessCreationInfo info[] = {
            {
              .executable = mSelf,
              .args = AProcess::ArgSingleString { "--help -a" },
            },
            {
              .executable = mSelf,
              .args = AProcess::ArgStringList { { "--help", "-a" } },
            },
        };
        return std::to_array(info);
    }
};

TEST_F(ProcessTest, Self) {
#if AUI_PLATFORM_WIN
    EXPECT_EQ(mSelf.filename(), "Tests.exe");
#else
    EXPECT_EQ(mSelf.filename(), "Tests");
#endif
    EXPECT_TRUE(mSelf.isAbsolute());
    EXPECT_TRUE(mSelf.isRegularFileExists());
    EXPECT_TRUE(mSelf.isEffectivelyAccessible(AFileAccess::X));
}

TEST_F(ProcessTest, ExitCode) {
    EXPECT_EQ(AProcess::executeWaitForExit(mSelf, "--help"), 0);
}

TEST_F(ProcessTest, Stdout) {
    for (const auto& i : info()) {
        auto p = AProcess::create(i);

        AString accumulator;
        AObject::connect(p->stdOut, p, [&](const AByteBuffer& buffer) { accumulator += AString::fromUtf8(buffer); });
        p->run();
        p->waitForExitCode();
        AThread::processMessages();
        EXPECT_TRUE(accumulator.contains("This program contains tests written using Google Test.")) << accumulator;
    }
}

class ProcessSignalReceiver: public AObject {
public:
    MOCK_METHOD(void, slotMock, ());
};

TEST_F(ProcessTest, FinishedSignal) {
    for (const auto& i : info()) {
        auto p = AProcess::create(i);
        auto receiver = _new<ProcessSignalReceiver>();
        EXPECT_CALL(*receiver, slotMock()).Times(1);
        AObject::connect(p->finished, slot(receiver)::slotMock);
        p->run();
        p->waitForExitCode();

        AThread::sleep(500ms);

        AThread::processMessages();

        receiver = nullptr;   // gmock wants object to be removed
    }
}


TEST_F(ProcessTest, StdoutSignal) {
    for (const auto& i : info()) {
        auto p = AProcess::create(i);
        auto receiver = _new<ProcessSignalReceiver>();
        EXPECT_CALL(*receiver, slotMock()).Times(testing::AtLeast(1));
        AObject::connect(p->stdOut, slot(receiver)::slotMock);
        p->run();
        p->waitForExitCode();

        AThread::sleep(500ms);

        AThread::processMessages();

        receiver = nullptr;   // gmock wants object to be removed
    }
}

TEST_F(ProcessTest, StartDetached) {
    for (const auto& i : info()) {
        auto p = AProcess::create(i);

        AString accumulator;
        AObject::connect(p->stdOut, p, [&](const AByteBuffer& buffer) { accumulator += AString::fromUtf8(buffer); });
        p->run(ASubProcessExecutionFlags::DETACHED);
        p->waitForExitCode();
        AThread::sleep(500ms);
        AThread::processMessages();
        EXPECT_TRUE(accumulator.contains("This program contains tests written using Google Test.")) << accumulator;
    }
    EXPECT_TRUE(AProcess::self()->getPathToExecutable().isEffectivelyAccessible(AFileAccess::X));
}
