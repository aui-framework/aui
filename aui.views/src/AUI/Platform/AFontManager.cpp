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

#include <AUI/Url/AUrl.h>
#include "AFontManager.h"
#include "AUI/Platform/APlatform.h"
#include "AUI/Font/FreeType.h"
#include "AUI/Logging/ALogger.h"
#include <AUI/Common/AByteBuffer.h>
#include <AUI/IO/AFileInputStream.h>

#if AUI_PLATFORM_WIN
// Reading a font file by its UTF-16 path (see loadOneFallbackLocked): the
// narrow CRT fopen() cannot address non-ASCII font paths (e.g. a Windows
// directory that contains non-ASCII characters).
#include <windows.h>
#include <cstdio>
#include <cwchar>
#include <string>
#endif

AFontManager::~AFontManager() {
    if (mFallbackThread.joinable()) {
        // Never destroy faces while discovery may still touch them.
        mFallbackThread.join();
    }
    std::scoped_lock lock(mFallbackMutex);
    for (auto& fb : mFallbackFaces) {
        // Hold both the library lock and this face's per-face lock so that
        // FT_Done_Face cannot overlap an in-flight glyph load on the same face.
        std::scoped_lock ftLock(FreeType::sFaceMutex, *fb.mtx);
        FT_Done_Face(fb.face);
    }
    mFallbackFaces.clear();
}

void AFontManager::initFallback() {
    if (mFallbackThread.joinable()) {
        return;   // already running (only joined at destruction)
    }
    mFallbackPending.store(true, std::memory_order_release);
    try {
        mFallbackThread = std::thread(&AFontManager::fallbackDiscoveryWorker, this);
    } catch (const std::system_error& e) {
        // Fallback discovery is best-effort; never fail font manager construction.
        mFallbackPending.store(false, std::memory_order_release);
        ALogger::warn("Font") << "Could not start fallback discovery thread: " << e.what();
    }
}

void AFontManager::fallbackDiscoveryWorker() {
    try {
        std::unique_lock lock(mFallbackMutex);
        ensureFallbackFaceLocked();
    } catch (const std::exception& e) {
        // Best-effort: an exception leaving a thread function would call
        // std::terminate, so discovery failures must not escape.
        ALogger::warn("Font") << "Fallback discovery failed: " << e.what();
    } catch (...) {
        ALogger::warn("Font") << "Fallback discovery failed with an unknown exception";
    }
    // Release-store: the mutex-protected discovery result (loaded faces,
    // deferred candidates) is visible to any thread that observes the clear.
    mFallbackPending.store(false, std::memory_order_release);
}

bool AFontManager::loadOneFallbackLocked(const FallbackCandidate& candidate) {
    // Read the font file into memory BEFORE taking FreeType::sFaceMutex:
    // FT_New_Face parses the file from disk while holding the lock, and that
    // lock serializes every glyph render, kerning query and hasGlyph call.
    // Disk IO must not run inside the critical section. (On Windows the file
    // is opened by its UTF-16 path: the narrow CRT fopen() cannot address
    // non-ASCII font paths.)
    AByteBuffer fontData;
#if AUI_PLATFORM_WIN
    const std::u16string utf16 = candidate.path.toUtf16();
    std::wstring wpath(utf16.begin(), utf16.end());
    if (FILE* file = _wfopen(wpath.c_str(), L"rb")) {
        if (fseek(file, 0, SEEK_END) == 0) {
            const long size = ftell(file);
            if (size > 0 && fseek(file, 0, SEEK_SET) == 0) {
                fontData.resize(size);
                if (fread(fontData.data(), 1, size, file) != size) {
                    fontData = {};   // truncated read: treat as a load failure
                }
            }
        }
        fclose(file);
    }
#else
    fontData = AByteBuffer::fromStream(std::make_unique<AFileInputStream>(candidate.path));
#endif
    if (fontData.empty()) {
        return false;
    }

    std::lock_guard lock(FreeType::sFaceMutex);
    FT_Face face = nullptr;
    // FT_New_Memory_Face does not copy the buffer: the FallbackFace entry
    // keeps it alive for the lifetime of the face.
    if (FT_New_Memory_Face(mFreeType->getFt(), (const FT_Byte*) fontData.data(), fontData.getSize(),
                           candidate.faceIndex, &face) != 0) {
        return false;
    }
    mFallbackFaces.push_back({face, std::move(fontData)});
    // Publish the new face: bump the generation (release) so cached
    // failed/provisional glyphs re-render at most once per face load.
    mFallbackGeneration.fetch_add(1, std::memory_order_release);
    ALogger::info("Font") << "Loaded CJK fallback font: " << candidate.path;
    return true;
}

void AFontManager::ensureFallbackFaceLocked() {
    if (mFallbackAttempted) {
        return;
    }
    mFallbackAttempted = true;   // Discovery is one-shot regardless of outcome.

    // Build the full candidate list per platform, then load only the first
    // candidate eagerly and defer the rest for lazy loading in lockFallbackFace.
    AVector<FallbackCandidate> allCandidates = fallbackCandidates();
    if (allCandidates.empty()) {
        // The platform hook has already logged the reason (no CJK-capable
        // font found, or fallback discovery unavailable on this platform).
        return;
    }

    // Load the first candidate eagerly (pre-warm).
    if (!loadOneFallbackLocked(allCandidates.first())) {
        // The eager candidate is deliberately not added to mDeferredCandidates
        // (it was already attempted), so a failure here is silent unless logged:
        // missing files (e.g. msyh.ttc on a Windows edition without Simplified
        // Chinese fonts) would otherwise be undiagnosable until a CJK codepoint
        // renders tofu.
        ALogger::warn("Font") << "Could not load fallback font: " << allCandidates.first().path;
    }

    // Defer remaining candidates for lazy loading in lockFallbackFace.
    for (size_t i = 1; i < allCandidates.size(); ++i) {
        mDeferredCandidates.push_back(allCandidates[i]);
    }
    if (!mDeferredCandidates.empty()) {
        // The candidate pool changed even if no face was loaded: publish the
        // change so cached failed/provisional glyphs retry against the
        // deferred candidates (a retry may consume them lazily).
        mFallbackGeneration.fetch_add(1, std::memory_order_release);
    }
    mDeferredCount.store(mDeferredCandidates.size(), std::memory_order_release);
}

AFontManager::FallbackFaceLock AFontManager::lockFallbackFace(char32_t codepoint) {
    // Discovery is still running on the worker thread: no face is ready yet,
    // so report a miss immediately rather than blocking the caller. AFont
    // re-renders provisional/failed glyphs while hasDeferredCandidates()
    // returns true, so these codepoints are retried once discovery completes.
    if (mFallbackPending.load(std::memory_order_acquire)) {
        return { nullptr };
    }

    std::unique_lock lk(mFallbackMutex);
    ensureFallbackFaceLocked();

    // Check all already-loaded faces first. Every FreeType call on any face
    // of the shared FT_Library must hold the library lock (sFaceMutex): the
    // library's internal state (e.g. the raster pool) is not thread-safe, so
    // the FT_Get_Char_Index probe must not run concurrently with rendering
    // on the primary face or another fallback face. The per-face mutex is
    // additionally held so a concurrent FT_Done_Face cannot destroy the face
    // mid-probe. Both locks are transferred into the returned FallbackFaceLock.
    for (auto& fb : mFallbackFaces) {
        std::unique_lock ftLock(FreeType::sFaceMutex);
        std::unique_lock faceLock(*fb.mtx);
        if (FT_Get_Char_Index(fb.face, codepoint) != 0) {
            return { fb.face, std::move(ftLock), std::move(faceLock) };
        }
    }

    // No loaded face has this codepoint; try deferred candidates lazily.
    // Bound to a few attempts per call to avoid draining the entire queue on
    // the UI thread for a codepoint that no candidate covers (e.g. emoji,
    // rare symbols). The counter counts attempts, not successes: failing
    // candidates (e.g. font files absent on this OS version) must not drain
    // the whole queue in one call either.
    constexpr int kMaxDeferredLoadAttemptsPerCall = 2;
    int attempts = 0;
    while (!mDeferredCandidates.empty() && attempts < kMaxDeferredLoadAttemptsPerCall) {
        auto c = mDeferredCandidates.first();
        mDeferredCandidates.erase(mDeferredCandidates.begin());
        mDeferredCount.fetch_sub(1, std::memory_order_release);
        ++attempts;
        if (!loadOneFallbackLocked(c)) {
            // The candidate pool changed even though no face was added.
            // Publish the change so cached failed/provisional glyphs retry
            // against the remaining candidates instead of being pinned to
            // this generation forever (a later candidate may cover them).
            mFallbackGeneration.fetch_add(1, std::memory_order_release);
            continue;
        }
        auto& fb = mFallbackFaces.last();
        std::unique_lock ftLock(FreeType::sFaceMutex);
        std::unique_lock faceLock(*fb.mtx);
        if (FT_Get_Char_Index(fb.face, codepoint) != 0) {
            return { fb.face, std::move(ftLock), std::move(faceLock) };
        }
    }

    return { nullptr };
}

_<AFont> AFontManager::loadFont(const AUrl& url) {
    return _new<AFont>(this, url);
}

AFontManager& AFontManager::inst() {
    static AFontManager f;
    return f;
}
