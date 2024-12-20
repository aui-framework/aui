/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//
// Created by alex2 on 31.10.2020.
//

#include <range/v3/numeric/accumulate.hpp>
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
    sei.lpFile = aui::win32::toWchar(applicationFile.c_str());
    sei.lpParameters = aui::win32::toWchar(args.c_str());
    sei.lpDirectory = aui::win32::toWchar(workingDirectory.c_str());
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
        APath result;
        result.resize(0x1000);
        result.resize(GetModuleFileNameEx(mHandle, nullptr, aui::win32::toWchar(result), result.length()));
        result.replaceAll('\\', '/');
        return result;
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
        APath result;
        result.resize(0x1000);
        result.resize(GetProcessImageFileName(mHandle, aui::win32::toWchar(result), result.length()));
        result.replaceAll('\\', '/');
        return result;
    }

    uint32_t getPid() const noexcept override {
        return GetProcessId(mHandle);
    }
    size_t processMemory() const override { 
        PROCESS_MEMORY_COUNTERS info;
        GetProcessMemoryInfo(mHandle, &info, sizeof(info));
        return (size_t)info.WorkingSetSize; 
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

    mStdoutAsync.init(pipeStdout.stealOut(), [&](const AByteBuffer& buffer) {
        if (buffer.size() > 0 && bool(stdOut)) {
            emit stdOut(buffer);
        }
    });

    auto args = "\"" + mInfo.executable + "\" " + std::visit(aui::lambda_overloaded {
        [](const ArgSingleString& single) {
            return single.arg;
        },
        [](const ArgStringList& list) {
            if (!list.win32WrapWhitespaceArgumentsWithQuots) {
                return list.list.join(' ');
            }
            AString result;
            result.reserve(list.list.size() + ranges::accumulate(list.list, 0, std::plus<>{}, &AString::length));
            for (const auto& i : list.list) {
                if (!result.empty()) {
                    result += " ";
                }
                if (i.contains(" ")) {
                    result += "\"";
                    result += i.replacedAll("\"", "\\\"");
                    result += "\"";
                } else {
                    result += i;
                }
            }
            return result;
        },
    }, mInfo.args);

    if (!CreateProcess(aui::win32::toWchar(mInfo.executable.c_str()),
                       aui::win32::toWchar(args),
                       nullptr,
                       nullptr,
                       true,
                       0,
                       nullptr,
                       mInfo.workDir.empty() ? nullptr : aui::win32::toWchar(mInfo.workDir),
                       &startupInfo,
                       &mProcessInformation)) {
        AString message = "Could not create process " + mInfo.executable;
        if (!args.empty())
            message += " with args " + args;
        if (!mInfo.workDir.empty())
            message += " in " + mInfo.workDir;
        message += ": " + aui::impl::formatSystemError().description;
        throw AProcessException(message);
    }
    mExitEvent.registerWaitForSingleObject(mProcessInformation.hProcess, [&] {
        assert(("process already finished; os signaled process termination second time",
                !isFinished()));
        DWORD exitCode;
        int r = GetExitCodeProcess(mProcessInformation.hProcess, &exitCode);
        AUI_ASSERT(r && r != STILL_ACTIVE);
        mExitCode.supplyValue(exitCode);
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
    return *mExitCode;
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

size_t AChildProcess::processMemory() const {
    PROCESS_MEMORY_COUNTERS info;
    GetProcessMemoryInfo(mProcessInformation.hProcess, &info, sizeof(info));
    return (size_t)info.WorkingSetSize; 
}


void AProcess::kill() const noexcept {
    auto process = OpenProcess(PROCESS_TERMINATE, false, getPid());
    TerminateProcess(process, 1);
    CloseHandle(process);
}
