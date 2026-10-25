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

#include "Pairing.h"

#include <random>

#include "AUI/ASS/ASS.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Thread/AThread.h"
#include "AUI/Util/ALayoutInflater.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "JsonRpc.h"

namespace aui::remote {

namespace {
constexpr auto LOG_TAG = "ARemoteAgent";

bool constantTimeEquals(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) {
        return false;
    }
    unsigned char difference = 0;
    for (std::size_t i = 0; i < a.size(); ++i) {
        difference |= static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i]);
    }
    return difference == 0;
}

std::string randomCode() {
    std::random_device device;
    std::uniform_int_distribution<int> digit(0, 9);
    std::string code;
    for (int i = 0; i < 6; ++i) {
        code.push_back(static_cast<char>('0' + digit(device)));
    }
    return code;
}
}   // namespace

void Pairing::authorize(const std::string& key) {
    std::lock_guard lock(mMutex);
    mKeys.push_back(key);
}

bool Pairing::isAuthorized(const std::string& key) const {
    std::lock_guard lock(mMutex);
    bool found = false;
    for (const auto& k : mKeys) {
        found |= constantTimeEquals(k, key);   // no early exit
    }
    return found && !key.empty();
}

AJson Pairing::begin() {
    std::string code;
    {
        std::lock_guard lock(mMutex);
        mCode = randomCode();
        mAttemptsLeft = MAX_ATTEMPTS;
        mExpires = std::chrono::steady_clock::now() + CODE_LIFETIME;
        code = mCode;
    }
    ALogger::info(LOG_TAG) << "A client asks to drive this application. Pairing code: " << code;
    showOverlay(code);
    return AJson {
        { "pending", true },
        { "expiresInSeconds", int(CODE_LIFETIME.count()) },
    };
}

AJson Pairing::confirm(const AString& code, const std::string& key) {
    if (key.size() < MIN_KEY_LENGTH) {
        throw RpcException(rpc_code::INVALID_PARAMS, "the key is too short",
                           "generate a random key of at least 16 characters");
    }
    int attemptsLeft = 0;
    {
        std::lock_guard lock(mMutex);
        const bool expired = std::chrono::steady_clock::now() > mExpires;
        if (mCode.empty() || mAttemptsLeft <= 0 || expired) {
            throw RpcException(rpc_code::UNAUTHORIZED, "there is no valid pairing code",
                               "call pair.begin and read the new code in the application window");
        }
        if (!constantTimeEquals(code.toStdString(), mCode)) {
            attemptsLeft = --mAttemptsLeft;
            if (attemptsLeft <= 0) {
                mCode.clear();
            }
        } else {
            mKeys.push_back(key);
            mCode.clear();
            mAttemptsLeft = 0;
            attemptsLeft = -1;
        }
    }
    if (attemptsLeft >= 0) {
        if (attemptsLeft == 0) {
            hideOverlay();
        }
        throw RpcException(rpc_code::UNAUTHORIZED, "wrong code",
                           attemptsLeft > 0 ? "attempts left: " + AString::number(attemptsLeft)
                                            : AString("no attempts left, call pair.begin again"));
    }
    hideOverlay();
    ALogger::info(LOG_TAG) << "A client is paired";
    return AJson { { "paired", true } };
}

std::string Pairing::currentCode() const {
    std::lock_guard lock(mMutex);
    return mCode;
}

void Pairing::showOverlay(std::string code) {
    AThread::main()->enqueue([this, code = std::move(code)] {
        hideOverlay();
        _<ASurface> window;
        for (const auto& w : ASurface::getWindowManager().getWindows()) {
            if (!w->isPopup()) {
                window = w;
                break;
            }
        }
        if (!window) {
            return;   // the code is in the log
        }
        constexpr glm::ivec2 SIZE { 380, 130 };
        mOverlay = window->createOverlappingSurface((window->getSize() - SIZE) / 2, SIZE);
        using namespace declarative;
        ALayoutInflater::inflate(mOverlay,
                                 Vertical {
                                   Label { "An agent wants to drive this application" },
                                   Label { AString::fromUtf8(code) } AUI_OVERRIDE_STYLE { FontSize { 28_pt } },
                                   Label { "Enter the code in aui-remote" },
                                 } AUI_OVERRIDE_STYLE { Padding { 12_dp }, BackgroundSolid { 0x303030_rgb }, Border { 1_px, 0x808080_rgb } });
        mOverlay->pack();
        mOverlayWindow = window;
    });
}

void Pairing::hideOverlay() {
    AThread::main()->enqueue([this] {
        if (auto window = mOverlayWindow.lock(); window && mOverlay) {
            window->closeOverlappingSurface(mOverlay);
        }
        mOverlay = nullptr;
    });
}

}   // namespace aui::remote
