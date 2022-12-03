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

#include "AProcess.h"
#include "AUI/IO/AFileOutputStream.h"

#include <AUI/Traits/memory.h>
#include <AUI/Logging/ALogger.h>
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
    auto f = APath::getDefaultPath(APath::TEMP) / ("." + yourProjectName + ".pid");
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
                AFileOutputStream fos(mPath);

                auto n = std::to_string(self()->getPid());
                fos.write(n.c_str(), n.length());
                fos.close();
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
