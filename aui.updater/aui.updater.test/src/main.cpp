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

#include <AUI/Platform/Entry.h>
#include <AUI/Updater/AUpdater.h>
#include "AUI/Util/kAUI.h"
#include "AUI/IO/AFileInputStream.h"
#include "AUI/Platform/AProcess.h"

static constexpr auto LOG_TAG = "aui.updater.test";

class MyUpdater : public AUpdater {
public:
    void handleStartup(const AStringVector& applicationArguments) override {
        ACommandLineArgs cmdline(applicationArguments);
        if (auto testDir = cmdline.value("test")) {
            ALogger::info(LOG_TAG) << "Test dir: " << *testDir;
            mTestDir = std::move(*testDir);
        }

        AUpdater::handleStartup(applicationArguments);
    }

protected:
    void triggerUpdateOnStartup() override {
        ALogger::info(LOG_TAG) << "triggerUpdateOnStartup()";
        if (mTestDir.empty()) {
            return;
        }
        AUpdater::triggerUpdateOnStartup();
    }

    AOptional<InstallCmdline> loadInstallCmdline() const override {
        auto l = makeDefaultInstallationCmdline();
        ALogger::info(LOG_TAG) << "loadInstallCmdline() = InstallCmdLine {.exe=\"" << l.installerExecutable << "\", .args=" << l.installerArguments << "}";
        return l;
    }

    APath getUnpackedUpdateDir() const override {
        if (!mTestDir.empty()) {
            return mTestDir;
        }
        return AUpdater::getUnpackedUpdateDir();
    }

    AFuture<void> downloadUpdateImpl(const APath& unpackedUpdateDir) override {
        return AUI_THREADPOOL {
            // download logic here
        };
    }

    AFuture<void> checkForUpdatesImpl() override {
        return AUI_THREADPOOL {
          // last version query logic here
        };
    }

    APath mTestDir;
};

AUI_ENTRY {
    ALogger::info(LOG_TAG) << "pid=" << AProcess::self()->getPid() << ", args=" << args;
    MyUpdater updater;
    updater.handleStartup(args);
    ALogger::info(LOG_TAG) << "App Normal Lifecycle, checking for dependency.txt...";
    AFileInputStream fis("dependency.txt");
    AString text;
    fis >> aui::serialize_sized(text);
    AUI_ASSERT(text == "1234");
    ALogger::info(LOG_TAG) << "dependency.txt is good.";
    return 0;
};