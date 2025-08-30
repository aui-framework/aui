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

#include "AProcess.h"
#include "AUI/IO/AFileOutputStream.h"

#include <AUI/Traits/memory.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/Util/ATokenizer.h>
#include "AProcess.h"
#include "AUI/IO/AFileOutputStream.h"

_<AChildProcess>
AProcess::create(AProcess::ProcessCreationInfo args) {
    auto p = aui::ptr::manage_shared(new AChildProcess);
    p->mInfo = std::move(args);
    return p;
}

int AProcess::executeWaitForExit(AString applicationFile, AString args, APath workingDirectory,
                                 ASubProcessExecutionFlags flags) {
    auto p = aui::ptr::manage_shared(new AChildProcess);
    p->mInfo = {
        .executable = std::move(applicationFile),
        .args = ArgSingleString { std::move(args) },
    };
    p->run(flags);

    return p->waitForExitCode();
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

AString AChildProcess::toString() const {
    return "<AChildProcess pid={}, exe={}, args=[ {} ], workdir={}>"_format(
        getPid(), getApplicationFile(),
        std::visit(
            aui::lambda_overloaded {
              [](const AProcess::ArgStringList& args) { return args.list.join(','); },
              [](const AProcess::ArgSingleString& arg) { return "\"" + arg.arg + "\""; },
            },
            getArgs()),
        getWorkingDirectory());
}


APath AChildProcess::getModuleName() {
    return APath(getApplicationFile()).filename();
}
