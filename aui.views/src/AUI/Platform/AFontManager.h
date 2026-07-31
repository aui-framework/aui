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
#include <memory>
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

    /**
     * A loaded fallback face with a per-face mutex to serialize FT operations.
     */
    struct FallbackFace {
        FT_FaceRec_* face = nullptr;
        std::unique_ptr<std::mutex> mtx = std::make_unique<std::mutex>();
    };

    std::mutex mFallbackMutex;
    AVector<FallbackFace> mFallbackFaces;
    AVector<FallbackCandidate> mDeferredCandidates;
    bool mFallbackAttempted = false;

    /**
     * @brief Loads a single fallback face from the given candidate.
     * @return true if the face was loaded successfully.
     */
    bool loadOneFallback(const FallbackCandidate& candidate);
    void ensureFallbackFaceLocked();
    void initFallback() {
        std::unique_lock lock(mFallbackMutex);
        ensureFallbackFaceLocked();
    }

    /**
     * Bundles a fallback face with the library and per-face mutex locks held
     * through the caller's FT operations on the returned face. The manager
     * mutex is NOT part of this lock: lockFallbackFace releases it before
     * returning, once face selection has finished.
     */
    struct FallbackFaceLock {
        FT_FaceRec_* face = nullptr;
        std::unique_lock<std::mutex> ftLock;    // FreeType::sFaceMutex (held through FT ops)
        std::unique_lock<std::mutex> faceLock;  // per-face mutex (held through FT ops)

        explicit operator bool() const noexcept { return face != nullptr; }
    };

    /**
     * Ensures fallback faces are loaded (once) and returns the first face
     * that contains the given codepoint. The manager mutex is released before
     * the function returns; the returned FallbackFaceLock carries the shared
     * FreeType library lock (sFaceMutex) and a per-face mutex, the latter two
     * held through the FT operations on the returned face.
     *
     * @note The first call may block while fallback fonts are discovered via
     *       fontconfig or filesystem probing under mFallbackMutex.
     */
    [[nodiscard]]
    FallbackFaceLock lockFallbackFace(char32_t codepoint);

    /**
     * @return true while deferred fallback candidates remain to be loaded
     *         lazily. Used by AFont to decide whether a cached failed glyph
     *         should be re-rendered: as long as discovery is still in
     *         progress, a later render may succeed once more faces load.
     */
    bool hasDeferredCandidates();

	AString getPathToFont(const AString& family);

    _<AFont> loadFont(const AUrl& url);


	friend class AFont;
};
