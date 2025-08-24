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
#include "AUI/Common/IStringable.h"

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
    TIE_STDERR = 0b100, DEFAULT = 0,

    /**
     * @brief If set, child process starts in "detached" way; i.e, when this process dies, child won't.
     * @details
     * On *nix systems, the process started with DETACHED flag is daemonized (i.e., reparented to process with pid 1)
     * using double fork technique.
     */
    DETACHED = 0b1000,
};

class AProcessException : public AException {
public:
    AProcessException(const AString& message) : AException(message) {}
};

/**
 * @brief Retrieves information about processes.
 * @ingroup core
 * @details
 * Process model that facilitates process creation, management, and interaction with other processes.
 *
 * In a sandboxed environment (especially in iOS and Android) this functionality is mostly irrelevant (except
 * `AProcess::self()`).
 *
 * The AProcess class is typically used for creating, controlling, and monitoring subprocesses (including
 * [self](AProcess::self)) in a platform-independent manner. It provides a way to run external applications from
 * within the application itself, which can be useful for tasks like running scripts, launching other programs, or
 * automating system operations through commands.
 *
 * <!-- aui:parse_tests aui.core/tests/ProcessTest.cpp -->
 */
class API_AUI_CORE AProcess : public aui::noncopyable {
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
     * @brief Process arguments represented as a single string.
     * @details
     * In general, prefer using AProcess::Args.
     */
    struct ArgSingleString {
        AString arg;
    };

    /**
     * @brief Process arguments represented as array of strings.
     */
    struct ArgStringList {
        /**
         * @details
         * Argument list.
         */
        AStringVector list;

        /**
         * @details
         * @specificto{windows}
         * Takes action only on Windows platform.
         *
         * If true, during conversion to a single command line string on Windows platforms elements of list containing
         * whitespaces are wrapped with quots escaping existing quots. As it's the only way on Windows platforms to
         * supply paths with whitespaces, executables generally handle these quots properly.
         *
         * If it does not work for your particular case, you may try setting this to false or use
         * AProcess::ArgSingleString to take full control of command line during process creation.
         *
         * Defaults to true.
         */
        bool win32WrapWhitespaceArgumentsWithQuots = true;
    };

    /**
     * @brief Process creation info.
     */
    struct ProcessCreationInfo {
        /**
         * @details Target executable file. Mandatory.
         */
        APath executable;

        /**
         * @details
         * Child process arguments.
         *
         * In common, prefer Args variant.
         *
         * Unix native APIs use arguments as array of strings. If ArgSingleString variant is chosen, AUI splits it with
         * whitespaces.
         *
         * @specificto{windows}
         * Windows native APIs use arguments as a single string. If ArgStringList variant is chosen, AUI converts array
         * of strings to a single command line string value. See ArgStringList for details of this conversion.
         */
        std::variant<ArgSingleString, ArgStringList> args;

        /**
         * @details
         * Process working directory. Defaults to working directory of the calling process.
         */
        APath workDir;
    };

    /**
     * @brief Launches an executable.
     * @param args designated-initializer-style args. See ProcessCreationInfo
     * @return AChildProcess instance. Use AChildProcess::run to execute.
     */
    static _<AChildProcess> create(ProcessCreationInfo args);

    /**
     * @brief Launches an executable.
     * @param applicationFile executable file
     * @param args arguments
     * @param workingDirectory working directory
     * @return AChildProcess instance. Use AChildProcess::run to execute.
     */
    [[deprecated("use AProcess::create instead")]]
    static _<AChildProcess> make(AString applicationFile, AString args = {}, APath workingDirectory = {}) {
        return create(
            { .executable = std::move(applicationFile),
              .args = ArgSingleString { std::move(args) },
              .workDir = std::move(workingDirectory) });
    }

    /**
     * @brief Launches executable.
     * @param applicationFile executable file.
     * @param args arguments.
     * @param workingDirectory working directory.
     * @param flags process execution flags. see ASubProcessExecutionFlags.
     * @return exit code
     */
    [[deprecated("use auto process = AProcess::make(); process->run(); process->waitForExitCode()")]]
    static int executeWaitForExit(
        AString applicationFile, AString args = {}, APath workingDirectory = {},
        ASubProcessExecutionFlags flags = ASubProcessExecutionFlags::DEFAULT);

#if AUI_PLATFORM_WIN

    /**
     * @brief Launches executable with administrator rights. (Windows only)
     * @param applicationFile executable file
     * @param args arguments
     * @param workingDirectory pro
     * @details
     * This function could not determine exit code because of MS Windows restrictions
     * @exclusivefor{windows}
     */
    static void executeAsAdministrator(
        const AString& applicationFile, const AString& args = {}, const APath& workingDirectory = {});

#endif

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
     * @details
     * This function might cause race condition if process is about to die. If process is not found, `nullptr` is
     * returned so you must check for `nullptr` before proceeding. However, if non-`nullptr` is returned, the process
     * handle is "acquired" and guaranteed to be valid during lifetime of `AProcess` instance.
     *
     * @return process by id
     */
    static _<AProcess> fromPid(uint32_t pid);

    void kill() const noexcept;
};

/**
 * Creates child process of this application.
 */
class API_AUI_CORE AChildProcess : public AProcess, public AObject, public IStringable {
    friend class AProcess;

public:
    ~AChildProcess();

    [[nodiscard]]
    const auto& getApplicationFile() const {
        return mInfo.executable;
    }

    [[nodiscard]]
    const auto& getArgs() const {
        return mInfo.args;
    }

    [[nodiscard]]
    const auto& getWorkingDirectory() const {
        return mInfo.workDir;
    }

    APath getPathToExecutable() override {
        return getApplicationFile();
    }

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
    AString toString() const override;

signals:
    emits<> finished;
    emits<AByteBuffer> stdOut;
    emits<AByteBuffer> stdErr;

private:
    AChildProcess() = default;
    ProcessCreationInfo mInfo;

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
