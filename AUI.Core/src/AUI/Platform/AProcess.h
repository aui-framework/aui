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

    void run();

    static void executeAsAdministrator(const AString& applicationFile, const AString& args = {}, const APath& workingDirectory = {});
    static void execute(const AString& applicationFile, const AString& args = {}, const APath& workingDirectory = {});
};


