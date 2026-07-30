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
#include <AUI/Common/AVector.h>
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
public:
    /**
     * A candidate font path with an optional face index (for TTC collections).
     */
    struct FallbackCandidate {
        AString path;
        int faceIndex = 0;
    };

private:
    AMap<AUrl, _<AFont>> mLoadedFont;
    AMap<AString, _<AFontFamily>> mFamilies;
    _<FreeType> mFreeType;
    _<AFontFamily> mDefaultFamily;
    _<AFont> mDefaultFont;

    std::mutex mFallbackMutex;
    AVector<FT_FaceRec_*> mFallbackFaces;
    bool mFallbackAttempted = false;

    void ensureFallbackFaceLocked();
    bool tryLoadFallback(std::initializer_list<FallbackCandidate> candidates);
    void initFallback() {
        std::unique_lock lock(mFallbackMutex);
        ensureFallbackFaceLocked();
    }

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
     * Ensures fallback faces are loaded (once) and acquires the fallback mutex.
     * Iterates loaded faces and returns the first one that contains the given
     * codepoint, or nullptr if none does. The returned face is valid only while
     * the returned lock is held.
     *
     * @note The first call may block while fallback fonts are discovered via
     *       fontconfig or filesystem probing under the mutex. Subsequent calls
     *       return immediately.
     */
    [[nodiscard]]
    FallbackFaceLock lockFallbackFace(char32_t codepoint);

	AString getPathToFont(const AString& family);

    _<AFont> loadFont(const AUrl& url);


	friend class AFont;
};
