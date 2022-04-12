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

int AChildProcess::waitForExitCode() {
    int loc;
    waitpid(mPid, &loc, 0);
    return WEXITSTATUS(loc);
}


uint32_t AChildProcess::getPid() {
    return mPid;
}
