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

namespace {
/**
 * @brief Programs the pixel size on the currently selected face.
 *
 * The primary face's size is cached (primaryPixelSize) so the metric-
 * recomputing FT_Set_Pixel_Sizes call is skipped when the size is already
 * programmed (getKerning uses the same guard). Fallback bitmap faces get
 * the strike closest to the requested size; when a fallback face cannot be
 * sized at all, the function releases the fallback library/per-face locks
 * and retries with the primary face (marking the render provisional).
 *
 * @param face in/out: the face to size; replaced with the primary face on
 *        the fallback retry path.
 * @param primaryFace the font's primary face, never freed here.
 * @param primaryPixelSize in/out: cached programmed size of the primary
 *        face (AFont::mFacePixelSize).
 * @param ftLock in/out: the primary-face library lock; acquired on the
 *        retry path, when it is not already held.
 * @param fallbackLock in/out: the fallback face's library/per-face locks
 *        (AFontManager::FallbackFaceLock, a private type: deduced here and
 *        only reset, never inspected); released on the retry path before
 *        locking the primary face.
 * @param provisional in/out: set to true when the render falls back to the
 *        primary face.
 * @return false only when the primary face could not be sized: the caller
 *         must then produce a glyphFailed result.
 */
template <typename FallbackLock>
bool programPixelSize(FT_Face& face,
                      FT_Face primaryFace,
                      unsigned& primaryPixelSize,
                      int size,
                      std::unique_lock<std::mutex>& ftLock,
                      FallbackLock& fallbackLock,
                      bool& provisional) {
    bool pixelSizeSet = false;
    // Skip re-programming when the primary face is already at this size:
    // FT_Set_Pixel_Sizes recomputes the face's size metrics (getKerning
    // skips it for the same reason). Fallback faces are always sized —
    // primaryPixelSize tracks the primary face only.
    const bool sizeAlreadyProgrammed = (face == primaryFace && primaryPixelSize == unsigned(size));
    if (sizeAlreadyProgrammed) {
        pixelSizeSet = true;
    } else if (FT_Set_Pixel_Sizes(face, 0, size) != 0) {
        // FT_Set_Pixel_Sizes fails for fixed-size bitmap faces unless the
        // requested pixel size is exactly one of the available strikes.
        // For a fallback face, select the strike closest to the requested
        // size before giving up and retrying the primary face.
        bool strikeSelected = false;
        if (face != primaryFace && !(face->face_flags & FT_FACE_FLAG_SCALABLE) && face->num_fixed_sizes > 0) {
            long bestStrike = 0;
            long bestDelta = std::numeric_limits<long>::max();
            for (FT_Int i = 0; i < face->num_fixed_sizes; ++i) {
                // available_sizes[].y_ppem is in FreeType 26.6 fixed-point
                // (64 units per pixel); convert before comparing to size.
                const long strikePx = face->available_sizes[i].y_ppem >> 6;
                const long delta = std::abs(strikePx - long(size));
                if (delta < bestDelta) {
                    bestDelta = delta;
                    bestStrike = i;
                }
            }
            // Without bitmap rescaling, any delta > 0 risks breaking UI layout bounds.
            // Accept only exact matches until rescaling logic is implemented.
            if (bestDelta == 0) {
                strikeSelected = FT_Select_Size(face, bestStrike) == 0;
            }
        }
        // A selected strike IS a programmed size: keep the flag truthful.
        pixelSizeSet = strikeSelected;
        if (!strikeSelected && face != primaryFace) {
            // Fallback face has no usable strike; retry with the primary face.
            fallbackLock = {};  // release library + per-face mutexes before locking sFaceMutex
            face = primaryFace;
            provisional = true;
            if (!ftLock.owns_lock()) ftLock.lock();  // acquire lock for the primary face
            pixelSizeSet = FT_Set_Pixel_Sizes(face, 0, size) == 0;
        }
    } else {
        pixelSizeSet = true;
    }
    if (face == primaryFace && pixelSizeSet) {
        primaryPixelSize = size;   // keep getKerning's programmed-size cache in sync
    }
    return pixelSizeSet;
}
}


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
    if (size != mFacePixelSize) {
        // FT_Set_Pixel_Sizes recomputes the face's size metrics, so avoid
        // re-programming the size on every adjacent character pair. If the
        // face cannot be sized (e.g. a fixed-size bitmap face), kerning is
        // undefined: return zero rather than using a stale size.
        if (FT_Set_Pixel_Sizes(mFace, 0, size) != 0) {
            return {0.f, 0.f};
        }
        mFacePixelSize = size;
    }

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
    bool provisional = false;   // rendered from the primary face after a failed fallback lookup
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
        } else {
            // No fallback face covers this codepoint (yet): mark the render
            // provisional so it is retried while deferred candidates remain.
            provisional = true;
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

        // Step 2: set pixel sizes (may release the fallback locks and switch
        // to the primary face if the fallback face cannot be sized).
        if (!programPixelSize(face, mFace, mFacePixelSize, size, ftLock, fallbackLock, provisional)) {
            return Character{
                .glyphFailed = true,
            };
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
            .provisional = provisional,
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
        .provisional = provisional,
    };
}

AFont::Character& AFont::getCharacterLocked(const FontEntry& charset, AChar glyph, std::unique_lock<std::mutex>& lock) {
    // Phase 1: cache probe under the lock. A cached glyph is returned as-is
    // unless it is failed/provisional and the fallback generation has
    // advanced (a later render can succeed once more faces load). Gating on
    // the generation alone — not on whether deferred candidates remain — is
    // what lets the FINAL fallback face reach already-cached glyphs: the
    // worker clears the deferred queue in the same step it publishes the last
    // face, so a count-based gate would suppress that final retry.
    lock = std::unique_lock(mCharDataMutex);
    {
        auto& chars = charset.second.characters;
        if (auto it = chars.find(glyph.codepoint()); it != chars.end() && it->second) {
            auto& slot = it->second;
            const bool mayImprove = mFontManager
                                 && slot->fallbackGeneration != mFontManager->fallbackGeneration();
            if (!mayImprove || (!slot->glyphFailed && !slot->provisional)) {
                return *slot;
            }
        }
    }
    lock.unlock();   // render outside the cache lock

    // Phase 2: render outside the cache lock. renderGlyph runs fallback-face
    // lookup and FreeType rendering (sFaceMutex), which must not stall
    // lookups of other glyphs of this font.
    //
    // Record the generation as of *before* the render attempt: if a face
    // loads while this attempt is in flight (lockFallbackFace's lazy
    // loading), the recorded value is stale, which only causes one extra
    // re-render later — the safe direction. Recording it afterwards could
    // instead miss a face loaded concurrently and cache a suboptimal glyph
    // without any further retry.
    const uint64_t renderGeneration = mFontManager ? mFontManager->fallbackGeneration() : 0;
    Character ch = renderGlyph(charset, glyph);
    ch.fallbackGeneration = renderGeneration;

    // Phase 3: install under the lock. Install a NEW slot instead of mutating
    // the cached Character: cached glyphs are immutable once published and
    // getCharacter returns snapshots by value, so a concurrent re-render must
    // never race with a reader's snapshot copy (including the _<AImage>
    // shared-pointer copy). A concurrently cached final result is preferred
    // over a worse (failed/provisional) one. Callers that draw later
    // (prerendered strings) must hold shared references to the image, never
    // raw pointers, because the image is replaced on re-render
    // (SoftwareRenderer's CharEntry does this).
    lock.lock();
    auto& chars = charset.second.characters;
    auto& slot = chars[glyph.codepoint()];
    // Never downgrade a drawable glyph to a failed one: a failed re-render
    // (e.g. FT_Load_Char failing on a newly loaded fallback face) must not
    // evict the previous drawable bitmap, which would turn the glyph into an
    // empty space-width advance. The kept glyph retains its old generation,
    // so later fallback loads still trigger its re-render.
    const bool newIsWorse = slot && ch.glyphFailed && !slot->glyphFailed;
    if (!slot || (!newIsWorse && (slot->glyphFailed || slot->provisional))) {
        ch.rendererData = slot ? slot->rendererData : nullptr;
        slot = std::make_unique<Character>(std::move(ch));
    } else if (newIsWorse && slot->fallbackGeneration != renderGeneration) {
        // Keep the drawable glyph, but record the attempted generation so the
        // failed re-render is not repeated on every lookup.
        auto kept = std::make_unique<Character>(*slot);
        kept->fallbackGeneration = renderGeneration;
        slot = std::move(kept);
    }
    return *slot;
}

AFont::Character AFont::getCharacter(const FontEntry& charset, AChar glyph) {
    std::unique_lock<std::mutex> lock;
    return getCharacterLocked(charset, glyph, lock);
}

AFont::GlyphMetrics AFont::getCharacterMetrics(const FontEntry& charset, AChar glyph) {
    std::unique_lock<std::mutex> lock;
    const Character& ch = getCharacterLocked(charset, glyph, lock);
    return { ch.horizontal.advance, ch.glyphFailed, ch.image != nullptr };
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
