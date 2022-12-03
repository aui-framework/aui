// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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
#include "AUI/Platform/Pipe.h"
#include "AUI/Platform/PipeInputStream.h"
#include "AUI/Platform/PipeOutputStream.h"
#include "UnixIoThread.h"
#include <sys/types.h>
#include <signal.h>
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
#include <AUI/Logging/ALogger.h>
#include <fcntl.h>

class AOtherProcess: public AProcess {
private:
    pid_t mHandle;

public:
    AOtherProcess(pid_t handle) : mHandle(handle) {}

    ~AOtherProcess() {
    }

    int waitForExitCode() override {
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
        return APath(buf, readlink(path, buf, sizeof(buf)));
    }

    uint32_t getPid() const noexcept override {
        return mHandle;
    }
};

AVector<_<AProcess>> AProcess::all() {
    AVector<_<AProcess>> result;
    for (auto& f : APath("/proc/").listDir(AFileListFlags::DIRS)) {
        if (auto pid = f.filename().toUInt()) {
            result << _new<AOtherProcess>(*pid);
        }
    }
    return result;
}

_<AProcess> AProcess::self() {
#if AUI_PLATFORM_APPLE
    return _new<AOtherProcess>(getpid());
#else
    char buf[0x100];
    return _new<AOtherProcess>(*AString::fromUtf8(buf, readlink("/proc/self", buf, sizeof(buf))).toUInt());
#endif
}

_<AProcess> AProcess::fromPid(uint32_t pid) {
#if !AUI_PLATFORM_APPLE
    if (!(APath("/proc") / AString::number(pid)).isDirectoryExists()) {
        return nullptr;
    }
#endif
    return _new<AOtherProcess>(pid_t(pid));
}

void AProcess::executeAsAdministrator(const AString& applicationFile, const AString& args, const APath& workingDirectory) {
    assert(0);
}
extern char **environ;

void AChildProcess::run(ASubProcessExecutionFlags flags) {
    if (!APath(mApplicationFile).isRegularFileExists()) {
        throw AFileNotFoundException(mApplicationFile);
    }

    bool mergeStdoutStderr = bool(flags & ASubProcessExecutionFlags::MERGE_STDOUT_STDERR);

    AVector<std::string> argsStdString;
    {
        auto splt = mArgs.split(' ');
        argsStdString.reserve(splt.size() + 1);
        argsStdString << mApplicationFile.toStdString();
        for (auto& arg: splt) {
            argsStdString << arg.toStdString();
        }
    }
    AVector<char*> argv;
    argv.reserve(argsStdString.size());
    for (auto& arg : argsStdString) {
        argv << arg.data();
    }
    argv << nullptr;


    Pipe pipeStdin;
    Pipe pipeStdout;
    Pipe pipeStderr;

    //fcntl(pipeStdout.out(), F_SETOWN, callback);

    auto pid = fork();
    if (pid == 0) {
        while ((dup2(pipeStdin.out(), STDIN_FILENO) == -1) && (errno == EINTR)) {}
        while ((dup2(pipeStdout.in(), STDOUT_FILENO) == -1) && (errno == EINTR)) {}
        while ((dup2(mergeStdoutStderr ? pipeStdout.in() : pipeStderr.in(), STDERR_FILENO) == -1) && (errno == EINTR)) {}

        //ipeStdin.closeIn();
        //ipeStdout.closeOut();
        //ipeStderr.closeOut();

        // we are in a new process
        chdir(mWorkingDirectory.toStdString().c_str());
        execve(mApplicationFile.toStdString().c_str(), argv.data(), environ);
        exit(-1);
    } else {
        // we are in old process
        ALOG_DEBUG("AProcess") << "Started new process: " << pid;

        mWatchdog = _new<AThread>([&] {
            int loc;
            waitpid(mPid, &loc, 0);
            mExitCode.supplyResult(WEXITSTATUS(loc));
            emit finished;
        });
        mWatchdog->start();

        mPid = pid;
        pipeStdin.closeOut();
        pipeStdout.closeIn();
        pipeStderr.closeIn();

        mStdoutAsync.init(pipeStdout.stealOut(), [&](const AByteBuffer& b) {
            if (stdOut && b.size() > 0) emit stdOut(b);
        });

        mStdInStream = _new<PipeOutputStream>(std::move(pipeStdin));
    }
}

AChildProcess::~AChildProcess() {

}

int AChildProcess::waitForExitCode() {
   return *mExitCode;
}


uint32_t AChildProcess::getPid() const noexcept {
    return mPid;
}

void AProcess::kill() const noexcept {
    ::kill(getPid(), SIGKILL);
}
