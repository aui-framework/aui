/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
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
#include <AUI/Platform/ErrorToException.h>
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
    return _new<AOtherProcess>(*AString(buf, readlink("/proc/self", buf, sizeof(buf))).toUInt());
}

_<AProcess> AProcess::fromPid(uint32_t pid) { return _new<AOtherProcess>(pid_t(pid)); }
#endif

extern char** environ;

void AChildProcess::run(ASubProcessExecutionFlags flags) {
    if (weak_from_this().lock() == nullptr) {
        throw AException("this object should be constructed as shared_ptr");
    }
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
              return split | ranges::views::transform([](const AString& s){ return s.toStdString(); }) | ranges::to_vector;
          },
          [](const ArgStringList& singleString) {
              return singleString.list | ranges::views::transform([](const AString& s){ return s.toStdString(); }) | ranges::to_vector;
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

    struct DetachedSpecific {
        // to catch the startup of the child
        Pipe startedPipe;

        // messages passed through startedPipe
        enum class Started: char {
            OK = '\0',
            FAILED,
        };


        // to catch the pid of the child
        Pipe pidPipe;

        DetachedSpecific() {
            // if exec* succeeds, close the pipe.
            ::fcntl(startedPipe.in(), F_SETFD, FD_CLOEXEC);
            ::fcntl(startedPipe.out(), F_SETFD, FD_CLOEXEC);
            ::fcntl(pidPipe.in(), F_SETFD, FD_CLOEXEC);
            ::fcntl(pidPipe.out(), F_SETFD, FD_CLOEXEC);
        }
    };
    AOptional<DetachedSpecific> detachedSpecific;
    if (bool(flags & ASubProcessExecutionFlags::DETACHED)) {
        detachedSpecific.emplace();
    }

    // fcntl(pipeStdout.out(), F_SETOWN, callback);

    auto pid = fork();
    if (pid == -1) {
        throw AProcessException("can't create fork");
    }
    if (pid == 0) {
        try {
            // we are in a new process
            auto execute = [&] {
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
                if (!mInfo.workDir.empty()) {
                    chdir(mInfo.workDir.toStdString().c_str());
                }
                execve(executable.c_str(), argv.data(), environ);
            };

            if (detachedSpecific) {
                // daemonisizing
                //
                // Parent (one who called AChildProcess::run)
                // Child (we are currently here)
                // Grandchild (we'll spawn him a little later)
                //
                // In child, we need to call setsid() to detach from terminal. setsid requires the process it called in
                // to not be a group leader (Parent probably is) hence we spawned Child. After a terminal for Child is
                // detached, we can now spawn Grandchild (which we will call execve for), and we will std::exit Child.
                // Grandchill will lose parent, whose terminal is detached, hence it will be reparented to init.

                // close redundant pipe sides
                detachedSpecific->startedPipe.closeOut();
                detachedSpecific->pidPipe.closeOut();

                struct sigaction noaction;
                memset(&noaction, 0, sizeof(noaction));
                noaction.sa_handler = SIG_IGN;
                ::sigaction(SIGPIPE, &noaction, nullptr);

                setsid();

                auto grandchild = fork();
                if (grandchild < 0) {
                    // in Child; Grandchild fork() failed
                    struct sigaction noaction;
                    memset(&noaction, 0, sizeof(noaction));
                    noaction.sa_handler = SIG_IGN;
                    ::sigaction(SIGPIPE, &noaction, nullptr);
                    detachedSpecific->startedPipe
                        << aui::serialize_raw(DetachedSpecific::Started::FAILED)
                        << aui::serialize_sized("fork failed: {}"_format(aui::impl::unix_based::formatSystemError().description));
                    detachedSpecific->startedPipe.closeIn();
                } else if (grandchild == 0) {
                    // in Grandchild
                    detachedSpecific->pidPipe.closeIn();
                    execute();

                    // if we reach here, it basically means execute() failed so report it.
                    struct sigaction noaction;
                    memset(&noaction, 0, sizeof(noaction));
                    noaction.sa_handler = SIG_IGN;
                    ::sigaction(SIGPIPE, &noaction, nullptr);
                    detachedSpecific->startedPipe
                        << aui::serialize_raw(DetachedSpecific::Started::FAILED)
                        << aui::serialize_sized("execve failed: {}"_format(aui::impl::unix_based::formatSystemError().description));
                    detachedSpecific->startedPipe.closeIn();
                    _exit(1);
                } else {
                    // in Child; fork() succeeded
                    detachedSpecific->startedPipe.closeIn();

                    // report pid of Grandchild to parent
                    detachedSpecific->pidPipe << aui::serialize_raw(grandchild);
                    detachedSpecific->pidPipe.closeIn();
                    _exit(1);
                }
            } else {
                execute();
                _exit(1);
            }
        } catch (const AException& e) {
            std::cerr << "(occurred in subprocess) failure: " << e << '\n';
        }

        return;
    }

    // here, we are still in parent (caller).

    // close pipes of parent's side.
    if (detachedSpecific) {
        detachedSpecific->startedPipe.closeIn(); // we'd read from startedPipe only, not write
        detachedSpecific->pidPipe.closeIn();     // we'd read from pidPipe only, not write
    }
    pipeStdin.closeOut(); // we'd write to stdin only, not read
    pipeStdout.closeIn(); // we'd read from stdout only, not write
    pipeStderr.closeIn(); // we'd read from stderr only, not write

    mPid = [&] {
      if (detachedSpecific) {
          // while detached, pid actually holds a child (intermediate process), not grandchild (AProcess user is
          // interested in the latter)
          //
          // at this moment, only child knows the pid of grandchild.
          // we can ask him for pid of grandchild.
          auto message = DetachedSpecific::Started::OK;
          detachedSpecific->startedPipe >> aui::serialize_raw(message);
          int loc = 0;
          waitpid(pid, &loc, 0);

          switch (message) {
              case DetachedSpecific::Started::OK:
                  break;
              case DetachedSpecific::Started::FAILED: {
                  AString s;
                  detachedSpecific->startedPipe >> aui::serialize_sized(s);
                  throw AProcessException("can't start subprocess: {}"_format(s));
              }
          }

          detachedSpecific->startedPipe.closeOut();
          pid = 0;
          detachedSpecific->pidPipe >> aui::serialize_raw(pid);
          AUI_ASSERT(pid != 0);
          detachedSpecific->pidPipe.closeOut();
      }
      return pid;
    }();

    mWatchdog = _new<AThread>([&, self = weak_from_this()] { // what the f*ck?? we have UnixIoAsync
        for (;;) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            auto selfLock = self.lock();
            if (!selfLock) {
                break;
            }

            int loc = 0;
            if (waitpid(mPid, &loc, WNOHANG) == 0) {
                continue;
            }

            mExitCode.supplyValue(WEXITSTATUS(loc));
            emit finished;
            break;
        }
    });
    mWatchdog->start();


    mStdoutAsync.init(pipeStdout.stealOut(), [&](const AByteBuffer& b) {
        if (stdOut && b.size() > 0)
            emit stdOut(b);
    });

    mStdInStream = _new<Pipe>(std::move(pipeStdin));
}

AChildProcess::~AChildProcess() {}

int AChildProcess::waitForExitCode() { return *mExitCode; }

uint32_t AChildProcess::getPid() const noexcept { return mPid; }

size_t AChildProcess::processMemory() const { return ::processMemory(mPid); }

void AProcess::kill() const noexcept { ::kill(getPid(), SIGKILL); }
