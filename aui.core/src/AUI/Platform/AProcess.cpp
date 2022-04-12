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

#include <AUI/IO/AFileInputStream.h>
#include <AUI/Util/ATokenizer.h>
#include "AProcess.h"
#include "AUI/IO/AFileOutputStream.h"

_<AChildProcess>
AProcess::make(AString applicationFile, AString args, APath workingDirectory) {
    auto p = _new<AChildProcess>();
    p->mApplicationFile = std::move(applicationFile);
    p->mArgs = std::move(args);
    p->mWorkingDirectory = std::move(workingDirectory);

    return p;
}

int AProcess::executeWaitForExit(AString applicationFile, AString args, APath workingDirectory) {
    AChildProcess p;
    p.mApplicationFile = std::move(applicationFile);
    p.mArgs = std::move(args);
    p.mWorkingDirectory = std::move(workingDirectory);
    p.run();

    return p.waitForExitCode();
}

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
