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

#include "AProcess.h"
#include "AUI/IO/AFileOutputStream.h"

#if AUI_PLATFORM_WIN
#include <windows.h>
#include <AUI/Traits/memory.h>
#include <AUI/Logging/ALogger.h>
#include <psapi.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/Util/ATokenizer.h>


int AProcess::execute(const AString& applicationFile, const AString& args, const APath& workingDirectory, bool waitForExit) {
    AChildProcess p(applicationFile);
    p.setArgs(args);
    p.setWorkingDirectory(workingDirectory);
    p.run();

    if (waitForExit)
        return p.wait();

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

    APath getPathToExecutable() override {
        wchar_t buf[0x800];
        GetProcessImageFileName(mHandle, buf, sizeof(buf));
        return APath(buf).filename();
    }

    int wait() override {
        WaitForSingleObject(mHandle, INFINITE);
        DWORD exitCode;
        wait();
        int r = GetExitCodeProcess(mHandle, &exitCode);
        assert(r && r != STILL_ACTIVE);
        return exitCode;
    }

    APath getModuleName() override {
        wchar_t buf[0x800];
        GetProcessImageFileName(mHandle, buf, sizeof(buf));
        return getPathToExecutable().filename();
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


    AString commandLine = "\"" + mApplicationFile + "\" " + mArgs;
    if (!CreateProcess(nullptr,
                       const_cast<wchar_t*>(commandLine.c_str()),
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

int AChildProcess::wait() {
    WaitForSingleObject(mProcessInformation.hProcess, INFINITE);
    DWORD exitCode;
    int r = GetExitCodeProcess(mProcessInformation.hProcess, &exitCode);
    assert(r && r != STILL_ACTIVE);
    return exitCode;
}


_<AProcess> AProcess::fromPid(uint32_t pid) {
    auto handle = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, pid);
    if (handle) {
        DWORD exitCode;
        GetExitCodeProcess(handle, &exitCode);
        if (exitCode == STILL_ACTIVE) {
            return _new<AOtherProcess>(handle);
        }
    }
    return nullptr;
}

uint32_t AChildProcess::getPid() {
    return mProcessInformation.dwProcessId;
}


#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cassert>
#include <spawn.h>
#include <AUI/Common/AStringVector.h>
#include <cstring>
#include <AUI/Util/ATokenizer.h>
#include <AUI/IO/AFileOutputStream.h>
#include <AUI/IO/AFileInputStream.h>

class AOtherProcess: public AProcess {
private:
    pid_t mHandle;

public:
    AOtherProcess(pid_t handle) : mHandle(handle) {}

    ~AOtherProcess() {
    }

    int wait() override {
        int loc;
        waitpid(mHandle, &loc, 0);
        return WEXITSTATUS(loc);
    }

    APath getModuleName() override {
        return getPathToExecutable().filename();














    }

    APath getPathToExecutable() override {
        char buf[0x800];
        char path[0x100];
        sprintf(path, "/proc/%u/exe", mHandle);
        readlink(path, buf, sizeof(buf));
        return APath(buf);
    }

    uint32_t getPid() override {
        return mHandle;
    }
};

AVector<_<AProcess>> AProcess::all() {
    AVector<_<AProcess>> result;
    for (auto& f : APath("/proc/").listDir(ListFlags::DIRS)) {
        pid_t p = f.filename().toUInt();
        if (p != 0) {
            result << _new<AOtherProcess>(p);
        }
    }
    return result;
}

_<AProcess> AProcess::self() {
    char buf[0x100];
    readlink("/proc/self", buf, sizeof(buf));
    return _new<AOtherProcess>(AString(buf).toUInt());
}

_<AProcess> AProcess::fromPid(uint32_t pid) {
    if (APath("/proc")[AString::number(pid)].isDirectoryExists()) {
        return _new<AOtherProcess>(pid_t(pid));
    }
    return nullptr;
}

int AProcess::execute(const AString& applicationFile, const AString& args, const APath& workingDirectory, bool waitForExit) {
    AChildProcess p(applicationFile);
    p.setArgs(args);
    p.setWorkingDirectory(workingDirectory);
    p.run();

    if (waitForExit) {
        auto f = fdopen(p.mPipes[0],"r");
        char buf[0x1000];
        int r;
        while ((r = fread(buf, 1, sizeof(buf), f)) > 0) {
            write(STDOUT_FILENO, buf, r);
        }

        fclose(f);
        return p.wait();
    }

    return 0;
}

void AProcess::executeAsAdministrator(const AString& applicationFile, const AString& args, const APath& workingDirectory) {
    assert(0);
}
extern char **environ;

void AChildProcess::run() {
    auto splt = mArgs.split(' ');
    char** argv = new char*[splt.size() + 1];
    size_t counter = 0;
    {
        auto s = mApplicationFile.toStdString();
        argv[0] = new char[s.length() + 1];
        strcpy(argv[0], s.c_str());
    }
    for (auto& s : splt) {
        auto stdString = s.toStdString();
        auto cString = new char[stdString.length() + 1];
        strcpy(cString, stdString.c_str());
        argv[++counter] = cString;
    }
    argv[splt.size()] = nullptr;

    if (pipe(mPipes) == -1) {
        throw AException("could not create unix pipe");
    }

    auto pid = fork();
    if (pid == 0) {
        while ((dup2(mPipes[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
        //close(mPipes[0]);
        //close(mPipes[1]);

        // we are in a new process
        chdir(mWorkingDirectory.toStdString().c_str());
        execve(mApplicationFile.toStdString().c_str(), argv, environ);
    } else {
        // we are in old process
        mPid = pid;
        close(mPipes[1]);
    }
}

int AChildProcess::wait() {
    int loc;
    waitpid(mPid, &loc, 0);
    return WEXITSTATUS(loc);
}


uint32_t AChildProcess::getPid() {
    return mPid;
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
        ATokenizer t(_new<AFileInputStream>(f));
        auto p = t.readInt();

        auto process = AProcess::fromPid(p);
        if (process) {
            return process;
        }
    } catch (...) {}

    try {
        struct RemoveHelper {
        private:
            APath mPath;

        public:
            RemoveHelper(APath&& path) : mPath(std::forward<APath>(path)) {
                auto fos = _new<AFileOutputStream>(mPath);

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

APath AChildProcess::getPathToExecutable() {
    return mApplicationFile;
}
