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

AFontManager::AFontManager():
        mFreeType(_new<FreeType>()),
        mDefaultFont(loadFont(":uni/font/Roboto.ttf"))
{
    // Pre-warm fallback face to avoid UI-thread stall on first missing-glyph render.
    initFallback();
}

AVector<AFontManager::FallbackCandidate> AFontManager::fallbackCandidates() {
    // macOS system CJK fonts (stable paths since OS X 10.11).
    return {
        { "/System/Library/Fonts/PingFang.ttc" },
        { "/System/Library/Fonts/AppleSDGothicNeo.ttc" },
        { "/System/Library/Fonts/Hiragino Sans.ttc" },
        { "/System/Library/Fonts/Supplemental/AppleSDGothicNeo.ttc" },
    };
}

AString AFontManager::getPathToFont(const AString &font) {
    return font;
}
