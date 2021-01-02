//
// Created by alex2 on 31.10.2020.
//

#include "AProcess.h"

#ifdef _WIN32
#include <windows.h>
#include <AUI/Traits/memory.h>
#include <AUI/Logging/ALogger.h>


int AProcess::execute(const AString& applicationFile, const AString& args, const APath& workingDirectory, bool waitForExit) {
    AProcess p(applicationFile);
    p.setArgs(args);
    p.setWorkingDirectory(workingDirectory);
    p.run();

    if (waitForExit)
        return p.getExitCode();

    return 0;
}

void AProcess::executeAsAdministrator(const AString& applicationFile, const AString& args, const APath& workingDirectory) {
    SHELLEXECUTEINFO sei = { sizeof(sei) };


    sei.lpVerb = L"runas";
    sei.lpFile = applicationFile.c_str();
    sei.lpParameters = args.c_str();
    sei.lpDirectory = workingDirectory.c_str();
    sei.hwnd = NULL;
    sei.nShow = SW_NORMAL;
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;

    if (!ShellExecuteEx(&sei)) {
        AString message = "Could not create process as admin " + applicationFile;
        if (!args.empty())
            message += " with args " + args;
        if (!workingDirectory.empty())
            message += " in " + workingDirectory;
        throw AProcessException(message);
    }
}

void AProcess::run() {
    STARTUPINFO startupInfo;
    aui::zero(startupInfo);
    startupInfo.cb = sizeof(startupInfo);


    if (!CreateProcess(mApplicationFile.c_str(),
                       const_cast<wchar_t*>(mArgs.empty() ? nullptr : mArgs.c_str()),
                       nullptr,
                       nullptr,
                       false,
                       0,
                       nullptr,
                       mWorkingDirectory.empty() ? nullptr : mWorkingDirectory.c_str(),
                       &startupInfo,
                       &mProcessInformation)) {
        AString message = "Could not create process " + mApplicationFile;
        if (!mArgs.empty())
            message += " with args " + mArgs;
        if (!mWorkingDirectory.empty())
            message += " in " + mWorkingDirectory;
        throw AProcessException(message);
    }

}

void AProcess::wait() {
    WaitForSingleObject(mProcessInformation.hProcess, INFINITE);
}

int AProcess::getExitCode() {
    DWORD exitCode;
    wait();
    int r = GetExitCodeProcess(mProcessInformation.hProcess, &exitCode);
    assert(r && r != STILL_ACTIVE);
    return exitCode;
}

#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cassert>

int AProcess::execute(const AString& applicationFile, const AString& args, const APath& workingDirectory, bool waitForExit) {
    assert(0);


    return 0;
}

void AProcess::executeAsAdministrator(const AString& applicationFile, const AString& args, const APath& workingDirectory) {
    assert(0);
}

void AProcess::run() {
    assert(0);
}

void AProcess::wait() {
    assert(0);
}

int AProcess::getExitCode() {
    assert(0);
    return 0;
}
#endif

