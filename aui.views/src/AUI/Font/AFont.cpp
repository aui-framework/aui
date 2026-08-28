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

#include <freetype/ftsnames.h>
#include "AUI/Font/FreeType.h"
#include "AUI/Platform/AFontManager.h"
#include "AUI/Logging/ALogger.h"
#include <cstdlib>
#include <fstream>
#include <limits>
#include <memory>
#include <string>
#include "AUI/Common/AStringVector.h"
#include "AFont.h"

namespace {
/**
 * @brief Programs the pixel size on the currently selected face.
 *
 * The primary face's size is cached (primaryPixelSize) so the metric-
 * recomputing FT_Set_Pixel_Sizes call is skipped when the size is already
 * programmed (getKerning uses the same guard). Fallback bitmap faces get
 * the strike closest to the requested size.
 */
bool programPixelSize(FT_Face& face,
                      FT_Face primaryFace,
                      unsigned& primaryPixelSize,
                      int size) {
    const bool sizeAlreadyProgrammed = (face == primaryFace && primaryPixelSize == unsigned(size));
    if (sizeAlreadyProgrammed) {
        return true;
    }
    if (FT_Set_Pixel_Sizes(face, 0, size) != 0) {
        bool strikeSelected = false;
        if (face != primaryFace && !(face->face_flags & FT_FACE_FLAG_SCALABLE) && face->num_fixed_sizes > 0) {
            long bestStrike = 0;
            long bestDelta = std::numeric_limits<long>::max();
            for (FT_Int i = 0; i < face->num_fixed_sizes; ++i) {
                const long strikePx = face->available_sizes[i].y_ppem >> 6;
                const long delta = std::abs(strikePx - long(size));
                if (delta < bestDelta) {
                    bestDelta = delta;
                    bestStrike = i;
                }
            }
            if (bestDelta == 0) {
                strikeSelected = FT_Select_Size(face, bestStrike) == 0;
            }
        }
        if (!strikeSelected && face != primaryFace) {
            face = primaryFace;
            if (FT_Set_Pixel_Sizes(face, 0, size) != 0) {
                return false;
            }
        } else if (!strikeSelected) {
            return false;
        }
    }
    if (face == primaryFace) {
        primaryPixelSize = size;
    }
    return true;
}
}


AFont::AFont(AFontManager* fm, const AString& path) :
        ft(fm->mFreeType),
        mFontManager(fm) {
    if (FT_New_Face(fm->mFreeType->getFt(), path.toStdString().c_str(), 0, &mFace)) {
        throw AException("Could not load font: " + path);
    }
}

AFont::AFont(AFontManager* fm, const AUrl& url) :
        ft(fm->mFreeType),
        mFontManager(fm) {
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

bool AFont::isBold() const {
    return mFace->style_flags & FT_STYLE_FLAG_BOLD;
}

bool AFont::isItalic() const {
    return mFace->style_flags & FT_STYLE_FLAG_ITALIC;
}

bool AFont::hasGlyph(char32_t codepoint) const {
    return FT_Get_Char_Index(mFace, codepoint) != 0;
}

glm::vec2 AFont::getKerning(char32_t left, char32_t right, unsigned size) {
    if (size != mFacePixelSize) {
        if (FT_Set_Pixel_Sizes(mFace, 0, size) != 0) {
            return {0.f, 0.f};
        }
        mFacePixelSize = size;
    }

    FT_UInt leftIndex = FT_Get_Char_Index(mFace, left);
    FT_UInt rightIndex = FT_Get_Char_Index(mFace, right);
    if (leftIndex == 0 || rightIndex == 0) {
        return {0.f, 0.f};
    }

    FT_Vector vec2;
    if (FT_Get_Kerning(mFace, leftIndex, rightIndex, FT_KERNING_DEFAULT, &vec2) != 0) {
        return {0.f, 0.f};
    }

    return {vec2.x >> 6, vec2.y >> 6};
}

AFont::Character AFont::renderGlyph(const FontEntry& fs, AChar glyph) {
    int size = fs.first.size;
    FontRendering fr = fs.first.fr;

    AByteBuffer data;
    glm::vec2 charSize{0.f};
    glm::vec2 hBearing{0.f};
    float hAdvance = 0.f;
    glm::vec2 vBearing{0.f};
    float vAdvance = 0.f;
    int width = 0;
    int height = 0;

    FT_Face face = nullptr;
    if (hasGlyph(glyph.codepoint())) {
        face = mFace;
    } else if (mFontManager) {
        face = mFontManager->getFallbackFace(glyph.codepoint());
    }
    if (!face) {
        face = mFace;
    }

    if (!programPixelSize(face, mFace, mFacePixelSize, size)) {
        return Character{
            .glyphFailed = true,
        };
    }

    FT_Int32 flags = FT_LOAD_RENDER;

    if (fr == FontRendering::SUBPIXEL)
        flags |= FT_LOAD_TARGET_LCD;
    if (fr == FontRendering::NEAREST)
        flags |= FT_LOAD_TARGET_MONO;

    FT_Error e = FT_Load_Char(face, glyph.codepoint(), flags);
    if (e) {
        ALogger::debug("Font") << "FT_Load_Char failed for U+"
                               << std::hex << std::uint32_t(glyph.codepoint()) << " (error " << std::dec << e << ")";
        return Character{
            .glyphFailed = true,
        };
    }

    FT_GlyphSlot g = face->glyph;
    const float div = 1.f / 64.f;

    if (g->bitmap.width && g->bitmap.rows) {
        width = g->bitmap.width;
        if (fr == FontRendering::SUBPIXEL)
            width /= 3;
        height = g->bitmap.rows;

        if (fr == FontRendering::NEAREST) {
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
    }

    charSize = { div * g->metrics.width, div * g->metrics.height };
    hBearing = { static_cast<float>(g->bitmap_left), static_cast<float>(g->bitmap_top) };
    hAdvance = div * g->metrics.horiAdvance;
    vBearing = { div * g->metrics.vertBearingX, div * g->metrics.vertBearingY };
    vAdvance = div * g->metrics.vertAdvance;

    if (data.empty()) {
        return Character{
            .image = nullptr,
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
    if (auto it = chars.find(glyph.codepoint()); it != chars.end()) {
        return it->second;
    }
    return chars.emplace(glyph.codepoint(), renderGlyph(charset, glyph)).first->second;
}

AFont::GlyphMetrics AFont::getCharacterMetrics(const FontEntry& charset, AChar glyph) {
    const Character& ch = getCharacter(charset, glyph);
    return { ch.horizontal.advance, ch.glyphFailed, !ch.empty() };
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

AString AFont::getFontFamilyName() const {
    return mFace->family_name;
}

AFontFamily::Weight AFont::getFontWeight() const {
    if (mFace->style_flags & FT_STYLE_FLAG_BOLD)
        return AFontFamily::BOLD;
    return AFontFamily::NORMAL;
}

int AFont::getAscenderHeight(unsigned size) const {
    if (mFace->height == 0) return 0;
    return int(mFace->ascender) * int(size) / mFace->height;
}

int AFont::getDescenderHeight(unsigned size) const {
    if (mFace->height == 0) return 0;
    return -int(mFace->descender) * int(size) / mFace->height;
}
