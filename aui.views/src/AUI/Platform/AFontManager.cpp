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
#endif

AFontManager::~AFontManager() {
    std::scoped_lock lock(mFallbackMutex);
    for (auto face : mFallbackFaces) {
        {
            std::lock_guard ftLock(FreeType::sFaceMutex);
            FT_Done_Face(face);
        }
    }
    mFallbackFaces.clear();
}

bool AFontManager::loadOneFallback(const FallbackCandidate& candidate) {
    FT_Face face = nullptr;
    {
        std::lock_guard lock(FreeType::sFaceMutex);
        if (FT_New_Face(mFreeType->getFt(), candidate.path.toStdString().c_str(), candidate.faceIndex, &face) == 0) {
            mFallbackFaces.push_back(face);
            ALogger::info("Font") << "Loaded CJK fallback font: " << candidate.path;
            return true;
        }
    }
    return false;
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
    FcResult result;
    FcFontSet* fontSet = FcFontSort(nullptr, pattern, FcTrue, nullptr, &result);
    if (fontSet) {
        for (int i = 0; i < fontSet->nfont; ++i) {
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

    // Check all already-loaded faces first.
    for (auto face : mFallbackFaces) {
        if (FT_Get_Char_Index(face, codepoint) != 0) {
            return { std::move(lk), face };
        }
    }

    // No loaded face has this codepoint; try deferred candidates lazily.
    while (!mDeferredCandidates.empty()) {
        auto c = mDeferredCandidates.first();
        mDeferredCandidates.erase(mDeferredCandidates.begin());
        if (loadOneFallback(c)) {
            // Check if the newly loaded face covers this codepoint.
            if (FT_Get_Char_Index(mFallbackFaces.last(), codepoint) != 0) {
                return { std::move(lk), mFallbackFaces.last() };
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
