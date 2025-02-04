// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <range/v3/all.hpp>
#include <AUI/Updater/AUpdater.h>
#include <gmock/gmock.h>
#include "AUI/Util/kAUI.h"
#include "AUI/Curl/ACurl.h"
#include "AUI/Platform/AProcess.h"
#include "AUI/Updater/GitHub.h"
#include "AUI/Updater/Semver.h"
#include "AUI/Updater/AppropriatePortablePackagePredicate.h"
#include "AUI/Platform/AMessageBox.h"

using namespace std::chrono_literals;

TEST(UpdaterTest, getInstallationDirectory) {
    class MyUpdater : public AUpdater {
    public:
        using AUpdater::getInstallationDirectory;

    protected:
        AFuture<void> downloadUpdateImpl(const APath& unpackedUpdateDir) override { return AFuture<void>(); }
        AFuture<void> checkForUpdatesImpl() override { return AFuture<void>(); }
    } myUpdater;

    // clang-format off
    EXPECT_EQ(
        myUpdater.getInstallationDirectory(
            { .selfProcessExePath = "/tmp/__aui_update_example_app/download/bin/example_app",
              .updaterDir         = "/tmp/__aui_update_example_app/download",
              .originExe          = "/home/alex2772/CLionProjects/example_app/cmake-build-debug/bin/example_app" }),
                                    "/home/alex2772/CLionProjects/example_app/cmake-build-debug"
        );

    // malformed structure
    EXPECT_ANY_THROW(
        myUpdater.getInstallationDirectory(
            { .selfProcessExePath = "/tmp/__aui_update_example_app/download/bin/example_app",
                .updaterDir         = "/tmp/__aui_update_example_app/download",
                .originExe          = "/home/alex2772/CLionProjects/example_app/cmake-build-debug/bin2/example_app" })
    );

    EXPECT_EQ(
        myUpdater.getInstallationDirectory(
            { .selfProcessExePath = "/tmp/__aui_update_example_app/download/example_app.exe",
              .updaterDir         = "/tmp/__aui_update_example_app/download",
              .originExe          = "/home/alex2772/CLionProjects/example_app/cmake-build-debug/example_app.exe" }),
                                    "/home/alex2772/CLionProjects/example_app/cmake-build-debug");

    // clang-format on
}

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
    auto tempDir = APath(temporary) / "white space";   // white space to check everything is ok on Win
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

        AProcess::ArgStringList args;
        args.list << "--test={}"_format(tempDir);
        auto process = AProcess::create({
          .executable = updaterPath,
          .args = std::move(args),
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
// @note
// Check out our [app template](https://github.com/aui-framework/example_app) for a GitHub-hosted app template with auto
// update implemented.
// @image html afljskfllkaf.gif
//
// # Supported platforms
// `aui::updater` supports the following platforms:
// - **Windows** - @ref PORTABLE_WINDOWS "portables" only, installers to user's directory only (@ref INNOSETUP)
// - **Linux** - portables only
//
// On a supported platform, `aui::updater` checks if the app executable is writable by the current user. If the
// executable is not writeable, or running on a non-supported platform, `AUpdater` stubs it's methods (i.e., they do
// nothing). You can check that the `aui::updater` functionality is engaged by calling `AUpdater::isAvailable()`.
//
// Updating process requires the initial application running instance to be stopped to replace its files with newer
// ones. Additionally, the updater process starts the newer version of the application after replacing the files
// (applying/deploying an update). To minimize downtime for end-users, the replacement should be seamless and quick and
// thus the deployment process just copies newer files (overwriting old ones), it does not involve network operations.
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
    AFuture<void> checkForUpdatesImpl() override { return async { /* stub */ }; }
    AFuture<void> downloadUpdateImpl(const APath& unpackedUpdateDir) override { return async { /* stub */ }; }
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
// # Observing update progress
//
// @copydetails AUpdater::status
//
// # Update process
//
// ## Checking for updates
//
// AUpdater expects @ref AUpdater::checkForUpdates to be called to check for updates. It can be called once per some
// period of time. It calls user-defined @ref AUpdater::checkForUpdatesImpl to perform an update checking.
//
// The update steps are reported by changing `AUpdater::status` property.
//
// @msc
// a[label = "Your App"],
// u[label = "AUpdater", URL = "@ref AUpdater"];
// a -> u [label = "handleStartup(...)", URL = "@ref AUpdater::handleStartup"];
// a <- u [label = "status = AUpdater::StatusIdle", URL = "@ref AUpdater::StatusIdle"];
// a <- u [label = "control flow"];
//
// --- [label="App Normal Lifecycle"];
// ...;
// a -> u [label = "checkForUpdates()", URL = "@ref AUpdater::checkForUpdates"];
// a <- u [label = "status = AUpdater::StatusCheckingForUpdates", URL = "@ref AUpdater::StatusCheckingForUpdates"];
// a <- u [label = "control flow"];
// u box u [label = "checkForUpdatesImpl()", URL = "@ref AUpdater::checkForUpdatesImpl"];
// a <- u [label = "status = AUpdater::StatusIdle", URL = "@ref AUpdater::StatusIdle"];
//
// ...;
// --- [label="Update published"];
// ...;
// a -> u [label = "checkForUpdates()", URL = "@ref AUpdater::checkForUpdates"];
// a <- u [label = "status = AUpdater::StatusCheckingForUpdates", URL = "@ref AUpdater::StatusCheckingForUpdates"];
// a <- u [label = "control flow"];
// u box u [label = "checkForUpdatesImpl()", URL = "@ref AUpdater::checkForUpdatesImpl"];
// u box u [label = "update was found"];
// a <- u [label = "status = AUpdater::StatusIdle", URL = "@ref AUpdater::StatusIdle"];
// ...;
// @endmsc
//
// You might want to store update check results (i.e., download url) in your implementation of
// @ref AUpdater::checkForUpdatesImpl so your @ref AUpdater::downloadUpdateImpl might reuse this information.
//
// ## Downloading the update
//
// When an update is found, your app should call @ref AUpdater::downloadUpdate to download and unpack the update. It is
// up to you to decide when to download an update. If you wish, you can call @ref AUpdater::downloadUpdate in
// @ref AUpdater::checkForUpdatesImpl to proceed to download process right after update was found (see
// @ref UPDATER_WORKFLOWS for more information about update workflow decisions). It calls
// user-defined @ref AUpdater::downloadUpdateImpl which might choose to call default
// `AUpdater::downloadAndUnpack(<YOUR DOWNLOAD URL>, unpackedUpdateDir)`.
//
// @msc
// a[label = "Your App"],
// u[label = "AUpdater", URL = "@ref AUpdater"];
// ...;
// a -> u [label = "downloadUpdate()", URL = "@ref AUpdater::downloadUpdate"];
// a <- u [label = "status = AUpdater::StatusDownloading", URL = "@ref AUpdater::StatusDownloading"];
// u box u [label = "downloadUpdateImpl()", URL = "@ref AUpdater::downloadUpdateImpl"];
// a <- u [label = "status = AUpdater::StatusWaitingForApplyAndRestart", URL = "@ref AUpdater::StatusWaitingForApplyAndRestart"];
// --- [label="Your App Prompts User to Update"];
// ...;
// @endmsc
//
// ## Applying (deploying) the update
//
// At this moment, AUpdater waits @ref AUpdater::applyUpdateAndRestart to be called. When
// @ref AUpdater::applyUpdateAndRestart is called (i.e., when user accepted update installation), AUpdater executes the
// newer copy of your app downloaded before with a special command line argument which is handled by
// @ref AUpdater::handleStartup in that executable. The initial app process is finished, closing your app window as
// well. From now, your app is in "downtime" state, so we need to apply the update and reopen app back again as quickly
// as possible. This action is required to perform update installation. The copy then replaces old application (where it
// actually installed) with itself (that is, the downloaded, newer copy). After operation is complete, it passes the
// control back to the updated application executable. At last, the newly updated application performs a cleanup after
// update.
//
// @msc
// a[label = "Your App"],
// u[label = "AUpdater", URL = "@ref AUpdater"],
// da[label = "Newer Copy of Your App"],
// du[label = "AUpdater in App Copy", URL = "@ref AUpdater"];
// a :> u [label = "applyUpdateAndRestart()", URL = "@ref AUpdater::applyUpdateAndRestart"];
// u :> da [label = "Execute with update arg"];
// u box u [label = "exit(0)"];
// a box u [label = "Process Finished"];
// da box du [label = "Process Started"];
// da -> du [label = "handleStartup", URL = "@ref AUpdater::handleStartup"];
// du box du [label = "AUpdater::deployUpdate(...)", URL = "@ref AUpdater::deployUpdate"];
// a <: du [label = "Execute"];
// du box du [label = "exit(0)"];
// da box du [label = "Process Finished"];
// a box u [label = "Process Started"];
// a -> u [label = "handleStartup", URL = "@ref AUpdater::handleStartup"];
// u box u [label = "cleanup download dir"];
// a box u [label="App Normal Lifecycle"];
// ...;
// @endmsc
//
// After these operations complete, your app is running in its normal lifecycle.
//

TEST(UpdaterTest, ExampleCase) { EXPECT_EQ(fake_entry({}), 0); }

TEST(UpdaterTest, Typical_Implementation) {   // HEADER_H1
    // AUpdater is an abstract class; it needs some functions to be implemented by you.
    //
    // In this example, let's implement auto update from GitHub release pages.
#ifdef AUI_CMAKE_PROJECT_VERSION
#undef AUI_CMAKE_PROJECT_VERSION
#endif
#define AUI_CMAKE_PROJECT_VERSION 0.0.1
    // AUI_DOCS_CODE_BEGIN
    static constexpr auto LOG_TAG = "MyUpdater";
    class MyUpdater: public AUpdater {
    public:
        ~MyUpdater() override = default;

    protected:
        AFuture<void> checkForUpdatesImpl() override {
            return async {
                try {
                    auto githubLatestRelease = aui::updater::github::latestRelease("aui-framework", "example_app");
                    ALogger::info(LOG_TAG) << "Found latest release: " << githubLatestRelease.tag_name;
                    auto ourVersion = aui::updater::Semver::fromString(AUI_PP_STRINGIZE(AUI_CMAKE_PROJECT_VERSION));
                    auto theirVersion = aui::updater::Semver::fromString(githubLatestRelease.tag_name);

                    if (theirVersion <= ourVersion) {
                        getThread()->enqueue([] {
                          AMessageBox::show(
                              nullptr, "No updates found", "You are running the latest version.", AMessageBox::Icon::INFO);
                        });
                        return;
                    }
                    aui::updater::AppropriatePortablePackagePredicate predicate {};
                    auto it = ranges::find_if(
                        githubLatestRelease.assets, predicate, &aui::updater::github::LatestReleaseResponse::Asset::name);
                    if (it == ranges::end(githubLatestRelease.assets)) {
                        ALogger::warn(LOG_TAG)
                            << "Newer version was found but a package appropriate for your platform is not available. "
                               "Expected: "
                            << predicate.getQualifierDebug() << ", got: "
                            << (githubLatestRelease.assets |
                                ranges::view::transform(&aui::updater::github::LatestReleaseResponse::Asset::name));
                        return;
                    }
                    ALogger::info(LOG_TAG) << "To download: " << (mDownloadUrl = it->browser_download_url);

                    getThread()->enqueue([this, self = shared_from_this(), version = githubLatestRelease.tag_name] {
                        if (AMessageBox::show(
                                nullptr, "New version found!", "Found version: {}\n\nWould you like to update?"_format(version),
                                AMessageBox::Icon::INFO, AMessageBox::Button::YES_NO) != AMessageBox::ResultButton::YES) {
                            return;
                        }

                        downloadUpdate();
                    });

                } catch (const AException& e) {
                    ALogger::err(LOG_TAG) << "Can't check for updates: " << e;
                    getThread()->enqueue([] {
                        AMessageBox::show(
                            nullptr, "Oops!", "There is an error occurred while checking for updates. Please try again later.",
                            AMessageBox::Icon::CRITICAL);
                    });
                }
            };
        }

        AFuture<void> downloadUpdateImpl(const APath& unpackedUpdateDir) override {
            return async {
              try {
                  AUI_ASSERTX(!mDownloadUrl.empty(), "make a successful call to checkForUpdates first");
                  downloadAndUnpack(mDownloadUrl, unpackedUpdateDir);
                  reportReadyToApplyAndRestart(makeDefaultInstallationCmdline());
              } catch (const AException& e) {
                  ALogger::err(LOG_TAG) << "Can't check for updates: " << e;
                  getThread()->enqueue([] {
                    AMessageBox::show(
                        nullptr, "Oops!", "There is an error occurred while downloading update. Please try again later.",
                        AMessageBox::Icon::CRITICAL);
                  });
              }
            };
        }

    private:
        AString mDownloadUrl;
    };
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


// # Updater workflows {#UPDATER_WORKFLOWS}
// When using AUpdater for your application, you need to consider several factors including usability, user experience,
// system resources, and particular needs of your project.
//
// Either way, you might want to implement a way to disable auto update feature in your application.
//
// ## Prompt user on every step
//
// This approach is implemented in AUI's [Example App Template](https://github.com/aui-framework/example_app).
//
// The updater checks for updater periodically or upon user request and informs the user that an update is available.
// The user then decides whether to proceed with update or not. If they agree the application will download and install
// the update.
//
// This way can be considered as better approach because the user may feel they control the situation and the
// application never does things that user never asked to (trust concerns). On the other hand, such requirement of
// additional user interaction to can distract them from doing their work, so these interactions should not be annoying.
//
// You should not use AMessageBox (unless user explicitly asked to check for update) as it literally interrupts the
// user's workflow, opting them to make a decision before they can continue their work. A great example of a bad auto
// update implementation is qBittorrent client on Windows: hence this application typically launches on OS startup,
// it checks for updates in background and pops the message box if update was found, **even if user is focused on
// another application or away from keyboard**.
//
// ## Silent download
//
// This approach is implemented in AUI's [Telegram client](https://github.com/aui-framework/telegram_client), as well
// as in official Qt-based Telegram Desktop client.
//
// The updater silently downloads the update in the background while the user continues working within the application
// or even other tasks. The update then is applied automatically upon restart.
// Optionally, the application might show a button/message/notification bubble to restart and apply update.
//
// Despite user trust concerns, this approach allows seamless experience - users don't need to be interrupted during
// their work. They even might not care about updates.
//
