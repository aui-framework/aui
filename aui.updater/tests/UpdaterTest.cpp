// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <AUI/Updater/AUpdater.h>
#include <gmock/gmock.h>
#include "AUI/Util/kAUI.h"
#include "AUI/Curl/ACurl.h"
#include "AUI/Platform/AProcess.h"

using namespace std::chrono_literals;

#ifdef AUI_UPDATER_TEST
TEST(UpdaterTest, ApplyUpdate) {
    // test update installation scenario

    auto updaterPath = [] {
        auto selfDir = AProcess::self()->getPathToExecutable().parent();
        if (auto paths = APath::find("aui.updater.test", { selfDir }); !paths.empty()) {
            return paths.first();
        }
        if (auto paths = APath::find("aui.updater.test.exe", { selfDir }); !paths.empty()) {
            return paths.first();
        }
        AUI_ASSERT_NO_CONDITION("can't find aui.updater.test");
        return APath();
    }();
    APathOwner temporary(APath::nextRandomTemporary());
    auto tempDir = APath(temporary) / "white space"; // white space to check everything is ok on Win
    tempDir.makeDirs();
    auto tempDirUpdater = tempDir / updaterPath.filename();

    // set up "downloaded" update
    APath::copy(updaterPath, tempDirUpdater);
    tempDirUpdater.chmod(0755);
    auto dependencyPath = updaterPath.parent() / "dependency.txt";
    dependencyPath.removeFileRecursive();
    AFileOutputStream(tempDir / "dependency.txt") << aui::serialize_sized(AString("1234"));

    // first launch. --test= flag instructs aui.updater.test to apply update from the specified dir.
    // this launch will launch a copy of aui.updater.test from tempDir and copy itself along with the dependency.txt
    // back to updaterPath.
    {
        // check that dependency.txt does not exist ("no update applied").
        EXPECT_FALSE(dependencyPath.isRegularFileExists());

        auto process = AProcess::create({
          .executable = updaterPath,
          .args = AProcess::ArgStringList { { "--test={}"_format(tempDir) } },
        });
        process->run(ASubProcessExecutionFlags::TIE_STDOUT | ASubProcessExecutionFlags::TIE_STDERR);
        EXPECT_EQ(process->waitForExitCode(), 0);
    }

    // wait a little bit to apply the update.
    AThread::sleep(1s);
    {
        // check that dependency.txt appeared.
        EXPECT_TRUE(dependencyPath.isRegularFileExists());

        // perform "app normal lifecycle". In case of aui.updater.test, it will check for dependency.txt.
        auto process = AProcess::create({
          .executable = updaterPath,
          .workDir = updaterPath.parent(),
        });
        process->run(ASubProcessExecutionFlags::TIE_STDOUT | ASubProcessExecutionFlags::TIE_STDERR);
        EXPECT_EQ(process->waitForExitCode(), 0);
    }
}
#endif

#ifdef AUI_ENTRY
#undef AUI_ENTRY
#endif
#define AUI_ENTRY static int fake_entry(const AStringVector& args)

// AUI_DOCS_OUTPUT: doxygen/intermediate/updater.h
// @defgroup updater aui::updater
// @brief Deliver updates on non-centralized distribution methods
// @details
// This module is purposed for delivering updates to your end users on distribution methods that do not support that by
// themselves (i.e., occasional Windows installers, portables for Windows and Linux, macOS app bundles downloaded from
// your website).
//
// `aui.updater` module expects your program to be installed to user's directory (i.e., updating does not require admin
// priveleges). If that's not your case, you'll need to update your @ref INNOSETUP "installer configuration" to install
// to user's directory (i.e., in `AppData`).
//
// # Supported platforms
// `aui::updater` supports the following platforms:
// - **Windows** - @ref PORTABLE_WINDOWS "portables" only, installers to user's directory only (@ref INNOSETUP)
// - **Linux** - portables only
// - **macOS** - @ref DragNDrop only
//
// On a supported platform, `aui::updater` checks if the app executable is writable by the current user. If the
// executable is not writeable, or running on a non-supported platform, `AUpdater` stubs it's methods (i.e., they do
// nothing). You can check that the `aui::updater` functionality is engaged by calling `AUpdater::isAvailable()`.
//
// # Getting started
//
// `AUpdater` lives inside entrypoint of your application. It needs you to pass program arguments. It might decide
// to terminate process execution via std::exit.
namespace {
class MainWindow {
public:
    MainWindow(const _<AUpdater>& updater) {}
    void show() {}
};

}   // namespace

// AUI_DOCS_CODE_BEGIN
class MyUpdater : public AUpdater {
protected:
    AFuture<void> checkForUpdatesImpl() override { return AFuture<void>(); }

protected:
    // stub
    AFuture<void> downloadUpdateImpl(const APath& unpackedUpdateDir) override { return {}; }
};

AUI_ENTRY {
    auto updater = _new<MyUpdater>();
    updater->handleStartup(args);

    // your program routines (i.e., open a window)
    _new<MainWindow>(updater)->show();
    return 0;
}
// AUI_DOCS_CODE_END
//
// You can pass updater instance to your window (as shown in the example) and display update information from
// `AUpdater::status` and perform the update when requested.
//
// # Update process
//
// AUpdater expects `AUpdater::checkForUpdates` to be called once per some period of time. Once update is found, it
// changes its `AUpdater::status` property to `AUpdater::StatusDownloading` and starts to download and unpack with
// `AUpdater::deliverUpdateIfNeeded`. Once unpacked, AUpdater changes its `AUpdater::status` property to
// `AUpdater::StatusWaitingForApplyAndRestart`, signaling your application that the update is ready to deploy. Your
// application might respond to that by showing the user a notification.
//
// @msc
// a[label = "Your App"],
// u[label = "AUpdater", URL = "@ref AUpdater"];
// a -> u [label = "handleStartup", URL = "@ref AUpdater::handleStartup"];
//
// --- [label="App Normal Lifecycle"];
// ...;
// a -> u [label = "checkForUpdates", URL = "@ref AUpdater::checkForUpdates"];
// a <- u;
//
// ...;
// --- [label="Update published"];
// ...;
// a -> u [label = "checkForUpdates", URL = "@ref AUpdater::checkForUpdates"];
// a <- u;
// a <- u [label = "status = AUpdater::StatusDownloading", URL = "@ref AUpdater::StatusDownloading"];
// u box u [label = "download and unpack update"];
// a <- u [label = "status = AUpdater::StatusWaitingForApplyAndRestart", URL = "@ref
// AUpdater::StatusWaitingForApplyAndRestart"];
// --- [label="Your App Prompts User to Update"];
// ...;
// @endmsc
//
// At this moment, AUpdater waits AUpdater::applyUpdateAndRestart to be called. When
// AUpdater::applyUpdateAndRestart is called (i.e., when user accepted update installation), AUpdater executes a copy of
// your app downloaded before with a special command line argument which is handled by AUpdater::handleStartup. The copy
// then replaces old application (where it actually installed) with itself (that is, the downloaded, newer copy). After
// operation is complete, it passes the control back to the application. At last, the newly updated application
// performs a cleanup after update.
//
// @msc
// a[label = "Your App"],
// u[label = "AUpdater", URL = "@ref AUpdater"],
// da[label = "Newer Copy of Your App"],
// du[label = "AUpdater in App Copy", URL = "@ref AUpdater"];
// a :> u [label = "applyUpdateAndRestart", URL = "@ref AUpdater::applyUpdateAndRestart"];
// u :> da [label = "Execute with update arg"];
// u box u [label = "exit(0)"];
// a box u [label = "Process Finished"];
// da box du [label = "Process Started"];
// da -> du [label = "handleStartup", URL = "@ref AUpdater::handleStartup"];
// du box du [label = "Replace Old App with Itself"];
// a <: du [label = "Execute"];
// du box du [label = "exit(0)"];
// da box du [label = "Process Finished"];
// a box u [label = "Process Started"];
// a -> u [label = "handleStartup", URL = "@ref AUpdater::handleStartup"];
// u box u [label = "cleanup download dir"];
// a box u [label="App Normal Lifecycle"];
// ...;
// @endmsc

TEST(UpdaterTest, ExampleCase) { EXPECT_EQ(fake_entry({}), 0); }

TEST(UpdaterTest, Typical_Implementation) {   // HEADER_H1
    // AUpdater is an abstract class; it needs some functions to be implemented by you.
    //
    // In this example, let's implement auto update from GitHub release pages.
    // AUI_DOCS_CODE_BEGIN
    /*
    class MyUpdater: public AUpdater {
    public:

    protected:
        AFuture<APath> deliverUpdateIfNeeded(const APath& unpackedUpdateDir) override {
            return async {
                ACurl::Builder()
            };
        }
    };*/
    // AUI_DOCS_CODE_END
}

TEST(UpdaterTest, WaitForProcess) {
    class UpdaterMock : public AUpdater {
    public:
        AFuture<void> downloadUpdateImpl(const APath& unpackedUpdateDir) override { return AFuture<void>(); }
        AFuture<void> checkForUpdatesImpl() override { return AFuture<void>(); }

        MOCK_METHOD(void, handleWaitForProcess, (uint32_t pid), (override));
    };

    UpdaterMock updater;
    EXPECT_CALL(updater, handleWaitForProcess(123));
    updater.handleStartup({ "--aui-updater-wait-for-process=123" });
}
