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
#include <AUI/Util/kAUI.h>

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
    {
        std::thread worker;
        {
            std::scoped_lock lock(mFallbackMutex);
            mFallbackShuttingDown = true;
            worker = std::move(mFallbackThread);
        }
        if (worker.joinable()) {
            worker.join();
        }
    }
    std::unique_lock lock(mFallbackMutex);
    // No worker may still be running when the members below are destroyed.
    // Joining the stored thread reaps the in-flight run; waiting for the
    // outstanding-run counter to reach zero additionally covers a run whose
    // final bookkeeping (pending clear, notify) is still in flight.
    mFallbackCv.wait(lock, [&] { return mFallbackRunsInFlight == 0; });
    for (auto& fb : mFallbackFaces) {
        // Hold both the library lock and this face's per-face lock so that
        // FT_Done_Face cannot overlap an in-flight glyph load on the same face.
        std::scoped_lock ftLock(FreeType::sFaceMutex, *fb.mtx);
        FT_Done_Face(fb.face);
    }
    mFallbackFaces.clear();
}

void AFontManager::initFallback() {
    std::lock_guard lock(mFallbackMutex);
    startFallbackWorker();
}

void AFontManager::startFallbackWorker() {
    // mFallbackMutex must be held by the caller.
    if (mFallbackShuttingDown) {
        return;
    }
    if (mFallbackPending.load(std::memory_order_acquire)) {
        return;   // a worker run is already in flight
    }
    if (mFallbackThread.joinable()) {
        // The previous run has finished its work (pending was false). Reap
        // it: joining a finished thread does not block on discovery work,
        // and it keeps every worker joinable so the destructor can
        // guarantee no worker outlives the manager. (A run that already
        // cleared pending never re-locks mFallbackMutex, so joining while
        // holding it cannot deadlock.)
        mFallbackThread.join();
    }
    mFallbackPending.store(true, std::memory_order_release);
    ++mFallbackRunsInFlight;
    try {
        mFallbackThread = std::thread(&AFontManager::fallbackDiscoveryWorker, this);
    } catch (const std::system_error& e) {
        // Fallback discovery is best-effort; never fail font manager construction.
        --mFallbackRunsInFlight;
        mFallbackPending.store(false, std::memory_order_release);
        mFallbackCv.notify_all();
        // No worker can ever run discovery now; lockFallbackFace must report
        // a miss instead of running it inline on the calling thread.
        mFallbackWorkerUnavailable = true;
        ALogger::warn("Font") << "Could not start fallback discovery thread: " << e.what();
    }
}

void AFontManager::fallbackDiscoveryWorker() {
    try {
        std::unique_lock lock(mFallbackMutex);
        ensureFallbackFaceLocked();

        // Load one deferred candidate per run: font-file IO and face parsing
        // must never happen on the calling (UI) thread, which is what the
        // former in-lookup lazy loading did. A miss in lockFallbackFace
        // re-kicks this worker, and the generation bump below (or inside
        // loadOneFallbackLocked on success) makes AFont re-render the
        // provisional glyph once per completed load, so the queue drains one
        // candidate per re-render cycle until a face covers the glyph or the
        // queue is empty.
        if (!mDeferredCandidates.empty()) {
            auto c = std::move(mDeferredCandidates.first());
            mDeferredCandidates.erase(mDeferredCandidates.begin());
            if (!loadOneFallbackLocked(std::move(c))) {
                // The candidate pool changed even though no face was added.
                // Publish the change so cached failed/provisional glyphs
                // retry against the remaining candidates instead of being
                // pinned to this generation forever.
                mFallbackGeneration.fetch_add(1, std::memory_order_release);
            }
        }
    } catch (const std::exception& e) {
        // Best-effort: an exception leaving a thread function would call
        // std::terminate, so discovery failures must not escape.
        ALogger::warn("Font") << "Fallback discovery failed: " << e.what();
    } catch (...) {
        ALogger::warn("Font") << "Fallback discovery failed with an unknown exception";
    }
    // Final actions of the run: publish the completion under the manager
    // mutex (the mutex-protected discovery result — loaded faces, deferred
    // candidates — is visible to any thread that observes the clear) and
    // decrement the outstanding-run counter so the destructor can wait for
    // every worker to finish. notify_all() then wakes lockFallbackFace
    // waiters: the run is over, loaded faces can be probed and the missed
    // glyph rendered instead of staying tofu.
    {
        std::lock_guard lock(mFallbackMutex);
        mFallbackPending.store(false, std::memory_order_release);
        --mFallbackRunsInFlight;
    }
    mFallbackCv.notify_all();
}

bool AFontManager::loadOneFallbackLocked(FallbackCandidate candidate) {
    // Read the font file into memory BEFORE taking FreeType::sFaceMutex:
    // FT_New_Face parses the file from disk while holding the lock, and that
    // lock serializes every glyph render, kerning query and hasGlyph call.
    // Disk IO must not run inside the critical section. (On Windows the file
    // is opened by its UTF-16 path: the narrow CRT fopen() cannot address
    // non-ASCII font paths.)
    AByteBuffer fontData;
    if (!candidate.data.empty()) {
        // The candidate is an owning copy; transfer the embedded buffer
        // instead of copying it.
        fontData = std::move(candidate.data);
    } else {

#if AUI_PLATFORM_WIN
    const std::u16string utf16 = candidate.path.toUtf16();
    std::wstring wpath(utf16.begin(), utf16.end());
    if (FILE* file = _wfopen(wpath.c_str(), L"rb")) {
        if (fseek(file, 0, SEEK_END) == 0) {
            const long size = ftell(file);
            if (size > 0 && fseek(file, 0, SEEK_SET) == 0) {
                fontData.resize(static_cast<size_t>(size));
                if (fread(fontData.data(), 1, static_cast<size_t>(size), file) != static_cast<size_t>(size)) {
                    fontData = {};   // truncated read: treat as a load failure
                }
            }
        }
        fclose(file);
    }
#else
    try {
        fontData = AByteBuffer::fromStream(std::make_unique<AFileInputStream>(candidate.path));
    } catch (const AException& e) {
        ALogger::warn("Font") << "Could not read fallback font " << candidate.path << ": " << e.getMessage();
        return false;
    }
#endif
    }
    if (fontData.empty()) {
        // Covers every Windows failure path (open, seek, truncated read) as
        // well as empty files on any platform: the POSIX branch above only
        // logs when the stream throws.
        ALogger::warn("Font") << "Could not read fallback font " << candidate.path;
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
    FT_Face faceToFree = face;
    AUI_DEFER { if (faceToFree) FT_Done_Face(faceToFree); };

    mFallbackFaces.push_back({face, std::move(fontData)});
    faceToFree = nullptr; // Success: cancel the cleanup
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

    // Load the first candidate eagerly (pre-warm); its embedded buffer (if
    // any) is moved into the face rather than copied, so record the path
    // for the failure log before the candidate is moved-from.
    const AString firstCandidatePath = allCandidates.first().path;
    if (!loadOneFallbackLocked(std::move(allCandidates.first()))) {
        // The eager candidate is deliberately not added to mDeferredCandidates
        // (it was already attempted), so a failure here is silent unless logged:
        // missing files (e.g. msyh.ttc on a Windows edition without Simplified
        // Chinese fonts) would otherwise be undiagnosable until a CJK codepoint
        // renders tofu.
        ALogger::warn("Font") << "Could not load fallback font: " << firstCandidatePath;
    }

    // Defer remaining candidates for lazy loading in lockFallbackFace.
    for (size_t i = 1; i < allCandidates.size(); ++i) {
        mDeferredCandidates.push_back(std::move(allCandidates[i]));
    }
    if (!mDeferredCandidates.empty()) {
        // The candidate pool changed even if no face was loaded: publish the
        // change so cached failed/provisional glyphs retry against the
        // deferred candidates (a retry may consume them lazily).
        mFallbackGeneration.fetch_add(1, std::memory_order_release);
    }
}

AFontManager::FallbackFaceLock AFontManager::lockFallbackFace(char32_t codepoint) {
    std::unique_lock lk(mFallbackMutex);

    // If discovery is in flight, wait for it to finish before probing. The
    // worker does the font-file IO, so this wait never runs IO on the calling
    // (UI) thread. Returning a miss here instead would render a provisional
    // .notdef (tofu) glyph and cache it; the generation-gate retry only
    // re-renders on a later getCharacter call, and a static (never
    // repainting) surface would show tofu forever. The wait is bounded by
    // the in-flight run: once initial discovery completes, pending is false
    // and later lookups proceed without waiting.
    mFallbackCv.wait(lk, [&] { return !mFallbackPending.load(std::memory_order_acquire); });
    if (mFallbackWorkerUnavailable) {
        // Discovery can never run off-thread (worker thread creation failed)
        // and must not run on the calling (UI) thread: report a miss. The
        // generation never advances, so AFont stops re-rendering provisional
        // glyphs instead of retrying forever.
        return { nullptr };
    }

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

    // No loaded face has this codepoint. Hand the next deferred candidate to
    // the discovery worker instead of loading it here: font-file IO and face
    // parsing must not run on the calling (UI) thread during text layout. The
    // worker bumps mFallbackGeneration when the load finishes, and the
    // generation gate in AFont::getCharacter re-renders the provisional glyph
    // once per bump, so the queue drains one candidate per re-render cycle
    // until a face covers this codepoint or the queue empties (retries then
    // stop: the generation stops advancing).
    if (!mDeferredCandidates.empty() && !mFallbackPending.load(std::memory_order_acquire)) {
        startFallbackWorker();
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
