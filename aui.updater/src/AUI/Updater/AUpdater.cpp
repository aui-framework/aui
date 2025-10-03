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

#include <AUI/Curl/ACurl.h>
#include <AUI/IO/AByteBufferInputStream.h>
#include <AUI/Util/Archive.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/Platform/AProcess.h>
#include <AUI/Json/Conversion.h>
#include <AUI/Util/kAUI.h>
#include "AUpdater.h"

static constexpr auto LOG_TAG = "AUpdater";

static constexpr auto ARG_AUI_UPDATER = "--aui-updater";
static constexpr auto ARG_AUI_UPDATER_CLEANUP = "--aui-updater-cleanup";
static constexpr auto ARG_AUI_UPDATER_WAIT_FOR_PROCESS = "--aui-updater-wait-for-process=";
static constexpr auto ARG_AUI_UPDATER_ORIGIN = "--aui-updater-origin=";
static constexpr auto ARG_AUI_UPDATER_DIR = "--aui-updater-dir=";
static constexpr auto ARG_AUI_UPDATER_FAILED = "--aui-updater-failed=";

AUpdater::AUpdater() {
    if (!isAvailable()) {
        status = StatusNotAvailable {};
    } else {
        status = StatusIdle {};
    }
}

void AUpdater::handleStartup(const AStringVector& applicationArguments) {
    if (!isAvailable()) {
        return;
    }

    APath updaterOrigin, updaterDir;

    for (const auto& arg : applicationArguments) {
        if (!arg.startsWith(ARG_AUI_UPDATER)) {
            continue;
        }

        if (arg.startsWith(ARG_AUI_UPDATER_CLEANUP)) {
            handlePostUpdateCleanup();
            continue;
        }

        if (arg.startsWith(ARG_AUI_UPDATER_WAIT_FOR_PROCESS)) {
            handleWaitForProcess(
                arg.substr(std::string_view(ARG_AUI_UPDATER_WAIT_FOR_PROCESS).length()).toLongOrException());
            continue;
        }

        if (arg.startsWith(ARG_AUI_UPDATER_ORIGIN)) {
            updaterOrigin = APath(arg.substr(std::string_view(ARG_AUI_UPDATER_ORIGIN).length()));
            continue;
        }

        if (arg.startsWith(ARG_AUI_UPDATER_DIR)) {
            updaterDir = APath(arg.substr(std::string_view(ARG_AUI_UPDATER_DIR).length()));
            continue;
        }


        if (arg.startsWith(ARG_AUI_UPDATER_FAILED)) {
            mLastDeploymentError = arg.substr(std::string_view(ARG_AUI_UPDATER_FAILED).length());
            continue;
        }
    }

    if (!updaterDir.empty() || !updaterOrigin.empty()) {
        try {
            AUI_ASSERT(!updaterDir.empty() && !updaterOrigin.empty());
            APath destinationDir = getInstallationDirectory({
                                     .updaterDir = updaterDir,
                                     .originExe = updaterOrigin,
                                   });
            ALogger::info(LOG_TAG) << "deploying update: " << updaterDir << " -> " << destinationDir;
            deployUpdate(updaterDir, destinationDir);
            AProcess::ArgStringList args;
            args.list = injectWaitForMyPid({});
            args.list << ARG_AUI_UPDATER_CLEANUP;
            auto p = AProcess::create({.executable = updaterOrigin, .args = std::move(args)});
            p->run(ASubProcessExecutionFlags::DETACHED);
            ALogger::info(LOG_TAG) << "Post-update launch: " << p->toString();
        } catch (const AException& e) {
            ALogger::err(LOG_TAG) << "Can't deploy update: " << e;
            ALogger::info(LOG_TAG) << "Update deployment failed, trying to launch original: " << updaterOrigin;
            AProcess::ArgStringList args;
            args.list << "{}{}"_format(ARG_AUI_UPDATER_FAILED, e.getMessage().replacedAll("\"", ""));
            AProcess::create({
                               .executable = updaterOrigin,
                               .args = std::move(args),
                             })->run(ASubProcessExecutionFlags::DETACHED);
        }
        std::exit(0);
    }

    if (auto cmdline = loadInstallCmdline()) {
        status = StatusWaitingForApplyAndRestart { std::move(*cmdline) };
        triggerUpdateOnStartup();
    }
}

void AUpdater::applyUpdateAndRestart() {
    if (!isAvailable()) {
        return;
    }
    auto* info = std::any_cast<StatusWaitingForApplyAndRestart>(&(*status));
    if (!info) {
        return;
    }
    auto finalArgs = injectWaitForMyPid(std::move(info->installCmdline.installerArguments));
    auto p = AProcess::create({
      .executable = info->installCmdline.installerExecutable,
      .args = AProcess::ArgStringList { finalArgs },
    });
    p->run(ASubProcessExecutionFlags::DEFAULT | ASubProcessExecutionFlags::DETACHED);
    ALogger::info(LOG_TAG)
        << "applyUpdateAndRestart: started process " << p->toString();
    std::exit(0);
}

void AUpdater::downloadAndUnpack(AString downloadUrl, const APath& unpackedUpdateDir) {
    // [APathOwner_example]
    APathOwner tempFilePath(APath::nextRandomTemporary());
    {
        AFileOutputStream tempFileOs(tempFilePath);
        size_t downloadedBytes = 0;
        *ACurl::Builder(std::move(downloadUrl))
             .withWriteCallback([&](ACurl& c, AByteBufferView toWrite) {
                 tempFileOs << toWrite;
                 downloadedBytes += toWrite.size();
                 static constexpr auto PRECISION = 100;
                 // NOLINTNEXTLINE(*-integer-division)
                 reportDownloadedPercentage(float(PRECISION * downloadedBytes / c.getContentLength()) / float(PRECISION));
                 return toWrite.size();
             })
             .runAsync();
    }
    aui::archive::zip::read(
        AFileInputStream(tempFilePath), aui::archive::ExtractTo {
          .prefix = unpackedUpdateDir,
          .pathProjection = &APath::withoutUppermostFolder,
        });
    // [APathOwner_example]
}

void AUpdater::reportDownloadedPercentage(aui::float_within_0_1 progress) {
    getThread()->enqueue([this, self = shared_from_this(), progress] {
        if (auto statusProgress = std::any_cast<StatusDownloading>(&(*status))) {
            statusProgress->progress = progress;
        }
    });
}

void AUpdater::checkForUpdates() {
    if (!isAvailable()) {
        return;
    }
    if (!std::any_cast<StatusIdle>(&(*status))) {
        return;
    }
    status = StatusCheckingForUpdates {};
    mAsync << checkForUpdatesImpl().onFinally([this, self = shared_from_this()] {
        getThread()->enqueue([this, self] {
            if (!std::any_cast<StatusCheckingForUpdates>(&(*status))) {
                return;
            }
            status = StatusIdle {};
        });
    });
}

void AUpdater::downloadUpdate() {
    if (!isAvailable()) {
        return;
    }
    status = StatusDownloading {};
    cleanupUnpackedUpdateDirBeforeDownloading();
    mAsync << downloadUpdateImpl(getUnpackedUpdateDir()).onFinally([this, self = shared_from_this()] {
        getThread()->enqueue([this, self] {
            if (!std::any_cast<StatusDownloading>(&(*status))) {
                return;
            }
            status = StatusIdle {};
        });
    });
}

APath AUpdater::getUnpackedUpdateDir() const {
    auto result = getTempWorkDir() / "download";
    result.makeDirs();
    return result;
}

APath AUpdater::getTempWorkDir() const {
    return APath::getDefaultPath(APath::TEMP) / ("__aui_update_" + getModuleName());
}

AString AUpdater::getModuleName() const { return AProcess::self()->getModuleName(); }

AUpdater::InstallCmdline AUpdater::makeDefaultInstallationCmdline() const {
    auto exe = APath::find(getModuleName(), { getUnpackedUpdateDir() }, APathFinder::SINGLE | APathFinder::RECURSIVE);
    if (exe.empty()) {
        return {};
    }
    return {
        .installerExecutable =
            exe.at(0),
        .installerArguments = {
          "{}{}"_format(ARG_AUI_UPDATER_ORIGIN, AProcess::self()->getPathToExecutable()),
          "{}{}"_format(ARG_AUI_UPDATER_DIR, getUnpackedUpdateDir()),
        },
    };
}

AJSON_FIELDS(AUpdater::InstallCmdline, AJSON_FIELDS_ENTRY(installerExecutable) AJSON_FIELDS_ENTRY(installerArguments))

void AUpdater::reportReadyToApplyAndRestart(AUpdater::InstallCmdline cmdline) {
    saveCmdline(cmdline);
    getThread()->enqueue([this, self = shared_from_this(), cmdline = std::move(cmdline)] {
        status = StatusWaitingForApplyAndRestart { .installCmdline = std::move(cmdline) };
    });
}

void AUpdater::saveCmdline(const AUpdater::InstallCmdline& cmdline) const {
    AFileOutputStream(getUnpackedUpdateDir().parent() / "install.json") << aui::to_json(cmdline);
}

AOptional<AUpdater::InstallCmdline> AUpdater::loadInstallCmdline() const {
    try {
        auto path = getUnpackedUpdateDir().parent() / "install.json";
        if (path.isRegularFileExists()) {
            AUI_DEFER { path.removeFile(); };
            auto result = aui::from_json<AUpdater::InstallCmdline>(AJson::fromStream(AFileInputStream(path)));
            ALogger::info(LOG_TAG) << "Successfully loaded update installation cmd line from prev session: " << path;
        }
    } catch (const AException& e) {
        ALogger::err(LOG_TAG) << "Can't restore InstallCmdline: " << e;
    }
    return std::nullopt;
}

void AUpdater::triggerUpdateOnStartup() {
    try {
        applyUpdateAndRestart();
    } catch (const AException& e) {
        ALogger::err(LOG_TAG) << "Can't trigger update on startup: " << e;
    }
}

void AUpdater::handlePostUpdateCleanup() {
    try {
        for (const auto& dir : { getUnpackedUpdateDir(), getTempWorkDir() }) {
            ALogger::info(LOG_TAG) << "Post update cleanup: " << dir;
            dir.removeFileRecursive();
        }
    } catch (const AException& e) {
        ALogger::err(LOG_TAG) << "Can't perform post update cleanup: " << e;
    }
}

bool AUpdater::isAvailable() {
#if AUI_PLATFORM_WIN || AUI_PLATFORM_LINUX || AUI_PLATFORM_MACOS
    static bool v = AProcess::self()->getPathToExecutable().isEffectivelyAccessible(AFileAccess::W | AFileAccess::X);
    return v;
#else
    return false;
#endif
}

void AUpdater::handleWaitForProcess(uint32_t pid) {
    auto process = AProcess::fromPid(pid);
    if (!process) {
        return;
    }
    auto i = process->waitForExitCode();
    AThread::sleep(std::chrono::seconds(1)); // some additional sleep to wait the OS to release files lock
    ALogger::info(LOG_TAG) << "--aui-updater-wait-for-process: " << pid << " exited with " << i;
}

AVector<AString> AUpdater::injectWaitForMyPid(AVector<AString> args) {
    args.insert(args.begin(), "{}{}"_format(ARG_AUI_UPDATER_WAIT_FOR_PROCESS, AProcess::self()->getPid()));
    return args;
}

void AUpdater::deployUpdate(const APath& source, const APath& destination) {
    for (const auto& sourceFile : source.listDir(AFileListFlags::RECURSIVE | AFileListFlags::REGULAR_FILES)) {
        auto destinationFile = destination / sourceFile.relativelyTo(source);
        try {
            try {
                APath::move(sourceFile, destinationFile);
                ALogger::info(LOG_TAG) << "Moved: " << sourceFile << " -> " << destinationFile;
            } catch (...) {
                APath::copy(sourceFile, destinationFile);
                ALogger::info(LOG_TAG) << "Copied: " << sourceFile << " -> " << destinationFile;
            }
            destinationFile.chmod(0755);
        } catch (...) {
            throw AException("While copying {} -> {}"_format(sourceFile, destinationFile), std::current_exception());
        }
    }
}

APath AUpdater::getInstallationDirectory(const AUpdater::GetInstallationDirectoryContext& context) {
    // By selfLocation and updaterDir we make an assumption about the installation structure.
    // We lose the ability to re-locate exe installation path but hope it's would never be a case.

    if (!context.selfProcessExePath.startsWith(context.updaterDir)) {
        throw AException("can't determine installation structure: selfLocation={}, updaterDir={}"_format(
            context.selfProcessExePath, context.updaterDir));
    }
    APath relativePath = context.selfProcessExePath.relativelyTo(context.updaterDir);
    AUI_ASSERT(relativePath.isRelative());
    if (!context.originExe.endsWith(relativePath)) {
        throw AException(
            "malformed origin's exe installation structure. context.originExe={}, relativePath={}, context.selfProcessExePath={}, context.updaterDir={}"_format(
                context.originExe, relativePath, context.selfProcessExePath, context.updaterDir));
    }

    auto out = context.originExe.substr(0, context.originExe.length() - relativePath.length() - 1);
    return APath(out);
}

void AUpdater::cleanupUnpackedUpdateDirBeforeDownloading() {
    getUnpackedUpdateDir().removeDirContentsRecursive();
}
