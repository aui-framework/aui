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

#include <string>
#include <iterator>
#include <glm/glm.hpp>
#include <AUI/Url/AUrl.h>

#include "AUI/Render/FontRendering.h"

#include "AUI/Render/SimpleTexturePacker.h"

#include "AUI/Common/AStringVector.h"
#include "AFontFamily.h"
#include <AUI/Common/AByteBuffer.h>

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
            if (glyphFailed) return spaceWidth;
            return horizontal.advance > 0.f ? horizontal.advance : 0.f;
        }

        void* rendererData = nullptr;
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
        AVector<AOptional<Character>> characters;
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

    FontData& getFontEntry(unsigned size, FontRendering fr) {
        return mCharData[FontKey{size, fr}];
    }

    /**
     * @brief Checks if the primary font contains the given glyph.
     * @return true if the glyph is available in the font.
     */
    bool hasGlyph(char32_t codepoint) const;


    Character renderGlyph(const FontEntry& fs, AChar glyph);

public:
    AFont(AFontManager* fm, const AString& path);

    AFont(AFontManager* fm, const AUrl& url);

    FontEntry getFontEntry(const FontKey& key) {
        return {key, mCharData[key]};
    }

    glm::vec2 getKerning(char32_t left, char32_t right, unsigned size);

    ~AFont();

    AFont(const AFont&) = delete;
    AFont& operator=(const AFont&) = delete;

    Character& getCharacter(const FontEntry& charset, AChar glyph);

    int length(const FontEntry& charset, AStringView text);

    int length(const FontEntry& charset, std::u32string_view text);

    template<class Iterator>
    int length(const FontEntry& charset, Iterator begin, Iterator end) {
        int size = charset.first.size;
        float prevLineAdvance = 0;
        float advance = 0;
        const bool hasKerning = isHasKerning();

        for (Iterator i = begin; i != end; ++i) {
            if (*i == U' ') {
                advance += getSpaceWidth(size);
            } else if (*i == U'\n') {
                advance = 0;
                prevLineAdvance = glm::max(prevLineAdvance, advance);
            } else {
                Character& ch = getCharacter(charset, *i);
                if (!ch.empty()) {
                    advance += ch.horizontal.advance;
                } else {
                    // Match the renderers (OpenGLRenderer/SoftwareRenderer):
                    // the kerning of an empty glyph with the following
                    // character is applied before the (possibly fallback)
                    // advance, so measured width equals the drawn width.
                    if (hasKerning) {
                        auto next = std::next(i);
                        if (next != end) {
                            advance += getKerning(*i, *next, size).x;
                        }
                    }
                    advance += ch.emptyAdvance(getSpaceWidth(size));
                }
            }
        }
        return int(glm::ceil(glm::max(prevLineAdvance, advance)));
    }

    AString
    trimStringToWidth(const FontEntry& charset, AString::iterator begin, AString::iterator end, float maxWidth) {
        float width = 0;
        for (auto i = begin; i != end; i++) {
            if (*i == '\n') {
                return AString(begin, i);
            }
            float charWidth = length(charset, i, std::next(i));
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