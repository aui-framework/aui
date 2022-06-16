/**
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
#include <AUI/Platform/win32/WinEventHandle.h>
#include <AUI/Platform/win32/WinIoAsync.h>
#include <windows.h>
#include "Pipe.h"
#else
#include "AUI/Platform/unix/UnixIoAsync.h"
#endif

class AChildProcess;

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
     * \return process' executable file name.
     */
    virtual APath getModuleName() = 0;

    /**
     * \return path to the process' executable.
     */
    virtual APath getPathToExecutable() = 0;

    /**
     * \return process' ID.
     */
    virtual uint32_t getPid() const noexcept = 0;

    /**
     * \brief Wait for process to be finished and returns exit code.
     * \return exit code
     */
    virtual int waitForExitCode() = 0;


    /**
     * \brief Launches executable.
     * \param applicationFile executable file
     * \param args arguments
     * \param workingDirectory working directory
     * \return AChildProcess instance
     */
    static _<AChildProcess> make(AString applicationFile,
                                       AString args = {},
                                       APath workingDirectory = {});


    /**
     * \brief Launches executable.
     * \param applicationFile executable file
     * \param args arguments
     * \param workingDirectory working directory
     * \return exit code
     */
    static int executeWaitForExit(AString applicationFile,
                                  AString args = {},
                                  APath workingDirectory = {});


    /**
     * \brief Launches executable with administrator rights.
     * \param applicationFile executable file
     * \param args arguments
     * \param workingDirectory pro
     * \note This function could not determine exit code because of MS Windows restrictions
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

AUI_ENUM_FLAG(ASubProcessExecutionFlags) {
    MERGE_STDOUT_STDERR = 0b1,
    DEFAULT = 0
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
    std::optional<int> exitCodeNonBlocking() const noexcept {
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
     * \brief Launches process.
     */
    void run(ASubProcessExecutionFlags flags = ASubProcessExecutionFlags::DEFAULT);

    /**
     * \brief Wait for process to be finished.
     * \return exit code
     */
    int waitForExitCode() override;

    uint32_t getPid() const noexcept override;

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
    WinEventHandle mExitEvent;
    WinIoAsync mStdoutAsync;
#else
    pid_t mPid;
    _<AThread> mWatchdog;
    UnixIoAsync mStdoutAsync;
#endif
};


