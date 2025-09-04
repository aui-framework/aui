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

#include <gtest/gtest.h>
#include <AUI/Common/AVector.h>
#include <AUI/Common/AByteBuffer.h>
#include <AUI/Traits/iterators.h>
#include <array>
#include "AUI/Platform/AProcess.h"
#include "AUI/Util/kAUI.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Util/Util.h"
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


#if !AUI_PLATFORM_WIN
// compilation breaks on older msvc
TEST_F(ProcessTest, ExitCode) {
    auto process = AProcess::create({
                                        .executable = mSelf,
                                        .args = AProcess::ArgStringList { { "--help"} },
                                    });
    process->run();
    EXPECT_EQ(process->waitForExitCode(), 0);
}
#endif

TEST_F(ProcessTest, Stdout) {
    for (const auto& i : info()) {
        auto p = AProcess::create(i);

        AString accumulator;
        AObject::connect(p->stdOut, p, [&](const AByteBuffer& buffer) { accumulator += AString::fromUtf8(buffer); });
        p->run();
        EXPECT_EQ(p->waitForExitCode(), 0);
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
        AObject::connect(p->finished, AUI_SLOT(receiver)::slotMock);
        p->run();
        p->waitForExitCode();

        AThread::sleep(500ms);

        AThread::processMessages();

        receiver = nullptr;
    }
}


TEST_F(ProcessTest, StdoutSignal) {
    for (const auto& i : info()) {
        auto p = AProcess::create(i);
        auto receiver = _new<ProcessSignalReceiver>();
        EXPECT_CALL(*receiver, slotMock()).Times(testing::AtLeast(1));
        AObject::connect(p->stdOut, AUI_SLOT(receiver)::slotMock);
        p->run();
        p->waitForExitCode();

        AThread::sleep(500ms);

        AThread::processMessages();

        receiver = nullptr;
    }
}

#if AUI_PLATFORM_LINUX
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

TEST_F(ProcessTest, StartDetachedBad) {
    AFileOutputStream("text.txt") << "test";
    auto p = AProcess::create({
      .executable = "test.txt",
    });
    EXPECT_ANY_THROW(p->run(ASubProcessExecutionFlags::DETACHED));
}

TEST_F(ProcessTest, StartDetachedSleep) {
    auto p = AProcess::create({
                                  .executable = "/usr/bin/sleep",
                                  .args = AProcess::ArgStringList{ { "5" } },
                              });
    auto runtime = util::measureExecutionTime<std::chrono::milliseconds>([&] {
        p->run(ASubProcessExecutionFlags::DETACHED);
    });
    EXPECT_LE(runtime, std::chrono::seconds(1));
}


TEST_F(ProcessTest, Launching_executable) { // HEADER_H2
    // To start a process, pass the name of application you want to run and optionally provide arguments and working dir
    // for that application. In this code snippet, we are starting another instance of the current executable with
    // specific arguments and capturing its standard output (stdOut).
    // AUI_DOCS_CODE_BEGIN
    auto self = AProcess::self()->getPathToExecutable();
    AProcess::ArgStringList args;
    args.list << "--help";
    args.list << "-a";
    auto p = AProcess::create({
      .executable = self,
      .args = std::move(args),
      .workDir = self.parent(),
    });

    AString accumulator;
    AObject::connect(p->stdOut, p, [&](const AByteBuffer& buffer) { accumulator += AString::fromUtf8(buffer); });

    p->run();

    EXPECT_EQ(p->waitForExitCode(), 0);
    AThread::processMessages(); // HIDE
    EXPECT_TRUE(accumulator.contains("This program contains tests written using Google Test.")) << accumulator;
    // AUI_DOCS_CODE_END
    //
    // We define an empty string accumulator to collect the output from the process. Then, we connect a lambda function
    // to the `stdOut` signal of the process. This lambda function converts the received buffer (a byte array) to a
    // UTF-8 string and appends it to `accumulator`.
    //
    // We start the new process by calling its `AProcess::run()` method, which will execute the specified application
    // with the provided arguments in the given working directory.
    //
    // We wait for the process to finish by calling `waitForExitCode()`, which blocks until the process exits and
    // returns its exit code. If the exit code is 0, it means the process completed successfully.
}
#endif
