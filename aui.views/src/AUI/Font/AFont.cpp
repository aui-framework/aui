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

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftsnames.h>
#include "AUI/Font/FreeType.h"
#include "AUI/Platform/AFontManager.h"
#include "AUI/Logging/ALogger.h"
#include <fstream>
#include <mutex>
#include <string>
#include "AUI/Common/AStringVector.h"
#include "AFont.h"


AFont::AFont(AFontManager* fm, const AString& path) :
        ft(fm->mFreeType) {
    if (FT_New_Face(fm->mFreeType->getFt(), path.toStdString().c_str(), 0, &mFace)) {
        throw AException("Could not load font: " + path);
    }
}

AFont::AFont(AFontManager* fm, const AUrl& url) :
        ft(fm->mFreeType) {
    if (url.schema() == "file") {
        if (FT_New_Face(fm->mFreeType->getFt(), url.path().toStdString().c_str(), 0, &mFace)) {
            throw AException("Could not load font: " + url.full());
        }
        return;
    }
    mFontDataBuffer = AByteBuffer::fromStream(url.open());

    if (FT_New_Memory_Face(fm->mFreeType->getFt(), (const FT_Byte*) mFontDataBuffer.data(), mFontDataBuffer.getSize(),
                           0, &mFace)) {
        throw AException("Could not load font: " + url.full());
    }
}

AFont::~AFont() {
    if (mFace) {
        FT_Done_Face(mFace);
    }
}

AString AFont::getFontFamilyName() const {
    FT_SfntName name;
    FT_Get_Sfnt_Name(mFace, 0, &name);
    return std::string(name.string, name.string + name.string_len);
}

AFontFamily::Weight AFont::getFontWeight() const {
    return AFontFamily::NORMAL;
}

bool AFont::isItalic() const {
    return mFace->style_flags & FT_STYLE_FLAG_ITALIC;
}

bool AFont::hasGlyph(char32_t codepoint) const {
    return FT_Get_Char_Index(mFace, codepoint) != 0;
}

glm::vec2 AFont::getKerning(wchar_t left, wchar_t right) {
    FT_Vector vec2;
    FT_Get_Kerning(mFace, left, right, FT_KERNING_DEFAULT, &vec2);

    return {vec2.x >> 6, vec2.y >> 6};
}

AFont::Character AFont::renderGlyph(const FontEntry& fs, AChar glyph) {
    int size = fs.first.size;
    FontRendering fr = fs.first.fr;

    // Determine which face to use: try primary, fall back to CJK font if glyph missing.
    FT_Face face = nullptr;
    bool usingFallback = false;
    AFontManager::FallbackFaceLock fallbackGuard;
    auto& fmgr = AFontManager::inst();
    if (hasGlyph(glyph.codepoint())) {
        face = mFace;
    } else {
        // lockFallbackFace ensures initialization AND acquires the fallback mutex.
        auto fb = fmgr.lockFallbackFace();
        if (fb && FT_Get_Char_Index(fb.face, glyph.codepoint()) != 0) {
            face = fb.face;
            usingFallback = true;
            fallbackGuard = std::move(fb);
        }
    }
    if (!face) {
        // No font has this glyph; let the primary face render .notdef (tofu).
        face = mFace;
    }

    FT_Set_Pixel_Sizes(face, 0, size);

    FT_Int32 flags = FT_LOAD_RENDER;

    if (fr == FontRendering::SUBPIXEL)
        flags |= FT_LOAD_TARGET_LCD;
    if (fr == FontRendering::NEAREST)
        flags |= FT_LOAD_TARGET_MONO;

    FT_Error e = FT_Load_Char(face, glyph.codepoint(), flags);
    if (e) {
        // Neither face can render this glyph.
        ALogger::debug("Font") << "FT_Load_Char failed for U+"
                               << std::hex << std::uint32_t(glyph.codepoint()) << " (error " << std::dec << e << ")";
        return Character{};
    }

    // Capture glyph bitmap and metrics while the fallback lock is held.
    FT_GlyphSlot g = face->glyph;
    const float div = 1.f / 64.f;

    AByteBuffer data;
    glm::vec2 charSize{};
    glm::vec2 hBearing{};
    float hAdvance = 0;
    glm::vec2 vBearing{};
    float vAdvance = 0;
    int width = 0;
    int height = 0;

    if (g->bitmap.width && g->bitmap.rows) {
        width = g->bitmap.width;
        if (fr == FontRendering::SUBPIXEL)
            width /= 3;
        height = g->bitmap.rows;

        if (fr == FontRendering::NEAREST) {
            // when nearest, freetype renders glyphs into the 1bit-depth image
            // but OpenGL requires at least 8bit-depth, so convert here
            data.resize(g->bitmap.rows * g->bitmap.width);

            for (unsigned r = 0; r < g->bitmap.rows; ++r) {
                unsigned char* bufPtr = g->bitmap.buffer + r * g->bitmap.pitch;
                for (unsigned c = 0; c < g->bitmap.width; ++c) {
                    data.at<std::uint8_t>(c + r * g->bitmap.width) = (bufPtr[c / 8] & (0b10000000 >> (c % 8))) ? 255
                                                                                                               : 0;
                }
            }
        } else {
            data.reserve(g->bitmap.rows * g->bitmap.pitch);

            for (unsigned r = 0; r < g->bitmap.rows; ++r) {
                unsigned char* bufPtr = g->bitmap.buffer + r * g->bitmap.pitch;
                data.write(reinterpret_cast<const char*>(bufPtr), g->bitmap.width);
            }
        }

        // Capture metrics from glyph slot while the face is still locked.
        charSize = { div * g->metrics.width, div * g->metrics.height };
        hBearing = { static_cast<float>(g->bitmap_left), static_cast<float>(g->bitmap_top) };
        hAdvance = div * g->metrics.horiAdvance;
        vBearing = { div * g->metrics.vertBearingX, div * g->metrics.vertBearingY };
        vAdvance = div * g->metrics.vertAdvance;
    }

    // Release fallback lock; all glyph data is now copied to locals.
    if (usingFallback) {
        usingFallback = false;
        fallbackGuard = AFontManager::FallbackFaceLock{};
    }

    if (data.empty()) {
        return Character{};
    }

    int imageFormat = APixelFormat::BYTE;
    if (fr == FontRendering::SUBPIXEL)
        imageFormat |= APixelFormat::RGB;
    else
        imageFormat |= APixelFormat::R;

    return Character{
        .image = _new<AImage>(data, glm::uvec2(width, height), imageFormat),
        .size = charSize,
        .horizontal = {
          .bearing = hBearing,
          .advance = hAdvance,
        },
        .vertical = {
          .bearing = vBearing,
          .advance = vAdvance,
        },
    };
}

AFont::Character& AFont::getCharacter(const FontEntry& charset, AChar glyph) {
    auto& chars = charset.second.characters;
    if (chars.size() > glyph && chars[glyph.codepoint()]) {
        return *chars[glyph.codepoint()];
    } else {
        if (chars.size() <= glyph) {
            chars.resize(glyph + 1, std::nullopt);
        }
        chars[glyph.codepoint()] = std::move(renderGlyph(charset, glyph));

        return *chars[glyph.codepoint()];
    }
}

int AFont::length(const FontEntry& charset, AStringView text) {
    return length(charset, text.utf8().begin(), text.utf8().end());
}

int AFont::length(const FontEntry& charset, std::u32string_view text) {
    return length(charset, text.begin(), text.end());
}

bool AFont::isHasKerning() {
    return FT_HAS_KERNING(mFace);
}

int AFont::getAscenderHeight(unsigned size) const {
    return int(mFace->ascender) * size / mFace->height;
}


int AFont::getDescenderHeight(unsigned size) const {
    return -int(mFace->descender) * size / mFace->height;
}
