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
// Created by alex2 on 31.10.2020.
//

#pragma once


#include <AUI/IO/APath.h>
#include <AUI/Common/AException.h>
#include <AUI/Common/AVector.h>
#include <AUI/Common/AObject.h>
#include <AUI/Common/ASignal.h>
#include <AUI/IO/IInputStream.h>
#include <AUI/IO/IOutputStream.h>
#include <AUI/Thread/AFuture.h>

#if AUI_PLATFORM_WIN
#include <AUI/Platform/win32/AWin32EventWait.h>
#include <AUI/Platform/win32/WinIoAsync.h>
#include <windows.h>
#include "Pipe.h"
#else
#include "AUI/Platform/unix/UnixIoAsync.h"
#endif

class AChildProcess;

/**
 * @brief Flag enum for AChildProcess::run
 * @ingroup core
 */
AUI_ENUM_FLAG(ASubProcessExecutionFlags) {
    /**
     * @brief Merges stdin and stdout streams in a child process
     */
    MERGE_STDOUT_STDERR = 0b001,
    /**
     * @brief If set, child and parent processes have the same stdout stream
     */
    TIE_STDOUT = 0b010,
    /**
     * @brief If set, child and parent processes have the same stderr stream
     */
    TIE_STDERR = 0b100,
    DEFAULT = 0
};

class AProcessException: public AException {
public:
    AProcessException(const AString& message): AException(message) {}
};

/**
 * Retrieves data about processes.
 */
class API_AUI_CORE AProcess: public aui::noncopyable {

public:
    virtual ~AProcess() = default;

    /**
     * @return process' executable file name.
     */
    virtual APath getModuleName() = 0;

    /**
     * @return path to the process' executable.
     */
    virtual APath getPathToExecutable() = 0;

    /**
     * @return process' ID.
     */
    virtual uint32_t getPid() const noexcept = 0;

    /**
     * @brief Wait for process to be finished and returns exit code.
     * @return exit code
     */
    virtual int waitForExitCode() = 0;

    /**
     * @brief Obtain process memory usage.
     */
    virtual size_t processMemory() const = 0;

    /**
     * @brief Launches executable.
     * @param applicationFile executable file
     * @param args arguments
     * @param workingDirectory working directory
     * @return AChildProcess instance
     */
    static _<AChildProcess> make(AString applicationFile,
                                       AString args = {},
                                       APath workingDirectory = {});


    /**
     * @brief Launches executable.
     * @param applicationFile executable file
     * @param args arguments
     * @param workingDirectory working directory
     * @return exit code
     */
    static int executeWaitForExit(AString applicationFile,
                                  AString args = {},
                                  APath workingDirectory = {},
                                  ASubProcessExecutionFlags flags = ASubProcessExecutionFlags::DEFAULT);


    /**
     * @brief Launches executable with administrator rights.
     * @param applicationFile executable file
     * @param args arguments
     * @param workingDirectory pro
     * @note This function could not determine exit code because of MS Windows restrictions
     */
    static void executeAsAdministrator(const AString& applicationFile,
                                      const AString& args = {},
                                      const APath& workingDirectory = {});

    /**
     * @return data about all other running processes.
     */
    static AVector<_<AProcess>> all();

    /**
     * @return data about this process.
     */
    static _<AProcess> self();

    /**
     * @brief tempFileName file name which will be used as lock
     * @return another instance of this application; nullptr, if not found
     */
    static _<AProcess> findAnotherSelfInstance(const AString& yourProjectName);


    /**
     * @return process by id
     */
    static _<AProcess> fromPid(uint32_t pid);

    void kill() const noexcept;
};

/**
 * Creates child process of this application.
 */
class API_AUI_CORE AChildProcess: public AProcess, public AObject {
friend class AProcess;
public:
    AChildProcess() = default;
    ~AChildProcess();
    
    void setApplicationFile(AString applicationFile) noexcept {
        mApplicationFile = std::move(applicationFile);
    }

    void setArgs(AString args) noexcept {
        mArgs = std::move(args);
    }

    void setWorkingDirectory(APath workingDirectory) noexcept {
        mWorkingDirectory = std::move(workingDirectory);
    }

    [[nodiscard]]
    const AString& getApplicationFile() const {
        return mApplicationFile;
    }

    [[nodiscard]]
    const AString& getArgs() const {
        return mArgs;
    }

    [[nodiscard]]
    const APath& getWorkingDirectory() const {
        return mWorkingDirectory;
    }

    APath getPathToExecutable() override;

    [[nodiscard]]
    AOptional<int> exitCodeNonBlocking() const noexcept {
        if (mExitCode.hasValue()) {
            return *mExitCode;
        }
        return std::nullopt;
    }

    [[nodiscard]]
    bool isFinished() const noexcept {
        return mExitCode.hasValue();
    }

    /**
     * @brief Launches process.
     */
    void run(ASubProcessExecutionFlags flags = ASubProcessExecutionFlags::DEFAULT);

    /**
     * @brief Wait for process to be finished.
     * @return exit code
     */
    int waitForExitCode() override;

    uint32_t getPid() const noexcept override;

    size_t processMemory() const override;

    APath getModuleName() override;

    [[nodiscard]]
    const _<IOutputStream>& getStdInStream() const {
        return mStdInStream;
    }

signals:
    emits<> finished;
    emits<AByteBuffer> stdOut;
    emits<AByteBuffer> stdErr;

private:
    AString mApplicationFile;
    AString mArgs;
    APath mWorkingDirectory;

    _<IOutputStream> mStdInStream;

    AFuture<int> mExitCode;
#if AUI_PLATFORM_WIN
    PROCESS_INFORMATION mProcessInformation;
    AWin32EventWait mExitEvent;
    WinIoAsync mStdoutAsync;
#else
    pid_t mPid;
    _<AThread> mWatchdog;
    UnixIoAsync mStdoutAsync;
#endif
};


