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

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>

#include "AUI/ASS/ASS.h"
#include "AUI/Crypt/AHash.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Thread/AThread.h"
#include "AUI/Util/ALayoutInflater.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "JsonRpc.h"
#include "AUI/View/AButton.h"

namespace aui::remote {

namespace {
constexpr auto LOG_TAG = "ARemoteDebug";

// FontSize in _pt and a declarative Button/CheckBox with a lambda in the same function crash MSVC (C1001),
// so the styled label is built here.
_<AView> codeLabel(const std::string& code) {
    using namespace declarative;
    return Label { AString::fromUtf8(code) } AUI_OVERRIDE_STYLE { TextColor { 0xffffff_rgb }, FontSize { 28_pt } };
}

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

std::string hashOf(const std::string& key) { return AHash::sha256(AByteBufferView(key)).toHexString().toStdString(); }

constexpr std::size_t MAX_STORED_KEYS = 64;

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

void Pairing::persistTo(APath file) {
    std::lock_guard lock(mMutex);
    mFile = std::move(file);
}

void Pairing::setAlwaysRequireCode(bool value) {
    std::lock_guard lock(mMutex);
    mAlwaysRequireCode = value;
}

void Pairing::authorize(const std::string& key) {
    std::lock_guard lock(mMutex);
    mLaunchKeys.push_back(key);
}

bool Pairing::isAuthorized(const std::string& key) const {
    if (key.empty()) {
        return false;
    }
    std::lock_guard lock(mMutex);
    bool found = false;
    for (const auto& k : mLaunchKeys) {
        found |= constantTimeEquals(k, key);   // no early exit
    }
    if (mAlwaysRequireCode) {
        return found;
    }
    for (const auto& k : mKeys) {
        found |= constantTimeEquals(k, key);
    }
    return found || isStored(key);
}

Pairing::Trust Pairing::trust() const {
    std::lock_guard lock(mMutex);
    if (mAlwaysRequireCode) {
        return Trust::CONNECTION;
    }
    return mFile.empty() ? Trust::SESSION : Trust::PERSISTENT;
}

bool Pairing::isStored(const std::string& key) const {
    if (mFile.empty()) {
        return false;
    }
    std::ifstream in(mFile.toStdString());
    const auto wanted = hashOf(key);
    bool found = false;
    for (std::string line; std::getline(in, line);) {
        found |= constantTimeEquals(line, wanted);
    }
    return found;
}

void Pairing::store(const std::string& key) {
    if (mAlwaysRequireCode) {
        return;   // the key is good for the connection that paired, nothing more
    }
    mKeys.push_back(key);
    if (mFile.empty()) {
        return;
    }
    try {
        std::vector<std::string> hashes;
        {
            std::ifstream in(mFile.toStdString());
            for (std::string line; std::getline(in, line);) {
                if (!line.empty()) {
                    hashes.push_back(line);
                }
            }
        }
        hashes.push_back(hashOf(key));
        if (hashes.size() > MAX_STORED_KEYS) {
            hashes.erase(hashes.begin(), hashes.end() - MAX_STORED_KEYS);
        }
        mFile.parent().makeDirs();
        { std::ofstream(mFile.toStdString(), std::ios::trunc); }   // restrict before the content gets in
        std::error_code ec;
        std::filesystem::permissions(mFile.toStdString(),
                                     std::filesystem::perms::owner_read | std::filesystem::perms::owner_write,
                                     std::filesystem::perm_options::replace, ec);
        std::ofstream out(mFile.toStdString(), std::ios::trunc);
        for (const auto& hash : hashes) {
            out << hash << '\n';
        }
    } catch (const AException& e) {
        ALogger::warn(LOG_TAG) << "Unable to remember the paired client: " << e;
    }
}

AJson Pairing::begin() {
    std::string code;
    {
        std::lock_guard lock(mMutex);
        const auto now = std::chrono::steady_clock::now();
        if (now < mBeginNotBefore) {
            const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(mBeginNotBefore - now).count() + 1;
            throw RpcException(rpc_code::RATE_LIMITED, "too many pairing codes were wasted",
                               "wait " + AString::number(int(seconds)) + " seconds before pair.begin");
        }
        if (!mCode.empty() && mAttemptsLeft > 0 && now <= mExpires) {
            wasteCode();   // the previous code is replaced before it was used
        }
        mCode = randomCode();
        mAttemptsLeft = MAX_ATTEMPTS;
        mExpires = std::chrono::steady_clock::now() + CODE_LIFETIME;
        code = mCode;
    }
    ALogger::info(LOG_TAG) << "An AUI debugger wants to attach. Pairing code: " << code;
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
                wasteCode();
            }
        } else {
            store(key);
            mCode.clear();
            mAttemptsLeft = 0;
            attemptsLeft = -1;
            mWastedCodes = 0;
            mBeginNotBefore = {};
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
    const char* trust = "connection";
    switch (this->trust()) {
        case Trust::CONNECTION: break;
        case Trust::SESSION: trust = "session"; break;
        case Trust::PERSISTENT: trust = "persistent"; break;
    }
    return AJson { { "paired", true }, { "trust", trust } };
}

void Pairing::wasteCode() {
    if (++mWastedCodes <= FREE_WASTED_CODES) {
        return;
    }
    const int doublings = std::min(mWastedCodes - FREE_WASTED_CODES - 1, 10);
    const auto penalty = std::min<std::chrono::seconds>(std::chrono::seconds(1LL << doublings), MAX_PENALTY);
    mBeginNotBefore = std::chrono::steady_clock::now() + penalty;
}

std::string Pairing::currentCode() const {
    std::lock_guard lock(mMutex);
    return mCode;
}

void Pairing::showOverlay(std::string code) {
    AThread::main()->enqueue([state = mOverlay, code = std::move(code)] {
        state->close();   // synchronously: a queued close would hit the overlay created below
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
        state->surface = window->createOverlappingSurface((window->getSize() - SIZE) / 2, SIZE);
        using namespace declarative;
        ALayoutInflater::inflate(state->surface,
                                 Vertical {
                                   Label { "An AUI debugger wants to attach to this application" } AUI_OVERRIDE_STYLE { TextColor { 0xffffff_rgb } },
                                   codeLabel(code),
                                   Label { "Enter the code to aui-remote" } AUI_OVERRIDE_STYLE { TextColor { 0xffffff_rgb } },
                                   Button {
                                       .content = Label { "Dismiss" },
                                       .onClick = [overlay = state->surface.weak()] {
                                           AUI_NULLSAFE(overlay.lock())->close();
                                       },
                                   },
                                 } AUI_OVERRIDE_STYLE { Padding { 12_dp }, BackgroundSolid { 0x303030_rgb }, Border { 1_px, 0x808080_rgb } });
        state->surface->pack();
        state->window = window;
    });
}

void Pairing::Overlay::close() {
    if (auto w = window.lock(); w && surface) {
        w->closeOverlappingSurface(surface);
    }
    surface = nullptr;
}

void Pairing::hideOverlay() {
    AThread::main()->enqueue([state = mOverlay] { state->close(); });
}

}   // namespace aui::remote
