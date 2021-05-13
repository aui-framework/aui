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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 31.10.2020.
//

#include "AProcess.h"

#ifdef _WIN32
#include <windows.h>
#include <AUI/Traits/memory.h>
#include <AUI/Logging/ALogger.h>
#include <psapi.h>
#include <AUI/IO/FileInputStream.h>
#include <AUI/Util/ATokenizer.h>


int AProcess::execute(const AString& applicationFile, const AString& args, const APath& workingDirectory, bool waitForExit) {
    AChildProcess p(applicationFile);
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

class AOtherProcess: public AProcess {
private:
    HANDLE mHandle;

public:
    AOtherProcess(HANDLE handle) : mHandle(handle) {}

    ~AOtherProcess() {
        CloseHandle(mHandle);
    }

    void wait() override {
        WaitForSingleObject(mHandle, INFINITE);
    }

    int getExitCode() override {
        DWORD exitCode;
        wait();
        int r = GetExitCodeProcess(mHandle, &exitCode);
        assert(r && r != STILL_ACTIVE);
        return exitCode;
    }

    APath getModuleName() override {
        wchar_t buf[0x800];
        GetProcessImageFileName(mHandle, buf, sizeof(buf));
        return APath(buf).filename();
    }

    uint32_t getPid() override {
        return GetProcessId(mHandle);
    }
};

AVector<_<AProcess>> AProcess::all() {
    // get the list of process identifiers.
    DWORD pids[4096], processCountInBytes, processCount;
    unsigned int i;

    if (!EnumProcesses(pids, sizeof(pids), &processCountInBytes))
    {
        throw AException("could not retrieve process data");
    }


    // calculate how many process identifiers were returned.
    processCount = processCountInBytes / sizeof(DWORD);

    AVector<_<AProcess>> result;
    for ( i = 0; i < processCount; i++ )
    {
        if(pids[i] != 0 )
        {
            if (auto p = fromPid(pids[i])) {
                result << p;
            }
        }
    }

    return result;
}

_<AProcess> AProcess::self() {
    return _new<AOtherProcess>(GetCurrentProcess());
}

void AChildProcess::run() {
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

void AChildProcess::wait() {
    WaitForSingleObject(mProcessInformation.hProcess, INFINITE);
}

int AChildProcess::getExitCode() {
    DWORD exitCode;
    wait();
    int r = GetExitCodeProcess(mProcessInformation.hProcess, &exitCode);
    assert(r && r != STILL_ACTIVE);
    return exitCode;
}

_<AProcess> AProcess::fromPid(uint32_t pid) {
    auto handle = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, pid);
    if (handle) {
        return _new<AOtherProcess>(handle);
    }
    return nullptr;
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



_<AProcess> AProcess::findAnotherSelfInstance(const AString& yourProjectName) {
    // try to find in task list
    auto list = all();
    auto s = self();
    auto name = s->getModuleName();
    auto pid = s->getPid();
    auto it = std::find_if(list.begin(), list.end(), [&](const _<AProcess>& v) {
        return v->getModuleName() == name && pid != v->getPid();
    });
    if (it != list.end()) {
        return *it;
    }

    // try to find by tmp file
    auto f = APath::getDefaultPath(APath::TEMP)["." + yourProjectName + ".pid"];
    try {
        ATokenizer t(_new<FileInputStream>(f));
        auto p = t.readInt();

        return AProcess::fromPid(p);
    } catch (...) {}

    try {
        struct RemoveHelper {
        private:
            APath mPath;

        public:
            RemoveHelper(APath&& path) : mPath(std::forward<APath>(path)) {
                auto fos = _new<FileOutputStream>(mPath);

                auto n = std::to_string(self()->getPid());
                fos->write(n.c_str(), n.length());
                fos->close();
            }

            ~RemoveHelper() {
                try {
                    mPath.removeFile();
                } catch (...) {

                }
            }
        };
        static RemoveHelper rh(std::move(f));
    } catch (...) {

    }


    return nullptr;
}


APath AChildProcess::getModuleName() {
    return APath(mApplicationFile).filename();
}

uint32_t AChildProcess::getPid() {
    return mProcessInformation.dwProcessId;
}
