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
#include <AUI/Common/AByteBuffer.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/Platform/apple/ACFPtr.h>
#include <CoreText/CoreText.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>
#include <climits>
#include <cstring>
#include <memory>
#include <string>

namespace {
/**
 * @brief Returns the index of the face in the font file at path whose
 *        PostScript name matches postScriptName, or 0 when it cannot be
 *        determined.
 * @details Core Text does not expose the face index of a CTFont inside a
 *          TrueType collection (kCTFontURLAttribute returns only the file
 *          path), so the index is located by name through FreeType. The
 *          returned index is exactly the one FT_New_Memory_Face will use
 *          when the candidate is loaded, which matters for TTC files such
 *          as PingFang.ttc or AppleSDGothicNeo.ttc: the face matching the
 *          matched CTFont's PostScript name is not necessarily at index 0.
 * @note Runs on the fallback-discovery worker thread. It probes with its
 *       own FreeType library (fallbackCandidates is static and must not
 *       touch AFontManager::mFreeType), so FreeType::sFaceMutex is not
 *       needed: that mutex serializes calls on the shared library only.
 */
int faceIndexForPostScriptName(const std::string& path, CFStringRef postScriptName) {
    const CFIndex psMaxLen =
        CFStringGetMaximumSizeForEncoding(CFStringGetLength(postScriptName), kCFStringEncodingUTF8) + 1;
    std::string psName(static_cast<size_t>(psMaxLen), '\0');
    if (!CFStringGetCString(postScriptName, psName.data(), psMaxLen, kCFStringEncodingUTF8)) {
        return 0;
    }
    psName.resize(std::strlen(psName.data()));

    AByteBuffer data;
    try {
        data = AByteBuffer::fromStream(std::make_unique<AFileInputStream>(AString(path)));
    } catch (const AException&) {
        return 0;   // unreadable font file: fall back to the previous index 0
    }

    FreeType probeLibrary;
    FT_Face probe = nullptr;
    // face_index -1: parse the collection without selecting a face, so
    // num_faces is populated for the loop bound.
    if (FT_New_Memory_Face(probeLibrary.getFt(), reinterpret_cast<const FT_Byte*>(data.data()), data.getSize(), -1,
                           &probe) != 0) {
        return 0;
    }
    const int numFaces = static_cast<int>(probe->num_faces);
    FT_Done_Face(probe);
    for (int i = 0; i < numFaces; ++i) {
        FT_Face face = nullptr;
        if (FT_New_Memory_Face(probeLibrary.getFt(), reinterpret_cast<const FT_Byte*>(data.data()), data.getSize(), i,
                               &face) != 0) {
            return 0;
        }
        const char* name = FT_Get_Postscript_Name(face);
        const bool match = name != nullptr && psName == name;
        FT_Done_Face(face);
        if (match) {
            return i;
        }
    }
    return 0;
}
}

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
        ACFPtr<CTFontRef> font(CTFontCreateWithName(name, 0.0, nullptr));
        if (!font) {
            continue;
        }
        ACFPtr<CFStringRef> postScriptName(CTFontCopyPostScriptName(font.get()));
        if (!postScriptName || CFStringCompare(postScriptName.get(), name, 0) != kCFCompareEqualTo) {
            continue;
        }
        ACFPtr<CFURLRef> url(static_cast<CFURLRef>(CTFontCopyAttribute(font.get(), kCTFontURLAttribute)));
        if (!url) {
            continue;
        }
        ACFPtr<CFStringRef> path(CFURLCopyFileSystemPath(url.get(), kCFURLPOSIXPathStyle));
        if (!path) {
            continue;
        }
        const CFIndex maxLen =
            CFStringGetMaximumSizeForEncoding(CFStringGetLength(path.get()), kCFStringEncodingUTF8) + 1;
        std::string cpath(static_cast<size_t>(maxLen), '\0');
        if (CFStringGetCString(path.get(), cpath.data(), maxLen, kCFStringEncodingUTF8)) {
            cpath.resize(std::strlen(cpath.data()));
            const int faceIndex = faceIndexForPostScriptName(cpath, postScriptName.get());
            candidates.push_back({ AString(std::move(cpath)), faceIndex, {} });
        }
    }
    return candidates;
}

AString AFontManager::getPathToFont(const AString &font) {
    return font;
}
