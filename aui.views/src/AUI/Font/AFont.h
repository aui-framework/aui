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

#pragma once

#include <cstdint>
#include <string>
#include <iterator>
#include <memory>
#include <mutex>
#include <glm/glm.hpp>
#include <AUI/Url/AUrl.h>

#include "AUI/Render/FontRendering.h"

#include "AUI/Render/SimpleTexturePacker.h"

#include "AUI/Common/AStringVector.h"
#include "AFontFamily.h"
#include <AUI/Common/AByteBuffer.h>
#include <AUI/Common/SharedPtrTypes.h>   // _unique (used by FontData::characters)

class AString;

class AFontManager;

class FreeType;


struct FT_FaceRec_;

class API_AUI_VIEWS AFont {
public:
    /**
     * @brief Character data for a glyph.
     * @details
     * https://freetype.org/freetype2/docs/tutorial/step2.html
     */
    struct Character {
        /**
         * @brief Bitmap of the glyph.
         */
        _<AImage> image;

        /**
         * @brief Glyph's image bounding box size. It's independent of the layout direction.
         */
        glm::vec2 size{};

        struct Metrics {
            /**
             * @brief Horizontal/vertical distance from the current cursor position to the left-top-most border of the
             * glyph image's bounding box. Cursor position is considered to be on the baseline.
             */
            glm::vec2 bearing{};

            /**
             * @brief Distance to increment the pen position when the glyph is drawn as part of a string of text.
             * @details
             * For horizontal text layouts, this increments x position.
             *
             * For vertical text layouts, this increments y position.
             */
            float advance{};
        };

        /**
         * @brief Metrics for horizontal text layout.
         */
        Metrics horizontal{};

        /**
         * @brief Metrics for vertical text layout.
         * @details
         * As not all fonts do contain vertical metrics, these values should not be considered reliable if the font does
         * not contain them.
         */
        Metrics vertical{};

        /**
         * @brief True when FT_Load_Char failed for this glyph (the font does not
         *        contain a usable glyph, not even .notdef). Such glyphs should get
         *        a space-width advance fallback rather than their (zero) advance.
         */
        bool glyphFailed = false;

        /**
         * @brief True when the glyph was rendered from the primary face because a
         *        fallback-face lookup came up empty (fallback discovery is lazy and
         *        deferred). Such glyphs are re-rendered whenever the fallback
         *        generation advances (a face load or a deferred-candidate
         *        consumption), so a subsequent render may succeed once more faces
         *        are loaded.
         */
        bool provisional = false;

        /**
         * @brief AFontManager::fallbackGeneration() as of the start of this
         *        glyph's most recent render attempt. getCharacter re-renders a
         *        failed/provisional glyph only when the manager's generation has
         *        advanced past this value, i.e. the fallback pool changed (a
         *        face load or a deferred-candidate consumption) since the last
         *        attempt. This bounds re-renders to at most one per pool change
         *        instead of one per lookup while fallback discovery runs. Final
         *        glyphs are unaffected (they are returned from the cache
         *        unconditionally).
         */
        uint64_t fallbackGeneration = 0;

        [[nodiscard]]
        bool empty() const {
            return image == nullptr;
        }

        /**
         * @return The effective horizontal advance for an empty (no-bitmap) glyph:
         *         the glyph's computed advance if positive, or zero for legitimately
         *         zero-advance glyphs (combining marks, ZWJ/ZWNJ, variation selectors).
         *         The space-width fallback is only applied when the glyph genuinely
         *         failed to load (glyphFailed == true).
         */
        [[nodiscard]]
        float emptyAdvance(float spaceWidth) const {
            return emptyAdvanceFor(horizontal.advance, glyphFailed, spaceWidth);
        }

        /**
         * @brief Renderer-owned per-glyph cache handle (e.g. the texture
         *        packer entry in OpenGLRenderer). Read and written only
         *        through AFont::withCharacterRendererData, which holds the
         *        glyph-cache lock, so it is never touched concurrently with a
         *        re-render.
         */
        void* rendererData = nullptr;
    };

    /**
     * @brief Lightweight per-glyph measurement data for text layout, without
     *        the bitmap (no _<AImage> shared-pointer traffic). Returned by
     *        getCharacterMetrics for measurement-only paths.
     */
    struct GlyphMetrics {
        /**
         * @brief Horizontal advance in pixels (Character::horizontal.advance).
         */
        float advance = 0.f;

        /**
         * @brief True when FT_Load_Char failed for this glyph (see
         *        Character::glyphFailed): the advance is not usable and the
         *        space width should be used instead.
         */
        bool glyphFailed = false;

        /**
         * @brief True when the glyph has a bitmap (Character::empty() ==
         *        !hasImage); empty-bitmap glyphs (spaces, combining marks)
         *        still carry a usable advance.
         */
        bool hasImage = false;

        /**
         * @return The effective advance for layout: the glyph's own advance
         *         when it has a bitmap, otherwise the space-width fallback
         *         for failed glyphs, the glyph's advance for empty-bitmap
         *         glyphs with a positive advance, or zero for legitimately
         *         zero-advance glyphs (combining marks, ZWJ/ZWNJ, variation
         *         selectors).
         */
        [[nodiscard]]
        float effectiveAdvance(float spaceWidth) const {
            if (hasImage) return advance;
            return emptyAdvanceFor(advance, glyphFailed, spaceWidth);
        }
    };

    struct FontKey {
        unsigned size;
        FontRendering fr;

        int hash() const {
            return (size << 2) | int(fr);
        }

        bool operator<(const FontKey& f) const {
            return hash() < f.hash();
        }
    };

    struct FontData {
        /**
         * Cached glyphs, keyed by codepoint. Sparse storage: memory scales with
         * the number of cached glyphs rather than the highest codepoint rendered
         * (a single CJK codepoint would otherwise size a vector past 40k slots).
         * Stored by pointer in a node-based map so that insertions never
         * invalidate the map; an absent key means "not cached yet". Re-renders
         * (provisional/failed glyphs) replace the slot with a NEW Character, so
         * cached glyphs are immutable once published: getCharacter returns
         * snapshots by value and no caller holds a reference into the cache. All
         * access is serialized by AFont::mCharDataMutex.
         */
        AMap<char32_t, _unique<Character>> characters;
        /**
         * @brief Renderer-owned per-font-size cache handle (e.g. the
         *        OpenGLRenderer font-entry data). Read and written only
         *        through AFont::withFontEntryRendererData, which holds the
         *        glyph-cache lock.
         */
        void* rendererData = nullptr;
    };


    using FontEntry = std::pair<FontKey, FontData&>;


private:
    _<FreeType> ft;
    AByteBuffer mFontDataBuffer;
    FT_FaceRec_* mFace = nullptr;
    /**
     * @brief Non-owning back-pointer to the font manager, used by renderGlyph
     *        for lazy CJK fallback face lookup. AFontManager is a
     *        process-lifetime singleton and every AFont is created by it, so
     *        the manager outlives all AFont instances; AFont handles must not
     *        be kept beyond the manager's lifetime (rendering a glyph after
     *        the manager is destroyed would dereference this pointer).
     */
    AFontManager* mFontManager = nullptr;

    AMap<FontKey, FontData> mCharData;

    /**
     * @brief Serializes access to mCharData (see getCharacter).
     */
    std::mutex mCharDataMutex;

    /**
     * @brief Last pixel size successfully programmed on mFace. Guarded by
     *        FreeType::sFaceMutex; lets getKerning skip the (metric-recomputing)
     *        FT_Set_Pixel_Sizes call on every adjacent character pair. 0 means
     *        "no size programmed yet" (FT_Set_Pixel_Sizes never succeeds with 0).
     */
    unsigned mFacePixelSize = 0;

    /**
     * @brief Checks if the primary font contains the given glyph.
     * @return true if the glyph is available in the font.
     */
    bool hasGlyph(char32_t codepoint) const;

    /**
     * @brief Shared empty-glyph advance fallback rule, used by both
     *        Character::emptyAdvance and GlyphMetrics::effectiveAdvance so
     *        the measurement path and the render path cannot diverge: the
     *        space-width fallback when the glyph failed to load, otherwise
     *        the glyph's own advance if positive, or zero for legitimately
     *        zero-advance glyphs (combining marks, ZWJ/ZWNJ, variation
     *        selectors).
     */
    static float emptyAdvanceFor(float advance, bool glyphFailed, float spaceWidth) {
        if (glyphFailed) return spaceWidth;
        return advance > 0.f ? advance : 0.f;
    }


    Character renderGlyph(const FontEntry& fs, AChar glyph);

    /**
     * @brief Ensures a usable cached glyph exists for the given codepoint
     *        (re-rendering when it is failed/provisional and the fallback
     *        generation has advanced) and returns a reference to it.
     * @param lock out: takes mCharDataMutex; the returned reference is only
     *        valid while this lock is held.
     */
    Character& getCharacterLocked(const FontEntry& charset, AChar glyph, std::unique_lock<std::mutex>& lock);

public:
    AFont(AFontManager* fm, const AString& path);

    AFont(AFontManager* fm, const AUrl& url);

    /**
     * @brief Returns the font entry for the given key, creating the per-size
     *        FontData on first use. The returned FontData reference stays
     *        valid (map nodes never move); its characters map and rendererData
     *        handle are accessed only through getCharacter /
     *        withFontEntryRendererData, which take the cache lock.
     */
    FontEntry getFontEntry(const FontKey& key) {
        std::lock_guard lock(mCharDataMutex);
        return {key, mCharData[key]};
    }

    glm::vec2 getKerning(char32_t left, char32_t right, unsigned size);

    ~AFont();

    AFont(const AFont&) = delete;
    AFont& operator=(const AFont&) = delete;

    /**
     * @brief Returns a snapshot of the cached glyph for the given codepoint,
     *        re-rendering it first when it is failed/provisional and fallback
     *        discovery may still improve it.
     * @details The returned value is a copy taken under the glyph-cache lock:
     *          the cache may re-render (replace) the glyph on another thread at
     *          any time, so callers must never hold references into it. The
     *          _<AImage> inside the snapshot keeps the bitmap alive.
     */
    Character getCharacter(const FontEntry& charset, AChar glyph);

    /**
     * @brief Returns the layout metrics of the cached glyph for the given
     *        codepoint without copying the bitmap: no _<AImage> shared-pointer
     *        traffic, which matters for measurement-heavy paths (length,
     *        trimStringToWidth). Re-renders a failed/provisional glyph under
     *        the same conditions as getCharacter, so measured widths stay in
     *        sync with drawn glyphs.
     */
    GlyphMetrics getCharacterMetrics(const FontEntry& charset, AChar glyph);

    /**
     * @brief Runs f with a reference to the cached glyph's renderer cache
     *        handle while holding the glyph-cache lock, so a check-then-set
     *        (e.g. texture-packer insert + handle store) is atomic across
     *        threads. Does nothing if the glyph is not cached yet.
     * @note f runs under the non-recursive glyph-cache mutex (mCharDataMutex).
     *       To prevent deadlocks, callers must not acquire mFontCacheMutex
     *       before calling this function. f must not call back into any AFont
     *       method (getCharacter, getCharacterMetrics, getFontEntry,
     *       withCharacterRendererData, withFontEntryRendererData), and it
     *       should return quickly: all glyph lookups on this font block until
     *       it does.
     */
    template <typename F>
    void withCharacterRendererData(const FontEntry& charset, AChar glyph, F&& f) {
        std::lock_guard lock(mCharDataMutex);
        auto& chars = charset.second.characters;
        if (auto it = chars.find(glyph.codepoint()); it != chars.end() && it->second) {
            f(it->second->rendererData);
        }
    }

    /**
     * @brief Runs f with a reference to the font-size entry's renderer cache
     *        handle (FontData::rendererData) while holding the glyph-cache
     *        lock, so a check-then-set is atomic across threads.
     * @note f runs under the non-recursive glyph-cache mutex (mCharDataMutex).
     *       To prevent deadlocks, callers must not acquire mFontCacheMutex
     *       before calling this function. f must not call back into any AFont
     *       method (getCharacter, getCharacterMetrics, getFontEntry,
     *       withCharacterRendererData, withFontEntryRendererData), and it
     *       should return quickly: all glyph lookups on this font block until
     *       it does.
     */
    template <typename F>
    void withFontEntryRendererData(const FontEntry& entry, F&& f) {
        std::lock_guard lock(mCharDataMutex);
        f(entry.second.rendererData);
    }

    int length(const FontEntry& charset, AStringView text);

    int length(const FontEntry& charset, std::u32string_view text);

    template<class Iterator>
    int length(const FontEntry& charset, Iterator begin, Iterator end) {
        int size = charset.first.size;
        float prevLineAdvance = 0;
        float advance = 0;
        const bool hasKerning = isHasKerning();
        const float spaceWidth = getSpaceWidth(size);

        for (Iterator i = begin; i != end; ++i) {
            if (*i == U' ') {
                advance += getSpaceWidth(size);
            } else if (*i == U'\n') {
                advance = 0;
                prevLineAdvance = glm::max(prevLineAdvance, advance);
            } else {
                // Metrics-only lookup: length measures but never draws, so
                // skip the Character snapshot (no _<AImage> refcount churn).
                const GlyphMetrics m = getCharacterMetrics(charset, *i);
                // Match the renderers (OpenGLRenderer/SoftwareRenderer): pair
                // kerning with the following character is applied before the
                // glyph's advance for both empty and regular glyphs, so the
                // measured width equals the drawn width.
                if (hasKerning) {
                    auto next = std::next(i);
                    if (next != end) {
                        advance += getKerning(*i, *next, size).x;
                    }
                }
                advance += m.effectiveAdvance(spaceWidth);
            }
        }
        return int(glm::ceil(glm::max(prevLineAdvance, advance)));
    }

    AString
    trimStringToWidth(const FontEntry& charset, AString::iterator begin, AString::iterator end, float maxWidth) {
        float width = 0;
        const bool hasKerning = isHasKerning();
        for (auto i = begin; i != end; i++) {
            if (*i == '\n') {
                return AString(begin, i);
            }
            auto next = std::next(i);
            float charWidth = length(charset, i, next);
            if (next != end && hasKerning) {
                charWidth += getKerning(*i, *next, charset.first.size).x;
            }
            if (width + charWidth > maxWidth) {
                return AString(begin, i);
            }
            width += charWidth;
        }
        return AString(begin, end);
    }


    bool isHasKerning();

    [[nodiscard]]
    AString getFontFamilyName() const;

    [[nodiscard]]
    AFontFamily::Weight getFontWeight() const;

    int getAscenderHeight(unsigned size) const;

    int getDescenderHeight(unsigned size) const;

    int getSpaceWidth(unsigned size) {
        return size * 10 / 23;
    }

    [[nodiscard]]
    bool isItalic() const;
};