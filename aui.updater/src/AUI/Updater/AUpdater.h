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

#pragma once

#include <AUI/Common/AObject.h>
#include <AUI/Common/AStringVector.h>
#include <AUI/Common/AProperty.h>
#include <AUI/Thread/AAsyncHolder.h>
#include "AUI/IO/APath.h"
#include "AUI/Thread/AFuture.h"
#include <AUI/Platform/AProcess.h>

/**
 * @defgroup updater aui::updater
 * @brief Deliver updates on non-centralized distribution methods
 * @details
 * <!-- aui:experimental -->
 *
 * This module is purposed for delivering updates to your end users on distribution methods that do not support that by
 * themselves (i.e., occasional Windows installers, portables for Windows and Linux, macOS app bundles downloaded from
 * your website). Here's small breakdown:
 *
 * | Platform   | Distribution method           | Auto updating solution |
 * | ---------- | ----------------------------- | ---------------------- |
 * | [windows]  | Installer/exe                 | [updater]              |
 * | [windows]  | Microsoft Store (used rarely) | Microsoft Store        |
 * | [linux]    | Portables                     | [updater]              |
 * | [linux]    | Flatpak                       | Flathub                |
 * | [macos]    | DMG (*.app)                   | -                      |
 * | [macos]    | Apple App Store               | Apple App Store        |
 * | [android]  | APK                           | Google Play Store      |
 * | [ios]      | IPA                           | Apple App Store        |
 *
 * ??? note "What is portable?"
 *
 *     Portable is a term used to describe a software distribution method that does not require the user to install
 *     the software on their computer.
 *
 *     Portable software is typically distributed as a single executable file (i.e., a single file with all
 *     dependencies packed into it), usually not requiring to "install" it to the system.
 *
 * `aui.updater` module expects your program to be installed to user's directory (i.e., updating does not require admin
 * privileges). If that's not your case, you'll need to update your installer configuration ([INNOSETUP]) to install
 * to user's directory (i.e., in `AppData`).
 *
 * !!! note
 *
 *     Check out our [app-template] for a GitHub-hosted app template with auto update implemented.
 *
 * ## Supported platforms
 *
 * `aui::updater` supports the following platforms:
 *
 * - **Windows** - portables ([PORTABLE_WINDOWS]) only, installers to user's directory only ([INNOSETUP])
 * - **Linux** - portables only
 *
 * On a supported platform, `aui::updater` checks if the app executable is writable by the current user. If the
 * executable is not writeable, or running on a non-supported platform, `AUpdater` stubs it's methods (i.e., they do
 * nothing). You can check that the `aui::updater` functionality is engaged by calling `AUpdater::isAvailable()`.
 *
 * Updating process requires the initial application running instance to be stopped to replace its files with newer
 * ones. Additionally, the updater process starts the newer version of the application after replacing the files
 * (applying/deploying an update). To minimize downtime for end-users, the replacement should be seamless and quick and
 * thus the deployment process just copies newer files (overwriting old ones), it does not involve network operations.
 *
 * <!-- aui:parse_tests aui.updater/tests/UpdaterTest.cpp -->
 */

/**
 * @brief Updater class.
 * @ingroup updater
 * @details
 * <!-- aui:experimental -->
 * AUpdater follows a strategy pattern, i.e., you are excepted to call its functions, but the behavior and conditions
 * are up to you.
 *
 * Refer to [aui::updater] for the update process overview.
 */
class API_AUI_UPDATER AUpdater : public AObject {
public:
    /**
     * @brief Data required to launch installation child process.
     */
    struct InstallCmdline {
        /**
         * @brief Absolute path to installer executable.
         * @details
         * By default, installerExecutable is a copy of your application executable. The installation process is
         * triggered via installerArguments.
         */
        APath installerExecutable;

        /**
         * @brief Arguments passed to installer.
         */
        AVector<AString> installerArguments;
    };
    /**
     * @brief Context for AUpdater::getInstallationDirectory
     */
    struct GetInstallationDirectoryContext {
        /**
         * @brief Self process exe. Typically `AProcess::self()->getPathToExecutable()`.
         */
        APath selfProcessExePath = AProcess::self()->getPathToExecutable();

        /**
         * @brief Updater dir. Typically parent of selfProcessExePath.
         */
        APath updaterDir;

        /**
         * @brief The path to executable who invoked the installation process. Typically full path to the original
         * exe to replace.
         */
        APath originExe;
    };

    AUpdater();
    ~AUpdater() override = default;

    /**
     * @brief Performs post update cleanup routines.
     * @details
     * Default implementation deletes AUpdater::getUnpackedUpdateDir() dir.
     */
    virtual void handlePostUpdateCleanup();

    /**
     * @brief Performs a pre-application AUpdater routine.
     * @param applicationArguments arguments to your program.
     * @details
     * The arguments starting with `--aui-updater` are should be ignored by your application.
     *
     * Performs autoupdate-specific routines on startup of your application. In best scenario, this function should be
     * called by your application as early as possible.
     *
     * This function might terminate current process with std::exit(0), in case of performing autoupdate routines.
     *
     * This function will attempt to restore InstallCmdline state (StatusWaitingForApplyAndRestart) via
     * AUpdater::loadInstallCmdline. If the latter succeeds, AUpdate::triggerUpdateOnStartup is called to perform
     * on-startup update.
     *
     * This function handles following arguments to your application:
     * - `--aui-updater-origin` -
     * - `--aui-updater-wait-for-process` - maps to [AUpdater::handleWaitForProcess()] that instructs AUpdater to wait
     *   the specified process to finish before processing next argument(s).
     * - `--aui-updater-cleanup` - maps to [AUpdater::handlePostUpdateCleanup()] and returns control flow to normal
     *   execution of your application (last updating step)
     * - `--aui-updater-failed` - reports last error occurred while update deployment. See AUpdater::getLastDeploymentError().
     *
     * Refer to [aui::updater] for update process overview.
     */
    virtual void handleStartup(const AStringVector& applicationArguments);

    /**
     * @brief Deploy the downloaded update.
     * @details
     * Basically about replacing files (no network operations will be performed).
     *
     * Requires [AUpdater::status] = StatusWaitingForApplyAndRestart, otherwise has no effect.
     *
     * Terminates current process with `std::exit(0)`
     *
     * ### Debugging update deployment
     *
     * You can locate update deployment logs by locating deployment process id (pid). The PID is printed by this
     * function:
     *
     * ```
     * [06:49:33][UI thread][AUpdater][INFO]: applyUpdateAndRestart: started process pid=3708325, exe=...
     * ```
     *
     * Log files location are printed in the beginning of every AUI-based programs (if they use ALogger):
     *
     * ```
     * [06:49:14][UI thread][Logger][INFO]: Log file: /tmp/aui.3707546.log
     * ```
     *
     * You can now locate the log file by using subprocess' PID printed earlier:
     *
     * ```
     * cat /tmp/aui.3708325.log
     * [06:49:33][UI thread][Logger][INFO]: Log file: /tmp/aui.3708325.log
     * [06:49:33][UI thread][AUpdater][INFO]: --aui-updater-wait-for-process: 3707546 exited with 0
     * [06:49:33][UI thread][AUpdater][INFO]: deploying update: /tmp/__aui_update_example_app/download -> /home/...
     * [06:49:33][UI thread][AUpdater][ERR]: Can't deploy update, trying to launch original: ...
     *  - at 0x5555558f3aae aui_entry(AStringVector const&)(main.cpp:9)
     *  - at 0x5555556b0028 aui_main(int, char**, int (*)(AStringVector const&))(OSDesktop.cpp:160)
     *  - at 0x7ffff7261248(?:0)
     *  - at 0x7ffff726130b(?:0)
     *  - at 0x555555654195(?:0)
     * Caused by: (AIOException) AFileOutputStream: could not open /home/...: Text file busy
     *  - at 0x5555556c0443 aui::impl::lastErrorToException(AString)(ErrorToExceptionImpl.cpp:21)
     *  - at 0x55555568ec97 AFileOutputStream::open(bool)(AFileOutputStream.cpp:67)
     *  - at 0x55555568ed44 AFileOutputStream::AFileOutputStream(AString, bool)(AFileOutputStream.cpp:24)
     *  - at 0x5555556964a2 APath::copy(APath const&, APath const&)(APath.cpp:291)
     *  - at 0x555555641585 AUpdater::deployUpdate(APath const&, APath const&)(AUpdater.cpp:267)
     *  - at 0x5555558fb2ff AUpdater::handleStartup(AStringVector const&)(AUpdater.cpp:76)
     *  - at 0x5555558f3aae aui_entry(AStringVector const&)(main.cpp:9)
     *  - at 0x5555556b0028 aui_main(int, char**, int (*)(AStringVector const&))(OSDesktop.cpp:160)
     *  - at 0x7ffff7261248(?:0)
     *  - at 0x7ffff726130b(?:0)
     *  - at 0x555555654195(?:0)
     * ```
     */
    virtual void applyUpdateAndRestart();

    /**
     * @brief Idling.
     */
    struct StatusIdle {};

    /**
     * @brief Checking for updates.
     */
    struct StatusCheckingForUpdates {};

    /**
     * @brief Downloading state.
     */
    struct StatusDownloading {
        mutable AProperty<aui::float_within_0_1> progress;
    };

    /**
     * @brief AUpdater::isAvailable() evaluated to false.
     * @sa AUpdater::isAvailable
     */
    struct StatusNotAvailable {};

    /**
     * @brief Waiting to applyUpdateAndRestart call state.
     */
    struct StatusWaitingForApplyAndRestart {
        InstallCmdline installCmdline;
    };

    /**
     * @brief State of the updater.
     * @details
     * Status of the AUpdater to observe from outside, i.e., by UI.
     *
     * `status` is updated in UI thread only.
     *
     * `status` is designed in such a way the user can use their own custom status types or any of predefined ones:
     *
     * - [AUpdater::StatusIdle]
     * - [AUpdater::StatusCheckingForUpdates]
     * - [AUpdater::StatusDownloading]
     * - [AUpdater::StatusWaitingForApplyAndRestart]
     * - [AUpdater::StatusNotAvailable]
     *
     * These statuses might be set by AUpdater itself.
     *
     * <!-- aui:parse_tests aui.updater/tests/UpdaterStatusTest.cpp -->
     */
    AProperty<std::any> status;

    /**
     * @brief Sets `status` to [AUpdater::StatusCheckingForUpdates] and calls AUpdater::checkForUpdatesImpl, implemented
     * by user.
     * @details
     * Requires [AUpdater::status] = StatusIdle, otherwise has no effect.
     */
    void checkForUpdates();

    /**
     * @brief Sets `status` to [AUpdater::StatusDownloading] and calls AUpdater::downloadUpdateImpl, implemented by user.
     * @details
     * An implementation might expect to checkForUpdates to be called first.
     */
    void downloadUpdate();

    [[nodiscard]]
    const AOptional<AString>& getLastDeploymentError() const noexcept { return mLastDeploymentError; }

    /**
     * @brief Checks that updater functionality is available.
     * @details
     * For cases when AUpdater is available see [aui::updater]
     */
    static bool isAvailable();

protected:
    /**
     * @brief Holder for async operations.
     */
    AAsyncHolder mAsync;

    /**
     * @brief Constructs InstallCmdline with default arguments.
     * @details
     * By default, installerExecutable is a copy of your application executable. The installation process is
     * triggered via special arguments in installerArguments.
     */
    InstallCmdline makeDefaultInstallationCmdline() const;

    /**
     * @brief Triggers update routine.
     * @details
     * The function is called by AUpdater::handleStartup. If triggerUpdateOnStartup succeeds, it should terminate
     * execution of the current process.
     *
     * Requires [AUpdater::status] = StatusWaitingForApplyAndRestart.
     *
     * If you'd like to disable applying downloaded update on startup, stub this function.
     */
    virtual void triggerUpdateOnStartup();

    /**
     * @brief Performs update delivery to the specified directory.
     * @param unpackedUpdateDir location to unpack an update to.
     * @details
     * Typically implemented as download to temporary dir and unpacking the archive to the specified
     * unpackedUpdateDir.
     *
     * Unpack your application files to `unpackedUpdateDir`. AUpdater is responsible for cleaning this dir in the
     * future.
     */
    virtual AFuture<void> downloadUpdateImpl(const APath& unpackedUpdateDir) = 0;

    /**
     * @brief Check for updates user's implementation.
     */
    virtual AFuture<void> checkForUpdatesImpl() = 0;

    /**
     * @brief Typical download and unpack implementation.
     * @details
     * Called by downloadUpdateImpl. Updates AUpdate::status progress.
     */
    void downloadAndUnpack(AString downloadUrl, const APath& unpackedUpdateDir);

    /**
     * @brief Being called by downloadUpdateImpl, reports download percentage to `status`.
     * @details
     * Requires [AUpdater::status] = StatusDownloading, otherwise has no effect. Updates the value in UI thread.
     */
    void reportDownloadedPercentage(aui::float_within_0_1 progress);

    /**
     * @brief Returns a module name of your (your_app_name or your_app_name.exe, without a leading path).
     * @details
     * Default implementation determines module name using AProcess::self().
     *
     * The module name is used to construct temporary directory and locate an executable in a downloaded portable.
     */
    virtual AString getModuleName() const;

    /**
     * @brief Dumps InstallCmdline to temporary download directory, indicating that an update is ready to install.
     * Updates status accordingly.
     *
     * @sa saveCmdline
     * @sa applyUpdateAndRestart
     */
    void reportReadyToApplyAndRestart(InstallCmdline cmdline);

    /**
     * @brief Working directory for AUpdater. By default, points to some path in TEMP.
     */
    virtual APath getTempWorkDir() const;

    /**
     * @brief The path where the update is unpacked to.
     * @details
     * The dir persists between different launches.
     */
    virtual APath getUnpackedUpdateDir() const;

    /**
     * @brief Restores install command line state, if any.
     * @details
     * AUpdater::handleStartup will try to restore the state via loadInstallCmdline. If InstallCmdline is successfully
     * restored, AUpdater immediately triggers update routine with triggerUpdateOnStartup which exits the application.
     *
     * The implementation must ensure that subsequent startups would not trigger on-startup update to avoid endless
     * loops in case of failure, i.e, by deleting a file.
     *
     * @sa triggerUpdateOnStartup
     * @sa saveCmdline
     */
    virtual AOptional<InstallCmdline> loadInstallCmdline() const;

    /**
     * @brief Saves install command line to restore state when application is restarted.
     *
     * @sa loadInstallCmdline
     */
    virtual void saveCmdline(const InstallCmdline& cmdline) const;

    /**
     * @brief Handles --aui-updater-wait-for-process.
     */
    virtual void handleWaitForProcess(uint32_t pid);

    /**
     * @brief Injects --aui-updater-wait-for-process=THIS_PROCESS_PID as the first argument.
     */
    virtual AVector<AString> injectWaitForMyPid(AVector<AString> args);

    /**
     * @brief Deploys update by recursively copying (moving) files from source dir to destination dir.
     * @details
     * Called in newly downloaded executable.
     */
    virtual void deployUpdate(const APath& source, const APath& destination);

    /**
     * @brief Retrieves installation directory based on given context.
     * @details
     * Default implementation guesses installation directory based on
     */
    virtual APath getInstallationDirectory(const GetInstallationDirectoryContext& context);

    /**
     * @brief called by AUpdater::downloadUpdate before downloading update to cleanup AUpdater::getUnpackedUpdateDir()
     * dir.
     */
    virtual void cleanupUnpackedUpdateDirBeforeDownloading();

private:
    /**
     * @brief Last error reported by update deployment process via --aui-updater-failed=.
     */
    AOptional<AString> mLastDeploymentError;
};
