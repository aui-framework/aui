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

#if defined(_WIN32)
#include <windows.h>
#endif

class AProcessException: public AException {
public:
    AProcessException(const AString& message): AException(message) {}
};

/**
 * Retrieves data about processes.
 */
class API_AUI_CORE AProcess {

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
    virtual uint32_t getPid() = 0;

    /**
     * \brief Wait for process to be finished and returns exit code.
     * \return exit code
     */
    virtual int wait() = 0;


    /**
     * \brief Launches executable.
     * \param applicationFile executable file
     * \param args arguments
     * \param workingDirectory pro
     * \param waitForExit should function wait for exit. If false, function will return zero because exit code of
     *        process is unknown during it's execution
     * \return exit code if waitForExit = true, zero otherwise
     */
    static int execute(const AString& applicationFile,
                       const AString& args = {},
                       const APath& workingDirectory = {},
                       bool waitForExit = true);


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
};

/**
 * Creates child process of this application.
 */
class AChildProcess: public AProcess {
friend class AProcess;
private:
    AString mApplicationFile;
    AString mArgs;
    APath mWorkingDirectory;

#if defined(_WIN32)
    PROCESS_INFORMATION mProcessInformation;
#else
    pid_t mPid;

    int mPipes[2];
#endif

public:
    AChildProcess(const AString& applicationFile) : mApplicationFile(applicationFile) {}
    const AString& getApplicationFile() const {
        return mApplicationFile;
    }

    const AString& getArgs() const {
        return mArgs;
    }

    void setArgs(const AString& args) {
        mArgs = args;
    }

    const APath& getWorkingDirectory() const {
        return mWorkingDirectory;
    }

    void setWorkingDirectory(const APath& workingDirectory) {
        mWorkingDirectory = workingDirectory;
    }

    APath getPathToExecutable() override;

    /**
     * \brief Launches process.
     */
    void run();

    /**
     * \brief Wait for process to be finished.
     * \return exit code
     */
    int wait() override;

    uint32_t getPid() override;

    APath getModuleName() override;
};


