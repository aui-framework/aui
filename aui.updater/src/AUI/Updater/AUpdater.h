// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once
#include <AUI/Common/AObject.h>
#include <AUI/Common/AStringVector.h>
#include <AUI/Common/AProperty.h>
#include <AUI/Thread/AAsyncHolder.h>
#include "AUI/IO/APath.h"
#include "AUI/Thread/AFuture.h"

/**
 * @brief Updater class.
 * @ingroup updater
 * @details
 * AUpdater follows strategy pattern, i.e., you are excepted to call its functions but the behaviour and conditions
 * are yours.
 */
class API_AUI_UPDATER AUpdater : public AObject {
public:
    ~AUpdater() override = default;

    /**
     * @brief Performs a pre-application AUpdater routine.
     * @param applicationArguments
     * @return
     */
    virtual bool needsExit(const AStringVector& applicationArguments);


    /**
     * @brief Deploy a downloaded update.
     * @details
     * Basically about replacing files (no network operations will be performed).
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
        AProperty<aui::float_within_0_1> progress;
    };

    /**
     * @brief Waiting to applyUpdateAndRestart call state.
     */
    struct StatusWaitingForApplyAndRestart {};

    using Status = std::variant<StatusIdle, StatusCheckingForUpdates, StatusDownloading, StatusWaitingForApplyAndRestart>;

    AProperty<Status> status;

    /**
     * @brief Sets status to StatusCheckingForUpdates and calls checkForUpdatesImpl, implemented by user.
     */
    void checkForUpdates() {
        status = StatusCheckingForUpdates{};
        mAsync << checkForUpdatesImpl().onFinally([this, self = shared_from_this()] {
            getThread()->enqueue([this, self] {
                status = StatusIdle{};
            });
        });
    }

    /**
     * @brief Starts downloading update. An implementation might expect to checkForUpdates to be called first.
     */
    void downloadUpdate() {
        status = StatusDownloading{};
        mAsync << downloadUpdateImpl((APath::getDefaultPath(APath::TEMP) / "__aui_update").makeDirs()).onFinally([this, self = shared_from_this()] {
            getThread()->enqueue([this, self] {
                status = StatusIdle{};
            });
        });
    }

protected:
    /**
     * @brief Holder for async operations.
     */
    AAsyncHolder mAsync;

    /**
     * @brief Performs update delivery to the specified directory.
     * @details
     * Typically implemented as download to temporary dir and unpacking the archive to the specified
     * unpackedUpdateDir.
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
};
