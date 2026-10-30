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
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "AUI/Common/AString.h"
#include "AUI/IO/APath.h"
#include "AUI/Json/AJson.h"
#include "AUI/Platform/ASurface.h"

namespace aui::remote {

/**
 * @brief Decides which clients may drive the application.
 * @details
 * A client proves that a human allowed it by typing a 6-digit code that the application shows in its window (and writes
 * to the log). The client generates a key and sends it along with the code. This is a one-time procedure: the
 * application stores a hash of the key (see Pairing::persistTo), and the client connects with the key afterwards, even
 * when the application was restarted. For debugging, the trust can be limited to the lifetime of the process
 * (`remember = false`) or dropped for every connection (Pairing::setAlwaysRequireCode).
 *
 * A code is valid for a couple of minutes and for 3 attempts. Every code that is wasted (spent on wrong guesses or
 * replaced by a new one before it was used) counts against the client: after a few of them, `begin` is refused for a
 * period that doubles with every next wasted code, so the code can't be brute-forced by asking for a new one again and
 * again. A successful pairing resets the penalty.
 *
 * Thread-safe; the code overlay is shown and hidden on the UI thread.
 */
class API_AUI_DEV_EXTRAS Pairing {
public:
    static constexpr int MIN_KEY_LENGTH = 16;
    static constexpr int MAX_ATTEMPTS = 3;
    static constexpr std::chrono::seconds CODE_LIFETIME { 120 };
    static constexpr int FREE_WASTED_CODES = 3;
    static constexpr std::chrono::seconds MAX_PENALTY { 600 };

    /**
     * @brief What the application did with the key a client has just paired with.
     */
    enum class Trust {
        /// The key works on this connection only.
        CONNECTION,
        /// The key works until the application exits.
        SESSION,
        /// The key is stored and works after the application restarts.
        PERSISTENT,
    };

    /**
     * @brief Remembers paired keys in `file` (hashes only), so that a client needs the code only once.
     * @details
     * The file is read on every check, so the keys paired with another instance of the same application are
     * accepted as well.
     */
    void persistTo(APath file);

    /**
     * @brief Every connection has to be paired with a code, even by a client with a known key. Debug scenario.
     */
    void setAlwaysRequireCode(bool value);

    /**
     * @brief Trusts the key without a code, i.e., for applications launched by the client itself.
     */
    void authorize(const std::string& key);

    [[nodiscard]] bool isAuthorized(const std::string& key) const;

    /**
     * @return what happens with a key that is paired right now
     */
    [[nodiscard]] Trust trust() const;

    /**
     * @brief Generates a new code and shows it to the user.
     * @throws RpcException with `RATE_LIMITED` when too many codes were wasted recently
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
    std::vector<std::string> mKeys;         // launch keys and keys paired during this run
    std::vector<std::string> mLaunchKeys;   // kept even if every connection requires a code
    APath mFile;
    bool mAlwaysRequireCode = false;
    std::string mCode;
    int mAttemptsLeft = 0;
    std::chrono::steady_clock::time_point mExpires;
    int mWastedCodes = 0;
    std::chrono::steady_clock::time_point mBeginNotBefore;

    void wasteCode();   // mMutex must be held
    [[nodiscard]] bool isStored(const std::string& key) const;   // mMutex must be held
    void store(const std::string& key);                          // mMutex must be held
    void showOverlay(std::string code);
    void hideOverlay();

    // touched on the UI thread only; shared with the queued tasks, which may outlive Pairing
    struct Overlay {
        _weak<ASurface> window;
        _<AOverlappingSurface> surface;
        void close();
    };
    std::shared_ptr<Overlay> mOverlay = std::make_shared<Overlay>();
};

}   // namespace aui::remote
