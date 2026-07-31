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

#if AUI_PLATFORM_LINUX
#include <fontconfig/fontconfig.h>
#elif AUI_PLATFORM_WIN
#include <windows.h>
#include <cstdio>
#include <cwchar>
#include <string>

namespace {
// FT_New_Face opens pathnames with the narrow CRT fopen(), which uses the
// ANSI code page and cannot address non-ASCII font paths (e.g. a Windows
// directory that contains non-ASCII characters). Open the font file by its
// UTF-16 path instead and hand FreeType a custom stream over it.
struct WideFontStream {
    FT_StreamRec stream{};
    FILE* file = nullptr;
};

unsigned long wideFontStreamRead(FT_Stream stream, unsigned long offset, unsigned char* buffer, unsigned long count) {
    auto* self = reinterpret_cast<WideFontStream*>(stream->descriptor.pointer);
    if (count == 0) {
        // Zero-count reads are seek-status probes (see FT_Stream_IoFunc):
        // non-zero indicates the seek failed (offset past the end of the stream).
        return offset > stream->size ? 1 : 0;
    }
    if (fseek(self->file, static_cast<long>(offset), SEEK_SET) != 0) {
        return 0;
    }
    // Report the actual number of bytes read. In particular, a zero result
    // (read past EOF) must be reported as zero, not synthesized as a success:
    // FreeType would otherwise treat an unread buffer as valid data.
    return static_cast<unsigned long>(fread(buffer, 1, count, self->file));
}

void wideFontStreamClose(FT_Stream stream) {
    auto* self = reinterpret_cast<WideFontStream*>(stream->descriptor.pointer);
    fclose(self->file);
    delete self;
}

FT_Face loadFallbackFaceWide(FT_Library ft, const AString& path, int faceIndex) {
    const std::u16string utf16 = path.toUtf16();
    std::wstring wpath(utf16.begin(), utf16.end());

    auto* wide = new WideFontStream;
    wide->file = _wfopen(wpath.c_str(), L"rb");
    if (!wide->file) {
        delete wide;
        return nullptr;
    }
    if (fseek(wide->file, 0, SEEK_END) != 0) {
        fclose(wide->file);
        delete wide;
        return nullptr;
    }
    wide->stream.size = static_cast<unsigned long>(ftell(wide->file));
    if (fseek(wide->file, 0, SEEK_SET) != 0) {
        fclose(wide->file);
        delete wide;
        return nullptr;
    }
    wide->stream.descriptor.pointer = wide;
    wide->stream.read = wideFontStreamRead;
    wide->stream.close = wideFontStreamClose;

    FT_Open_Args args{};
    args.flags = FT_OPEN_STREAM;
    args.stream = &wide->stream;

    // From here on the stream is owned by FreeType: it is closed (via
    // wideFontStreamClose) both when the face is destroyed and when face
    // creation fails.
    FT_Face face = nullptr;
    if (FT_Open_Face(ft, &args, faceIndex, &face) != 0) {
        return nullptr;
    }
    return face;
}
}
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
    }
    // Release-store: the mutex-protected discovery result (loaded faces,
    // deferred candidates) is visible to any thread that observes the clear.
    mFallbackPending.store(false, std::memory_order_release);
}

bool AFontManager::loadOneFallback(const FallbackCandidate& candidate) {
    std::lock_guard lock(FreeType::sFaceMutex);
    FT_Face face = nullptr;
#if AUI_PLATFORM_WIN
    face = loadFallbackFaceWide(mFreeType->getFt(), candidate.path, candidate.faceIndex);
#else
    if (FT_New_Face(mFreeType->getFt(), candidate.path.toStdString().c_str(), candidate.faceIndex, &face) != 0) {
        return false;
    }
#endif
    if (!face) {
        return false;
    }
    mFallbackFaces.push_back({face});
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
    AVector<FallbackCandidate> allCandidates;

#if AUI_PLATFORM_LINUX
    // Use fontconfig to find a CJK-capable sans-serif font.
    // Ensure fontconfig is initialized before using default config (nullptr).
    if (!FcInit()) {
        ALogger::warn("Font") << "FcInit() failed; CJK fallback unavailable";
        return;
    }
    // Use a charset-based query with representative codepoints from each CJK
    // script (Han, Hiragana, Hangul) so that Kana-only and Hangul-only fonts
    // can enter the candidate list, not just Han-capable ones.
    static constexpr FcChar32 kProbeCodepoints[] = { 0x4E2D /*中*/, 0x3042 /*あ*/, 0xAC00 /*가*/ };
    FcPattern* pattern = FcPatternCreate();
    FcCharSet* charset = FcCharSetCreate();
    for (auto probe : kProbeCodepoints) {
        FcCharSetAddChar(charset, probe);
    }
    FcPatternAddCharSet(pattern, FC_CHARSET, charset);
    FcPatternAddString(pattern, FC_FAMILY, reinterpret_cast<const FcChar8*>("sans"));
    FcConfigSubstitute(nullptr, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);
    FcCharSetDestroy(charset);

    // Use FcFontSort to obtain an ordered set of CJK-capable candidates.
    // FcFontMatch would only return a single best match, which cannot populate
    // the deferred-candidate pool for lazy loading on codepoint miss.
    // The list is capped: loaded fallback faces are never released, so an
    // unbounded candidate pool would keep opening font files for the whole
    // process lifetime on codepoint misses (e.g. emoji, rare symbols).
    constexpr int kMaxFallbackCandidates = 8;
    FcResult result;
    FcFontSet* fontSet = FcFontSort(nullptr, pattern, FcTrue, nullptr, &result);
    if (fontSet) {
        for (int i = 0; i < fontSet->nfont && allCandidates.size() < kMaxFallbackCandidates; ++i) {
            FcPattern* font = fontSet->fonts[i];
            // Verify the font covers at least one of the CJK scripts.
            FcCharSet* fontCharset = nullptr;
            if (FcPatternGetCharSet(font, FC_CHARSET, 0, &fontCharset) == FcResultMatch) {
                bool coversAny = false;
                for (auto probe : kProbeCodepoints) {
                    if (FcCharSetHasChar(fontCharset, probe)) {
                        coversAny = true;
                        break;
                    }
                }
                if (!coversAny) {
                    continue;   // Skip fonts that cover none of the CJK scripts.
                }
            }
            FcChar8* path = nullptr;
            int faceIndex = 0;
            if (FcPatternGetString(font, FC_FILE, 0, &path) == FcResultMatch) {
                FcPatternGetInteger(font, FC_INDEX, 0, &faceIndex);
                allCandidates.push_back({ AString(reinterpret_cast<const char*>(path)), faceIndex });
            }
        }
        FcFontSetDestroy(fontSet);
    }
    FcPatternDestroy(pattern);
    if (allCandidates.empty()) {
        ALogger::warn("Font") << "No CJK fallback font found via fontconfig";
        return;
    }
#elif AUI_PLATFORM_WIN
    // Try known CJK font files from the Windows Fonts directory.
    // Order: most-comprehensive first (Microsoft YaHei covers CJK + Kana,
    // Malgun Gothic covers CJK + Hangul, then region-specific fallbacks).
    const AString fontsDir = [] {
        wchar_t buf[MAX_PATH];
        UINT len = GetWindowsDirectoryW(buf, MAX_PATH);
        if (len > 0 && len < MAX_PATH) {
            // Construct from exactly len characters (not relying on null termination).
            return AString(reinterpret_cast<const char16_t*>(buf), len) + "\\Fonts\\";
        }
        ALogger::warn("Font") << "GetWindowsDirectoryW() failed, falling back to C:\\Windows\\Fonts\\";
        return AString("C:\\Windows\\Fonts\\");
    }();
    allCandidates = {
        { fontsDir + "msyh.ttc" },       // Microsoft YaHei (Simplified Chinese, includes CJK + Kana)
        { fontsDir + "malgun.ttf" },     // Malgun Gothic (Korean, includes Hangul + CJK)
        { fontsDir + "simsun.ttc" },     // SimSun (Simplified Chinese)
        { fontsDir + "msgothic.ttc" },   // MS Gothic (Japanese)
        { fontsDir + "yugothr.ttc" },    // Yu Gothic (Japanese)
        { fontsDir + "meiryo.ttc" },     // Meiryo (Japanese)
    };
#elif AUI_PLATFORM_MACOS
    // macOS system CJK fonts (stable paths since OS X 10.11).
    allCandidates = {
        { "/System/Library/Fonts/PingFang.ttc" },
        { "/System/Library/Fonts/AppleSDGothicNeo.ttc" },
        { "/System/Library/Fonts/Hiragino Sans.ttc" },
        { "/System/Library/Fonts/Supplemental/AppleSDGothicNeo.ttc" },
    };
#elif AUI_PLATFORM_ANDROID
    // Android system CJK fonts (varies by API level).
    allCandidates = {
        { "/system/fonts/NotoSansCJK-Regular.ttc" },  // Android 5-9, pan-CJK
        { "/system/fonts/NotoSansSC-Regular.otf" },    // Android 10+ (Chinese)
        { "/system/fonts/NotoSansKR-Regular.otf" },    // Android 10+ (Korean)
        { "/system/fonts/NotoSansJP-Regular.otf" },    // Android 10+ (Japanese)
    };
#elif AUI_PLATFORM_IOS
    // iOS system CJK fonts.
    allCandidates = {
        { "/System/Library/Fonts/PingFang.ttc" },
        { "/System/Library/Fonts/AppleSDGothicNeo.ttc" },
    };
#else
    // Unknown platform; mark fallback as unavailable.
    ALogger::warn("Font") << "CJK font fallback not available on this platform";
    return;
#endif

    // Load the first candidate eagerly (pre-warm).
    loadOneFallback(allCandidates.first());

    // Defer remaining candidates for lazy loading in lockFallbackFace.
    for (size_t i = 1; i < allCandidates.size(); ++i) {
        mDeferredCandidates.push_back(allCandidates[i]);
    }

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
        ++attempts;
        if (loadOneFallback(c)) {
            auto& fb = mFallbackFaces.last();
            std::unique_lock ftLock(FreeType::sFaceMutex);
            std::unique_lock faceLock(*fb.mtx);
            if (FT_Get_Char_Index(fb.face, codepoint) != 0) {
                return { fb.face, std::move(ftLock), std::move(faceLock) };
            }
        }
    }

    return { nullptr };
}

bool AFontManager::hasDeferredCandidates() {
    // While discovery is pending, report true so that AFont keeps retrying
    // provisional/failed glyphs: a retry may succeed once the worker loads
    // the first face. (Lock-free fast path; mFallbackPending is an atomic.)
    if (mFallbackPending.load(std::memory_order_acquire)) {
        return true;
    }
    std::lock_guard lock(mFallbackMutex);
    return !mDeferredCandidates.empty();
}

_<AFont> AFontManager::loadFont(const AUrl& url) {
    return _new<AFont>(this, url);
}

AFontManager& AFontManager::inst() {
    static AFontManager f;
    return f;
}
