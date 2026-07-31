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
    if (!count && offset > stream->size) {
        return 1;
    }
    if (stream->pos != offset) {
        if (fseek(self->file, static_cast<long>(offset), SEEK_SET) != 0) {
            return 0;
        }
        stream->pos = offset;
    }
    if (count > 0) {
        count = static_cast<unsigned long>(fread(buffer, 1, count, self->file));
        if (!count) {
            return 1;
        }
    }
    stream->pos += count;
    return count;
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

    FT_OpenArgs args{};
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
    std::scoped_lock lock(mFallbackMutex);
    for (auto& fb : mFallbackFaces) {
        // Hold both the library lock and this face's per-face lock so that
        // FT_Done_Face cannot overlap an in-flight glyph load on the same face.
        std::scoped_lock ftLock(FreeType::sFaceMutex, *fb.mtx);
        FT_Done_Face(fb.face);
    }
    mFallbackFaces.clear();
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
        mFallbackAttempted = true;   // FcInit is a one-shot init; failure is definitive.
        return;
    }
    // Use a charset-based query with a common CJK character to ensure the font actually has CJK glyphs.
    FcPattern* pattern = FcPatternCreate();
    FcCharSet* charset = FcCharSetCreate();
    FcCharSetAddChar(charset, 0x4E2D); // U+4E2D = '中' (common CJK character)
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
            // Verify the font actually has CJK support.
            FcCharSet* fontCharset = nullptr;
            if (FcPatternGetCharSet(font, FC_CHARSET, 0, &fontCharset) == FcResultMatch) {
                if (!FcCharSetHasChar(fontCharset, 0x4E2D)) {
                    continue;   // Skip fonts that don't cover this CJK character.
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
    mFallbackAttempted = true;
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
    std::unique_lock lk(mFallbackMutex);
    ensureFallbackFaceLocked();

    // Check all already-loaded faces first. The per-face mutex must be held
    // for the FT_Get_Char_Index probe: other threads may be rendering on this
    // face right now (holding fb.mtx), and an FT_Face supports one thread at
    // a time. The same lock is transferred into the returned FallbackFaceLock.
    for (auto& fb : mFallbackFaces) {
        std::unique_lock faceLock(*fb.mtx);
        if (FT_Get_Char_Index(fb.face, codepoint) != 0) {
            return { std::move(lk), fb.face, std::move(faceLock) };
        }
    }

    // No loaded face has this codepoint; try deferred candidates lazily.
    // Bound to a few per call to avoid draining the entire queue on the UI thread
    // for a codepoint that no candidate covers (e.g. emoji, rare symbols).
    constexpr int kMaxDeferredLoadsPerCall = 2;
    int loaded = 0;
    while (!mDeferredCandidates.empty() && loaded < kMaxDeferredLoadsPerCall) {
        auto c = mDeferredCandidates.first();
        mDeferredCandidates.erase(mDeferredCandidates.begin());
        if (loadOneFallback(c)) {
            ++loaded;
            auto& fb = mFallbackFaces.last();
            std::unique_lock faceLock(*fb.mtx);
            if (FT_Get_Char_Index(fb.face, codepoint) != 0) {
                return { std::move(lk), fb.face, std::move(faceLock) };
            }
        }
    }

    return { std::move(lk), nullptr };
}

_<AFont> AFontManager::loadFont(const AUrl& url) {
    return _new<AFont>(this, url);
}

AFontManager& AFontManager::inst() {
    static AFontManager f;
    return f;
}
