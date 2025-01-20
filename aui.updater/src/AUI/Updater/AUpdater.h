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
#include "AUI/IO/APath.h"
#include "AUI/Thread/AFuture.h"

/**
 * @brief Updater class.
 * @ingroup updater
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
     * @brief Checks for an update.
     */
    void checkForUpdates();

    /**
     * @brief Deploy a downloaded update.
     * @details
     * Basically about replacing files (no network operations will be performed).
     */
    virtual void performUpdate();


    /**
     * @brief Idling.
     */
    struct StatusIdle {};

    /**
     * @brief Downloading state.
     */
    struct StatusDownloading {

    };

    /**
     * @brief Waiting to performUpdate call state.
     */
    struct StatusWaitingToPerformUpdate {

    };

    using Status = std::variant<StatusIdle, StatusDownloading, StatusWaitingToPerformUpdate>;

protected:
    /**
     * @brief Performs update delivery to the specified directory.
     * @details
     * Typically implemented as download to temporary dir and unpacking the archive to the specified
     * unpackedUpdateDir.
     */
    virtual AFuture<APath> deliverUpdateIfNeeded(const APath& unpackedUpdateDir) = 0;
};
