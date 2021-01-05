//
// Created by alex2 on 31.10.2020.
//

#pragma once


#include <AUI/IO/APath.h>
#include <AUI/Common/AException.h>

#if defined(_WIN32)
#include <windows.h>
#endif

class AProcessException: public AException {
public:
    AProcessException(const AString& message): AException(message) {}
};

class API_AUI_CORE AProcess {
private:
    AString mApplicationFile;
    AString mArgs;
    APath mWorkingDirectory;

#if defined(_WIN32)
    PROCESS_INFORMATION mProcessInformation;
#endif

public:
    AProcess(const AString& applicationFile) : mApplicationFile(applicationFile) {}
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

    /**
     * \brief Launches process.
     */
    void run();

    /**
     * \brief Wait for process to be finished.
     */
    void wait();

    /**
     * \brief Wait for process to be finished and returns exit code
     * \return exit code
     */
    int getExitCode();

    /**
     * \brief Launches executable
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
     * \brief Launches executable with administrator rights
     * \param applicationFile executable file
     * \param args arguments
     * \param workingDirectory pro
     * \note This function could not determine exit code because of MS Windows restrictions
     */
    static void executeAsAdministrator(const AString& applicationFile,
                                      const AString& args = {},
                                      const APath& workingDirectory = {});
};


