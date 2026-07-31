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
#include <cstdlib>
#include <fstream>
#include <limits>
#include <memory>
#include <mutex>
#include <string>
#include "AUI/Common/AStringVector.h"
#include "AFont.h"


AFont::AFont(AFontManager* fm, const AString& path) :
        ft(fm->mFreeType),
        mFontManager(fm) {
    {
        std::lock_guard lock(FreeType::sFaceMutex);
        if (FT_New_Face(fm->mFreeType->getFt(), path.toStdString().c_str(), 0, &mFace)) {
            throw AException("Could not load font: " + path);
        }
    }
}

AFont::AFont(AFontManager* fm, const AUrl& url) :
        ft(fm->mFreeType),
        mFontManager(fm) {
    if (url.schema() == "file") {
        {
            std::lock_guard lock(FreeType::sFaceMutex);
            if (FT_New_Face(fm->mFreeType->getFt(), url.path().toStdString().c_str(), 0, &mFace)) {
                throw AException("Could not load font: " + url.full());
            }
        }
        return;
    }
    mFontDataBuffer = AByteBuffer::fromStream(url.open());

    {
        std::lock_guard lock(FreeType::sFaceMutex);
        if (FT_New_Memory_Face(fm->mFreeType->getFt(), (const FT_Byte*) mFontDataBuffer.data(), mFontDataBuffer.getSize(),
                               0, &mFace)) {
            throw AException("Could not load font: " + url.full());
        }
    }
}

AFont::~AFont() {
    if (mFace) {
        std::lock_guard lock(FreeType::sFaceMutex);
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
    std::lock_guard lock(FreeType::sFaceMutex);
    return FT_Get_Char_Index(mFace, codepoint) != 0;
}

glm::vec2 AFont::getKerning(char32_t left, char32_t right, unsigned size) {
    std::lock_guard lock(FreeType::sFaceMutex);
    FT_Set_Pixel_Sizes(mFace, 0, size);

    FT_UInt leftIndex = FT_Get_Char_Index(mFace, left);
    FT_UInt rightIndex = FT_Get_Char_Index(mFace, right);
    if (leftIndex == 0 || rightIndex == 0) {
        // One or both glyphs are not in the primary face;
        // kerning is undefined across fonts so return zero.
        return {0.f, 0.f};
    }

    FT_Vector vec2;
    FT_Get_Kerning(mFace, leftIndex, rightIndex, FT_KERNING_DEFAULT, &vec2);

    return {vec2.x >> 6, vec2.y >> 6};
}

AFont::Character AFont::renderGlyph(const FontEntry& fs, AChar glyph) {
    int size = fs.first.size;
    FontRendering fr = fs.first.fr;

    // Capture glyph data into locals; fallback lock is released before image allocation.
    AByteBuffer data;
    glm::vec2 charSize{0.f};
    glm::vec2 hBearing{0.f};
    float hAdvance = 0.f;
    glm::vec2 vBearing{0.f};
    float vAdvance = 0.f;
    int width = 0;
    int height = 0;

    FT_Face face = nullptr;
    // Holds the library (sFaceMutex) and per-face mutexes for fallback face FT ops.
    AFontManager::FallbackFaceLock fallbackLock;

    // Step 1: determine which face to use.
    // hasGlyph has internal sFaceMutex locking.
    if (hasGlyph(glyph.codepoint())) {
        face = mFace;
    } else if (mFontManager) {
        auto fb = mFontManager->lockFallbackFace(glyph.codepoint());
        if (fb) {
            face = fb.face;
            // Transfer the library and per-face locks; the manager mutex was
            // already released inside lockFallbackFace after face selection.
            fallbackLock.ftLock = std::move(fb.ftLock);
            fallbackLock.faceLock = std::move(fb.faceLock);
        }
    }
    if (!face) {
        // No font has this glyph; let the primary face render .notdef (tofu).
        face = mFace;
    }

    // Step 2 & 3: set pixel sizes, render glyph, and capture metrics.
    // Every FT_Face operation on any face of the shared FT_Library must be
    // serialized with sFaceMutex: it is taken below for the primary face and
    // was already taken by lockFallbackFace (fallbackLock.ftLock) for a
    // fallback face. The per-face mutex (fallbackLock.faceLock) additionally
    // guards the fallback face against a concurrent FT_Done_Face.
    {
        std::unique_lock ftLock(FreeType::sFaceMutex, std::defer_lock);
        if (face == mFace) ftLock.lock();

        // Step 2: set pixel sizes.
        if (FT_Set_Pixel_Sizes(face, 0, size) != 0) {
            // FT_Set_Pixel_Sizes fails for fixed-size bitmap faces unless the
            // requested pixel size is exactly one of the available strikes.
            // For a fallback face, select the strike closest to the requested
            // size before giving up and retrying the primary face.
            bool strikeSelected = false;
            if (face != mFace && !(face->face_flags & FT_FACE_FLAG_SCALABLE) && face->num_fixed_sizes > 0) {
                long bestStrike = 0;
                long bestDelta = std::numeric_limits<long>::max();
                for (FT_Int i = 0; i < face->num_fixed_sizes; ++i) {
                    const long delta = std::abs(long(face->available_sizes[i].y_ppem) - long(size));
                    if (delta < bestDelta) {
                        bestDelta = delta;
                        bestStrike = i;
                    }
                }
                strikeSelected = FT_Select_Size(face, bestStrike) == 0;
            }
            if (!strikeSelected && face != mFace) {
                // Fallback face has no usable strike; retry with the primary face.
                fallbackLock = {};  // release library + per-face mutexes before locking sFaceMutex
                face = mFace;
                if (!ftLock.owns_lock()) ftLock.lock();  // acquire lock for mFace
                FT_Set_Pixel_Sizes(face, 0, size);
            }
            // If both fail, FT_Load_Char will also fail and glyphFailed is set.
        }

        FT_Int32 flags = FT_LOAD_RENDER;

        if (fr == FontRendering::SUBPIXEL)
            flags |= FT_LOAD_TARGET_LCD;
        if (fr == FontRendering::NEAREST)
            flags |= FT_LOAD_TARGET_MONO;

        // Step 3: render glyph and capture metrics.
        FT_Error e = FT_Load_Char(face, glyph.codepoint(), flags);
        if (e) {
            // Neither face can render this glyph.
            ALogger::debug("Font") << "FT_Load_Char failed for U+"
                                   << std::hex << std::uint32_t(glyph.codepoint()) << " (error " << std::dec << e << ")";
            return Character{
                .glyphFailed = true,
            };
        }

        // Capture glyph bitmap and metrics while the render lock is held.
        FT_GlyphSlot g = face->glyph;
        const float div = 1.f / 64.f;

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
        }

        // Capture metrics unconditionally (even for empty-bitmap glyphs like spaces).
        charSize = { div * g->metrics.width, div * g->metrics.height };
        hBearing = { static_cast<float>(g->bitmap_left), static_cast<float>(g->bitmap_top) };
        hAdvance = div * g->metrics.horiAdvance;
        vBearing = { div * g->metrics.vertBearingX, div * g->metrics.vertBearingY };
        vAdvance = div * g->metrics.vertAdvance;
    }   // ftLock released here.

    fallbackLock = {};  // release library + per-face locks before the image allocation below

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
    // The cache is guarded so that concurrent lookups, resizes and insertions
    // cannot race. Character objects are stored by pointer, so resizing the
    // vector never reallocates (invalidates) an already returned reference.
    std::lock_guard lock(mCharDataMutex);
    auto& chars = charset.second.characters;
    if (chars.size() > glyph) {
        auto& slot = chars[glyph.codepoint()];
        if (slot) {
            // Failed glyphs are cached, but re-rendered while lazy fallback
            // discovery is still in progress: a later render may succeed once
            // more faces become available. Once no deferred candidates remain
            // they stay cached, so unsupported codepoints do not re-run the
            // locked failure path on every layout.
            if (!slot->glyphFailed || !mFontManager || !mFontManager->hasDeferredCandidates()) {
                return *slot;
            }
            slot = nullptr;
        }
    } else {
        chars.resize(glyph + 1);
    }

    Character ch = renderGlyph(charset, glyph);
    chars[glyph.codepoint()] = std::make_unique<Character>(std::move(ch));

    return *chars[glyph.codepoint()];
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
