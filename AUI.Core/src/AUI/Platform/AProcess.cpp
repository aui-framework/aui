//
// Created by alex2 on 31.10.2020.
//

#include "AProcess.h"

#ifdef _WIN32
#include <windows.h>
#include <AUI/Traits/memory.h>

void AProcess::executeAsAdministrator(const AString& applicationFile, const AString& args, const APath& workingDirectory) {
    ShellExecute(nullptr, L"runas", applicationFile.c_str(), args.c_str(),
                 workingDirectory.empty() ? nullptr : workingDirectory.c_str(), SW_SHOWNORMAL);
}

void AProcess::execute(const AString& applicationFile, const AString& args, const APath& workingDirectory) {
    AProcess p(applicationFile);
    p.setArgs(args);
    p.setWorkingDirectory(workingDirectory);
    p.run();
}

void AProcess::run() {
    STARTUPINFO startupInfo;
    aui::zero(startupInfo);
    startupInfo.cb = sizeof(startupInfo);

    PROCESS_INFORMATION pi;

    if (!CreateProcess(mApplicationFile.c_str(),
                       const_cast<wchar_t*>(mArgs.empty() ? nullptr : mArgs.c_str()),
                       nullptr,
                       nullptr,
                       false,
                       0,
                       nullptr,
                       mWorkingDirectory.empty() ? nullptr : mWorkingDirectory.c_str(),
                       &startupInfo,
                       &pi)) {
        AString message = "Could not create process " + mApplicationFile;
        if (!mArgs.empty())
            message += " with args " + mArgs;
        if (!mWorkingDirectory.empty())
            message += " in " + mWorkingDirectory;
        throw AProcessException(message);
    }
    WaitForSingleObject(pi.hProcess, 0);
}

#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cassert>

void AProcess::execute(const AString& command, const AString& args, const APath& workingDirectory) {
    assert(0);
}

#endif

