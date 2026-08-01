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
#include <AUI/Logging/ALogger.h>

AFontManager::AFontManager():
    mFreeType(_new<FreeType>()),
    mDefaultFont(loadFont(":uni/font/Roboto.ttf"))
{
    // Deliberately no initFallback(): Emscripten builds may lack thread
    // support (std::thread requires -pthread with worker support), and
    // fallbackCandidates() returns an empty list anyway, so discovery would
    // find nothing. Fallback stays disabled: the deferred queue is never
    // populated, so lockFallbackFace never kicks a worker either.
}

AVector<AFontManager::FallbackCandidate> AFontManager::fallbackCandidates() {
    // Emscripten has no CJK-capable system font; keep the fallback pool empty.
    ALogger::warn("Font") << "CJK fallback discovery is not implemented on this platform";
    return {};
}

AString AFontManager::getPathToFont(const AString &font) {
    return font;
}
