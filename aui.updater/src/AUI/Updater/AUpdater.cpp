// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <AUI/Curl/ACurl.h>
#include <AUI/IO/AByteBufferInputStream.h>
#include <AUI/Util/Archive.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/Platform/AProcess.h>
#include <AUI/Json/Conversion.h>
#include <AUI/Util/kAUI.h>
#include "AUpdater.h"

static constexpr auto LOG_TAG = "AUpdater";

AUpdater::AUpdater() {
    if (!isAvailable()) {
        status = StatusNotAvailable{};
    }
}

void AUpdater::handleStartup(const AStringVector& applicationArguments) {
    if (!isAvailable()) {
        return;
    }
    for (const auto& arg : applicationArguments) {
        if (!arg.startsWith("--aui-updater")) {
            continue;
        }

        if (arg.startsWith("--aui-updater-cleanup")) {
            postUpdateCleanup();
            continue;
        }
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
    auto& info = std::get<StatusWaitingForApplyAndRestart>(*status);
    auto p = AProcess::create({
      .executable = info.installCmdline.installerExecutable,
      .args = AProcess::ArgStringList { std::move(info.installCmdline.installerArguments) },
    });
    p->run(ASubProcessExecutionFlags::DEFAULT | ASubProcessExecutionFlags::DETACHED);
    std::exit(0);
}

void AUpdater::downloadAndUnpack(AString downloadUrl, const APath& unpackedUpdateDir) {
    // [APathOwner example]
    APathOwner tempFilePath(APath::randomTemporary());
    {
        AFileOutputStream tempFileOs(tempFilePath);
        size_t downloadedBytes = 0;
        *ACurl::Builder(std::move(downloadUrl))
             .withWriteCallback([&](ACurl& c, AByteBufferView toWrite) {
                 tempFileOs << toWrite;
                 downloadedBytes += toWrite.size();
                 static constexpr auto PRECISION = 100;
                 reportDownloadedPercentage(
                     float((PRECISION * downloadedBytes / c.getContentLength())) / float(PRECISION));
                 return toWrite.size();
             })
             .runAsync();
    }
    aui::archive::zip::read(AFileInputStream(tempFilePath), aui::archive::ExtractTo { unpackedUpdateDir });
    // [APathOwner example]
}

void AUpdater::reportDownloadedPercentage(aui::float_within_0_1 progress) {
    getThread()->enqueue([this, self = shared_from_this(), progress] {
        if (!std::holds_alternative<StatusDownloading>(*status)) {
            return;
        }
        status = StatusDownloading { .progress = progress };
    });
}

void AUpdater::checkForUpdates() {
    if (!isAvailable()) {
        return;
    }
    status = StatusCheckingForUpdates {};
    mAsync << checkForUpdatesImpl().onFinally([this, self = shared_from_this()] {
        getThread()->enqueue([this, self] {
            if (!std::holds_alternative<StatusCheckingForUpdates>(*status)) {
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
    mAsync << downloadUpdateImpl(getDownloadDstDir()).onFinally([this, self = shared_from_this()] {
        getThread()->enqueue([this, self] {
            if (!std::holds_alternative<StatusDownloading>(*status)) {
                return;
            }
            status = StatusIdle {};
        });
    });
}

APath AUpdater::getDownloadDstDir() const {
    auto result = getTempWorkDir() / "download";
    result.makeDirs();
    return result;
}

APath AUpdater::getTempWorkDir() const {
    return APath::getDefaultPath(APath::TEMP) / ("__aui_update_" + getModuleName());
}

AString AUpdater::getModuleName() const { return AProcess::self()->getModuleName(); }

AUpdater::InstallCmdline AUpdater::makeDefaultInstallationCmdline() const {
    return {
        .installerExecutable =
            APath::find(getModuleName(), { getDownloadDstDir() }, APathFinder::SINGLE | APathFinder::RECURSIVE).at(0),
        .installerArguments = { "--aui-updater-origin={}"_format(AProcess::self()->getPathToExecutable()) }
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
    AFileOutputStream(getDownloadDstDir().parent() / "install.json") << aui::to_json(cmdline);
}

AOptional<AUpdater::InstallCmdline> AUpdater::loadInstallCmdline() const {
    try {
        auto path = getDownloadDstDir().parent() / "install.json";
        if (path.isRegularFileExists()) {
            AUI_DEFER { path.removeFile(); };
            return aui::from_json<AUpdater::InstallCmdline>(AJson::fromStream(AFileInputStream(path)));
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

void AUpdater::postUpdateCleanup() {
    getDownloadDstDir().removeFileRecursive();
}

bool AUpdater::isAvailable() {
#if AUI_PLATFORM_WIN || AUI_PLATFORM_LINUX || AUI_PLATFORM_MACOS
    static bool v = AProcess::self()->getPathToExecutable().isEffectivelyAccessible(AFileAccess::W | AFileAccess::X);
    return v;
#else
    return false;
#endif
}
