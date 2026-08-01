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
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
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
     * True while fallback discovery runs on mFallbackThread. While set,
     * lockFallbackFace returns no face immediately (rather than blocking the
     * caller), and hasDeferredCandidates reports true so that AFont keeps
     * re-rendering provisional/failed glyphs until discovery completes.
     */
    std::atomic<bool> mFallbackPending = false;
    std::thread mFallbackThread;

    /**
     * Incremented (release) whenever a new fallback face is loaded into
     * mFallbackFaces (see loadOneFallback). AFont records this value on each
     * cached glyph and re-renders a failed/provisional glyph only when the
     * counter has advanced, so a single face load triggers at most one
     * re-render per glyph instead of a re-render on every lookup.
     */
    std::atomic<uint64_t> mFallbackGeneration{0};

    /**
     * @brief Loads a single fallback face from the given candidate.
     * @return true if the face was loaded successfully.
     */
    bool loadOneFallback(const FallbackCandidate& candidate);
    void ensureFallbackFaceLocked();
    void fallbackDiscoveryWorker();
    /**
     * @brief Starts fallback discovery on a worker thread (once), so the
     *        fontconfig/filesystem probe and the eager first-face load never
     *        stall the calling (UI) thread.
     */
    void initFallback();

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
     * @note Discovery runs on a worker thread (initFallback). Calls made
     *       before discovery completes return no face immediately; once it
     *       completes, calls serialize on mFallbackMutex and may lazily load
     *       deferred candidates (bounded per call). The returned
     *       FallbackFaceLock must be released before calling
     *       hasDeferredCandidates again: the latter locks mFallbackMutex,
     *       while the former may still hold FreeType::sFaceMutex.
     */
    [[nodiscard]]
    FallbackFaceLock lockFallbackFace(char32_t codepoint);

    /**
     * @return true while fallback discovery is still running or deferred
     *         fallback candidates remain to be loaded lazily. Used by AFont to
     *         decide whether a cached failed/provisional glyph should be
     *         re-rendered: as long as this returns true, a later render may
     *         succeed once more faces load.
     */
    bool hasDeferredCandidates();

    /**
     * @return The fallback-discovery generation: incremented whenever a new
     *         fallback face becomes available. AFont re-renders a cached
     *         failed/provisional glyph only when this value differs from the
     *         generation recorded on the glyph (see AFont::Character::
     *         fallbackGeneration), so re-renders happen at most once per face
     *         load rather than on every lookup.
     */
    uint64_t fallbackGeneration() const {
        return mFallbackGeneration.load(std::memory_order_acquire);
    }

	AString getPathToFont(const AString& family);

    _<AFont> loadFont(const AUrl& url);


	friend class AFont;
};
