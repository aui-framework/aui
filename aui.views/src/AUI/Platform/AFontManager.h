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
#include <cstddef>
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
     * data keeps the font file bytes alive: FT_New_Memory_Face does not copy
     * the buffer, so it must outlive the face.
     */
    struct FallbackFace {
        FT_FaceRec_* face = nullptr;
        AByteBuffer data;
        std::unique_ptr<std::mutex> mtx = std::make_unique<std::mutex>();
    };

    std::mutex mFallbackMutex;
    AVector<FallbackFace> mFallbackFaces;
    AVector<FallbackCandidate> mDeferredCandidates;
    bool mFallbackAttempted = false;

    /**
     * True once std::thread creation for the fallback worker failed: no
     * worker can ever run discovery, and discovery must not run on the
     * calling (UI) thread, so lockFallbackFace reports a miss instead.
     * Written under mFallbackMutex (startFallbackWorker) and read under
     * mFallbackMutex (lockFallbackFace).
     */
    bool mFallbackWorkerUnavailable = false;

    /**
     * True while a fallback worker run (discovery or a deferred candidate
     * load) is in flight on mFallbackThread. While set, lockFallbackFace
     * returns no face immediately (rather than blocking the caller).
     */
    std::atomic<bool> mFallbackPending = false;
    std::thread mFallbackThread;

    /**
     * Incremented (release) whenever a new fallback face is loaded into
     * mFallbackFaces or the deferred candidate pool changes (see
     * loadOneFallbackLocked and lockFallbackFace). AFont records this value
     * on each cached glyph and re-renders a failed/provisional glyph only
     * when the counter has advanced, so a pool change triggers at most one
     * re-render per glyph instead of a re-render on every lookup.
     */
    std::atomic<uint64_t> mFallbackGeneration{0};

    /**
     * @brief Loads a single fallback face from the given candidate.
     * @note The caller must hold mFallbackMutex: this function mutates
     *       mFallbackFaces and bumps mFallbackGeneration on success.
     * @return true if the face was loaded successfully.
     */
    bool loadOneFallbackLocked(const FallbackCandidate& candidate);

    /**
     * @brief Returns the platform's CJK fallback candidates, best first.
     *        Implemented per platform in
     *        AUI/Platform/<platform>/AFontManagerImpl.cpp. May log the
     *        reason and return an empty list when no candidates are available.
     *        Does not access instance state; callable from any thread.
     */
    static AVector<FallbackCandidate> fallbackCandidates();

    void ensureFallbackFaceLocked();
    void fallbackDiscoveryWorker();
    /**
     * @brief Starts one fallback worker run (discovery on first call, then
     *        one deferred candidate load per subsequent call). The caller
     *        must hold mFallbackMutex. No-op while a run is in flight.
     */
    void startFallbackWorker();
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
     *       completes, calls serialize on mFallbackMutex and may kick the
     *       worker to load a deferred candidate in the background (never on
     *       the calling thread). If no worker could be started at all
     *       (thread creation failed), every call returns no face.
     */
    [[nodiscard]]
    FallbackFaceLock lockFallbackFace(char32_t codepoint);

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
