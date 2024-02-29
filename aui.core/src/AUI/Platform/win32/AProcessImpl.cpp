// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by alex2 on 31.10.2020.
//

#include <AUI/Platform/AProcess.h>
#include "AUI/IO/AFileOutputStream.h"
#include "WinIoCompletionPort.h"
#include "WinIoAsync.h"
#include <windows.h>
#include <AUI/Traits/memory.h>
#include <AUI/Logging/ALogger.h>
#include <psapi.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/Util/ATokenizer.h>
#include <AUI/Platform/Pipe.h>
#include <AUI/Platform/PipeInputStream.h>
#include <AUI/Platform/PipeOutputStream.h>
#include <AUI/Platform/ErrorToException.h>


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
        AUI_ASSERT(r && r != STILL_ACTIVE);
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
    bool tieStdout = bool(flags & ASubProcessExecutionFlags::TIE_STDOUT);
    bool tieStderr = bool(flags & ASubProcessExecutionFlags::TIE_STDERR);

    Pipe pipeStdin;
    Pipe pipeStdout;
    Pipe pipeStderr;


    if (tieStderr) {
        startupInfo.hStdError = mergeStdoutStderr ?
                GetStdHandle(STD_OUTPUT_HANDLE) : GetStdHandle(STD_ERROR_HANDLE);
    }
    else {
        startupInfo.hStdError = mergeStdoutStderr ? pipeStdout.in() : pipeStderr.in();
    }
    if (tieStdout) {
        startupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    }
    else {
        startupInfo.hStdOutput = pipeStdout.in();
    }
    startupInfo.hStdInput = pipeStdin.out();
    startupInfo.dwFlags = STARTF_USESTDHANDLES;
    startupInfo.cb = sizeof(startupInfo);

    for (auto handle : {pipeStdout.out(), pipeStderr.out(), pipeStdin.in() }) {
        if (!SetHandleInformation(handle, HANDLE_FLAG_INHERIT, 0)) {
            AUI_ASSERT((!"SetHandleInformation failed"));
        }
    }


    AString commandLine = "\"" + mApplicationFile + "\" " + mArgs;

    mStdoutAsync.init(pipeStdout.stealOut(), [&](const AByteBuffer& buffer) {
        if (buffer.size() > 0 && bool(stdOut)) {
            emit stdOut(buffer);
        }
    });

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
        message += ": " + aui::impl::formatSystemError().description;
        throw AProcessException(message);
    }
    mExitEvent.registerWaitForSingleObject(mProcessInformation.hProcess, [&] {
        assert(("process already finished; os signaled process termination second time",
                !isFinished()));
        mExitCode.supplyResult(waitForExitCode());
        emit finished;
    }, INFINITE, WT_EXECUTEDEFAULT | WT_EXECUTEONLYONCE);



    /*
    // setup pipe event handlers
    if (mergeStdoutStderr) {
        mStdOutEvent.registerWaitForSingleObject(pipeStdout.out(), [this] {
            emit readyReadStdOut;
            emit readyReadStdErr;
        });
    } else {
        mStdOutEvent.registerWaitForSingleObject(pipeStdout.out(), [this] {
            emit readyReadStdOut;
        });
        mStdErrEvent.registerWaitForSingleObject(pipeStderr.out(), [this] {
            emit readyReadStdErr;
        });
    }*/

    // close subprocess' side of pipes otherwise ReadFile of PipeInputStream would block the thread
    pipeStdout.closeIn();
    pipeStderr.closeIn();
    pipeStdin.closeOut();

    mStdInStream = _new<PipeOutputStream>(std::move(pipeStdin));
}


int AChildProcess::waitForExitCode() {
    AUI_ASSERTX(mProcessInformation.hProcess != nullptr, "process handle is null; have you ever run the process?");
    WaitForSingleObject(mProcessInformation.hProcess, INFINITE);
    DWORD exitCode;
    int r = GetExitCodeProcess(mProcessInformation.hProcess, &exitCode);
    AUI_ASSERT(r && r != STILL_ACTIVE);
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
