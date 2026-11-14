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
#include <AUI/IO/APath.h>
#include <algorithm>

AFontManager::AFontManager():
    mFreeType(_new<FreeType>()),
    mDefaultFont(loadFont(":uni/font/Roboto.ttf"))
{
}

AVector<AFontManager::FallbackCandidate> AFontManager::fallbackCandidates() {
    // The browser gives no access to the system fonts. An application that needs glyphs its fonts lack (i.e., «», ♥,
    // emoji, CJK) preloads fallback fonts into the virtual filesystem, i.e., --preload-file=fonts@/usr/share/fonts;
    // they are tried in the file name order.
    const APath dir = "/usr/share/fonts";
    if (!dir.isDirectoryExists()) {
        ALogger::warn("Font") << "No fallback fonts: nothing is preloaded to " << dir;
        return {};
    }
    auto fonts = dir.listDir(AFileListFlags::REGULAR_FILES | AFileListFlags::RECURSIVE);
    std::sort(fonts.begin(), fonts.end());
    AVector<FallbackCandidate> result;
    for (const auto& font : fonts) {
        const auto extension = font.extension().lowercase();
        if (extension == "ttf" || extension == "otf" || extension == "ttc") {
            result << FallbackCandidate { .path = font };
        }
    }
    return result;
}

AString AFontManager::getPathToFont(const AString &font) {
    return font;
}
