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

#include <AUI/Platform/AFontManager.h>
#include <AUI/Font/FreeType.h>
#include <CoreText/CoreText.h>
#include <CoreFoundation/CoreFoundation.h>
#include <limits.h>
AFontManager::AFontManager():
        mFreeType(_new<FreeType>()),
        mDefaultFont(loadFont(":uni/font/Roboto.ttf"))
{
    // Pre-warm fallback face to avoid UI-thread stall on first missing-glyph render.
    initFallback();
}

AVector<AFontManager::FallbackCandidate> AFontManager::fallbackCandidates() {
    AVector<AFontManager::FallbackCandidate> candidates;

    CFStringRef fontNames[] = {
        CFSTR("PingFangSC-Regular"),
        CFSTR("AppleSDGothicNeo-Regular")
    };

    for (CFStringRef name : fontNames) {
        if (CTFontRef font = CTFontCreateWithName(name, 0.0, NULL)) {
            CFStringRef postScriptName = CTFontCopyPostScriptName(font);
            if (postScriptName && CFStringCompare(postScriptName, name, 0) == kCFCompareEqualTo) {
                if (CFURLRef url = (CFURLRef)CTFontCopyAttribute(font, kCTFontURLAttribute)) {
                    UInt8 buffer[PATH_MAX];
                    if (CFURLGetFileSystemRepresentation(url, true, buffer, sizeof(buffer))) {
                        candidates.push_back({ AString((const char*)buffer) });
                    }
                    CFRelease(url);
                }
            }
            if (postScriptName) {
                CFRelease(postScriptName);
            }
            CFRelease(font);
        }
    }
    return candidates;
}

AString AFontManager::getPathToFont(const AString &font) {
    return font;
}
