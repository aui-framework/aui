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

#include "AUI/Common/SharedPtr.h"
#include "AUI/Util/Manager.h"
#include "AUI/Font/AFontFamily.h"
#include <mutex>
#include "AUI/Font/AFont.h"

class API_AUI_VIEWS AFontManager {
public:
	AFontManager();
    AFontManager(const AFontManager&) = delete;
    virtual ~AFontManager();

    static AFontManager& inst();

	[[nodiscard]] _<AFontFamily> getDefaultFamily() {
        return mDefaultFamily;
    }
	[[nodiscard]] _<AFont> getDefaultFont() {
        return mDefaultFont;
    }

    [[nodiscard]]
    _<AFontFamily> getFontFamily(const AString& name) const {
        if (auto c = mFamilies.contains(name)) {
            return c->second;
        }
        return nullptr;
    }
    [[nodiscard]]
    _<AFont> getFont(const AUrl& url) {
        if (auto c = mLoadedFont.contains(url)) {
            return c->second;
        }
        return mLoadedFont[url] = loadFont(url);
    }
private:
    AMap<AUrl, _<AFont>> mLoadedFont;
    AMap<AString, _<AFontFamily>> mFamilies;
    _<FreeType> mFreeType;
    _<AFontFamily> mDefaultFamily;
    _<AFont> mDefaultFont;

    std::mutex mFallbackMutex;
    AByteBuffer mFallbackFontDataBuffer;
    FT_FaceRec_* mFallbackFace = nullptr;
    AString mFallbackFontPath;
    bool mFallbackAttempted = false;

    void ensureFallbackFaceLocked();
    bool tryLoadFallback(std::initializer_list<AString> candidates);

public:
    /**
     * Bundles the fallback mutex lock with the face pointer so the face can only
     * be accessed while the lock is held.
     */
    struct FallbackFaceLock {
        std::unique_lock<std::mutex> lock;
        FT_FaceRec_* face = nullptr;

        explicit operator bool() const noexcept { return face != nullptr; }
    };

    /**
     * Ensures the fallback face is loaded (once) and acquires the fallback mutex.
     * The returned face is valid only while the returned lock is held.
     *
     * @note The first call may block while the fallback font is discovered via
     *       fontconfig or filesystem probing under the mutex. Subsequent calls
     *       return immediately.
     */
    [[nodiscard]]
    FallbackFaceLock lockFallbackFace() {
        std::unique_lock lk(mFallbackMutex);
        ensureFallbackFaceLocked();
        return { std::move(lk), mFallbackFace };
    }

	AString getPathToFont(const AString& family);

    _<AFont> loadFont(const AUrl& url);


	friend class AFont;
};
