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

#include <chrono>
#include <mutex>
#include <string>
#include <vector>

#include "AUI/Common/AString.h"
#include "AUI/Json/AJson.h"
#include "AUI/Platform/ASurface.h"

namespace aui::remote {

/**
 * @brief Decides which clients may drive the application.
 * @details
 * A client proves that a human allowed it by typing a 6-digit code that the application shows in its window (and writes
 * to the log). The client generates a key and sends it along with the code; the key is remembered until the application
 * exits and lets the same client connect later without the code.
 *
 * A code is valid for a couple of minutes and for 3 attempts.
 *
 * Thread-safe; the code overlay is shown and hidden on the UI thread.
 */
class Pairing {
public:
    static constexpr int MIN_KEY_LENGTH = 16;
    static constexpr int MAX_ATTEMPTS = 3;
    static constexpr std::chrono::seconds CODE_LIFETIME { 120 };

    /**
     * @brief Trusts the key without a code, i.e., for applications launched by the client itself.
     */
    void authorize(const std::string& key);

    [[nodiscard]] bool isAuthorized(const std::string& key) const;

    /**
     * @brief Generates a new code and shows it to the user.
     */
    AJson begin();

    /**
     * @brief Checks the code and trusts the key.
     * @throws RpcException when the code is wrong or expired, or the key is too short
     */
    AJson confirm(const AString& code, const std::string& key);

    /**
     * @return the current code; empty if there is none. For tests.
     */
    [[nodiscard]] std::string currentCode() const;

private:
    mutable std::mutex mMutex;
    std::vector<std::string> mKeys;
    std::string mCode;
    int mAttemptsLeft = 0;
    std::chrono::steady_clock::time_point mExpires;

    void forgetCode();
    void showOverlay(std::string code);
    void hideOverlay();

    // touched on the UI thread only
    _weak<ASurface> mOverlayWindow;
    _<AOverlappingSurface> mOverlay;
};

}   // namespace aui::remote
