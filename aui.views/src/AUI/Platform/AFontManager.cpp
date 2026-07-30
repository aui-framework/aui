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
    if (!mFallbackFaces.empty()) {
        return; // already loaded
    }
    if (mFallbackAttempted) {
        // Already tried and failed; don't retry every glyph
        return;
    }

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

    FcResult result;
    FcPattern* match = FcFontMatch(nullptr, pattern, &result);
    if (match) {
        if (result == FcResultMatch) {
            // FcFontMatch may return a best-effort match that doesn't satisfy the
            // requested charset. Verify the matched font actually contains U+4E2D.
            FcCharSet* resultCharset = nullptr;
            if (FcPatternGetCharSet(match, FC_CHARSET, 0, &resultCharset) == FcResultMatch) {
                if (!FcCharSetHasChar(resultCharset, 0x4E2D)) {
                    ALogger::warn("Font") << "Fontconfig fallback font lacks U+4E2D, skipping";
                    FcPatternDestroy(match);
                    FcPatternDestroy(pattern);
                    mFallbackAttempted = true;   // Fontconfig is working but no CJK font is available.
                    return;
                }
            }
            FcChar8* path = nullptr;
            int faceIndex = 0;
            if (FcPatternGetString(match, FC_FILE, 0, &path) == FcResultMatch) {
                FcPatternGetInteger(match, FC_INDEX, 0, &faceIndex);
                allCandidates = { { AString(reinterpret_cast<const char*>(path)), faceIndex } };
            } else {
                ALogger::warn("Font") << "Fontconfig matched font has no FC_FILE path";
            }
        }
        FcPatternDestroy(match);
    }
    FcPatternDestroy(pattern);
    if (allCandidates.empty()) {
        ALogger::warn("Font") << "No CJK fallback font found via fontconfig";
        mFallbackAttempted = true;
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
    if (allCandidates.empty()) {
        ALogger::warn("Font") << "No CJK fallback font found on Windows";
        mFallbackAttempted = true;
        return;
    }
#elif AUI_PLATFORM_MACOS
    // macOS system CJK fonts (stable paths since OS X 10.11).
    allCandidates = {
        { "/System/Library/Fonts/PingFang.ttc" },
        { "/System/Library/Fonts/AppleSDGothicNeo.ttc" },
        { "/System/Library/Fonts/Hiragino Sans.ttc" },
        { "/System/Library/Fonts/Supplemental/AppleSDGothicNeo.ttc" },
    };
    if (allCandidates.empty()) {
        ALogger::warn("Font") << "No CJK fallback font found on macOS";
        mFallbackAttempted = true;
        return;
    }
#elif AUI_PLATFORM_ANDROID
    // Android system CJK fonts (varies by API level).
    allCandidates = {
        { "/system/fonts/NotoSansCJK-Regular.ttc" },  // Android 5-9, pan-CJK
        { "/system/fonts/NotoSansSC-Regular.otf" },    // Android 10+ (Chinese)
        { "/system/fonts/NotoSansKR-Regular.otf" },    // Android 10+ (Korean)
        { "/system/fonts/NotoSansJP-Regular.otf" },    // Android 10+ (Japanese)
    };
    if (allCandidates.empty()) {
        ALogger::warn("Font") << "No CJK fallback font found on Android";
        mFallbackAttempted = true;
        return;
    }
#elif AUI_PLATFORM_IOS
    // iOS system CJK fonts.
    allCandidates = {
        { "/System/Library/Fonts/PingFang.ttc" },
        { "/System/Library/Fonts/AppleSDGothicNeo.ttc" },
    };
    if (allCandidates.empty()) {
        ALogger::warn("Font") << "No CJK fallback font found on iOS";
        mFallbackAttempted = true;
        return;
    }
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

    if (mFallbackFaces.empty() && mDeferredCandidates.empty()) {
        mFallbackAttempted = true;
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
