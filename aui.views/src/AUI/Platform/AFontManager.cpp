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
    for (auto& fb : mFallbackFaces) {
        if (fb.face) {
            FT_Done_Face(fb.face);
        }
    }
    mFallbackFaces.clear();
}

void AFontManager::initFallback() {
    ensureFallbackFaceLoaded();
}

bool AFontManager::loadOneFallback(FallbackCandidate candidate) {
    AByteBuffer fontData;
    if (!candidate.data.empty()) {
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
                    fontData = {};
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
        ALogger::warn("Font") << "Could not read fallback font " << candidate.path;
        return false;
    }

    FT_Face face = nullptr;
    if (FT_New_Memory_Face(mFreeType->getFt(), (const FT_Byte*) fontData.data(), fontData.getSize(),
                           candidate.faceIndex, &face) != 0) {
        return false;
    }

    mFallbackFaces.push_back({face, std::move(fontData)});
    ALogger::info("Font") << "Loaded CJK fallback font: " << candidate.path;
    return true;
}

void AFontManager::ensureFallbackFaceLoaded() {
    if (mFallbackAttempted) {
        return;
    }
    mFallbackAttempted = true;

    AVector<FallbackCandidate> allCandidates = fallbackCandidates();
    if (allCandidates.empty()) {
        return;
    }

    const AString firstCandidatePath = allCandidates.first().path;
    if (!loadOneFallback(std::move(allCandidates.first()))) {
        ALogger::warn("Font") << "Could not load fallback font: " << firstCandidatePath;
    }

    for (size_t i = 1; i < allCandidates.size(); ++i) {
        mDeferredCandidates.push_back(std::move(allCandidates[i]));
    }
}

FT_FaceRec_* AFontManager::getFallbackFace(char32_t codepoint) {
    ensureFallbackFaceLoaded();

    for (auto& fb : mFallbackFaces) {
        if (FT_Get_Char_Index(fb.face, codepoint) != 0) {
            return fb.face;
        }
    }

    while (!mDeferredCandidates.empty()) {
        auto candidate = std::move(mDeferredCandidates.first());
        mDeferredCandidates.erase(mDeferredCandidates.begin());
        if (loadOneFallback(std::move(candidate))) {
            if (FT_Get_Char_Index(mFallbackFaces.last().face, codepoint) != 0) {
                return mFallbackFaces.last().face;
            }
        }
    }

    return nullptr;
}

_<AFont> AFontManager::loadFont(const AUrl& url) {
    return _new<AFont>(this, url);
}

AFontManager& AFontManager::inst() {
    static AFontManager f;
    return f;
}
