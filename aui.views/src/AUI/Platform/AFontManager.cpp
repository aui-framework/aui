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
    if (mFallbackFace) {
        FT_Done_Face(mFallbackFace);
        mFallbackFace = nullptr;
    }
    mFallbackFontPath.clear();
}

void AFontManager::ensureFallbackFaceLocked() {
    if (mFallbackFace) {
        return; // already loaded
    }
    if (mFallbackAttempted) {
        // Already tried and failed; don't retry every glyph
        return;
    }
    mFallbackAttempted = true;

#if AUI_PLATFORM_LINUX
    // Use fontconfig to find a CJK-capable sans-serif font.
    // Ensure fontconfig is initialized before using default config (nullptr).
    if (!FcInit()) {
        ALogger::warn("Font") << "FcInit() failed; CJK fallback unavailable";
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
                    return;
                }
            }
            FcChar8* path = nullptr;
            if (FcPatternGetString(match, FC_FILE, 0, &path) == FcResultMatch) {
                if (!tryLoadFallback({ AString(reinterpret_cast<const char*>(path)) })) {
                    mFallbackFontPath.clear();
                }
            } else {
                ALogger::warn("Font") << "Fontconfig matched font has no FC_FILE path";
            }
        }
        FcPatternDestroy(match);
    }
    FcPatternDestroy(pattern);
#elif AUI_PLATFORM_WIN
    // Try known CJK font files from the Windows Fonts directory.
    // Order: most-comprehensive first (Microsoft YaHei covers CJK + Kana,
    // Malgun Gothic covers CJK + Hangul, then region-specific fallbacks).
    const AString fontsDir = [] {
        wchar_t buf[MAX_PATH];
        UINT len = GetWindowsDirectoryW(buf, MAX_PATH);
        if (len > 0 && len < MAX_PATH) {
            return AString(reinterpret_cast<char16_t*>(buf)) + "\\Fonts\\";
        }
        ALogger::warn("Font") << "GetWindowsDirectoryW() failed, falling back to C:\\Windows\\Fonts\\";
        return AString("C:\\Windows\\Fonts\\");
    }();
    if (!tryLoadFallback({
            fontsDir + "msyh.ttc",       // Microsoft YaHei (Simplified Chinese, includes CJK + Kana)
            fontsDir + "malgun.ttf",     // Malgun Gothic (Korean, includes Hangul + CJK)
            fontsDir + "simsun.ttc",     // SimSun (Chinese Traditional)
            fontsDir + "msgothic.ttc",   // MS Gothic (Japanese)
            fontsDir + "yugothic.ttf",   // Yu Gothic (Japanese)
            fontsDir + "meiryo.ttc",     // Meiryo (Japanese)
        })) {
        ALogger::warn("Font") << "No CJK fallback font found on Windows";
    }
#elif AUI_PLATFORM_MACOS
    // macOS system CJK fonts (stable paths since OS X 10.11).
    if (!tryLoadFallback({
            "/System/Library/Fonts/PingFang.ttc",
            "/System/Library/Fonts/AppleSDGothicNeo.ttc",
            "/System/Library/Fonts/Hiragino Sans.ttc",
            "/System/Library/Fonts/Supplemental/AppleSDGothicNeo.ttc",
        })) {
        ALogger::warn("Font") << "No CJK fallback font found on macOS";
    }
#elif AUI_PLATFORM_ANDROID
    // Android system CJK fonts (varies by API level).
    if (!tryLoadFallback({
            "/system/fonts/NotoSansCJK-Regular.ttc",  // Android 5-9, pan-CJK
            "/system/fonts/NotoSansSC-Regular.otf",    // Android 10+ (Chinese)
            "/system/fonts/NotoSansKR-Regular.otf",    // Android 10+ (Korean)
            "/system/fonts/NotoSansJP-Regular.otf",    // Android 10+ (Japanese)
        })) {
        ALogger::warn("Font") << "No CJK fallback font found on Android";
    }
#elif AUI_PLATFORM_IOS
    // iOS: try system font paths first (some iOS versions allow reading them),
    // then fall back to a bundled resource if available.
    {
        bool loaded = tryLoadFallback({
            "/System/Library/Fonts/PingFang.ttc",
            "/System/Library/Fonts/AppleSDGothicNeo.ttc",
        });
        if (!loaded) {
            // Try bundled resource
            try {
                mFallbackFontDataBuffer = AByteBuffer::fromStream(AUrl(":uni/font/NotoSansCJK-Fallback.ttf").open());
                if (FT_New_Memory_Face(mFreeType->getFt(),
                        (const FT_Byte*) mFallbackFontDataBuffer.data(),
                        mFallbackFontDataBuffer.getSize(), 0, &mFallbackFace) == 0) {
                    mFallbackFontPath = AString(":uni/font/NotoSansCJK-Fallback.ttf");
                    ALogger::info("Font") << "Loaded CJK fallback font from bundle";
                    loaded = true;
                }
            } catch (...) {
                // bundled font not present
            }
        }
        if (!loaded) {
            ALogger::warn("Font") << "No CJK fallback font found on iOS";
        }
    }
#else
    // Unknown platform; mark fallback as unavailable.
    ALogger::warn("Font") << "CJK font fallback not available on this platform";
#endif
}

bool AFontManager::tryLoadFallback(std::initializer_list<AString> candidates) {
    for (const auto& path : candidates) {
        if (FT_New_Face(mFreeType->getFt(), path.toStdString().c_str(), 0, &mFallbackFace) == 0) {
            mFallbackFontPath = path;
            ALogger::info("Font") << "Loaded CJK fallback font: " << path;
            return true;
        }
        mFallbackFace = nullptr;
    }
    return false;
}

_<AFont> AFontManager::loadFont(const AUrl& url) {
    return _new<AFont>(this, url);
}

AFontManager& AFontManager::inst() {
    static AFontManager f;
    return f;
}
