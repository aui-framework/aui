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

#include <range/v3/view/transform.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/concat.hpp>
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

namespace {
size_t processMemory(pid_t pid) {
    // avoid fancy apis here for performance reasons.
    long rss = 0L;
    FILE* fp = nullptr;
    char path[0x100];
    *fmt::format_to_n(std::begin(path), sizeof(path), "/proc/{}/statm", pid).out = '\0';
    if ((fp = fopen(path, "r")) == nullptr) {
        return (size_t) 0L;
    }
    if (fscanf(fp, "%*s%ld", &rss) != 1) {
        fclose(fp);
        return (size_t) 0L;
    }
    fclose(fp);
    return (size_t) rss * (size_t) sysconf(_SC_PAGESIZE);
}
}   // namespace

class AOtherProcess : public AProcess {
private:
    pid_t mHandle;

public:
    AOtherProcess(pid_t handle) : mHandle(handle) {}

    ~AOtherProcess() {}

    int waitForExitCode() override {
        int loc = 0;
        waitpid(mHandle, &loc, 0);
        return WEXITSTATUS(loc);
    }

    APath getModuleName() override { return getPathToExecutable().filename(); }

    APath getPathToExecutable() override {
        char buf[0x800];
        char path[0x100];
        *fmt::format_to_n(std::begin(path), sizeof(path), "/proc/{}/exe", mHandle).out = '\0';
        return APath(buf, readlink(path, buf, sizeof(buf)));
    }

    uint32_t getPid() const noexcept override { return mHandle; }

    size_t processMemory() const override { return ::processMemory(mHandle); }
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

#if !AUI_PLATFORM_APPLE
_<AProcess> AProcess::self() {
    char buf[0x100];
    return _new<AOtherProcess>(*AString::fromUtf8(buf, readlink("/proc/self", buf, sizeof(buf))).toUInt());
}

_<AProcess> AProcess::fromPid(uint32_t pid) { return _new<AOtherProcess>(pid_t(pid)); }
#endif

extern char** environ;

void AChildProcess::run(ASubProcessExecutionFlags flags) {
    if (!getApplicationFile().isRegularFileExists()) {
        throw AFileNotFoundException(getApplicationFile());
    }

    bool mergeStdoutStderr = bool(flags & ASubProcessExecutionFlags::MERGE_STDOUT_STDERR);
    bool tieStdout = bool(flags & ASubProcessExecutionFlags::TIE_STDOUT);
    bool tieStderr = bool(flags & ASubProcessExecutionFlags::TIE_STDERR);

    auto argsStdString = std::visit(
        aui::lambda_overloaded {
          [](const ArgSingleString& singleString) {
              auto split = singleString.arg.split(' ');
              return split | ranges::views::transform(&AString::toStdString) | ranges::to_vector;
          },
          [](const ArgStringList& singleString) {
              return singleString.list | ranges::views::transform(&AString::toStdString) | ranges::to_vector;
          },
        },
        mInfo.args);

    auto executable = mInfo.executable.toStdString();

    auto argv = [&] {
        auto executableRange = std::to_array({ executable.data() });
        static constexpr auto nullRange = std::to_array({ (char*) nullptr });
        return ranges::views::concat(executableRange,
                                    argsStdString | ranges::views::transform([](auto& s) { return s.data(); }),
                                    nullRange)
               | ranges::to_vector;
    }();

    Pipe pipeStdin;
    Pipe pipeStdout;
    Pipe pipeStderr;

    // fcntl(pipeStdout.out(), F_SETOWN, callback);

    auto pid = fork();
    if (pid == 0) {
        // we are in a new process
        if (bool(flags & ASubProcessExecutionFlags::DETACHED)) {
            // daemonisizing
            //
            // Parent (one who called AChildProcess)
            // Child (we are currently here)
            // Grandchild (we'll spawn him a little later)
            //
            // In child, we need to call setsid() to detach from terminal. setsid requires the process it called in
            // to not be a group leader (Parent probably is) hence we spawned Child. After a terminal for Child is
            // detached, we can now spawn Grandchild (which we will call execve for), and we will exit Child.
            // Grandchill will lose parent, whose terminal is detached, hence it will be reparented to init.

            auto grandchild = fork();
            if (grandchild < 0) {
                fprintf(stderr, "grandchild fork error\n");
            }

            if (grandchild != 0) {
                // still in Child
                // so exit
                exit(0);
            }
        }

        while ((dup2(pipeStdin.out(), STDIN_FILENO) == -1) && (errno == EINTR)) {
        }
        if (!tieStdout) {
            while ((dup2(pipeStdout.in(), STDOUT_FILENO) == -1) && (errno == EINTR)) {
            }
        }
        if (!tieStderr) {
            while ((dup2(mergeStdoutStderr ? pipeStdout.in() : pipeStderr.in(), STDERR_FILENO) == -1) &&
                   (errno == EINTR)) {
            }
        }

        // ipeStdin.closeIn();
        // ipeStdout.closeOut();
        // ipeStderr.closeOut();

        if (!mInfo.workDir.empty()) {
            chdir(mInfo.workDir.toStdString().c_str());
        }
        execve(executable.c_str(), argv.data(), environ);
        exit(-1);
    } else {
        mPid = pid;

        mWatchdog = _new<AThread>([&] {
            int loc = 0;
            waitpid(mPid, &loc, 0);
            mExitCode.supplyValue(WEXITSTATUS(loc));
            emit finished;
        });
        mWatchdog->start();

        pipeStdin.closeOut();
        pipeStdout.closeIn();
        pipeStderr.closeIn();

        mStdoutAsync.init(pipeStdout.stealOut(), [&](const AByteBuffer& b) {
            if (stdOut && b.size() > 0)
                emit stdOut(b);
        });

        mStdInStream = _new<PipeOutputStream>(std::move(pipeStdin));
    }
}

AChildProcess::~AChildProcess() {}

int AChildProcess::waitForExitCode() { return *mExitCode; }

uint32_t AChildProcess::getPid() const noexcept { return mPid; }

size_t AChildProcess::processMemory() const { return ::processMemory(mPid); }

void AProcess::kill() const noexcept { ::kill(getPid(), SIGKILL); }
