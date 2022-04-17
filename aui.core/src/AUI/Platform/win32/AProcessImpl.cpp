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

#include <AUI/Platform/AProcess.h>
#include "AUI/IO/AFileOutputStream.h"
#include <windows.h>
#include <AUI/Traits/memory.h>
#include <AUI/Logging/ALogger.h>
#include <psapi.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/Util/ATokenizer.h>
#include <AUI/Platform/Pipe.h>
#include <AUI/Platform/PipeInputStream.h>
#include <AUI/Platform/PipeOutputStream.h>


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
        auto length = GetModuleFileNameEx(mHandle, nullptr, buf, sizeof(buf));
        return APath(buf, length);
    }

    int waitForExitCode() override {
        WaitForSingleObject(mHandle, INFINITE);
        DWORD exitCode;
        waitForExitCode();
        int r = GetExitCodeProcess(mHandle, &exitCode);
        assert(r && r != STILL_ACTIVE);
        return exitCode;
    }

    APath getModuleName() override {
        wchar_t buf[0x800];
        auto length = GetProcessImageFileName(mHandle, buf, sizeof(buf));
        return APath(buf, length).filename();
    }

    uint32_t getPid() const noexcept override {
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

void AChildProcess::run(ASubProcessExecutionFlags flags) {
    STARTUPINFO startupInfo;
    aui::zero(startupInfo);

    bool mergeStdoutStderr = bool(flags & ASubProcessExecutionFlags::MERGE_STDOUT_STDERR);

    Pipe pipeStdin;
    Pipe pipeStdout;
    Pipe pipeStderr;


    startupInfo.hStdError = mergeStdoutStderr ? pipeStdout.in() : pipeStderr.in();
    startupInfo.hStdOutput = pipeStdout.in();
    startupInfo.hStdInput = pipeStdin.out();
    startupInfo.dwFlags = STARTF_USESTDHANDLES;
    startupInfo.cb = sizeof(startupInfo);

    for (auto handle : {pipeStdout.out(), pipeStderr.out(), pipeStdin.in() }) {
        if (!SetHandleInformation(handle, HANDLE_FLAG_INHERIT, 0)) {
            assert((!"SetHandleInformation failed"));
        }
    }


    AString commandLine = "\"" + mApplicationFile + "\" " + mArgs;


    if (!CreateProcess(nullptr,
                       const_cast<wchar_t*>(commandLine.c_str()),
                       nullptr,
                       nullptr,
                       true,
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
    mExitEvent.registerWaitForSingleObject(mProcessInformation.hProcess, [&] {
        assert(("process already finished; os signaled process termination second time",
                !isFinished()));
        mExitCode = waitForExitCode();
        emit finished;
    }, INFINITE, WT_EXECUTEDEFAULT | WT_EXECUTEONLYONCE);

    // setup pipe event handlers
    mStdOutEvent.registerWaitForSingleObject(pipeStdout.out(), [this] {
        emit readyReadStdOut;
    });
    mStdErrEvent.registerWaitForSingleObject(pipeStdout.out(), [this] {
        emit readyReadStdErr;
    });

    // close subprocess' side of pipes otherwise ReadFile of PipeInputStream would block the thread
    pipeStdout.closeIn();
    pipeStderr.closeIn();
    pipeStdin.closeOut();

    // create std pipes
    mStdOutStream = _new<PipeInputStream>(std::move(pipeStdout));
    if (mergeStdoutStderr) {
        mStdErrStream = mStdOutStream;
    } else {
        mStdErrStream = _new<PipeInputStream>(std::move(pipeStderr));
    }

    mStdInStream = _new<PipeOutputStream>(std::move(pipeStdin));
}


int AChildProcess::waitForExitCode() {
    assert(("process handle is null; have you ever run the process?", mProcessInformation.hProcess != nullptr));
    WaitForSingleObject(mProcessInformation.hProcess, INFINITE);
    DWORD exitCode;
    int r = GetExitCodeProcess(mProcessInformation.hProcess, &exitCode);
    assert(r && r != STILL_ACTIVE);
    return exitCode;
}

AChildProcess::~AChildProcess() = default;

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

uint32_t AChildProcess::getPid() const noexcept {
    return mProcessInformation.dwProcessId;
}


void AProcess::kill() const noexcept {
    auto process = OpenProcess(PROCESS_TERMINATE, false, getPid());
    TerminateProcess(process, 1);
    CloseHandle(process);
}
